#include "procedure.h"

// groups the url, password header, and curl handler
struct attributes {
	char* url;
	CURL* curl;
	char* header;
};

/**
 * Free memory allocated for the url, header, and curl.
 */
static void freeAttributes(struct attributes a) {
	free(a.url);
	free(a.header);
	curl_easy_cleanup(a.curl);
}

/**
 * Free memory allocated for the temporary strings used in initAttributes.
 */
#define ATTR_CLEANUP(a, b, c) do {\
	free(a);\
	free(b);\
	free(c);\
} while(0)

/**
 * Creates a message queue, URL and header strings, and a curl easy handle.
 * @param  a
 * @param  data
 * @return      A non-zero code if an error occurs, zero otherwise.
 */
static DWORD initAttributes(struct attributes* a, InstanceData* data) {
	// initialise all members to NULL
	a->url = a->curl = a->header = NULL;

	// force initialisation of the message queue
	MSG msg;

	PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	// create a curl easy handle
	CURL* curl = curl_easy_init();

	if (!curl) {
		return ARE_CURL;
	}

	// convert the wide char buffers into char buffers
	char* address = wstrToStr(data->address);
	char* channel = wstrToStr(data->channel);
	char* password = wstrToStr(data->password);

	if (!address || !channel || !password) {
		// out of memory
		freeAttributes(*a);
		ATTR_CLEANUP(address, channel, password);

		return ARE_OUT_OF_MEM;
	}

	// create the url and password header strings
	a->url = createPublishURL(address, channel);
	a->header = createPasswordHeader(password);

	// temporary strings no longer required
	ATTR_CLEANUP(address, channel, password);

	if (!a->url || !a->header) {
		// out of memory
		freeAttributes(*a);

		return ARE_OUT_OF_MEM;
	}

	// initialised successfully
	return 0;
}

/**
 * Convert the data in the memory maps to a non-formatted JSON
 * string and handle associated errors.
 */
static char* toJSON(struct memMaps prev, struct memMaps curr, bool sendProps) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	// convert HUD parameters to a JSON object
	cJSON* hud = hudToJSON(curr.hud, prev.hud);

	if (!hud) {
		// if memory isn't being allocated something is seriously wrong
		cJSON_Delete(obj);

		return NULL;
	}

	// add it to the parent object
	if (!cJSON_AddItemToObject(obj, "hud", hud)) {
		cJSON_Delete(hud);
		cJSON_Delete(obj);

		return NULL;
	}

	// conver physics parameters to a JSON object
	cJSON* physics = physicsToJSON(curr.physics, prev.physics);

	if (!physics) {
		cJSON_Delete(obj);

		return NULL;
	}

	// add it to the parent
	if (!cJSON_AddItemToObject(obj, "physics", physics)) {
		cJSON_Delete(physics);
		cJSON_Delete(obj);

		return NULL;
	}

	// only send props if a change in the properties was detected
	// i.e. the user changed sessions
	// the properties don't need to be checked every time because they
	// are only updated in shared memory if the user changed sessions
	if (sendProps) {
		cJSON* props = propertiesToJSON(curr.props);

		if (!props) {
			return NULL;
		}

		if (!cJSON_AddItemToObject(obj, "properties", props)) {
			cJSON_Delete(obj);
			cJSON_Delete(props);

			return NULL;
		}
	}

	// TODO: determine the most appropriate value for JSON_BUF_SIZE in order
	// to minimise re-allocations
	char* str = cJSON_PrintBuffered(obj, JSON_BUF_SIZE, 0);

	if (!str) {
		return NULL;
	}

	// json printed; objects are no longer required
	cJSON_Delete(obj);

	// remember to free the string
	return str;
}

/**
 * Checks the message queue for termination.
 */
static bool terminate() {
	MSG msg;
	PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE);

	return (msg.message == WM_QUIT);
}

/**
 * Push a JSON data packet to the server.
 * @param  a
 * @param  json free'd once sent.
 * @return      Non-zero error code on failure, zero otherwise.
 */
static DWORD push(struct attributes a, char* json) {
	// push the JSON string to the server
	Response* res = publishData(a.curl, a.url, a.header, json);

	// once sent the json string is no longer required
	free(json);

	if (!res) {
		// out of memory
		return ARE_OUT_OF_MEM;
	}

	// check for a curl error
	if (res->curlCode != 0) {
		// programmer error with libcurl (most likely)
		wchar_t* error = strToWstr(curl_easy_strerror(res->curlCode));

		if (!error) {
			// out of memory
			return ARE_OUT_OF_MEM;
		}

		msgBoxErr(NULL, ARE_CURL, error);
		free(error);

		return ARE_CURL;
	}

	// check for a request or server error
	if (res->status >= 400) {
		// something is fundamentally wrong with either:
		// (a) the request or
		// (b) the server
		// in either case it is best to terminate because (a) will
		// require re-compilation and (b) requires server maintainence
		wchar_t* error = strToWstr(res->body->data);

		if (!error) {
			// out of memory
			return ARE_OUT_OF_MEM;
		}

		msgBoxErr(NULL, ARE_REQ, error);
		free(error);

		return ARE_REQ;
	}

	// response no longer required
	free(res);

	return 0;
}

/**
 * Main loop. Implements ThreadProc.
 * @param  arg Cast to InstanceData*
 * @return     0 on success, an error code defined in error.h otherwise.
 */
DWORD WINAPI procedure(void* arg) {
	struct attributes attr;
	InstanceData* data = (InstanceData*) arg;

	// init message queue, curl, and necessary strings
	DWORD result = initAttributes(&attr, data);

	if (result != 0) {
		// initialisation failed
		return result;
	}

	while (!terminate()) {
		if (data->sm->curr.hud->status != STATUS_LIVE) {
			// wait until the player is in the car
			Sleep(SLEEP_DURATION);
			continue;
		}

		// get a time stamp to measure the length of the process
		ULONGLONG start = GetTickCount64();

		// get a (delta) JSON string of the shared memory values
		char* json = toJSON(
			data->sm->prev,
			data->sm->curr,
			!data->sm->prev.props ||
			propertiesUpdated(data->sm->prev.props, data->sm->curr.props)
		);

		if (!json) {
			// out of memory
			return ARE_OUT_OF_MEM;
		}

		// send the json to the server
		result = push(attr, json);

		if (result != 0) {
			// something went wrong with curl or no memory
			return result;
		}

		// copy the current frame's data to the previous frame
		sharedMemCurrToPrev(data->sm);

		// determine how long the above process took and sleep
		// for MAX_LOOP_TIME - duration (if at all)
		ULONGLONG duration = GetTickCount64() - start;
		ULONGLONG ms = MAX_LOOP_TIME - duration;

		// since ms is unsigned, the sleep duration could work out to be longer
		// than MAX_LOOP_TIME due to an underflow. So only sleep if the value is between
		// zero and MAX_LOOP_TIME
		if (ms > 0 && ms <= MAX_LOOP_TIME) {
			Sleep((DWORD) ms);
		}
	}

	// free all the mallocs
	freeAttributes(attr);

	return 0;
}
