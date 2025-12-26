/**
 * @file util/json_exporter.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Thread-safe JSON exporter for dashboard integration
 * @version 1.0.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <fstream>
#include <format>

/**
 * @brief Represents a security alert to be displayed on the dashboard.
 */
struct DashboardAlert {
    std::string type;       ///< Type of alert (e.g., "PORT_SCAN", "MODBUS_WRITE").
    std::string src_ip;     ///< Source IP address triggering the alert.
    std::string timestamp;  ///< Time when the alert occurred.
    std::string reason;     ///< Description of the alert cause.
};

/**
 * @brief Manages the export of system state and alerts to a JSON file.
 * 
 * This class aggregates statistics and events from the IPS pipeline and 
 * writes them to a JSON file that can be consumed by a frontend dashboard.
 * It ensures thread safety for all update operations.
 */
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
    /**
     * @brief Construct a new Json Exporter object.
     * 
     * @param path Path to the output JSON file. Defaults to "www/dashboard_data.json".
     */
    explicit JsonExporter(const std::string& path = "www/dashboard_data.json");
    
    // Update methods
    /**
     * @brief Updates the overall system status string.
     * 
     * @param status The new status (e.g., "RUNNING", "STOPPED").
     */
    void set_system_status(const std::string& status);

    /**
     * @brief Increments the counter for total blocked connection attempts.
     */
    void increment_blocked();

    /**
     * @brief Adds an IP address to the list of currently blocked IPs.
     * 
     * @param ip The IP address to add.
     */
    void add_blocked_ip(const std::string& ip);

    /**
     * @brief Removes an IP address from the list of currently blocked IPs.
     * 
     * @param ip The IP address to remove.
     */
    void remove_blocked_ip(const std::string& ip);

    /**
     * @brief Sets the details for the most recent alert.
     * 
     * @param type The type of alert.
     * @param src_ip The source IP address.
     * @param reason The reason for the alert.
     */
    void set_latest_alert(const std::string& type, const std::string& src_ip, 
                          const std::string& reason);

    /**
     * @brief Increments the total count of detected threats.
     */
    void increment_threats();

    /**
     * @brief Updates the count of total packets analyzed.
     * 
     * @param count The new total count.
     */
    void set_packets_analyzed(int count);
    
    // Write to disk (called periodically or on events)
    /**
     * @brief Writes the current state to the JSON file.
     * 
     * This method serializes the current internal state to JSON format
     * and writes it to the configured output path.
     */
    void write();
    
private:
    std::string get_current_timestamp() const;
    int get_uptime_seconds() const;
    std::string escape_json_string(const std::string& str) const;
    void write_json_file();
};