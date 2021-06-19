#include "hud.h"

// struct grouping together the json key and the function that creates the json object.
struct item {
	char* key;
	cJSON* (*create)(const HUD*, const HUD*);
};

/**
 * last, best, current, delta, estimated, lastSplit, isDeltaPositive, isValidLap.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createLaptimes(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	// current lap time should always be different so need for comparison
	NUM_2_OBJ(obj, "current", curr->currentTime);

	NUM_2_OBJ_CMP(obj, "last", prev, prev->lastTime, curr->lastTime);
	NUM_2_OBJ_CMP(obj, "best", prev, prev->bestTime, curr->bestTime);
	NUM_2_OBJ_CMP(obj, "delta", prev, prev->delta, curr->delta);
	NUM_2_OBJ_CMP(obj, "estimated", prev, prev->estimatedLapTime, curr->estimatedLapTime);
	NUM_2_OBJ_CMP(obj, "lastSplit", prev, prev->lastSplit, curr->lastSplit);

	BOOL_2_OBJ_CMP(obj, "isDeltaPositive", prev, prev->isDeltaPositive, curr->isDeltaPositive);
	BOOL_2_OBJ_CMP(obj, "isValidLap", prev, prev->isValidLap, curr->isValidLap);

	return obj;
}

/**
 * tc, tcCut, engineMap, abs, rainLight, flashingLights, lights, wiperLevel, leftIndicator,
 * rightIndicator.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createElectronics(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "tc", prev, prev->tc, curr->tc);
	NUM_2_OBJ_CMP(obj, "tcCut", prev, prev->tcCut, curr->tcCut);
	NUM_2_OBJ_CMP(obj, "engineMap", prev, prev->engineMap, curr->engineMap);
	NUM_2_OBJ_CMP(obj, "abs", prev, prev->abs, curr->abs);
	NUM_2_OBJ_CMP(obj, "rainLight", prev, prev->rainLight, curr->rainLight);
	NUM_2_OBJ_CMP(obj, "flashingLights", prev, prev->flashingLights, curr->flashingLights);
	NUM_2_OBJ_CMP(obj, "lights", prev, prev->lights, curr->lights);
	NUM_2_OBJ_CMP(obj, "wiperLevel", prev, prev->wiperLevel, curr->wiperLevel);

	BOOL_2_OBJ_CMP(obj, "leftIndicator", prev, prev->leftIndicator, curr->leftIndicator);
	BOOL_2_OBJ_CMP(obj, "rightIndicator", prev, prev->rightIndicator, curr->rightIndicator);

	return obj;
}

/**
 * type, sessionTimeLeft, activeCars, clock.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createSession(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	if (!prev || prev->session != curr->session) {
		char* str;

		switch (curr->session) {
			case ST_PRACTICE:
				str = "Practice";
				break;
			case ST_QUALIFY:
				str = "Qualifying";
				break;
			case ST_RACE:
				str = "Race";
				break;
			case ST_HOTLAP:
				str = "Hot Lap";
				break;
			case ST_HOTSTINT:
				str = "Hot Stint";
				break;
			case ST_SUPERPOLE:
				str = "Super Pole";
				break;
			default:
				str = "Unknown";
		}

		if (!cJSON_AddStringToObject(obj, "type", str)) {
			RET_NULL(obj);
		}
	}

	NUM_2_OBJ_CMP(obj, "timeLeft", prev, prev->sessionTimeLeft, curr->sessionTimeLeft);
	NUM_2_OBJ_CMP(obj, "activeCars", prev, prev->activeCars, curr->activeCars);
	NUM_2_OBJ_CMP(obj, "clock", prev, prev->clock, curr->clock);

	return obj;
}

/**
 * Determine the rain intensity level and embed the corresponding string
 * in obj under key.
 *
 * @param ri
 * @param obj
 * @param key
 */
static cJSON* rainIntensity(RainIntensity ri, cJSON* obj, char* key) {
	char* str;

	switch (ri) {
		case R_DRIZZLE:
			str = "Drizzle";
			break;
		case R_LIGHT:
			str = "Light";
			break;
		case R_MEDIUM:
			str = "Medium";
			break;
		case R_HEAVY:
			str = "Heavy";
			break;
		case R_THUNDERSTORM:
			str = "Thunderstorm";
			break;
		default:
			str = "None";
	}

	return cJSON_AddStringToObject(obj, key, str);
}

/**
 * current, in10, in30.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createRain(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	if (!prev || prev->rainIntensityCurr != curr->rainIntensityCurr) {
		if (!rainIntensity(curr->rainIntensityCurr, obj, "current")) {
			RET_NULL(obj);
		}
	}

	if (!prev || prev->rainIntensity10 != curr->rainIntensity10) {
		if (!rainIntensity(curr->rainIntensity10, obj, "in10")) {
			RET_NULL(obj);
		}
	}

	if (!prev || prev->rainIntensity30 != curr->rainIntensity30) {
		if (!rainIntensity(curr->rainIntensity30, obj, "in30")) {
			RET_NULL(obj);
		}
	}

	return obj;
}

/**
 * windSpeed, windDirection, track, rain, rain.current, rain.in10, rain.in30.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createConditions(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "windSpeed", prev, prev->windSpeed, curr->windSpeed);
	NUM_2_OBJ_CMP(obj, "windDirection", prev, prev->windDirection, curr->windDirection);

	// track grip
	if (!prev || prev->trackGrip != curr->trackGrip) {
		char* str;

		switch (curr->trackGrip) {
			case TG_FAST:
				str = "Fast";
				break;
			case TG_OPTIMUM:
				str = "Optimum";
				break;
			case TG_GREASY:
				str = "Greasy";
				break;
			case TG_DAMP:
				str = "Damp";
				break;
			case TG_WET:
				str = "Wet";
				break;
			case TG_FLOODED:
				str = "Flooded";
				break;
			default:
				str = "Green";
		}

		if (!cJSON_AddStringToObject(obj, "track", str)) {
			RET_NULL(obj);
		}
	}

	// add rain parameters
	cJSON* ptr = createRain(curr, prev);

	if (!ptr) {
		RET_NULL(obj);
	}

	if (cJSON_GetArraySize(ptr) > 0) {
		if (!cJSON_AddItemToObject(obj, "rain", ptr)) {
			cJSON_Delete(ptr);
			cJSON_Delete(obj);

			return NULL;
		}
	}

	return obj;
}

/**
 * fl, fr, rl, rr.
 */
static cJSON* createPressure(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->pitStopFL, curr->pitStopFL);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->pitStopFR, curr->pitStopFR);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->pitStopRL, curr->pitStopRL);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->pitStopRR, curr->pitStopRR);

	return obj;
}

/**
 * tyreSet, fuel, pressure, pressure.fl, pressure.fr, pressure.rl, pressure.rr.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createPitstop(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "tyreSet", prev, prev->pitStopTyreSet, curr->pitStopTyreSet);
	NUM_2_OBJ_CMP(obj, "strategyTyreSet", prev, prev->strategyTyreSet, curr->strategyTyreSet);
	NUM_2_OBJ_CMP(obj, "fuel", prev, prev->pitStopFuel, curr->pitStopFuel);

	cJSON* pressure = createPressure(curr, prev);

	if (!pressure) {
		RET_NULL(obj);
	}

	if (cJSON_GetArraySize(pressure) > 0) {
		if (!cJSON_AddItemToObject(obj, "pressure", pressure)) {
			cJSON_Delete(obj);
			cJSON_Delete(pressure);

			return NULL;
		}
	}

	return obj;
}

/**
 * type, duration.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createPenalty(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "type", prev, prev->penalty, curr->penalty);
	NUM_2_OBJ_CMP(obj, "duration", prev, prev->penaltyTime, curr->penaltyTime);

	return obj;
}

/**
 * totalRemaining, stintRemaining.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createDrivingTime(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "totalRemaining", prev, prev->totalTimeLeft, curr->totalTimeLeft);
	NUM_2_OBJ_CMP(obj, "stintRemaining", prev, prev->stintTimeLeft, curr->stintTimeLeft);

	return obj;
}

/**
 * used, rate.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createFuel(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "used", prev, prev->fuelUsed, curr->fuelUsed);
	NUM_2_OBJ_CMP(obj, "rate", prev, prev->fuelPerLap, curr->fuelPerLap);

	return obj;
}

static cJSON* createYellow(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	BOOL_2_OBJ_CMP(obj, "global", prev, prev->globalYellow, curr->globalYellow);
	BOOL_2_OBJ_CMP(obj, "sector1", prev, prev->yellow1, curr->yellow1);
	BOOL_2_OBJ_CMP(obj, "sector2", prev, prev->yellow2, curr->yellow2);
	BOOL_2_OBJ_CMP(obj, "sector3", prev, prev->yellow3, curr->yellow3);

	return obj;
}

/**
 * current, yellow, yellow.global, yellow.sector1, yellow.sector2, yellow.sector3, white,
 * green, chequered, red.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
static cJSON* createFlag(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "current", prev, prev->flag, curr->flag);
	BOOL_2_OBJ_CMP(obj, "green", prev, prev->globalGreen, curr->globalGreen);
	BOOL_2_OBJ_CMP(obj, "chequered", prev, prev->chequered, curr->chequered);
	BOOL_2_OBJ_CMP(obj, "red", prev, prev->globalRed, curr->globalRed);
	BOOL_2_OBJ_CMP(obj, "white", prev, prev->globalWhite, curr->globalWhite);

	cJSON* yellow = createYellow(curr, prev);

	if (!yellow) {
		RET_NULL(obj);
	}

	if (cJSON_GetArraySize(yellow) > 0) {
		if (!cJSON_AddItemToObject(obj, "yellow", yellow)) {
			cJSON_Delete(yellow);
			cJSON_Delete(obj);

			return NULL;
		}
	}

	return obj;
}

// remember to update when adding additional sub-objects
#define HUD_ITEM_COUNT 9

static const struct item items[HUD_ITEM_COUNT] = {
	{"laptimes", &createLaptimes},
	{"electronics", &createElectronics},
	{"session", &createSession},
	{"conditions", &createConditions},
	{"pitstop", &createPitstop},
	{"penalty", &createPenalty},
	{"drivingTime", &createDrivingTime},
	{"fuel", &createFuel},
	{"flag", &createFlag}
};

/**
 * Adds the sub-objects above along with: trackStatus, position, distanceTraveled, gameStatus,
 * laps, isBoxed, isInPitLane, mandatoryPitDone, rainTyres, tyreCompound.
 *
 * @param  curr Current frame HUD data.
 * @param  prev Previous frame HUD data.
 */
cJSON* hudToJSON(const HUD* curr, const HUD* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	// now for some windows-flavoured string fun
	// compare if a previous frame was provided then convert to a multi-byte char*
	if (!prev || wcscmp(prev->trackStatus, curr->trackStatus) != 0) {
		if (!addWstrToObject(obj, "trackStatus", curr->trackStatus)) {
			RET_NULL(obj);
		}
	}

	// would this ever change?
	if (!prev || wcscmp(prev->tyreCompound, curr->tyreCompound) != 0) {
		if (!addWstrToObject(obj, "tyreCompound", curr->tyreCompound)) {
			RET_NULL(obj);
		}
	}

	NUM_2_OBJ_CMP(obj, "position", prev, prev->position, curr->position);
	NUM_2_OBJ_CMP(obj, "distanceTraveled", prev, prev->distanceTraveled, curr->distanceTraveled);
	NUM_2_OBJ_CMP(obj, "gameStatus", prev, prev->status, curr->status);
	NUM_2_OBJ_CMP(obj, "laps", prev, prev->completedLaps, curr->completedLaps);
	NUM_2_OBJ_CMP(obj, "tyreSet", prev, prev->currTyreSet, curr->currTyreSet);

	BOOL_2_OBJ_CMP(obj, "isBoxed", prev, prev->isBoxed, curr->isBoxed);
	BOOL_2_OBJ_CMP(obj, "isInPitLane", prev, prev->isInPitLane, curr->isInPitLane);
	BOOL_2_OBJ_CMP(obj, "mandatoryPitDone", prev, prev->mandatoryPitDone, curr->mandatoryPitDone);
	BOOL_2_OBJ_CMP(obj, "rainTyres", prev, prev->rainTyres, curr->rainTyres);

	// add sub-objects
	for (int i = 0; i < HUD_ITEM_COUNT; i++) {
		cJSON* ptr = items[i].create(curr, prev);

		if (!ptr) {
			RET_NULL(obj);
		}

		// only add ptr to the object if it has at least one sub key
		if (cJSON_GetArraySize(ptr) > 0) {
			if (!cJSON_AddItemToObject(obj, items[i].key, ptr)) {
				cJSON_Delete(ptr);
				cJSON_Delete(obj);

				return NULL;
			}
		}
	}

	return obj;
}

/**
 * Get a status as a wchar_t*.
 * @param  s
 * @return   No need to free.
 */
const wchar_t* wstrStatus(Status s) {
	switch (s) {
	case STATUS_OFF:
		return L"Not in session";
	case STATUS_REPLAY:
		return L"Watching replay";
	case STATUS_LIVE:
		return L"In car";
	case STATUS_PAUSE:
		return L"Paused";
	}

	return L"Unknown";
}
