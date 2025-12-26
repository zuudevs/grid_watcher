/**
 * @file core/sniffer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the PacketSniffer class for capturing raw network traffic.
 * @version 1.0.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <vector>
#include <cstdint>
#include <thread>
#include <atomic>
#include "util/ts_queue.hpp"

/**
 * @brief Captures raw network packets.
 * 
 * This class manages a raw socket to sniff network traffic and pushes
 * captured packet data into a thread-safe queue for analysis.
 */
class PacketSniffer {
    ThreadSafeQueue<std::vector<uint8_t>>& packet_queue_;
    std::atomic<bool> running_{false};
    int socket_fd_ = -1;
    
#ifdef _WIN32
    void* wsa_data_ = nullptr; // WSADATA pointer
#endif

public:
    /**
     * @brief Construct a new Packet Sniffer object.
     * 
     * @param queue Reference to the queue where captured packets will be stored.
     */
    explicit PacketSniffer(ThreadSafeQueue<std::vector<uint8_t>>& queue);

    /**
     * @brief Destroy the Packet Sniffer object and ensures socket cleanup.
     */
    ~PacketSniffer();
    
    /**
     * @brief Initializes the socket and starts the sniffing process.
     */
    void start();

    /**
     * @brief Stops the sniffing process.
     */
    void stop();

    /**
     * @brief The main sniffing loop.
     * 
     * Continuously reads from the raw socket and pushes data to the queue.
     */
    void run();
    
private:
    /**
     * @brief Initializes the raw socket.
     * 
     * @return true If initialization is successful.
     * @return false If initialization fails.
     */
    bool init_socket();

    /**
     * @brief Closes the socket and cleans up resources (e.g., WSA on Windows).
     */
    void cleanup_socket();
};