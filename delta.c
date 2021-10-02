#include "delta.h"

struct carOffset {
	wchar_t* id;
	int offset;
};

// tie car IDs to their brake bias offset
struct carOffset carOffsets[] = {
	{L"amr_v12_vantage_gt3", -7},
	{L"audi_r8_lms", -14},
	{L"bentley_continental_gt3_2016", -7},
	{L"bentley_continental_gt3_2018", -7},
	{L"bmw_m6_gt3", -15},
	{L"jaguar_g3", -7},
	{L"ferrari_488_gt3", -17},
	{L"honda_nsx_gt3", -14},
	{L"lamborghini_gallardo_rex", -14},
	{L"lamborghini_huracan_gt3", -14},
	{L"lamborghini_huracan_st", -14},
	{L"lexus_rc_f_gt3", -14},
	{L"mclaren_650s_gt3", -17},
	{L"mercedes_amg_gt3", -14},
	{L"nissan_gt_r_gt3_2017", -15},
	{L"nissan_gt_r_gt3_2018", -15},
	{L"porsche_991_gt3_r", -21},
	{L"porsche_991ii_gt3_cup", -5},
	{L"amr_v8_vantage_gt3", -7},
	{L"audi_r8_lms_evo", -14},
	{L"honda_nsx_gt3_evo", -14},
	{L"lamborghini_huracan_gt3_evo", -14},
	{L"mclaren_720s_gt3", -17},
	{L"porsche_991ii_gt3_r", -21},
	{L"alpine_a110_gt4", -15},
	{L"amr_v8_vantage_gt4", -20},
	{L"audi_r8_gt4", -15},
	{L"bmw_m4_gt4", -22},
	{L"chevrolet_camaro_gt4r", -18},
	{L"ginetta_g55_gt4", -18},
	{L"ktm_xbow_gt4", -20},
	{L"maserati_mc_gt4", -15},
	{L"mclaren_570s_gt4", -9},
	{L"mercedes_amg_gt4", -20},
	{L"porsche_718_cayman_gt4_mr", -20},
	{L"ferrari_488_gt3_evo", -17},
	{L"mercedes_amg_gt3_evo", -14}
};

// length of the above array
size_t carOffsetsLen = sizeof(carOffsets) / sizeof(struct carOffset);

/**
 * Calculate and insert the brake bias of the current car.
 * @param  parent
 * @param  sm
 * @param  complete Set to true to forgo comparisons to previous values.
 */
static cJSON* brakeBias(cJSON* parent, SharedMem* sm, bool complete) {
	float bias = truncf(sm->curr.physics->brakeBias * 1000);
	
	if (complete || truncf(sm->curr.physics->brakeBias) != bias) {
		cJSON* brakes = NULL;

		// check if the parent object already has an item under "brakes"
		// if it does; retrieve it, otherwise create a new object
		// and add it
		if (cJSON_HasObjectItem(parent, "brakes")) {
			brakes = cJSON_GetObjectItemCaseSensitive(parent, "brakes");
		} else {
			brakes = cJSON_CreateObject();

			if (!cJSON_AddItemToObject(parent, "brakes", brakes)) {
				// out of memory
				cJSON_Delete(parent);
				cJSON_Delete(brakes);

				return NULL;
			}
		}

		// find the car model and offset the bias
		for (size_t i = 0; i < carOffsetsLen; i++) {
			if (wcscmp(carOffsets[i].id, sm->curr.props->carModel) == 0) {
				// add the (usually negative) offset
				bias += carOffsets[i].offset;
			}
		}

		// add the bias as a raw value
		char raw[JSON_RAW_FLOAT_WIDTH];

		// format: xy.z
		snprintf(raw, JSON_RAW_FLOAT_WIDTH, "%.2f", bias / 10);

		// add to the object
		if (!cJSON_AddRawToObject(brakes, "bias", raw)) {
			// out of memory
			cJSON_Delete(parent);
			cJSON_Delete(brakes);

			return NULL;
		}
	}

	return parent;
}

/**
 * Adds the key "newSession" with the value set to true if the session has changed.
 * @param  parent
 * @param  sm
 */
static cJSON* newSession(cJSON* parent, SharedMem* sm) {
	struct memMaps prev = sm->prev;
	struct memMaps curr = sm->curr;

	if (prev.hud->sessionIndex != curr.hud->sessionIndex ||
		// track has changed
		wcscmp(prev.props->track, curr.props->track) != 0 ||
		// car has changed
		wcscmp(prev.props->carModel, curr.props->carModel) != 0) {
		BOOL_2_OBJ(parent, "newSession", true);
	}

	return parent;
}

/**
 * Add the previous sector time once the sector has been completed.
 * @param  parent
 * @param  sm
 * @param  t
 * @param  complete
 */
static cJSON* prevSector(cJSON* parent, SharedMem* sm, Tracked* t) {
	// only add the sector time if the indices differ
	if (sm->prev.hud->currSectorIndex >= 0 && (sm->curr.hud->currSectorIndex != sm->prev.hud->currSectorIndex)) {
		cJSON* laptimes = NULL;

		// check if the parent object already has an item under "laptimes"
		// if it does; retrieve it, otherwise create a new object
		// and add it to the parent
		if (cJSON_HasObjectItem(parent, "laptimes")) {
			laptimes = cJSON_GetObjectItemCaseSensitive(parent, "laptimes");
		} else {
			laptimes = cJSON_CreateObject();

			if (!cJSON_AddItemToObject(parent, "laptimes", laptimes)) {
				cJSON_Delete(laptimes);
				cJSON_Delete(parent);

				return NULL;
			}
		}
		
		int prevSector = 0;

		if (sm->curr.hud->completedLaps > sm->prev.hud->completedLaps) {
			// new lap started
			prevSector = addSector(t, sm->prev.hud->currSectorIndex, sm->curr.hud->prevLapTime);
			resetSectors(t);
		} else {
			// same lap, new sector
			prevSector = addSector(t, sm->prev.hud->currSectorIndex, sm->curr.hud->cumulativeSectorTime);
		}

		INT_2_OBJ(laptimes, "prevSector", prevSector);
	}

	return parent;
}

/**
 * Create a delta JSON string from data in shared memory.
 * @param  t
 * @param  sm
 * @param  complete Set to true to ignore the previous data (if any).
 */
char* deltaJSON(SharedMem* sm, Tracked* t, bool complete) {
	struct memMaps curr = sm->curr;
	struct memMaps prev = sm->prev;
	cJSON* parent = cJSON_CreateObject();

	if (!parent) {
		return NULL;
	}

	if (complete) {
		// don't do any comparisons
		hudToJSON(parent, curr.hud, NULL);
		physicsToJSON(parent, curr.physics, NULL);
		parent = propertiesToJSON(parent, curr.props);
	} else {
		// compare with the previous sample
		hudToJSON(parent, curr.hud, prev.hud);
		parent = physicsToJSON(parent, curr.physics, prev.physics);
	}

	if (!parent) {
		return NULL;
	}

	// custom parameters requiring additional information
	brakeBias(parent, sm, complete);
	newSession(parent, sm);
	parent = prevSector(parent, sm, t);

	if (!parent) {
		return NULL;
	}

	char* str = cJSON_PrintBuffered(parent, JSON_BUF_SIZE, 0);

	// raw objects no longer required
	cJSON_Delete(parent);

	return str;
}
