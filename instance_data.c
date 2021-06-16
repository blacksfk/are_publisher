#include "instance_data.h"

/**
 * Allocate memory for an instance data struct and initialise various members.
 * @param  curl
 * @return      Returns NULL if memory could not be allocated.
 */
InstanceData* createInstanceData(
	SharedMem* sm, void (*cleanup)(struct instanceData*)
) {
	InstanceData* data = malloc(sizeof(*data));

	if (!data) {
		// out of memory
		return NULL;
	}

	// allocate memory for text input controls
	data->address = malloc(FORM_CTRL_BUF_SIZE);
	data->channel = malloc(FORM_CTRL_BUF_SIZE);
	data->password = malloc(FORM_CTRL_BUF_SIZE);

	if (!data->address || !data->channel || !data->password) {
		// out of memory
		freeInstanceData(data);

		return NULL;
	}

	// calculate the number of characters (should be FORM_CTRL_BUF_SIZE / 2)
	size_t count = FORM_CTRL_BUF_SIZE / sizeof(wchar_t);

	// initialise all text input buffers with wchar_t null bytes
	wmemset(data->address, L'\0', count);
	wmemset(data->channel, L'\0', count);
	wmemset(data->password, L'\0', count);

	data->sm = sm;
	data->running = false;
	data->cleanup = cleanup;
	data->thread = NULL;
	data->threadId = 0;

	return data;
}

/**
 * Free instance data along with all heap allocated members.
 */
void freeInstanceData(InstanceData* data) {
	free(data->address);
	free(data->channel);
	free(data->password);
	free(data);
}
