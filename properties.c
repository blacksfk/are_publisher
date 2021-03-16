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

	NUM_2_OBJ(obj, "maxRPM", props->maxRPM);
	NUM_2_OBJ(obj, "tankCap", props->tankCap);

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

	NUM_2_OBJ(obj, "sectors", props->sectorCount);

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

	NUM_2_OBJ(obj, "start", props->pitWindowStart);
	NUM_2_OBJ(obj, "end", props->pitWindowEnd);

	return obj;
}

/**
 * penaltiesEnabled, fuelRate, tyreRate, damageRate, tyreBlankets, sessions, cars.
 */
static cJSON* createWeekend(const Properties* props) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	BOOL_2_OBJ(obj, "penaltiesEnabled", props->penaltiesEnabled);
	BOOL_2_OBJ(obj, "tyreBlankets", props->allowTyreBlankets);

	NUM_2_OBJ(obj, "fuelRate", props->fuelRate);
	NUM_2_OBJ(obj, "tyreRate", props->tyreRate);
	NUM_2_OBJ(obj, "damageRate", props->damageRate);
	NUM_2_OBJ(obj, "sessions", props->sessions);
	NUM_2_OBJ(obj, "cars", props->cars);

	return obj;
}

#define PROPS_ITEM_COUNT 5

static const struct item items[PROPS_ITEM_COUNT] = {
	{"player", &createPlayer},
	{"car", &createCar},
	{"track", &createTrack},
	{"pitWindow", &createPitWindow},
	{"weekend", &createWeekend}
};

/**
 * Adds the sub-objects above along with: sharedMemVer, accVer, dryTyre, wetTyre.
 * Properties contains static information and is only changed on a new instance
 * initialisation. Eg. when the player joins a server or creates a new weekend etc.
 */
cJSON* propertiesToJSON(const Properties* props) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	if (!addWstrToObject(obj, "sharedMemVer", props->sharedMemVer)) {
		RET_NULL(obj);
	}

	if (!addWstrToObject(obj, "accVer", props->accVer)) {
		RET_NULL(obj);
	}

	if (!addWstrToObject(obj, "dryTyre", props->dryTyreName)) {
		RET_NULL(obj);
	}

	if (!addWstrToObject(obj, "wetTyre", props->wetTyreName)){
		RET_NULL(obj);
	}

	for (int i = 0; i < PROPS_ITEM_COUNT; i++) {
		cJSON* ptr = items[i].create(props);

		if (!ptr) {
			RET_NULL(obj);
		}

		cJSON_AddItemToObject(obj, items[i].key, ptr);
	}

	return obj;
}
