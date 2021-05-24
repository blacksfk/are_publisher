#include "main.h"

// volatile qualifier to prevent compiler optimisation
static volatile bool terminate = false;

static void signalHandler(int v) {
	// discard v
	(void) v;

	printf("User-requested termination\n");
	terminate = true;
}

int main(int argc, char** argv) {
	struct config conf = {NULL, NULL, NULL};

	if (!args(argc, argv, &conf)) {
		return EXIT_FAILURE;
	}

	// initialise memory maps
	size_t szHud = sizeof(HUD), szPhysics = sizeof(Physics);
	size_t szProps = sizeof(Properties);
	struct memMaps prev = {NULL, NULL, NULL};
	struct memMaps curr = {
		.hud = (HUD*) mapSharedMemory(SM_HUD, szHud),
		.physics = (Physics*) mapSharedMemory(SM_PHYSICS, szPhysics),
		.props = (Properties*) mapSharedMemory(SM_PROPS, szProps)
	};

	// if any of the maps are NULL then something went wrong
	if (!curr.hud || !curr.physics || !curr.props) {
		fprintf(stderr, "%d: Shared memory map initialisation failed\n", ARE_SHARED_MEM_INIT);

		return EXIT_FAILURE;
	}

	// register handling of SIGINT and SIGTERM
	signal(SIGINT, &signalHandler);
	signal(SIGTERM, &signalHandler);

	// init curl
	CURL* curl = initCurl();

	if (!curl) {
		return EXIT_FAILURE;
	}

	int status = EXIT_SUCCESS;

	// loop until sigint is received
	while (!terminate) {
		// wait until the player is in the car
		// TODO: refactor
		// TODO: handle SIGINT/SIGTERM in this loop
		while (curr.hud->status != STATUS_LIVE) {
			Sleep(WAIT_FOR_LIVE);
		}

		// milliseconds since system boot
		ULONGLONG start = GetTickCount64();
		char* json;

		if (!prev.props || (prev.props && propertiesUpdated(prev.props, curr.props))) {
			json = toJSON(prev, curr, true);
		} else {
			json = toJSON(prev, curr, false);
		}

		if (!json) {
			status = EXIT_FAILURE;
			break;
		}

		// publish the data
		if (!publish(curl, conf, json)) {
			status = EXIT_FAILURE;
			break;
		}

		// free the previous frame
		free(prev.hud);
		free(prev.props);
		free(prev.physics);

		// clone each of the memory maps for comparison in the next iteration
		prev.hud = (HUD*) clone(curr.hud, szHud);
		prev.props = (Properties*) clone(curr.props, szProps);
		prev.physics = (Physics*) clone(curr.physics, szPhysics);


		if (!prev.hud || !prev.props || !prev.physics) {
			fprintf(
				stderr,
				"%d: Could not allocate memory for previous frame\n",
				ARE_OUT_OF_MEM
			);
			status = EXIT_FAILURE;
			break;
		}

		// determine how long the above process took and
		// sleep for MAX_LOOP_TIME - duration (if at all)
		ULONGLONG duration = GetTickCount64() - start;
		DWORD ms = (DWORD) (MAX_LOOP_TIME - duration);

		if (ms > 0) {
			Sleep(ms);
		} else {
			fprintf(stderr, "%d: Process took %llums\n", ARE_DURATION, duration);
		}
	}

	// clean up the curl handle and library
	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return status;
}

/**
 * Helper function to populate the configuration struct.
 */
bool args(int argc, char** argv, struct config* conf) {
	if (argc != MIN_ARGS) {
		fprintf(
			stderr,
			"%d: Please provide %d arguments in the format: "
			"./%s <address> <channel_id> <password>\n",
			ARE_INVALID_ARGS,
			MIN_ARGS - 1,
			argv[0]
		);

		return false;
	}

	conf->address = argv[1];
	conf->id = argv[2];
	conf->pw = argv[3];

	return true;
}

/**
 * Create a pointer to the shared memory at location.
 *
 * @param  location The shared memory location to map.
 * @param  size     The size of the map.
 * @return          A pointer to the start of the map or NULL if the file mapping failed.
 */
LPVOID mapSharedMemory(wchar_t* location, size_t size) {
	HANDLE file = CreateFileMappingW(
		INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD) size, location
	);

	if (!file) {
		return NULL;
	}

	return MapViewOfFile(file, FILE_MAP_READ, 0, 0, size);
}

/**
 * Helper function to initialise a curl handle.
 */
CURL* initCurl() {
	CURLcode cc = curl_global_init(CURL_GLOBAL_DEFAULT);

	if (cc != 0) {
		fprintf(stderr, "%d: Could not initialise curl: %s\n", ARE_CURL, curl_easy_strerror(cc));

		return NULL;
	}

	CURL* curl = curl_easy_init();

	if (!curl) {
		fprintf(stderr, "%d: Could not initialise a curl easy handle\n", ARE_CURL);
		curl_global_cleanup();

		return NULL;
	}

	return curl;
}

/**
 * Helper function to convert the data in the memory maps to an unformatted JSON
 * string and handle associated errors.
 */
char* toJSON(struct memMaps prev, struct memMaps curr, bool sendProps) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		fprintf(stderr, "%d: Could not allocate memory for JSON object\n", ARE_OUT_OF_MEM);

		return NULL;
	}

	// convert HUD parameters to a JSON object
	cJSON* hud = hudToJSON(curr.hud, prev.hud);

	if (!hud) {
		// if memory isn't being allocated something is seriously wrong
		fprintf(
			stderr,
			"%d: Could not allocate memory for HUD JSON object\n",
			ARE_OUT_OF_MEM
		);
		cJSON_Delete(obj);

		return NULL;
	}

	// add it to the parent object
	if (!cJSON_AddItemToObject(obj, "hud", hud)) {
		fprintf(stderr, "%d: Could not add HUD to the JSON object\n", ARE_OUT_OF_MEM);
		cJSON_Delete(hud);
		cJSON_Delete(obj);

		return NULL;
	}

	// conver physics parameters to a JSON object
	cJSON* physics = physicsToJSON(curr.physics, prev.physics);

	if (!physics) {
		fprintf(
			stderr,
			"%d: Could not allocate memory for Physics JSON object\n",
			ARE_OUT_OF_MEM
		);
		cJSON_Delete(obj);

		return NULL;
	}

	// add it to the parent
	if (!cJSON_AddItemToObject(obj, "physics", physics)) {
		fprintf(stderr, "%d: Could not add Physics to the JSON object\n", ARE_OUT_OF_MEM);
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
			fprintf(
				stderr,
				"%d: Could not allocate memory for properties JSON object\n",
				ARE_OUT_OF_MEM
			);
		}

		if (!cJSON_AddItemToObject(obj, "properties", props)) {
			fprintf(stderr, "%d: Could not add Properties to the JSON object\n", ARE_OUT_OF_MEM);
			cJSON_Delete(obj);
			cJSON_Delete(props);

			return NULL;
		}
	}

	// TODO: determine the most appropriate value for JSON_BUF_SIZE in order
	// to minimise re-allocations
	char* str = cJSON_PrintBuffered(obj, JSON_BUF_SIZE, 0);

	if (!str) {
		fprintf(stderr, "Could not stringify JSON object\n");

		return NULL;
	}

	// json printed; objects are no longer required
	cJSON_Delete(obj);

	// remember to free the string
	return str;
}

/**
 * Helper function to send a request with a JSON body to the server and handle associated
 * errors.
 */
bool publish(CURL* curl, struct config conf, char* json) {
	// publish the json
	Response* res = publishData(curl, conf.address, conf.id, conf.pw, json);

	// json string is no longer required
	free(json);

	if (!res) {
		fprintf(stderr, "%d: Could not allocate memory for response\n", ARE_OUT_OF_MEM);

		return false;
	}

	if (res->curlCode != 0) {
		// something went wrong with the curl handling
		fprintf(stderr, "%d: %s\n", ARE_CURL, curl_easy_strerror(res->curlCode));
		free(res);

		return false;
	}

	if (res->status != 200) {
		// something is wrong with the body or headers or the server has a problem
		fprintf(stderr, "%d: %d: %s\n", ARE_REQ, res->status, res->body->data);
		free(res);

		return false;
	}

	// no longer need the response
	free(res);

	return true;
}

/**
 * Copies count bytes from the region of memory pointed to by ptr to
 * a newly-allocated region. Remember to free it! Returns NULL if
 * the allocation was unsuccessful.
 *
 * @param  ptr   Pointer to a region of memory to clone.
 * @param  count The size of the region of memory.
 * @return       A shallow clone of the region of memory pointed to by ptr.
 */
void* clone(const void* ptr, size_t count) {
	void* dupe = malloc(count);

	if (!dupe) {
		return NULL;
	}

	memcpy(dupe, ptr, count);

	return dupe;
}
