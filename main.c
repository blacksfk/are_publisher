#include "main.h"

/**
 * Free allocations by curl, shared memory, and instance data.
 */
#define CLEANUP(s, d) do {\
	curl_global_cleanup();\
	freeSharedMem(s);\
	freeInstanceData(d);\
} while(0)

/**
 * WIN32 API entry point.
 */
int WINAPI wWinMain(HINSTANCE curr, HINSTANCE prev, wchar_t* args, int cmdShow) {
	// discard prev instance (always NULL) and command line args
	(void) prev;
	(void) args;

#ifdef DEBUG
	// debug flag defined so create a terminal for I/O
	attachTerm();
#endif

	// initialise curl globally
	CURLcode cc = curl_global_init(CURL_GLOBAL_DEFAULT);

	if (cc != 0) {
		msgBoxErr(NULL, ARE_CURL, L"Curl global initialisation failed");

		return EXIT_FAILURE;
	}

	// initialise ACC shared memory maps
	SharedMem* sm = createSharedMem();

	if (!sm) {
		CLEANUP(NULL, NULL);
		msgBoxErr(NULL, ARE_SHARED_MEM_INIT, L"Shared memory initialisation failed");

		return EXIT_FAILURE;
	}

	// initialise data tied to the UI (input buffers and handlers)
	InstanceData* data = createInstanceData(sm);

	if (!data) {
		CLEANUP(sm, NULL);
		msgBoxErr(NULL, ARE_OUT_OF_MEM, L"Out of memory");

		return EXIT_FAILURE;
	}

	// create and run the GUI
	gui(curr, cmdShow, data);
	CLEANUP(sm, data);

#ifdef DEBUG
	// debug defined so don't exit until the user requests it
	wprintf(L"Press enter to exit...\n");
	fgetwc(stdin);
#endif

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
