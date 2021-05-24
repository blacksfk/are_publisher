# ACC Race Engineer publisher
Work like a real race engineer on a virtual car! This application simply reads from the ACC data stored in shared memory and sends that data to a server via an HTTP POST request perodically.

## Compiling
1. In order to build this program, you require [CMake](https://cmake.org/download/) and the package manager [conan](https://conan.io/).
2. Once both are installed, open a terminal (eg. git bash) and create a build directory for the output files. Eg. `<path_to_project/build`.
3. `cd build`
4. Install dependencies with: `conan install ..` (or the correct path to the directory containing conanfile.txt).
5. Generate the build configuration with: `cmake ..` (or the correct path to the directory containing CMakeLists.txt).
6. Build with: `cmake --build .`
7. The executable is now available in the `bin` sub-directory.

## Running
`are_publisher.exe <server_address> <channel_id> <channel_password>`

Eg.: `are_publisher.exe example.com 12345 joestactiv`

## Licence
BSD-3-Clause
