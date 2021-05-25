#include "physics.h"

struct item {
	char* key;
	cJSON* (*create)(const Physics*, const Physics*);
};

/**
 * accelerator, brake, clutch, steeringAngle, pitLimiter.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createInput(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "accelerator", prev, prev->accelerator, curr->accelerator);
	NUM_2_OBJ_CMP(obj, "brake", prev, prev->brake, curr->brake);
	NUM_2_OBJ_CMP(obj, "clutch", prev, prev->clutch, curr->clutch);
	NUM_2_OBJ_CMP(obj, "steeringAngle", prev, prev->steeringAngle, curr->steeringAngle);
	BOOL_2_OBJ_CMP(obj, "pitLimiter", prev, prev->pitLimiter, curr->pitLimiter);

	return obj;
}

static cJSON* createBrakePressure(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->brakePressure[W_FL], curr->brakePressure[W_FL]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->brakePressure[W_FR], curr->brakePressure[W_FR]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->brakePressure[W_RL], curr->brakePressure[W_RL]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->brakePressure[W_RR], curr->brakePressure[W_RR]);

	return obj;
}

static cJSON* createPadWear(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->padWear[W_FL], curr->padWear[W_FL]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->padWear[W_FR], curr->padWear[W_FR]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->padWear[W_RL], curr->padWear[W_RL]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->padWear[W_RR], curr->padWear[W_RR]);

	return obj;
}

static cJSON* createDiscWear(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->discWear[W_FL], curr->discWear[W_FL]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->discWear[W_FR], curr->discWear[W_FR]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->discWear[W_RL], curr->discWear[W_RL]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->discWear[W_RR], curr->discWear[W_RR]);

	return obj;
}

static cJSON* createBrakeTemp(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->brakeTemp[W_FL], curr->brakeTemp[W_FL]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->brakeTemp[W_FR], curr->brakeTemp[W_FR]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->brakeTemp[W_RL], curr->brakeTemp[W_RL]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->brakeTemp[W_RR], curr->brakeTemp[W_RR]);

	return obj;
}

static cJSON* createBrakeCompound(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "front", prev, prev->frontBrakeCompound, curr->frontBrakeCompound);
	NUM_2_OBJ_CMP(obj, "rear", prev, prev->rearBrakeCompound, curr->rearBrakeCompound);

	return obj;
}

#define PHYSICS_BRAKE_ITEM_COUNT 5

struct item brakeItems[PHYSICS_BRAKE_ITEM_COUNT] = {
	{"pressure", &createBrakePressure},
	{"compound", &createBrakeCompound},
	{"padWear", &createPadWear},
	{"discWear", &createDiscWear},
	{"temp", &createBrakeTemp}
};

/**
 * bias, pressure, pressure.fl, pressure.fr, pressure.rl, pressure.rr,
 * compound, compound.front, compound.rear,
 * padWear, padWear.fl, padWear.fr, padWear.rl, padWear.rr,
 * discWear, discWear.fl, discWear.fr, discWear.rl, discWear.rr,
 * temp, temp.fl, temp.fr, temp.rl, temp.rr.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createBrakes(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "bias", prev, prev->brakeBias, curr->brakeBias);

	for (int i = 0; i < PHYSICS_BRAKE_ITEM_COUNT; i++) {
		cJSON* ptr = brakeItems[i].create(curr, prev);

		if (!ptr) {
			RET_NULL(obj);
		}

		if (cJSON_GetArraySize(ptr) > 0) {
			if (!cJSON_AddItemToObject(obj, brakeItems[i].key, ptr)) {
				cJSON_Delete(obj);
				cJSON_Delete(ptr);

				return NULL;
			}
		}
	}

	return obj;
}

/**
 * ambient, track.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createTemperature(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "ambient", prev, prev->ambientTemp, curr->ambientTemp);
	NUM_2_OBJ_CMP(obj, "track", prev, prev->trackTemp, curr->trackTemp);

	return obj;
}

/**
 * waterTemp, running, starter, igntion, rpm, boostPressure.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createMotor(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "waterTemp", prev, prev->waterTemp, curr->waterTemp);
	NUM_2_OBJ_CMP(obj, "rpm", prev, prev->rpm, curr->rpm);
	NUM_2_OBJ_CMP(obj, "boostPressure", prev, prev->boostPressure, curr->boostPressure);

	BOOL_2_OBJ_CMP(obj, "running", prev, prev->engineRunning, curr->engineRunning);
	BOOL_2_OBJ_CMP(obj, "starter", prev, prev->starterMotorOn, curr->starterMotorOn);
	BOOL_2_OBJ_CMP(obj, "ignition", prev, prev->ignitionOn, curr->ignitionOn);

	return obj;
}

static cJSON* createTyrePressure(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->tyrePressure[W_FL], curr->tyrePressure[W_FL]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->tyrePressure[W_FR], curr->tyrePressure[W_FR]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->tyrePressure[W_RL], curr->tyrePressure[W_RL]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->tyrePressure[W_RR], curr->tyrePressure[W_RR]);

	return obj;
}

static cJSON* createTyreTemp(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->tyreCoreTemp[W_FL], curr->tyreCoreTemp[W_FL]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->tyreCoreTemp[W_FR], curr->tyreCoreTemp[W_FR]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->tyreCoreTemp[W_RL], curr->tyreCoreTemp[W_RL]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->tyreCoreTemp[W_RR], curr->tyreCoreTemp[W_RR]);

	return obj;
}

#define PHYSICS_TYRE_ITEM_COUNT 2

struct item tyreItems[PHYSICS_TYRE_ITEM_COUNT] = {
	{"pressure", &createTyrePressure},
	{"temp", &createTyreTemp}
};

/**
 * pressure, pressure.fl, pressure.fr, pressure.rl, pressure.rr,
 * temp, temp.fl, temp.fr, temp.rl, temp.rr.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createTyres(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	for (int i = 0; i < PHYSICS_TYRE_ITEM_COUNT; i++) {
		cJSON* ptr = tyreItems[i].create(curr, prev);

		if (!ptr) {
			RET_NULL(obj);
		}

		if (cJSON_GetArraySize(ptr) > 0) {
			if (!cJSON_AddItemToObject(obj, tyreItems[i].key, ptr)) {
				cJSON_Delete(obj);
				cJSON_Delete(ptr);

				return NULL;
			}
		}
	}

	return obj;
}

/**
 * pitch, yaw, roll.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createAngle(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "pitch", prev, prev->pitch, curr->pitch);
	NUM_2_OBJ_CMP(obj, "roll", prev, prev->roll, curr->roll);
	NUM_2_OBJ_CMP(obj, "yaw", prev, prev->yaw, curr->yaw);

	return obj;
}

/**
 * front, left, centre, right, rear.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createDamage(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "front", prev, prev->carDamage[DMG_F], curr->carDamage[DMG_F]);
	NUM_2_OBJ_CMP(obj, "rear", prev, prev->carDamage[DMG_B], curr->carDamage[DMG_B]);
	NUM_2_OBJ_CMP(obj, "left", prev, prev->carDamage[DMG_L], curr->carDamage[DMG_L]);
	NUM_2_OBJ_CMP(obj, "right", prev, prev->carDamage[DMG_R], curr->carDamage[DMG_R]);
	NUM_2_OBJ_CMP(obj, "centre", prev, prev->carDamage[DMG_C], curr->carDamage[DMG_C]);

	return obj;
}

/**
 * fl, fr, rl, rr.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
static cJSON* createSuspensionTravel(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->suspensionTravel[W_FL], curr->suspensionTravel[W_FL]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->suspensionTravel[W_FR], curr->suspensionTravel[W_FR]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->suspensionTravel[W_RL], curr->suspensionTravel[W_RL]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->suspensionTravel[W_RR], curr->suspensionTravel[W_RR]);

	return obj;
}

#define PHYSICS_ITEM_COUNT 8

static const struct item items[PHYSICS_ITEM_COUNT] = {
	{"input", &createInput},
	{"brakes", &createBrakes},
	{"temp", &createTemperature},
	{"motor", &createMotor},
	{"tyres", &createTyres},
	{"angle", &createAngle},
	{"damage", &createDamage},
	{"suspensionTravel", &createSuspensionTravel}
};

/**
 * Adds the sub-objects above along with: speed, gear, tc, abs.
 * If prev is not null, it is compared against to determine whether
 * the parameter should be included in the JSON object or not. Identical
 * values between frames will result in the key and its value being excluded
 * from the JSON object in an effort to save bandwidth.
 *
 * @param curr Current frame Physics data.
 * @param prev Previous frame Physics data.
 */
cJSON* physicsToJSON(const Physics* curr, const Physics* prev) {
	cJSON* physics = cJSON_CreateObject();

	if (!physics) {
		return NULL;
	}

	NUM_2_OBJ_CMP(physics, "speed", prev, prev->speed, curr->speed);
	NUM_2_OBJ_CMP(physics, "gear", prev, prev->gear, curr->gear);
	NUM_2_OBJ_CMP(physics, "tc", prev, prev->tc, curr->tc);
	NUM_2_OBJ_CMP(physics, "abs", prev, prev->abs, curr->abs);

	// sub-objects
	for (int i = 0; i < PHYSICS_ITEM_COUNT; i++) {
		cJSON* obj = items[i].create(curr, prev);

		if (!obj) {
			RET_NULL(physics);
		}

		if (cJSON_GetArraySize(obj) > 0) {
			if (!cJSON_AddItemToObject(physics, items[i].key, obj)) {
				cJSON_Delete(physics);
				cJSON_Delete(obj);

				return NULL;
			}
		}
	}

	return physics;
}
