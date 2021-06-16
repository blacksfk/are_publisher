cmake_minimum_required(VERSION 3.10)

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
	auxiliary.c
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
target_link_libraries(are_publisher ${CONAN_LIBS})
