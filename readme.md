# ACC Race Engineer publisher
Work like a real race engineer on a virtual car! This application simply reads from the ACC data stored in shared memory and sends that data to a server via an HTTP POST request perodically.

## Setup
1. In order to build this program, you require [CMake](https://cmake.org/download/) and the package manager [conan](https://conan.io/) along with Windows SDKs and libraries (via Visual Studio).
2. Once installed, open a terminal (eg. git bash) and create a build directory for the output files. Eg. `<path_to_project>/build`.
3. `cd build`
4. Install dependencies with: `conan install ..` (or the correct path to the directory containing conanfile.txt).
5. Generate the build configuration with: `cmake ..` (or the correct path to the directory containing CMakeLists.txt).

## Build time flags
* **DEBUG**: Creates a terminal window for diagnostics and error output.
* **DISABLE_BROADCAST**: Prevents the POST request from occurring.
* **RECORD_DATA**: Saves the JSON data to data.json in a JSON array.

## Compiling
MSVC defaults to building for a debug environment so building for production requires an extra flag. The executable will be available under the `bin` sub-directory in either environment mode.

### Development
`cmake --build .`

### Production
1. `cmake .. -D DEBUG=OFF -D DISABLE_BROADCAST=OFF -D RECORD_DATA=OFF`
2. `cmake --build . --config Release`

## Broadcast data structure
Below is the complete data structure with data types. The empty string `""` represents string values. `false` represents values which are booleans. `0` represents a value which will only ever be an integer, while `0.0` represents a value which is a float. Only values which have changed since the last sample will be present in the broadcast's body.

```javascript
let complete = {
	/**
	 * HUD parameters.
	 */
	// > 0
	position: 0,

	// distance covered in a session (in metres)
	distanceTraveled: 0.0,

	// no. of completed laps. i.e. current lap is `laps` + 1
	laps: 0,

	// tyre set bolted to the car.
	// 0: rain tyres
	// n: tyre set n
	tyreSet: 0,

	// whether or not the car is in its pit box
	isBoxed: false,

	// whether or not the car is in pit lane
	isInPitLane: false,

	// whether or not all of the mandatory pitstops have been completed
	mandatoryPitDone: false,

	// whether or not rain tyres are bolted to the car
	rainTyres: false,

	// lap and sector times
	laptimes: {
		// current lap time expressed in milliseconds
		curr: 0,

		// previous lap time expressed in milliseconds
		prev: 0,

		// best session lap time expressed in milliseconds
		best: 0,

		// estimated lap time expressed in milliseconds
		estimated: 0,

		// best stint lap delta expressed in milliseconds
		delta: 0,

		// current sector. i.e. sector (n+1)
		currSectorIndex: 0,

		// current sector time expressed in milliseconds
		currSector: 0,

		// previous sector time expressed in milliseconds
		prevSector: 0,

		// whether or not the delta is positive
		isDeltaPositive: false,

		// whether or not the current lap is valid
		isValidLap: false
	},

	// user adjustable car settings (while driving)
	electronics: {
		// TC1
		// 0: off
		// n: level n
		tc: 0,

		// TC2
		// 0: off or not adjustable in the current car
		// n: level n
		tcCut: 0,

		// engine map
		engineMap: 0,

		// ABS
		// 0: off
		// n: level n
		abs: 0,

		// 0: off
		// 1: low beam
		// 2: high beam
		headlightState: 0,

		// 0: off
		// n: level n
		wiperState: 0,

		// whether or not the rain light is on
		rainLight: false,

		// whether or not the headlights are being flashed
		flasher: false,

		// whether or not the left indicator is on
		leftIndicator: false,

		// whether or not the right indicator is on
		rightIndicator: false
	},

	// session information
	session: {
		// one of: "Practice", "Qualifying", "Race", "Hot Lap", "Hot Stint", "Super Pole", "Unknown"
		type: "",

		// session time remaining expressed in milliseconds
		timeLeft: 0.0,

		// cars on the server
		activeCars: 0,

		// time of day expressed in seconds
		clock: 0.0
	},

	// weather and circuit conditions
	conditions: {
		// wind speed in m/s
		windSpeed: 0.0,

		// wind direction in radians relative to the car's current yaw
		windDirection: 0.0,

		// one of: "Green", "Fast", "Optimum", "Damp", "Wet", "Flooded"
		track: "",

		// rain intensity forecast
		// each value will be one of: "Drizzle", "Light", "Medium", "Heavy",
		// "Thunderstorm", "None"
		rain: {
			curr: "",
			in10: "",
			in30: ""
		}
	},

	// next pitstop settings
	pitstop: {
		// selected tyre set
		tyreSet: 0,

		// amount of fuel to fill in litres
		fuel: 0,

		// tyre pressure settings in PSI
		pressure: {
			fl: 0.0,
			fr: 0.0,
			rl: 0.0,
			rr: 0.0
		}
	},

	// penalty information
	penalty: {
		// refer to the penalty enumeration in hud.h
		type: 0,

		// penalty time in seconds
		duration: 0.0
	},

	// driving time information
	// values are expressed in milliseconds
	// < 0 indicates that there is no limit
	drivingTime: {
		totalRemaining: 0,
		stintRemaining: 0
	},

	// fuel usage information
	fuel: {
		// fuel consumed for the current stint in litres
		used: 0.0,

		// current usage in litres/lap
		rate: 0.0
	},

	// flag information
	flag: {
		// refer to the flag enumeration in hud.h
		curr: 0,

		// green is true when there are no yellows
		green: false,
		chequered: false,
		red: false,
		white: false,
		yellow: {
			global: false,
			sector1: false,
			sector2: false,
			sector3: false
		}
	},

	/**
	 * Physics parameters.
	 */
	// current speed in km/h
	speed: 0.0,

	// current gear
	// 0: reverse
	// 1: neutral
	// 2 .. n: 1st .. nth
	gear: 0,

	// 0 .. 1. How much is the traction control intervening
	tcIntervention: 0.0,

	// 0 .. 1. How much is the ABS intervening
	absIntervention: 0.0,

	// fuel remaining in litres
	fuelRemaining: 0.0,

	// inputs
	input: {
		// 0 .. 1
		accelerator: 0.0,

		// 0 .. 1
		brake: 0.0,

		// -1 .. 1
		steering: 0.0,

		// whether or not the pit limiter is enabled
		pitLimiter: false
	},

	// brake information
	brakes: {
		// this is the raw value; it should be manipulated as described in the
		// shared memory documentation on a per car basis
		bias: 0.0,

		// brake pad compound
		// n: pad n
		compound: {
			front: 0,
			rear: 0,
		},

		// remaining brake pad material (most likely mm)
		padDepth: {
			fl: 0.0,
			fr: 0.0,
			rl: 0.0,
			rr: 0.0
		},

		// remaining brake disc material (most likely mm)
		rotorDepth: {
			fl: 0.0,
			fr: 0.0,
			rl: 0.0,
			rr: 0.0
		},

		// brake temperature in degrees celsius
		temp: {
			fl: 0.0,
			fr: 0.0,
			rl: 0.0,
			rr: 0.0
		}
	},

	// temperature
	// values are in degrees celsius
	temp: {
		ambient: 0.0,
		track: 0.0
	},

	// various engine parameters
	motor: {
		// current engine RPM
		rpm: 0,

		// current boost pressure
		boostPressure: 0.0,

		// engine state
		running: false,

		// whether or not the starter is firing
		starter: false,

		// ignition state
		ignition: false
	},

	// tyre information
	tyres: {
		// tyre pressure (in PSI)
		pressure: {
			fl: 0.0,
			fr: 0.0,
			rl: 0.0,
			rr: 0.0
		},

		// tyre core temperature in degrees celsius
		temp: {
			fl: 0.0,
			fr: 0.0,
			rl: 0.0,
			rr: 0.0
		}
	},

	// damage
	damage: {
		front: 0.0,
		rear: 0.0,
		left: 0.0,
		right: 0.0,
		centre: 0.0
	},

	/**
	 * Static parameters - these should only change if the user changes servers
	 */
	// no. of sessions for this weekend
	sessions: 0,

	// shared memory version
	sharedMemVer: "",

	// assetto corsa competizione version
	accVer: "",

	// user's name
	player: {
		firstname: "",
		surname: "",
		nickname: ""
	},

	// car information
	car: {
		// unique string identifying the car
		// Eg.: "nissan_gt_r_gt3_2018"
		// refer to the shared memory documentation for more information
		model: "",

		// RPM at which the limiter kicks in
		maxRPM: 0,

		// fuel tank capacity
		// warning: always shows total capacity, and never the capacity
		// limited by the BoP
		tankCap: 0.0
	},

	// track information
	track: {
		// track name without the BoP year
		// Eg.: "Paul_Ricard"
		name: "",

		// sector count
		sectors: 0
	},

	// when the pit window starts and ends (if there is one)
	pitWindow: {
		start: 0,
		end: 0
	}
}
```

## Licence
BSD-3-Clause
