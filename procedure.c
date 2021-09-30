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

	// create the password header string
	char* pwHeader = createPasswordHeader(data->password);

	if (!pwHeader) {
		// out of memory
		freeAttributes(*a);

		return ARE_OUT_OF_MEM;
	}

	// initialise the curl handle with the required parameters
	a->headers = publishInit(a->curl, API_URL, data->channel, pwHeader);

	// temporary string no longer required
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

	#ifndef DISABLE_BROADCAST
		printf("broadcasting...\n");
		// send the json to the server
		result = publish(attr.curl, json);

		if (result != 0) {
			// something went wrong with curl or no memory
			break;
		}
	#endif

		// json no longer required
		free(json);

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
