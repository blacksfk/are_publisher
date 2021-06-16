#include "main.h"

int WINAPI wWinMain(HINSTANCE curr, HINSTANCE prev, wchar_t* args, int cmdShow) {
	// discard prev instance (always NULL) and command line args
	(void) prev;
	(void) args;

	// initialise curl globally
	CURLcode cc = curl_global_init(CURL_GLOBAL_DEFAULT);

	if (cc != 0) {
		msgBoxErr(ARE_CURL, L"Curl global initialisation failed");

		return EXIT_FAILURE;
	}

	// initialise ACC shared memory maps
	SharedMem* sm = createSharedMem();

	if (!sm) {
		curl_global_cleanup();
		msgBoxErr(ARE_SHARED_MEM_INIT, L"Shared memory initialisation failed");

		return EXIT_FAILURE;
	}

	// initialise data tied to the UI (input buffers and handlers)
	InstanceData* data = createInstanceData(sm, &release);

	if (!data) {
		curl_global_cleanup();
		freeSharedMem(sm);
		msgBoxErr(ARE_OUT_OF_MEM, L"Out of memory");

		return EXIT_FAILURE;
	}

	// create and run the GUI
	bool success = gui(curr, cmdShow, data);

	if (!success) {
		msgBoxErr(ARE_GUI, L"Failed to create window");

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Attach a terminal window to the process for debugging output.
 */
void attachTerm() {
	// allocate a new console window for this process
	AllocConsole();

	// re-assign stdin, stdout, stderr to the new console
	// ignore return value because closing is handled by FreeConsole()
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
}

/**
 * Free allocated resources for curl, shared memory, and instance data objects.
 * Implements cleanupFunc.
 * @param data
 */
void release(InstanceData* data) {
	curl_global_cleanup();
	freeSharedMem(data->sm);
	freeInstanceData(data);
}
