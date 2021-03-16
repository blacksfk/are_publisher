#ifndef HUD_H
#define HUD_H

#include "auxiliary.h"
#include "enums.h"

/**
 * HUD elements. Updated once per frame.
 * Player car only with a couple of exceptions listed below.
 */
typedef struct hud {
	// current step index
	int packetId;

	// game status
	Status status;

	// current session
	SessionType session;

	// lap times as a widechar string
	wchar_t strCurrentTime[15];
	wchar_t strLastTime[15];
	wchar_t strBestTime[15];
	wchar_t strSplit[15];

	// no. of completed laps
	int completedLaps;

	// current position
	int position;

	// lap times expressed in milliseconds
	int currentTime;
	int lastTime;
	int bestTime;

	// time remaining in the current session
	float sessionTimeLeft;

	// distance traveled in the current stint
	float distanceTraveled;

	// whether or not the car is in its pit box
	int isBoxed;

	// current track sector
	int currentSectorIndex;

	// last sector in milliseconds
	int lastSectorTime;

	// duplicate of completedLaps
	int numberOfLaps;

	// tyre compound (I imagine this would be DHD2 or DHE etc.)
	wchar_t tyreCompound[33];

	// not used in ACC
	float replayTimeMultiplier;

	// car pos. on track spline (range: 0 to 1.0)
	float normalizedCarPosition;

	// no. of cars on track (or in the server probably...)
	int activeCars;

	// co-ordinates of the cars on track
	float carCoordinates[60][3];

	// car IDs of the cars on track
	int carID[60];

	// current car ID
	int playerCarID;

	// current penalty length
	float penaltyTime;

	// flag type being waved
	FlagType flag;

	// penalty type received
	PenaltyType penalty;

	// whether or not the ideal line assist is on
	int idealLineOn;

	// whether or not the car is on pit lane
	int isInPitLane;

	// not used in ACC (always 0 apparently)
	float surfaceGrip;

	// whether or not all mandatory pit stops have been completed
	int mandatoryPitDone;

	// wind speed in m/s
	float windSpeed;

	// wind direction in radians according to the car yaw angle
	float windDirection;

	// whether or not the current driver is working on the setup
	int isSetupMenuVisible;

	// dash page
	int mainDisplayIndex;

	// race logic page
	int secondaryDisplayIndex;

	// traction control level
	int tc;

	// traction control cut level
	int tcCut;

	// engine map
	int engineMap;

	// ABS level
	int abs;

	// litres of fuel being consumed per lap
	int fuelPerLap;

	// whether or not the rain light is on
	int rainLight;

	// whether or not the headlights are being flashed
	int flashingLights;

	// headlight level (driving, on, endurance)
	int lights;

	// exhaust temperature
	float exhaustTemperature;

	// windscreen wiper level
	int wiperLevel;

	// current driver's total time remaining in milliseconds
	int totalTimeLeft;

	// current driver's stint time remaining in milliseconds
	int stintTimeLeft;

	// whether or not wet weather tyres are being used
	int rainTyres;

	// session index for the weekend
	int sessionIndex;

	// fuel consumption since last re-fuelling
	float fuelUsed;

	// lap time delta expressed as a widechar string
	wchar_t strDelta[15];

	// lap time delta expressed in milliseconds
	int delta;

	// estimated lap time expressed as a widechar string
	wchar_t strEstimatedLap[15];

	// estimated lap time expressed in milliseconds
	int estimatedLapTime;

	// whether or not the delta is positive
	int isDeltaPositive;

	// last split time expressed in milliseconds (possible duplicate of lastSectorTime)
	int lastSplit;

	// whether or not the current lap is valid
	int isValidLap;

	// predicted laps remaining based on the fuel level
	float estimatedLapsRemaining;

	// track status ("Green", "Fast", "Optimum", "Damp", "Wet")
	wchar_t trackStatus[33];

	// remaining mandatory pitstops
	int remainingMandatoryPitstops;

	// time of the day expressed in seconds
	float clock;

	// whether or not the indicator(s) are/is on
	int leftIndicator;
	int rightIndicator;

	// whether or not a yellow flag is being waved
	int globalYellow;

	// whether or not a yellow flag is being waved in the appropriate sector
	int yellow1;
	int yellow2;
	int yellow3;

	// whether or not a white flag is being waved
	int globalWhite;

	// whether or not a green flag is being waved
	int globalGreen;

	// whether or not the chequered flag is being waved
	int chequered;

	// whether or not the red flag is being waved
	int globalRed;

	// selected tyre set in the pitstop MFD page
	int pitStopTyreSet;

	// amount of fuel being added at the next stop in litres
	float pitStopFuel;

	// tyre pressure for the next tyre set
	float pitStopFL;
	float pitStopFR;
	float pitStopRL;
	float pitStopRR;

	// track grip
	TrackGrip trackGrip;

	// rain intensity levels
	RainIntensity rainIntensityCurr;
	RainIntensity rainIntensity10;
	RainIntensity rainIntensity30;
} HUD;

cJSON* hudToJSON(HUD*, HUD*);

#endif
