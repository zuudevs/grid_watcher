/**
 * @file src/executor.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation of the ActionExecutor class for enforcing security policies and mitigation actions.
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
 : action_queue_(queue), json_exporter_("www/dashboard_data.json") {
	json_exporter_.set_system_status("INITIALIZING");
}

void ActionExecutor::start() {
    running_ = true;
	json_exporter_.set_system_status("RUNNING");
    json_exporter_.write();  // Write initial state
    Logger::info("Action executor started");
}

void ActionExecutor::stop() {
    running_ = false;
	json_exporter_.set_system_status("STOPPED");
    json_exporter_.write();
    Logger::info("Action executor stopped");
}

void ActionExecutor::run() {
    auto last_write = std::chrono::steady_clock::now();
    
    while(running_) {
        auto action_opt = action_queue_.pop();
        if(!action_opt) break;
        
        execute_action(*action_opt);
        
        // Periodic write every 2 seconds (even if no events)
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_write).count() >= 2) {
            json_exporter_.write();
            last_write = now;
        }
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
                    
                    // UPDATE JSON: Add blocked IP and alert
                    json_exporter_.increment_blocked();
                    json_exporter_.increment_threats();
                    json_exporter_.add_blocked_ip(action.ip_address);
                    json_exporter_.set_latest_alert(
                        action.reason.find("Modbus") != std::string::npos ? "MODBUS_WRITE" : "PORT_SCAN",
                        action.ip_address,
                        action.reason
                    );
                    json_exporter_.write();  // Write to disk immediately
                }
                break;
                
            case ActionType::UNBLOCK_IP:
                unblock_ip(action.ip_address);
                
                // UPDATE JSON: Remove from blocked list
                json_exporter_.remove_blocked_ip(action.ip_address);
                json_exporter_.write();
                break;
                
            case ActionType::LOG_SUSPICIOUS:
                Logger::warn(std::format("Suspicious: {} - {}", 
                    action.ip_address, action.reason));
                
                // UPDATE JSON: Log as alert but don't block
                json_exporter_.set_latest_alert(
                    "SUSPICIOUS",
                    action.ip_address,
                    action.reason
                );
                json_exporter_.write();
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