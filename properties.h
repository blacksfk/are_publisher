#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "auxiliary.h"

/**
 * Unchanging data that is only set upon server join.
 */
typedef struct properties {
	// shared memory version
	wchar_t sharedMemVer[15];

	// ACC version
	wchar_t accVer[15];

	// no. of sessions for the weekend
	int sessions;

	// no. of cars
	int cars;

	// car model
	wchar_t carModel[33];

	// track name
	wchar_t track[33];

	// driver's first name
	wchar_t firstname[33];

	// driver's last name
	wchar_t surname[33];

	// driver's nickname
	wchar_t nickname[33];

	// number of sectors on track
	int sectorCount;

	// not used in ACC
	float maxTorque;
	float maxPower;

	// maximum car RPM
	int maxRPM;

	// fuel tank capacity
	float tankCap;

	// not used in ACC
	float suspensionMaxTravel[4];
	float tyreRadius[4];
	float maxBoost;
	float deprecated_1;
	float deprecated_2;

	// weekend settings, consumption rates, and driver aids
	int penaltiesEnabled;
	float fuelRate;
	float tyreRate;
	float damageRate;
	float allowTyreBlankets;
	float aidStability;
	int aidAutoClutch;
	int aidAutoBlip;

	// not used in ACC
	int hasDRS;
	int hasERS;
	int hasKERS;
	float kersMaxJ;
	int engineBrakeSettingsCount;
	int ersPowerControllerCount;
	float trackSplineLength;
	wchar_t trackConfiguration;
	float ersMaxJ;
	int isTimedRace;
	int hasExtraLap;
	wchar_t carSkin[33];
	int reversedGridPositions;

	// pit window times (in seconds? milliseconds? nfi.)
	int pitWindowStart;
	int pitWindowEnd;

	// whether or not the current session is multiplayer
	int isMultiplayer;

	// tyre names (possibly DHD2 or DHE...)
	wchar_t dryTyreName[33];
	wchar_t wetTyreName[33];
} Properties;

cJSON* propertiesToJSON(const Properties*);
bool propertiesUpdated(const Properties* a, const Properties* b);

#endif
