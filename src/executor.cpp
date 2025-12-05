/**
 * @file src/executor.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "core/executor.hpp"
#include "util/logger.hpp"
#include <format>
#include <cstdlib>

ActionExecutor::ActionExecutor(ThreadSafeQueue<ActionEvent>& queue)
    : action_queue_(queue) {}

void ActionExecutor::start() {
    running_ = true;
    Logger::info("Action executor started");
}

void ActionExecutor::stop() {
    running_ = false;
    Logger::info("Action executor stopped");
}

void ActionExecutor::run() {
    while(running_) {
        auto action_opt = action_queue_.pop();
        if(!action_opt) break;
        
        execute_action(*action_opt);
    }
}

bool ActionExecutor::is_blocked(const std::string& ip) {
    std::lock_guard<std::mutex> lock(blocked_mutex_);
    return blocked_ips_.contains(ip);
}

void ActionExecutor::execute_action(const ActionEvent& action) {
    switch(action.type) {
        case ActionType::BLOCK_IP:
            if(!is_blocked(action.ip_address)) {
                block_ip(action.ip_address);
            }
            break;
            
        case ActionType::UNBLOCK_IP:
            unblock_ip(action.ip_address);
            break;
            
        case ActionType::LOG_SUSPICIOUS:
            Logger::warn(std::format("Suspicious: {} - {}", 
                action.ip_address, action.reason));
            break;
    }
}

void ActionExecutor::block_ip(const std::string& ip) {
    {
        std::lock_guard<std::mutex> lock(blocked_mutex_);
        blocked_ips_.insert(ip);
    }
    
    Logger::critical(std::format("BLOCKING IP: {}", ip));
    
#ifdef _WIN32
    std::string cmd = std::format(
        "netsh advfirewall firewall add rule name=\"GridWatcher_{}\" "
        "dir=in interface=any action=block remoteip={}", ip, ip
	);
#else
    std::string cmd = std::format("iptables -A INPUT -s {} -j DROP", ip);
#endif
    
    int ret = std::system(cmd.c_str());
    if(ret != 0) {
        Logger::error(std::format("Failed to execute firewall rule for {}", ip));
    }
}

void ActionExecutor::unblock_ip(const std::string& ip) {
    {
        std::lock_guard<std::mutex> lock(blocked_mutex_);
        blocked_ips_.erase(ip);
    }
    
    Logger::info(std::format("UNBLOCKING IP: {}", ip));
    
#ifdef _WIN32
    std::string cmd = std::format(
        "netsh advfirewall firewall delete rule name=\"GridWatcher_{}\"", ip);
#else
    std::string cmd = std::format("iptables -D INPUT -s {} -j DROP", ip);
#endif
    
    std::system(cmd.c_str());
}