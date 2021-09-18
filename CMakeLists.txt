cmake_minimum_required(VERSION 3.10)

# compile time flags
option(DEBUG "Enable logging for debugging purposes" ON)
option(DISABLE_BROADCAST "Disable broadcasting during debugging" OFF)
option(RECORD_DATA "Record JSON data to data.json" OFF)
set(API_URL, "" CACHE STRING "API URL")
configure_file(config.h.in config.h)

# project name
project(are_publisher)

# compiler options
add_compile_options(/W4 /WX /std:c17)

# dependencies
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# executable
add_executable(
	are_publisher
	WIN32
	api.c
	auxiliary.c
	channel.c
	controls.c
	gui.c
	hud.c
	instance_data.c
	physics.c
	procedure.c
	properties.c
	response.c
	request.c
	main.c
	shared_mem.c
)
target_include_directories(are_publisher PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(are_publisher ${CONAN_LIBS})
