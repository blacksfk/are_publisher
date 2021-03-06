#include "properties.h"

struct item {
	char* key;
	cJSON* (*create)(const Properties*);
};

/**
 * firstname, surname, nickname.
 */
static cJSON* createPlayer(const Properties* props) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	if (!addWstrToObject(obj, "firstname", props->firstname)) {
		RET_NULL(obj);
	}

	if (!addWstrToObject(obj, "surname", props->surname)) {
		RET_NULL(obj);
	}

	if (!addWstrToObject(obj, "nickname", props->nickname)) {
		RET_NULL(obj);
	}

	return obj;
}

/**
 * model, maxRPM, tankCap.
 */
static cJSON* createCar(const Properties* props) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	if (!addWstrToObject(obj, "model", props->carModel)) {
		RET_NULL(obj);
	}

	INT_2_OBJ(obj, "maxRPM", props->maxRPM);
	FLOAT_2_OBJ(obj, "tankCap", props->tankCap);

	return obj;
}

/**
 * name, sectors.
 */
static cJSON* createTrack(const Properties* props) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	if (!addWstrToObject(obj, "name", props->track)) {
		RET_NULL(obj);
	}

	INT_2_OBJ(obj, "sectors", props->sectorCount);

	return obj;
}

/**
 * start, end.
 */
static cJSON* createPitWindow(const Properties* props) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	INT_2_OBJ(obj, "start", props->pitWindowStart);
	INT_2_OBJ(obj, "end", props->pitWindowEnd);

	return obj;
}

#define PROPS_ITEM_COUNT 4

static const struct item items[PROPS_ITEM_COUNT] = {
	{"player", &createPlayer},
	{"car", &createCar},
	{"track", &createTrack},
	{"pitWindow", &createPitWindow}
};

/**
 * Adds the sub-objects above along with: sessions, sharedMemVer, accVer, dryTyre, wetTyre.
 * Properties contains static information and is only changed on a new instance
 * initialisation. Eg. when the player joins a server or creates a new weekend etc.
 */
cJSON* propertiesToJSON(cJSON* obj, const Properties* props) {
	INT_2_OBJ(obj, "sessions", props->sessions);

	if (!addWstrToObject(obj, "sharedMemVer", props->sharedMemVer)) {
		RET_NULL(obj);
	}

	if (!addWstrToObject(obj, "accVer", props->accVer)) {
		RET_NULL(obj);
	}

	for (int i = 0; i < PROPS_ITEM_COUNT; i++) {
		cJSON* ptr = items[i].create(props);

		if (!ptr) {
			RET_NULL(obj);
		}

		if (!cJSON_AddItemToObject(obj, items[i].key, ptr)) {
			cJSON_Delete(obj);
			cJSON_Delete(ptr);

			return NULL;
		}
	}

	return obj;
}

// macros to ease typing
#define INT_CMP(x, y) do {\
	if (x != y) {\
		return true;\
	}\
} while(0)

#define WCS_CMP(x, y) do {\
	if (wcscmp(x, y) != 0) {\
		return true;\
	}\
} while(0)

/**
 * Whether or not the two Properties objects differ.
 */
bool propertiesUpdated(const Properties* a, const Properties* b) {
	INT_CMP(a->sessions, b->sessions);
	INT_CMP(a->cars, b->cars);

	WCS_CMP(a->carModel, b->carModel);
	WCS_CMP(a->track, b->track);
	WCS_CMP(a->firstname, b->firstname);
	WCS_CMP(a->surname, b->surname);
	WCS_CMP(a->nickname, b->nickname);

	INT_CMP(a->pitWindowStart, b->pitWindowStart);
	INT_CMP(a->pitWindowEnd, b->pitWindowEnd);

	INT_CMP(a->isMultiplayer, b->isMultiplayer);

	// if all above match then 99% sure it's the same weekend
	return false;
}
