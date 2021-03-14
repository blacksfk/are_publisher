#ifndef PROPERTIES_H
#define PROPERTIES_H

/**
 * Unchanging data that is only set upon server join.
 */
typedef struct properties {
	// shared memory version
	wchar_t sharedMemVer[15];

	// ACC version
	wchar_t accVer[15];

	// no. of sessions for the weekend
	int numSessions;

	// no. of cars
	int numCars;

	// car model (TODO: create enum)
	wchar_t carModel[33];

	// track name
	wchar_t track[33];

	// driver's first name
	wchar_t firstName[33];

	// driver's last name
	wchar_t lastName[33];

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
	float aidFuelRate;
	float aidTyreRate;
	float aidMechanicalDamage;
	float aidAllowTyreBlankets;
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

#endif
