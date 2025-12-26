/**
 * @file init.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Generates a platform-specific runner script (run.bat or run.sh)
 * to launch the Dashboard and IPS Engine simultaneously.
 * @version 1.0.0
 * @date 2025-12-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Detects the operating system and generates the appropriate launcher script.
 * 
 * On Windows, it creates 'run.bat'.
 * On Linux, it creates 'run.sh' and attempts to set the executable permission (+x).
 */
void generate_run_file() {
    // Determine filename based on OS
    #ifdef _WIN32
        std::string filename = "../run.bat"; 
    #else
        std::string filename = "../run.sh";
    #endif

    std::ofstream run_file(filename);
    
    if (!run_file.is_open()) {
        std::cerr << "[Error] Failed to create file: " << filename << " (Check folder permissions!)" << std::endl;
        return;
    }

    // Write script content using Raw String Literals
    #ifdef _WIN32
        // Windows Batch Script
        run_file << R"(
@echo off
title Grid Watcher Launcher

REM 1. Check Binary Existence
if not exist "bin\grid_watcher.exe" (
    echo [Error] File bin/grid_watcher.exe not found! Please build first!
    pause
    exit /b
)

echo [Info] Launching Grid Watcher System...

REM 2. Launch Dashboard in a new window
start "GW Dashboard" cmd /k "cd www && python -m http.server 8080"

REM 3. Launch IPS Engine in a new window (Requires Administrator privileges)
start "GW IPS Engine" cmd /k "bin\grid_watcher.exe"

echo [Info] All systems are running!
echo [Info] Please check the newly opened windows.
)";
    #else
        // Linux Bash Script (Includes Shebang and Trap for cleanup)
        run_file << R"(#!/bin/bash

# Function to kill background processes on exit (Ctrl+C)
cleanup() {
    echo -e "\n[Info] Shutting down services..."
    kill $(jobs -p) 2>/dev/null
    exit
}
trap cleanup SIGINT SIGTERM

# 1. Check Binary Existence
if [ ! -f "./bin/grid_watcher" ]; then
    echo "[Error] Binary not found! Please build first: cmake --build build"
    exit 1
fi

# 2. Start Dashboard in background
echo "[Info] Starting Dashboard at http://localhost:8080..."
(cd www && python3 -m http.server 8080) > /dev/null 2>&1 &
DASHBOARD_PID=$!

sleep 1

# 3. Start IPS Engine (Requires sudo for raw sockets)
echo "[Info] Starting Grid Watcher IPS..."
echo "[Info] Enter your sudo password if requested!"
sudo ./bin/grid_watcher

# Wait for dashboard process (keeps script alive until cleanup)
wait $DASHBOARD_PID
)";
    #endif

    run_file.close();
    std::cout << "[Success] Launcher script generated: " << filename << std::endl;

    // Set file permissions on Linux (chmod +x)
    #ifndef _WIN32
        try {
            fs::permissions(filename, 
                fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, 
                fs::perm_options::add);
            std::cout << "[Info] File permission set to +x (executable)." << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[Warning] Failed to set permissions: " << e.what() << std::endl;
            std::cerr << "Perform manually: chmod +x " << filename << std::endl;
        }
    #endif
}

int main() {
    generate_run_file();
    return 0;
}