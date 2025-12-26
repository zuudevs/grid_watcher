/**
 * @file util/logger.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Simple thread-safe logging utility.
 * @version 1.0.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <chrono>
#include <format>

/**
 * @brief Defines severity levels for log messages.
 */
enum class LogLevel : uint8_t { 
	INFO, WARNING, ERROR, CRITICAL 
};

/**
 * @brief Provides static methods for thread-safe logging to the console.
 */
class Logger {
    static inline std::mutex mtx_;
    
    static std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return std::format("{}.{:03d}", buf, ms.count());
    }
    
    static std::string level_str(LogLevel level) {
        switch(level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRIT";
        }
        return "UNKNOWN";
    }

public:
    /**
     * @brief Logs a message with a specific severity level.
     * 
     * @param level The severity level of the message.
     * @param msg The message content to log.
     */
    static void log(LogLevel level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        std::cout << std::format("[{}] [{}] {}\n", 
            timestamp(), level_str(level), msg);
    }
    
    /**
     * @brief Logs an informational message.
     * 
     * @param msg The message content.
     */
    static void info(const std::string& msg) { log(LogLevel::INFO, msg); }

    /**
     * @brief Logs a warning message.
     * 
     * @param msg The message content.
     */
    static void warn(const std::string& msg) { log(LogLevel::WARNING, msg); }

    /**
     * @brief Logs an error message.
     * 
     * @param msg The message content.
     */
    static void error(const std::string& msg) { log(LogLevel::ERROR, msg); }

    /**
     * @brief Logs a critical system failure message.
     * 
     * @param msg The message content.
     */
    static void critical(const std::string& msg) { log(LogLevel::CRITICAL, msg); }
};