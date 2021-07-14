#ifndef PHYSICS_H
#define PHYSICS_H

#include "auxiliary.h"

// Car damage position enum.
enum carDamage {
	DMG_F = 0,
	DMG_B,
	DMG_L,
	DMG_R,
	DMG_C
};

// Wheel type enumeration.
enum wheelType {
	W_FL = 0,
	W_FR,
	W_RL,
	W_RR
};

/**
 * Physics information. Updated once per frame. Player car only.
 */
typedef struct physics {
	// current step index
	int packetId;

	// accelerator input (range: 0 to 1.0)
	float accelerator;

	// brake input (range: 0 to 1.0)
	float brake;

	// fuel remaining in litres
	float fuelRemaining;

	// current gear
	// 0: reverse
	// 1: neutral
	// 2..n: 1st .. nth
	int gear;

	// current engine RPM
	int rpm;

	// steering angle (range: -1.0 to 1.0)
	float steering;

	// current speed (in km/h)
	float speed;

	// velocity in terms of global co-ordinates
	float velocityVector[3];

	// accleration in terms of global co-ordiantes
	float accelerationVector[3];

	// slip for each tyre (FL, FR, RL, RR)
	float wheelSlip[4];

	// not used in ACC
	float wheelLoad[4];

	// tyre pressure for each tyre (FL, FR, RL, RR)
	float tyrePressure[4];

	// wheel angular speed expressed in radians/s (FL, FR, RL, RR)
	float wheelAngularSpeed[4];

	// not used in ACC
	float tyreWear[4];
	float tyreDirtyLevel[4];

	// tyre core temperature (FL, FR, RL, RR)
	float tyreCoreTemp[4];

	// not used in ACC
	float camberRAD[4];

	// suspension travel (FL, FR, RL, RR)
	float suspensionTravel[4];

	// not used in ACC
	float drs;

	// traction control intervention (0 .. 1)
	float tcIntervention;

	// yaw angle
	float yaw;

	// pitch angle
	float pitch;

	// roll angle
	float roll;

	// not used in ACC
	float cgHeight;

	// damage amount (front, rear, left, right, centre)
	float carDamage[5];

	// not used in ACC
	int numberOfTyresOut;

	// whether the pit limiter is enabled or not
	int pitLimiter;

	// ABS intervention (0 .. 1)
	float absIntervention;

	// not used in ACC
	float kersCharge;
	float kersInput;

	// whether or not automatic transmission is being used
	int autoShifterOn;

	// not used in ACC
	float rideHeight[2];

	// boost pressure
	float boostPressure;

	// not used in ACC
	float ballast;
	float airDensity;

	// air temperature
	float ambientTemp;

	// track temperature
	float trackTemp;

	// angular velocity vector in local co-ordiantes
	float localAngularVel[3];

	// force feedback signal
	float finalFF;

	// not used in ACC
	float performanceMeter;
	int engineBrake;
	int ersRecoveryLevel;
	int ersPowerLevel;
	int ersHeatCharging;
	int ersIsCharging;
	float kersCurrentKJ;
	int drsAvailable;
	int drsEnabled;

	// brake disc temperature (FL, FR, RL, RR)
	float brakeTemp[4];

	// clutch input (range: 0 to 1.0)
	// doesn't appear to work
	float clutch;

	// not used in ACC
	float tyreTempI[4];
	float tyreTempM[4];
	float tyreTempO[4];

	// whether or not the car is being controller by the AI
	int isAIControlled;

	// tyre contact point co-ordiantes (FL, FR, RL, RR)
	float tyreContactPoint[4][3];

	// tyre contact normal
	float tyreContactNormal[4][3];

	// tyre contact heading
	float tyreContactHeading[4][3];

	// forward brake bias
	float brakeBias;

	// velocity vector in local co-ordinates
	float localVelocity[3];

	// not used in ACC
	int P2PActivations;
	int P2PStatus;
	float currentMaxRpm;
	float mz[4];
	float fx[4];
	float fy[4];

	// tyre slip ratio in radians (FL, FR, RL, RR)
	float slipRatio[4];

	// tyre slip angle (FL, FR, RL, RR)
	float slipAngle[4];

	// not used in ACC
	int tcinAction;
	int absInAction;
	float suspensionDamage[4];
	float tyreTemp[4];

	// water temperature
	float waterTemp;

	// brake pressure (FL, FR, RL, RR)
	float brakePressure[4];

	// brake pad compound
	// 0: pad 1
	// n: pad (n + 1)
	int frontBrakeCompound;
	int rearBrakeCompound;

	// brake pad wear (FL, FR, RL, RR)
	float padWear[4];

	// brake rotor wear (FL, FR, RL, RR)
	float discWear[4];

	// whether or not the ignition is on
	int ignitionOn;

	// whether or not the starter motor is firing
	int starterMotorOn;

	// whether or not the engine is running
	int engineRunning;

	// force feedback signals
	float kerbVibration;
	float slipVibration;
	float forceVibration;
	float absVibration;
} Physics;

cJSON* physicsToJSON(const Physics*, const Physics*);
bool physicsIsInCar(const Physics*);

#ifdef DEBUG
void printPhysics(const Physics*, FILE*);
#endif

#endif
