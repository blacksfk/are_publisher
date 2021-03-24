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

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->brakePressure[0], curr->brakePressure[0]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->brakePressure[1], curr->brakePressure[1]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->brakePressure[2], curr->brakePressure[2]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->brakePressure[3], curr->brakePressure[3]);

	return obj;
}

static cJSON* createPadWear(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->padWear[0], curr->padWear[0]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->padWear[1], curr->padWear[1]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->padWear[2], curr->padWear[2]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->padWear[3], curr->padWear[3]);

	return obj;
}

static cJSON* createDiscWear(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->discWear[0], curr->discWear[0]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->discWear[1], curr->discWear[1]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->discWear[2], curr->discWear[2]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->discWear[3], curr->discWear[3]);

	return obj;
}

static cJSON* createBrakeTemp(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->brakeTemp[0], curr->brakeTemp[0]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->brakeTemp[1], curr->brakeTemp[1]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->brakeTemp[2], curr->brakeTemp[2]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->brakeTemp[3], curr->brakeTemp[3]);

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

		if (!cJSON_AddItemToObject(obj, brakeItems[i].key, ptr)) {
			cJSON_Delete(obj);
			cJSON_Delete(ptr);

			return NULL;
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

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->tyrePressure[0], curr->tyrePressure[0]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->tyrePressure[1], curr->tyrePressure[1]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->tyrePressure[2], curr->tyrePressure[2]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->tyrePressure[3], curr->tyrePressure[3]);

	return obj;
}

static cJSON* createTyreTemp(const Physics* curr, const Physics* prev) {
	cJSON* obj = cJSON_CreateObject();

	if (!obj) {
		return NULL;
	}

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->tyreCoreTemp[0], curr->tyreCoreTemp[0]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->tyreCoreTemp[1], curr->tyreCoreTemp[1]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->tyreCoreTemp[2], curr->tyreCoreTemp[2]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->tyreCoreTemp[3], curr->tyreCoreTemp[3]);

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

		if (!cJSON_AddItemToObject(obj, tyreItems[i].key, ptr)) {
			cJSON_Delete(obj);
			cJSON_Delete(ptr);

			return NULL;
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

	NUM_2_OBJ_CMP(obj, "front", prev, prev->carDamage[0], curr->carDamage[0]);
	NUM_2_OBJ_CMP(obj, "rear", prev, prev->carDamage[1], curr->carDamage[1]);
	NUM_2_OBJ_CMP(obj, "left", prev, prev->carDamage[2], curr->carDamage[2]);
	NUM_2_OBJ_CMP(obj, "right", prev, prev->carDamage[3], curr->carDamage[3]);
	NUM_2_OBJ_CMP(obj, "centre", prev, prev->carDamage[4], curr->carDamage[4]);

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

	NUM_2_OBJ_CMP(obj, "fl", prev, prev->suspensionTravel[0], curr->suspensionTravel[0]);
	NUM_2_OBJ_CMP(obj, "fr", prev, prev->suspensionTravel[1], curr->suspensionTravel[1]);
	NUM_2_OBJ_CMP(obj, "rl", prev, prev->suspensionTravel[2], curr->suspensionTravel[2]);
	NUM_2_OBJ_CMP(obj, "rr", prev, prev->suspensionTravel[3], curr->suspensionTravel[3]);

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

		if (!cJSON_AddItemToObject(physics, items[i].key, obj)) {
			cJSON_Delete(physics);
			cJSON_Delete(obj);

			return NULL;
		}
	}

	return physics;
}
