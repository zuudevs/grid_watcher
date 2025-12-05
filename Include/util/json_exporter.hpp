#pragma once

/**
 * @file util/json_exporter.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Thread-safe JSON exporter for dashboard integration
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <fstream>
#include <format>

struct DashboardAlert {
    std::string type;
    std::string src_ip;
    std::string timestamp;
    std::string reason;
};

class JsonExporter {
    std::string output_path_;
    mutable std::mutex mtx_;
    std::chrono::steady_clock::time_point start_time_;
    
    // System state
    std::string system_status_{"INITIALIZING"};
    int total_blocked_{0};
    std::vector<std::string> blocked_ips_;
    DashboardAlert latest_alert_;
    int total_threats_{0};
    int packets_analyzed_{0};

public:
    explicit JsonExporter(const std::string& path = "www/dashboard_data.json");
    
    // Update methods
    void set_system_status(const std::string& status);
    void increment_blocked();
    void add_blocked_ip(const std::string& ip);
    void remove_blocked_ip(const std::string& ip);
    void set_latest_alert(const std::string& type, const std::string& src_ip, 
                          const std::string& reason);
    void increment_threats();
    void set_packets_analyzed(int count);
    
    // Write to disk (called periodically or on events)
    void write();
    
private:
    std::string get_current_timestamp() const;
    int get_uptime_seconds() const;
    std::string escape_json_string(const std::string& str) const;
    void write_json_file();
};