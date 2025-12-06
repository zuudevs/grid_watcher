#pragma once

/**
 * @file core/sniffer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <vector>
#include <cstdint>
#include <thread>
#include <atomic>
#include "util/ts_queue.hpp"

class PacketSniffer {
    ThreadSafeQueue<std::vector<uint8_t>>& packet_queue_;
    std::atomic<bool> running_{false};
    int socket_fd_ = -1;
    
#ifdef _WIN32
    void* wsa_data_ = nullptr; // WSADATA pointer
#endif

public:
    explicit PacketSniffer(ThreadSafeQueue<std::vector<uint8_t>>& queue);
    ~PacketSniffer();
    
    void start();
    void stop();
    void run();
    
private:
    bool init_socket();
    void cleanup_socket();
};