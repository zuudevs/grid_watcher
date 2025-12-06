#pragma once

/**
 * @file core/executor.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "util/ts_queue.hpp"
#include "util/json_exporter.hpp"
#include "analyzer.hpp"
#include <unordered_set>
#include <mutex>

class ActionExecutor {
    ThreadSafeQueue<ActionEvent>& action_queue_;
    std::atomic<bool> running_{false};
    std::unordered_set<std::string> blocked_ips_;
    std::mutex blocked_mutex_;
	JsonExporter json_exporter_;

public:
    explicit ActionExecutor(ThreadSafeQueue<ActionEvent>& queue);
    
    void start();
    void stop();
    void run();

private:
    void execute_action(const ActionEvent& action);
    void block_ip(const std::string& ip);
    void unblock_ip(const std::string& ip);
    bool is_blocked(const std::string& ip);
};