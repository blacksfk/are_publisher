#include "main.h"

int main() {
	HUD* hud = (HUD*) mapSharedMemory(SM_HUD, sizeof(*hud));
	Physics* physics = (Physics*) mapSharedMemory(SM_PHYSICS, sizeof(*physics));
	Properties* props = (Properties*) mapSharedMemory(SM_PROPS, sizeof(*props));

	// if any of the maps are NULL then something went wrong
	if (!hud || !physics || !props) {
		printf("map initialisation failed\n");

		return EXIT_FAILURE;
	}

	// convert memory maps to JSON
	cJSON* container = cJSON_CreateObject();

	if (!container) {
		printf("container is null\n");

		return EXIT_FAILURE;
	}

	cJSON* hudObj = hudToJSON(hud, NULL);

	if (!hudObj) {
		cJSON_Delete(container);
		printf("hudObj is null!\n");

		return EXIT_FAILURE;
	}

	cJSON_AddItemToObject(container, "hud", hudObj);

	cJSON* physicsObj = physicsToJSON(physics, NULL);

	if (!physicsObj) {
		cJSON_Delete(container);
		printf("physicsObj is null!\n");

		return EXIT_FAILURE;
	}

	cJSON_AddItemToObject(container, "physics", physicsObj);

	cJSON* propsObj = propertiesToJSON(props);

	if (!propsObj) {
		cJSON_Delete(container);
		printf("propsObj is null\n");

		return EXIT_FAILURE;
	}

	cJSON_AddItemToObject(container, "props", propsObj);

	char* str = cJSON_PrintUnformatted(container);

	printf("JSON: %s\n", str);

	cJSON_Delete(container);

	// all gucci
	return EXIT_SUCCESS;
}

/**
 * Create a pointer to the shared memory at location.
 *
 * @param  location The shared memory location to map.
 * @param  size     The size of the map.
 * @return          A pointer to the start of the map or NULL if the file mapping failed.
 */
LPVOID mapSharedMemory(wchar_t* location, size_t size) {
	HANDLE file = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD) size, location);

	if (!file) {
		return NULL;
	}

	return MapViewOfFile(file, FILE_MAP_READ, 0, 0, size);
}
