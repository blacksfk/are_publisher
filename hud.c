#include "hud.h"

/**
 * last, best, current, delta, estimated, lastSplit, isDeltaPositive, isValidLap.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createLaptimes(HUD* curr, HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	// current lap time should always be different so need for comparison
	if (!cJSON_AddNumberToObject(obj, "current", curr->currentTime)) {
		RET_NULL(obj);
	}

	NUM_2_OBJ(obj, "last", prev, prev->lastTime, curr->lastTime);
	NUM_2_OBJ(obj, "best", prev, prev->bestTime, curr->bestTime);
	NUM_2_OBJ(obj, "delta", prev, prev->delta, curr->delta);
	NUM_2_OBJ(obj, "estimated", prev, prev->estimatedLapTime, curr->estimatedLapTime);
	NUM_2_OBJ(obj, "lastSplit", prev, prev->lastSplit, curr->lastSplit);

	BOOL_2_OBJ(obj, "isDeltaPositive", prev, prev->isDeltaPositive, curr->isDeltaPositive);
	BOOL_2_OBJ(obj, "isValidLap", prev, prev->isValidLap, curr->isValidLap);

	return obj;
}

/**
 * tc, tcCut, engineMap, abs, rainLight, flashingLights, lights, wiperLevel, leftIndicator,
 * rightIndicator.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createElectronics(HUD* curr, HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ(obj, "tc", prev, prev->tc, curr->tc);
	NUM_2_OBJ(obj, "tcCut", prev, prev->tcCut, curr->tcCut);
	NUM_2_OBJ(obj, "engineMap", prev, prev->engineMap, curr->engineMap);
	NUM_2_OBJ(obj, "abs", prev, prev->abs, curr->abs);
	NUM_2_OBJ(obj, "rainLight", prev, prev->rainLight, curr->rainLight);
	NUM_2_OBJ(obj, "flashingLights", prev, prev->flashingLights, curr->flashingLights);
	NUM_2_OBJ(obj, "lights", prev, prev->lights, curr->lights);
	NUM_2_OBJ(obj, "wiperLevel", prev, prev->wiperLevel, curr->wiperLevel);

	BOOL_2_OBJ(obj, "leftIndicator", prev, prev->leftIndicator, curr->leftIndicator);
	BOOL_2_OBJ(obj, "rightIndicator", prev, prev->rightIndicator, curr->rightIndicator);

	return obj;
}

/**
 * type, sessionTimeLeft, activeCars, clock.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createSession(HUD* curr, HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ(obj, "type", prev, prev->session, curr->session);
	NUM_2_OBJ(obj, "sessionTimeLeft", prev, prev->sessionTimeLeft, curr->sessionTimeLeft);
	NUM_2_OBJ(obj, "activeCars", prev, prev->activeCars, curr->activeCars);
	NUM_2_OBJ(obj, "clock", prev, prev->clock, curr->clock);

	return obj;
}

/**
 * surfaceGrip, windSpeed, windDirection, trackCondition, rain, rain.current, rain.in10, rain.in30.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createConditions(HUD* curr, HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ(obj, "surfaceGrip", prev, prev->surfaceGrip, curr->surfaceGrip);
	NUM_2_OBJ(obj, "windSpeed", prev, prev->windSpeed, curr->windSpeed);
	NUM_2_OBJ(obj, "windDirection", prev, prev->windDirection, curr->windDirection);
	NUM_2_OBJ(obj, "trackCondition", prev, prev->trackGrip, curr->trackGrip);

	cJSON* rain = cJSON_CreateObject();

	if (!rain) {
		RET_NULL(obj);
	}

	NUM_2_OBJ(rain, "current", prev, prev->rainIntensityCurr, curr->rainIntensityCurr);
	NUM_2_OBJ(rain, "in10", prev, prev->rainIntensity10, curr->rainIntensity10);
	NUM_2_OBJ(rain, "in30", prev, prev->rainIntensity30, curr->rainIntensity30);

	cJSON_AddItemToObject(obj, "rain", rain);

	return obj;
}

/**
 * tyreSet, fuel, pressure, pressure.fl, pressure.fr, pressure.rl, pressure.rr.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createPitstop(HUD* curr, HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ(obj, "tyreSet", prev, prev->pitStopTyreSet, curr->pitStopTyreSet);
	NUM_2_OBJ(obj, "fuel", prev, prev->pitStopFuel, curr->pitStopFuel);

	cJSON* pressure = cJSON_CreateObject();

	NUM_2_OBJ(pressure, "fl", prev, prev->pitStopFL, curr->pitStopFL);
	NUM_2_OBJ(pressure, "fr", prev, prev->pitStopFR, curr->pitStopFR);
	NUM_2_OBJ(pressure, "rl", prev, prev->pitStopRL, curr->pitStopRL);
	NUM_2_OBJ(pressure, "rr", prev, prev->pitStopRR, curr->pitStopRR);

	cJSON_AddItemToObject(obj, "pressure", pressure);

	return obj;
}

/**
 * trackStatus, position, distanceTraveled, gameStatus, laps, isBoxed, isInPitLane,
 * penalty, penalty.type, penalty.duration, mandatoryPitDone,
 * drivingTime, drivingTime.totalRemaining, drivingTime.stintRemaining, rainTyres,
 * fuel, fuel.used, fuel.rate,
 * flag, flag.current, flag.yellow, flag.yellow.global, flag.yellow.sector1,
 * flag.yellow.sector2, flag.yellow.sector3, flag.white, flag.white.global, flag.white.sector1,
 * flag.white.sector2, flag.white.sector3, flag.green, flag.chequered, flag.red.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createStatus(HUD* curr, HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	// now for some windows-flavoured string fun
	// compare if a previous frame was provided then convert to a multi-byte char*
	if (!prev || wcscmp(prev->trackStatus, curr->trackStatus) != 0) {
		// how to be lazy 101
		char mbs[MB_STR_LEN + 1];
		size_t len = sizeof(mbs);

		wcstombs_s(NULL, mbs, len, curr->trackStatus, len - 1);

		if (!cJSON_AddStringToObject(obj, "trackStatus", mbs)) {
			RET_NULL(obj);
		}
	}

	NUM_2_OBJ(obj, "position", prev, prev->position, curr->position);
	NUM_2_OBJ(obj, "distanceTraveled", prev, prev->distanceTraveled, curr->distanceTraveled);
	NUM_2_OBJ(obj, "gameStatus", prev, prev->status, curr->status);
	NUM_2_OBJ(obj, "laps", prev, prev->completedLaps, curr->completedLaps);

	BOOL_2_OBJ(obj, "isBoxed", prev, prev->isBoxed, curr->isBoxed);
	BOOL_2_OBJ(obj, "isInPitLane", prev, prev->isInPitLane, curr->isInPitLane);
	BOOL_2_OBJ(obj, "mandatoryPitDone", prev, prev->mandatoryPitDone, curr->mandatoryPitDone);
	BOOL_2_OBJ(obj, "rainTyres", prev, prev->rainTyres, curr->rainTyres);

	cJSON* penalty = cJSON_CreateObject();

	if (!penalty) {
		RET_NULL(obj);
	}

	NUM_2_OBJ(penalty, "type", prev, prev->penalty, curr->penalty);
	NUM_2_OBJ(penalty, "duration", prev, prev->penaltyTime, curr->penaltyTime);

	cJSON_AddItemToObject(obj, "penalty", penalty);

	cJSON* drivingTime = cJSON_CreateObject();

	if (!drivingTime) {
		RET_NULL(obj);
	}

	// totalTimeLeft should always be decreasing
	if (!cJSON_AddNumberToObject(drivingTime, "totalRemaining", curr->totalTimeLeft)) {
		RET_NULL(obj);
	}

	// stintTimeLeft should always be decreasing
	if (!cJSON_AddNumberToObject(drivingTime, "stintRemaining", curr->stintTimeLeft)) {
		RET_NULL(obj);
	}

	cJSON_AddItemToObject(obj, "drivingTime", drivingTime);

	cJSON* fuel = cJSON_CreateObject();

	if (!fuel) {
		RET_NULL(obj);
	}

	cJSON_AddItemToObject(obj, "fuel", fuel);
	NUM_2_OBJ(fuel, "used", prev, prev->fuelUsed, curr->fuelUsed);
	NUM_2_OBJ(fuel, "rate", prev, prev->fuelPerLap, curr->fuelPerLap);

	cJSON* flag = cJSON_CreateObject();

	if (!flag) {
		RET_NULL(obj);
	}

	cJSON_AddItemToObject(obj, "flag", flag);
	NUM_2_OBJ(flag, "current", prev, prev->flag, curr->flag);
	BOOL_2_OBJ(flag, "green", prev, prev->globalGreen, curr->globalGreen);
	BOOL_2_OBJ(flag, "chequered", prev, prev->chequered, curr->chequered);
	BOOL_2_OBJ(flag, "red", prev, prev->globalRed, curr->globalRed);

	cJSON* yellow = cJSON_CreateObject();

	if (!yellow) {
		RET_NULL(obj);
	}

	cJSON_AddItemToObject(flag, "yellow", yellow);
	BOOL_2_OBJ(yellow, "sector1", prev, prev->yellow1, curr->yellow1);
	BOOL_2_OBJ(yellow, "sector2", prev, prev->yellow2, curr->yellow2);
	BOOL_2_OBJ(yellow, "sector3", prev, prev->yellow3, curr->yellow3);

	cJSON* white = cJSON_CreateObject();

	if (!white) {
		RET_NULL(obj);
	}

	cJSON_AddItemToObject(obj, "white", white);
	BOOL_2_OBJ(white, "sector1", prev, prev->white1, curr->white1);
	BOOL_2_OBJ(white, "sector2", prev, prev->white2, curr->white2);
	BOOL_2_OBJ(white, "sector3", prev, prev->white3, curr->white3);

	return obj;
}

// struct grouping together the json key and the function that creates the json object.
struct subObject {
	char* key;
	cJSON* (*fn)(HUD*, HUD*);
};

// remember to update when adding additional sub-objects
#define HUD_OBJ_COUNT 6

static const struct subObject subObjects[HUD_OBJ_COUNT] = {
	{"laptimes", &createLaptimes},
	{"electronics", &createElectronics},
	{"session", &createSession},
	{"status", &createStatus},
	{"nextstop", &createPitstop},
	{"conditions", &createConditions}
};

cJSON* hudToJSON(HUD* curr, HUD* prev) {
	cJSON* json = cJSON_CreateObject();

	if (!json) {
		return NULL;
	}

	for (int i = 0; i < HUD_OBJ_COUNT; i++) {
		cJSON* ptr = subObjects[i].fn(curr, prev);

		if (!ptr) {
			RET_NULL(json);
		}

		cJSON_AddItemToObject(json, subObjects[i].key, ptr);
	}

	return json;
}
