/**
 * @file src/json_exporter.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation of thread-safe JSON exporter
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "util/json_exporter.hpp"
#include "util/logger.hpp"
#include <sstream>
#include <algorithm>
#include <filesystem>

JsonExporter::JsonExporter(const std::string& path)
    : output_path_(path),
      start_time_(std::chrono::steady_clock::now()) {
    
    // Ensure directory exists
    std::filesystem::path file_path(path);
    if (file_path.has_parent_path()) {
        std::filesystem::create_directories(file_path.parent_path());
    }
    
    // Write initial empty state
    write();
    Logger::info("JSON Exporter initialized: " + path);
}

void JsonExporter::set_system_status(const std::string& status) {
    std::lock_guard<std::mutex> lock(mtx_);
    system_status_ = status;
}

void JsonExporter::increment_blocked() {
    std::lock_guard<std::mutex> lock(mtx_);
    total_blocked_++;
}

void JsonExporter::add_blocked_ip(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = std::find(blocked_ips_.begin(), blocked_ips_.end(), ip);
    if (it == blocked_ips_.end()) {
        blocked_ips_.push_back(ip);
    }
}

void JsonExporter::remove_blocked_ip(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = std::find(blocked_ips_.begin(), blocked_ips_.end(), ip);
    if (it != blocked_ips_.end()) {
        blocked_ips_.erase(it);
    }
}

void JsonExporter::set_latest_alert(const std::string& type, 
                                     const std::string& src_ip,
                                     const std::string& reason) {
    std::lock_guard<std::mutex> lock(mtx_);
    latest_alert_.type = type;
    latest_alert_.src_ip = src_ip;
    latest_alert_.reason = reason;
    latest_alert_.timestamp = get_current_timestamp();
}

void JsonExporter::increment_threats() {
    std::lock_guard<std::mutex> lock(mtx_);
    total_threats_++;
}

void JsonExporter::set_packets_analyzed(int count) {
    std::lock_guard<std::mutex> lock(mtx_);
    packets_analyzed_ = count;
}

void JsonExporter::write() {
    std::lock_guard<std::mutex> lock(mtx_);
    write_json_file();
}

std::string JsonExporter::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    return std::string(buf);
}

int JsonExporter::get_uptime_seconds() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    return static_cast<int>(duration.count());
}

std::string JsonExporter::escape_json_string(const std::string& str) const {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c >= 0 && c <= 0x1f) {
                    oss << "\\u" << std::format("{:04x}", static_cast<int>(c));
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

void JsonExporter::write_json_file() {
    std::ostringstream json;
    
    json << "{\n";
    json << "  \"system_status\": \"" << escape_json_string(system_status_) << "\",\n";
    json << "  \"total_blocked\": " << total_blocked_ << ",\n";
    json << "  \"total_threats\": " << total_threats_ << ",\n";
    json << "  \"packets_analyzed\": " << packets_analyzed_ << ",\n";
    json << "  \"uptime_seconds\": " << get_uptime_seconds() << ",\n";
    
    // Latest alert
    json << "  \"latest_alert\": {\n";
    json << "    \"type\": \"" << escape_json_string(latest_alert_.type) << "\",\n";
    json << "    \"src_ip\": \"" << escape_json_string(latest_alert_.src_ip) << "\",\n";
    json << "    \"reason\": \"" << escape_json_string(latest_alert_.reason) << "\",\n";
    json << "    \"timestamp\": \"" << escape_json_string(latest_alert_.timestamp) << "\"\n";
    json << "  },\n";
    
    // Blocked IPs list
    json << "  \"blocked_list\": [\n";
    for (size_t i = 0; i < blocked_ips_.size(); ++i) {
        json << "    \"" << escape_json_string(blocked_ips_[i]) << "\"";
        if (i < blocked_ips_.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    
    // Timestamp
    json << "  \"last_update\": \"" << get_current_timestamp() << "\"\n";
    json << "}\n";
    
    // Atomic write: write to temp file, then rename
    std::string temp_path = output_path_ + ".tmp";
    
    std::ofstream file(temp_path, std::ios::binary);
    if (!file) {
        Logger::error("Failed to open JSON export file: " + temp_path);
        return;
    }
    
    file << json.str();
    file.close();
    
    if (!file) {
        Logger::error("Failed to write JSON export file");
        return;
    }
    
    // Atomic rename
    std::filesystem::rename(temp_path, output_path_);
}
