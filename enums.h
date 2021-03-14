#ifndef ENUMS_H
#define ENUMS_H

// Flag type enumeration.
typedef enum flagType {
	F_NONE = 0,
	F_BLUE,
	F_YELLOW,
	F_BLACK,
	F_WHITE,
	F_CHEQUERED,
	F_PENALTY,
	F_GREEN,
	F_ORANGE
} FlagType;

// Penalty type enumeration.
typedef enum penaltyType {
	P_NONE = 0,

	// track cutting penalties
	P_CUTTING_DT,
	P_CUTTING_SG_10,
	P_CUTTING_SG_20,
	P_CUTTING_SG_30,
	P_CUTTING_DSQ,
	P_CUTTING_REMOVE_BEST_LAP,

	// pit lane speeding penalties
	P_SPEEDING_DT,
	P_SPEEDING_SG_10,
	P_SPEEDING_SG_20,
	P_SPEEDING_SG_30,
	P_SPEEDING_DSQ,
	P_SPEEDING_REMOVE_BEST_LAP,

	P_MANDATORY_PIT_DSQ,
	P_POST_RACE_TIME,
	
	P_DSQ_TROLLING,
	P_DSQ_PIT_ENTRY,
	P_DSQ_PIT_EXIT,
	P_DSQ_WRONG_WAY,

	P_DT_IGNORED_DRIVER_STINT,
	P_DSQ_IGNORED_DRIVER_STINT,
	P_DSQ_EXCEEDED_DRVIER_STINT
} PenaltyType;

// Session type enumeration.
typedef enum sessionType {
	ST_UNKNOWN = -1,
	ST_PRACTICE = 0,
	ST_QUALIFY,
	ST_RACE,
	ST_HOTLAP,
	ST_TIMEATTACK,
	ST_DRIFT,
	ST_DRAG,
	ST_HOTSTINT,
	ST_SUPERPOLE
} SessionType;

// Game status enumeration.
typedef enum status {
	STATUS_OFF = 0,
	STATUS_REPLAY,
	STATUS_LIVE,
	STATUS_PAUSE
} Status;

// Wheel type enumeration.
typedef enum wheelType {
	W_FL = 0,
	W_FR,
	W_RL,
	W_RR
} WheelType;

// Track grip level enumeration.
typedef enum trackGrip {
	TG_GREEN = 0,
	TG_FAST,
	TG_OPTIMUM,
	TG_GREASY,
	TG_DAMP,
	TG_WET,
	TG_FLOODED
} TrackGrip;

// Rain intensity enumeration.
typedef enum rainIntensity {
	R_NONE = 0,
	R_DRIZZLE,
	R_LIGHT,
	R_MEDIUM,
	R_HEAVY,
	R_THUNDERSTORM
} RainIntensity;

#endif
