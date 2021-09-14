#ifndef CHANNEL_H
#define CHANNEL_H

#include <cJSON/cJSON.h>
#include "auxiliary.h"

typedef struct channel {
	wchar_t* id;
	wchar_t* name;
} Channel;

typedef struct channelNode {
	Channel* chan;
	struct channelNode* next;
} ChannelNode;

typedef struct channelList {
	ChannelNode* head;
	int length;
} ChannelList;

ChannelList* channelListFromJSON(cJSON*);
bool insertChannel(ChannelList*, Channel*);
void freeChannelList(ChannelList*);

#endif
