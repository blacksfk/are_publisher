#include "channel.h"

/**
 * Free the memory held by a channel and its members.
 * @param chan
 */
static void freeChannel(Channel* chan) {
	if (!chan) {
		return;
	}

	free(chan->id);
	free(chan->name);
	free(chan);
}

/**
 * Create a channel from a JSON object.
 * @param  raw
 */
static Channel* channelFromJSON(cJSON* raw) {
	Channel* chan = malloc(sizeof(*chan));

	if (!chan) {
		// out of memory
		return NULL;
	}

	cJSON* id = cJSON_GetObjectItemCaseSensitive(raw, "id");

	if (!cJSON_IsString(id) || !id->valuestring) {
		// missing ID
		freeChannel(chan);

		return NULL;
	}

	cJSON* name = cJSON_GetObjectItemCaseSensitive(raw, "name");

	if (!cJSON_IsString(name) || !name->valuestring) {
		// missing name
		freeChannel(chan);

		return NULL;
	}

	// convert to wide strings
	chan->id = strToWstr(id->valuestring);
	chan->name = strToWstr(name->valuestring);

	if (!chan->id || !chan->name) {
		// out of memory
		freeChannel(chan);

		return NULL;
	}

	return chan;
}

/**
 * Allocate memory for a channel node.
 * @param  chan
 */
static ChannelNode* createChannelNode(Channel* chan) {
	ChannelNode* node = malloc(sizeof(*node));

	if (!node) {
		return NULL;
	}

	node->next = NULL;
	node->chan = chan;

	return node;
}

/**
 * Free the memory held by a channel node and the channel object.
 * @param node
 */
static void freeChannelNode(ChannelNode* node) {
	freeChannel(node->chan);
	free(node);
}

/**
 * Create a Channel List from a JSON array.
 * @param  array
 */
ChannelList* channelListFromJSON(cJSON* array) {
	ChannelList* list = malloc(sizeof(*list));

	if (!list) {
		return NULL;
	}

	list->length = 0;
	list->head = NULL;

	for (cJSON* raw = array->child; raw; raw = raw->next) {
		Channel* chan = channelFromJSON(raw);

		if (!chan || !insertChannel(list, chan)) {
			freeChannel(chan);
			freeChannelList(list);

			return NULL;
		}
	}

	return list;
}

/**
 * Insert a channel into the list, sorted alphabetically.
 * @param  list
 * @param  chan
 * @return      False if out of memory and true otherwise.
 */
bool insertChannel(ChannelList* list, Channel* chan) {
	ChannelNode* new = createChannelNode(chan);

	if (!new) {
		return false;
	}

	if (!list->head) {
		// first insert
		list->head = new;
	} else {
		ChannelNode* prev = NULL;
		ChannelNode* curr = list->head;

		for (; curr && wcscmp(curr->chan->name, chan->name) >= 0;
			prev = curr, curr = curr->next);

		prev->next = new;
		new->next = curr;
	}

	list->length++;

	return true;
}

/**
 * Free the memory allocated to a channel list.
 * @param list
 */
void freeChannelList(ChannelList* list) {
	if (!list) {
		return;
	}

	// free all nodes and their respective channels
	for (ChannelNode* node = list->head; node;) {
		ChannelNode* next = node->next;

		freeChannelNode(node);
		node = next;
	}

	// free the list
	free(list);
}
