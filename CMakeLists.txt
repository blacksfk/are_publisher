cmake_minimum_required(VERSION 3.10)

# project name
project(are_publisher)

# compiler options
add_compile_options(/W4 /WX /std:c17)

# classic MS and non-conforming code
# suppress C5105 for the windows header files
add_compile_options(/wd5105)

# dependencies
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# executable
add_executable(are_publisher auxiliary.c hud.c physics.c properties.c main.c)
target_link_libraries(are_publisher ${CONAN_LIBS})
