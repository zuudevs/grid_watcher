cmake_minimum_required(VERSION 3.20)
project(grid_watcher CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main executable
add_executable(grid_watcher
    src/main.cpp
    src/sniffer.cpp
    src/analyzer.cpp
    src/executor.cpp
	src/json_exporter.cpp
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(grid_watcher ws2_32)
else()
    target_link_libraries(grid_watcher pthread)
endif()

# Attack simulator
add_executable(attack_sim tests/attack_sim.cpp)

if(WIN32)
    target_link_libraries(attack_sim ws2_32)
endif()