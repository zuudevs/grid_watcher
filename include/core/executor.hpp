/**
 * @file core/executor.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the ActionExecutor class for enforcing security policies and mitigation actions.
 * @version 1.0.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "util/ts_queue.hpp"
#include "util/json_exporter.hpp"
#include "analyzer.hpp"
#include <unordered_set>
#include <mutex>

/**
 * @brief Executes actions based on analyzed threats.
 * 
 * This class consumes ActionEvents from a queue and performs the necessary
 * mitigation steps, such as blocking IPs or logging alerts.
 */
class ActionExecutor {
    ThreadSafeQueue<ActionEvent>& action_queue_;
    std::atomic<bool> running_{false};
    std::unordered_set<std::string> blocked_ips_;
    std::mutex blocked_mutex_;
	JsonExporter json_exporter_;

public:
    /**
     * @brief Construct a new Action Executor object.
     * 
     * @param queue Reference to the thread-safe queue containing action events.
     */
    explicit ActionExecutor(ThreadSafeQueue<ActionEvent>& queue);
    
    /**
     * @brief Starts the executor service.
     */
    void start();

    /**
     * @brief Stops the executor service.
     */
    void stop();

    /**
     * @brief The main execution loop.
     * 
     * Continuously polls the action queue and processes events.
     */
    void run();

private:
    /**
     * @brief Processes a single action event.
     * 
     * @param action The action event to execute.
     */
    void execute_action(const ActionEvent& action);

    /**
     * @brief Blocks a specific IP address.
     * 
     * @param ip The IP address string to block.
     */
    void block_ip(const std::string& ip);

    /**
     * @brief Unblocks a specific IP address.
     * 
     * @param ip The IP address string to unblock.
     */
    void unblock_ip(const std::string& ip);

    /**
     * @brief Checks if an IP address is currently blocked.
     * 
     * @param ip The IP address to check.
     * @return true If the IP is blocked.
     * @return false If the IP is not blocked.
     */
    bool is_blocked(const std::string& ip);
};