#include "procedure.h"

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
 * Frees all allocations in the procedure function.
 */
#define CLEANUP do {\
	free(header);\
	free(url);\
	free(password);\
	free(channel);\
	free(address);\
	curl_easy_cleanup(curl);\
} while(0)

/**
 * Main loop. Implements ThreadProc.
 * @param  arg Cast to InstanceData*
 * @return     0 on success, an error code defined in error.h otherwise.
 */
DWORD WINAPI procedure(void* arg) {
	MSG msg;

	// initialise the message queue (used for thread termination)
	PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	// initialise a curl easy handle
	CURL* curl = curl_easy_init();

	if (!curl) {
		return ARE_CURL;
	}

	// cast to an instance data pointer
	InstanceData* data = (InstanceData*) arg;

	// convert each wide string to a multi-byte string for use with curl
	char* address = wstrToStr(data->address);
	char* channel = wstrToStr(data->channel);
	char* password = wstrToStr(data->password);
	char* url = NULL;
	char* header = NULL;

	if (!address || !channel || !password) {
		// out of memory
		CLEANUP;

		return ARE_OUT_OF_MEM;
	}

	url = createPublishURL(address, channel);
	header = createPasswordHeader(password);

	if (!url || !header) {
		// out of memory
		CLEANUP;

		return ARE_OUT_OF_MEM;
	}

	int exitCode = 0;
	bool exit = false;

	while (true) {
		// wait until the player is in the car or
		// this thread is asked to terminate
		while (true) {
			// check if a session has been initialised and the player is
			// in the car
			if (data->sm->curr.hud->status == STATUS_LIVE) {
				break;
			}

			// check the message queue for termination
			exit = terminate();

			if (exit) {
				break;
			}

			// sleep thread and check again in >= SLEEP_DURATION
			Sleep(SLEEP_DURATION);
		}

		if (exit) {
			// parent thread has requested termination
			break;
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
			exitCode = ARE_OUT_OF_MEM;
			break;
		}

		// push the JSON string to the server
		Response* res = publishData(curl, url, header, json);

		// once sent the json string is no longer required
		free(json);

		if (!res) {
			// out of memory
			exitCode = ARE_OUT_OF_MEM;
			break;
		}

		// check for a curl error
		if (res->curlCode != 0) {
			// programmer error with libcurl (most likely)
			wchar_t* error = strToWstr(curl_easy_strerror(res->curlCode));

			if (!error) {
				// out of memory
				exitCode = ARE_OUT_OF_MEM;
			} else {
				msgBoxErr(ARE_CURL, error);
				exitCode = ARE_CURL;
				free(error);
			}

			break;
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
				exitCode = ARE_OUT_OF_MEM;
			} else {
				msgBoxErr(ARE_REQ, error);
				exitCode = ARE_REQ;
				free(error);
			}

			break;
		}

		// response no longer required
		free(res);

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
	CLEANUP;

	return exitCode;
}
