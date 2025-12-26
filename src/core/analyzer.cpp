/**
 * @file src/analyzer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation of the PacketAnalyzer class for inspecting network traffic and detecting security threats.
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "core/analyzer.hpp"
#include "util/logger.hpp"
#include <span>
#include <format>

PacketAnalyzer::PacketAnalyzer(ThreadSafeQueue<std::vector<uint8_t>>& pkt_q,
                               ThreadSafeQueue<ActionEvent>& act_q)
    : packet_queue_(pkt_q), action_queue_(act_q) {}

void PacketAnalyzer::start() {
    running_ = true;
    Logger::info("Packet analyzer started");
}

void PacketAnalyzer::stop() {
    running_ = false;
    Logger::info("Packet analyzer stopped");
}

void PacketAnalyzer::run() {
    while(running_) {
        auto packet_opt = packet_queue_.pop();
        if(!packet_opt) break;
        
        analyze_packet(*packet_opt);
    }
}

std::string PacketAnalyzer::extract_src_ip(const std::vector<uint8_t>& packet) {
    constexpr size_t offset = get_ip_offset();
    if(packet.size() < offset + 20) return "";
    
    std::span<const uint8_t> ip_header(packet.data() + offset, packet.size() - offset);
    
    return std::format("{}.{}.{}.{}", 
        ip_header[12], ip_header[13], ip_header[14], ip_header[15]);
}

uint16_t PacketAnalyzer::extract_dst_port(const std::vector<uint8_t>& packet) {
    constexpr size_t offset = get_ip_offset();
    if(packet.size() < offset + 20 + 4) return 0;
    
    std::span<const uint8_t> ip_header(packet.data() + offset, packet.size() - offset);
    uint8_t ihl = (ip_header[0] & 0x0F) * 4;
    
    if(packet.size() < offset + ihl + 4) return 0;
    
    // TCP/UDP destination port is at bytes 2-3 of transport header
    uint16_t port = (ip_header[ihl + 2] << 8) | ip_header[ihl + 3];
    return port;
}

bool PacketAnalyzer::is_syn_packet(const std::vector<uint8_t>& packet) {
    constexpr size_t offset = get_ip_offset();
    if(packet.size() < offset + 20 + 14) return false;
    
    std::span<const uint8_t> ip_header(packet.data() + offset, packet.size() - offset);
    
    // Check if TCP (protocol 6)
    if(ip_header[9] != 6) return false;
    
    uint8_t ihl = (ip_header[0] & 0x0F) * 4;
    if(packet.size() < offset + ihl + 14) return false;
    
    // TCP flags are at offset 13 in TCP header
    uint8_t flags = ip_header[ihl + 13];
    return (flags & 0x02) && !(flags & 0x10); // SYN set, ACK not set
}

void PacketAnalyzer::check_modbus_write(const std::vector<uint8_t>& packet) {
    uint16_t dst_port = extract_dst_port(packet);
    if(dst_port != 502) return; // Modbus TCP port
    
    constexpr size_t offset = get_ip_offset();
    std::span<const uint8_t> ip_header(packet.data() + offset, packet.size() - offset);
    
    uint8_t ihl = (ip_header[0] & 0x0F) * 4;
    
    // Check for TCP and Modbus payload
    if(ip_header[9] != 6) return; // Not TCP
    if(packet.size() < offset + ihl + 20 + 8) return; // Need Modbus header
    
    // Modbus function code is at MBAP header (7 bytes) + function code
    size_t modbus_offset = offset + ihl + 20 + 7;
    if(packet.size() <= modbus_offset) return;
    
    uint8_t func_code = ip_header[ihl + 20 + 7];
    
    // Write function codes: 0x05 (Write Single Coil), 0x06 (Write Single Register),
    // 0x10 (Write Multiple Registers)
    if(func_code == 0x05 || func_code == 0x06 || func_code == 0x10) {
        std::string src_ip = extract_src_ip(packet);
        Logger::warn(std::format("SCADA Write Attempt from {} (Function Code: 0x{:02X})", 
            src_ip, func_code));
        
        action_queue_.push(ActionEvent{
            ActionType::BLOCK_IP,
            src_ip,
            std::format("Unauthorized Modbus write (0x{:02X})", func_code)
        });
    }
}

void PacketAnalyzer::check_port_scan(const std::string& src_ip, uint16_t dst_port) {
    if(src_ip.empty() || dst_port == 0) return;
    
    std::lock_guard<std::mutex> lock(scan_mutex_);
    auto& tracker = scan_map_[src_ip];
    tracker.last_seen = std::chrono::steady_clock::now();
    tracker.port_attempts[dst_port]++;
    
    // Threshold: 10 different ports in tracking window
    if(tracker.port_attempts.size() >= 10) {
        Logger::warn(std::format("Port scan detected from {}", src_ip));
        action_queue_.push(ActionEvent{
            ActionType::BLOCK_IP,
            src_ip,
            "Port scanning activity"
        });
        scan_map_.erase(src_ip);
    }
}

void PacketAnalyzer::analyze_packet(const std::vector<uint8_t>& packet) {
    constexpr size_t offset = get_ip_offset();
    if(packet.size() < offset + 20) return;
    
    std::string src_ip = extract_src_ip(packet);
    uint16_t dst_port = extract_dst_port(packet);
    
    // SCADA protection
    check_modbus_write(packet);
    
    // Port scan detection
    if(is_syn_packet(packet)) {
        check_port_scan(src_ip, dst_port);
    }
}