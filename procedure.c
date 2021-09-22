#include "procedure.h"

// groups the curl handler, header list, and tracked extra data
struct attributes {
	CURL* curl;
	Tracked* tracked;
	struct curl_slist* headers;
};

/**
 * Free memory allocated for the url, header, and curl.
 */
static void freeAttributes(struct attributes a) {
	freeTracked(a.tracked);
	curl_easy_cleanup(a.curl);
	curl_slist_free_all(a.headers);
}

/**
 * Creates a message queue, URL and header strings, and a curl easy handle.
 * @param  a
 * @param  data
 * @return      A non-zero code if an error occurs, zero otherwise.
 */
static DWORD initAttributes(struct attributes* a, InstanceData* data) {
	// initialise all members to NULL
	a->curl = NULL;
	a->headers = NULL;
	a->tracked = NULL;

	// force initialisation of the message queue
	MSG msg;

	PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	// create a curl easy handle
	a->curl = curl_easy_init();

	if (!a->curl) {
		return ARE_CURL;
	}

#ifdef CURL_SKIP_VERIFY
	curl_easy_setopt(a->curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef DEBUG
	wprintf(L"channel ID: %ls\npassword: %ls\n",
			data->channel, data->password);
#endif

	// convert the wide char buffers into char buffers
	char* channel = wstrToStr(data->channel);
	char* password = wstrToStr(data->password);

	if (!channel || !password) {
		// out of memory
		freeAttributes(*a);
		free(channel);
		free(password);

		return ARE_OUT_OF_MEM;
	}

	// create the password header string
	char* pwHeader = createPasswordHeader(password);

	// temporary password string no longer required
	free(password);

	if (!pwHeader) {
		// out of memory
		freeAttributes(*a);
		free(channel);

		return ARE_OUT_OF_MEM;
	}

	// initialise the curl handle with the required parameters
	a->headers = publishInit(a->curl, API_URL, channel, pwHeader);

	// temporary strings no longer required
	free(channel);
	free(pwHeader);

	if (!a->headers) {
		// out of memory
		freeAttributes(*a);

		return ARE_OUT_OF_MEM;
	}

	a->tracked = createTracked(DEFAULT_SECTOR_COUNT);

	if (!a->tracked) {
		freeAttributes(*a);

		return ARE_OUT_OF_MEM;
	}

	// signal the event in order for the parent to proceed
	if (!SetEvent(data->threadEvent)) {
		return ARE_EVENT;
	}

	// initialised successfully
	return 0;
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
	Response* res = publish(a.curl, json);

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

	#ifdef DEBUG
		fwprintf(stderr, L"Curl: %ls\n", error);
	#endif

		wprintf(L"%d: %ls\n", ARE_CURL, error);
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

	#ifdef DEBUG
		fwprintf(stderr, L"Status: %d\nBody: %ls\n", res->status, error);
	#endif

		wprintf(L"%d: %ls\n", ARE_REQ, error);
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

#ifdef RECORD_DATA
	// open file in binary mode so that fseek doesn't interpret
	// a line feed character as two bytes due to windows
	// convention being \r\n
	FILE* out = fopen("data.json", "wb");

	if (!out) {
		return ARE_THREAD;
	}

	fprintf(out, "[\n");
#endif

	// complete data set on the first run and any time the user
	// gets back into the car
	bool completeData = true;

	while (!terminate()) {
		if (!physicsIsInCar(data->sm->curr.physics)) {
			// wait until the player is in the car
			Sleep(SLEEP_DURATION);

			// get the complete data when the player gets back into the car again
			completeData = true;
			continue;
		}

		if (completeData) {
			// update the track sector count
			resetSectors(attr.tracked);

			if (!setSectorCount(attr.tracked, data->sm->curr.props->sectorCount)) {
				// re-allocation failed
				result = ARE_OUT_OF_MEM;
				break;
			}
		}

		// get a time stamp to measure the length of the process
		ULONGLONG start = GetTickCount64();
		char* json = deltaJSON(data->sm, attr.tracked, completeData);

		if (completeData) {
			completeData = false;
		}

		if (!json) {
			// out of memory
			result = ARE_OUT_OF_MEM;
			break;
		}

	#ifdef RECORD_DATA
		fprintf(out, "\t%s,\n", json);
	#endif

	#ifdef DISABLE_BROADCAST
		// json no longer required
		free(json);
	#else
		printf("broadcasting...\n");
		// send the json to the server
		result = push(attr, json);

		if (result != 0) {
			// something went wrong with curl or no memory
			break;
		}
	#endif

		// copy the current frame's data to the previous frame
		sharedMemCurrToPrev(data->sm);

		// determine how long the above process took and sleep
		// for MAX_LOOP_TIME - duration (if at all)
		ULONGLONG duration = GetTickCount64() - start;
		ULONGLONG ms = MAX_LOOP_TIME - duration;

	#ifdef DEBUG
		wprintf(L"duration: %llu, ms: %llu\n", duration, ms);
	#endif

		// since ms is unsigned, the sleep duration could work out to be longer
		// than MAX_LOOP_TIME due to an underflow. So only sleep if the value is between
		// zero and MAX_LOOP_TIME
		if (ms > 0 && ms <= MAX_LOOP_TIME) {
			Sleep((DWORD) ms);
		}
	}

#ifdef RECORD_DATA
	// go back two to overwrite the last comma
	// encoding should be utf-8 so only need to go back 2 bytes
	fseek(out, -2L, SEEK_CUR);
	fprintf(out, "\n]\n");
	fclose(out);
#endif

	// free all the mallocs
	freeAttributes(attr);

	return result;
}
