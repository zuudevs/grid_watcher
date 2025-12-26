cmake_minimum_required(VERSION 3.20)
project(grid_watcher
	VERSION 1.0.0
	DESCRIPTION "SCADA Intrusion Prevention System"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_executable(grid_watcher
    src/main.cpp
    src/core/sniffer.cpp
    src/core/analyzer.cpp
    src/core/executor.cpp
	src/util/json_exporter.cpp
)

# Include directories
target_include_directories(grid_watcher PRIVATE 
    "${CMAKE_SOURCE_DIR}/include"
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
else()
    target_link_libraries(grid_watcher pthread)
endif()

# Initial script generation
add_executable(launcher_gen scripts/init.cpp)

add_custom_command(TARGET launcher_gen POST_BUILD
    COMMAND launcher_gen
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "[Info] Generating auto-run script (run.bat/run.sh)..."
)

if(WIN32)
    add_custom_target(run
        COMMAND cmd /c "${CMAKE_SOURCE_DIR}/run.bat"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS grid_watcher launcher_gen
        COMMENT "[Info] Launching System..."
    )
else()
    add_custom_target(run
        COMMAND "${CMAKE_SOURCE_DIR}/run.sh"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS grid_watcher launcher_gen
        COMMENT "[Info] Launching System..."
    )
endif()