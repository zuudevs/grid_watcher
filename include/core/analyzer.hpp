#pragma once

/**
 * @file core/analyzer.hpp
 * @author zuudevs (zuudevs@domain.com)
 * @brief 
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string>
#include <unordered_map>
#include <chrono>
#include "util/ts_queue.hpp"

enum class ActionType : uint8_t {
    BLOCK_IP,
    LOG_SUSPICIOUS,
    UNBLOCK_IP
};

struct ActionEvent {
    ActionType type;
    std::string ip_address;
    std::string reason;
};

class PacketAnalyzer {
    ThreadSafeQueue<std::vector<uint8_t>>& packet_queue_;
    ThreadSafeQueue<ActionEvent>& action_queue_;
    std::atomic<bool> running_{false};
    
    // Port scan detection
    struct ScanTracker {
        std::unordered_map<uint16_t, int> port_attempts;
        std::chrono::steady_clock::time_point last_seen;
    };
    std::unordered_map<std::string, ScanTracker> scan_map_;
    std::mutex scan_mutex_;

public:
    PacketAnalyzer(ThreadSafeQueue<std::vector<uint8_t>>& pkt_q,
                   ThreadSafeQueue<ActionEvent>& act_q);
    
    void start();
    void stop();
    void run();

private:
    static constexpr size_t get_ip_offset() {
#ifdef _WIN32
        return 0;  // Windows: starts at IP header
#else
        return 14; // Linux: skip Ethernet header
#endif
    }
    
    void analyze_packet(const std::vector<uint8_t>& packet);
    std::string extract_src_ip(const std::vector<uint8_t>& packet);
    uint16_t extract_dst_port(const std::vector<uint8_t>& packet);
    bool is_syn_packet(const std::vector<uint8_t>& packet);
    void check_modbus_write(const std::vector<uint8_t>& packet);
    void check_port_scan(const std::string& src_ip, uint16_t dst_port);
};