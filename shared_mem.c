#include "shared_mem.h"

/**
 * Create a pointer to the shared memory at location.
 * @param  location The shared memory location to map.
 * @param  size     The size of the map.
 * @return          A pointer to the start of the map or NULL if the file mapping failed.
 */
static void* mapSharedMemory(wchar_t* location, size_t size) {
	HANDLE file = CreateFileMappingW(
		INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD) size, location
	);

	if (!file) {
		return NULL;
	}

	return MapViewOfFile(file, FILE_MAP_READ, 0, 0, size);
}

/**
 * Creates a shared memory object on the heap and initialises pointers to the
 * ACC shared memory locations.
 */
SharedMem* createSharedMem() {
	SharedMem* sm = malloc(sizeof(*sm));

	if (!sm) {
		// out of memory
		return NULL;
	}

	sm->szHud = sizeof(HUD);
	sm->szPhysics = sizeof(Physics);
	sm->szProps = sizeof(Properties);

	// allocate memory for the data to be copied from the
	// shared memory locations at a later time.
	sm->prev.hud = malloc(sm->szHud);
	sm->prev.props = malloc(sm->szProps);
	sm->prev.physics = malloc(sm->szPhysics);

	if (!sm->prev.hud || !sm->prev.props || !sm->prev.physics) {
		// out of memory
		free(sm);

		return NULL;
	}

	// map the shared memory for each location
	sm->curr.hud = mapSharedMemory(SM_HUD, sm->szHud);
	sm->curr.props = mapSharedMemory(SM_PROPS, sm->szProps);
	sm->curr.physics = mapSharedMemory(SM_PHYSICS, sm->szPhysics);

	if (!sm->curr.hud || !sm->curr.physics || !sm->curr.props) {
		// something went wrong with the memory mapping
		freeSharedMem(sm);

		return NULL;
	}

	return sm;
}

/**
 * De-allocate a shared memory struct. Does nothing if sm is NULL.
 * @param sm
 */
void freeSharedMem(SharedMem* sm) {
	if (!sm) {
		return;
	}

	free(sm->prev.hud);
	free(sm->prev.props);
	free(sm->prev.physics);
	free(sm);
}

/**
 * Copy the data in shared memory (pointed to by pointers in curr) to heap
 * memory (pointed to by pointers in prev) and overwrite it
 * @param sm
 */
void sharedMemCurrToPrev(SharedMem* sm) {
	// there's no way that this could result in invalid data being left
	// on the heap and screwing the values, right??
	memcpy(sm->prev.hud, sm->curr.hud, sm->szHud);
	memcpy(sm->prev.props, sm->curr.props, sm->szProps);
	memcpy(sm->prev.physics, sm->curr.physics, sm->szPhysics);
}
