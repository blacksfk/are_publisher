# ACC Race Engineer publisher
Work like a real race engineer on a virtual car! This application simply reads from the ACC data stored in shared memory and sends that data to a server via an HTTP POST request perodically.

## Setup
1. In order to build this program, you require [CMake](https://cmake.org/download/) and the package manager [conan](https://conan.io/) along with Windows SDKs and libraries (via Visual Studio).
2. Once installed, open a terminal (eg. git bash) and create a build directory for the output files. Eg. `<path_to_project>/build`.
3. `cd build`
4. Install dependencies with: `conan install ..` (or the correct path to the directory containing conanfile.txt).
5. Generate the build configuration with: `cmake ..` (or the correct path to the directory containing CMakeLists.txt).

## Compiling
MSVC defaults to building for a debug environment so building for production requires switching the application's debug flag. The executable will be available under the `bin` sub-directory in either environment mode.

### Development
`cmake --build .`

### Production
1. `cmake .. -D DEBUG=OFF`
2. MSVC is a multi-environment compiler and therefore cmake requires an extra flag at build time: `cmake --build . --config Release`

## Licence
BSD-3-Clause
