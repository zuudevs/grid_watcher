/**
 * @file src/sniffer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation of the PacketSniffer class for capturing raw network traffic.
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "core/sniffer.hpp"
#include "util/logger.hpp"
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
	#include <mstcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <linux/if_packet.h>
    #include <net/ethernet.h>
    #include <unistd.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
#endif

PacketSniffer::PacketSniffer(ThreadSafeQueue<std::vector<uint8_t>>& queue)
    : packet_queue_(queue) {
#ifdef _WIN32
    WSADATA* wsa = new WSADATA;
    wsa_data_ = wsa;
    if(WSAStartup(MAKEWORD(2, 2), wsa) != 0) {
        Logger::critical("WSAStartup failed");
        delete wsa;
        wsa_data_ = nullptr;
    }
    Logger::warn("Running in Windows Compatibility Mode. TCP capture may be limited by OS.");
#endif
}

PacketSniffer::~PacketSniffer() {
    stop();
    cleanup_socket();
#ifdef _WIN32
    if(wsa_data_) {
        WSACleanup();
        delete static_cast<WSADATA*>(wsa_data_);
    }
#endif
}

bool PacketSniffer::init_socket() {
#ifdef _WIN32
    socket_fd_ = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    if(socket_fd_ == INVALID_SOCKET) {
        Logger::error("Failed to create raw socket (admin rights required)");
        return false;
    }
    
    // Bind to local interface
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(socket_fd_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        Logger::error("Socket bind failed");
        closesocket(socket_fd_);
        return false;
    }
    
    // Enable promiscuous mode
    DWORD flag = 1;
    if(ioctlsocket(socket_fd_, SIO_RCVALL, &flag) == SOCKET_ERROR) {
        Logger::warn("Failed to enable promiscuous mode");
    }
#else
    socket_fd_ = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(socket_fd_ < 0) {
        Logger::error("Failed to create raw socket (run with sudo)");
        return false;
    }
#endif
    
    Logger::info("Packet capture socket initialized");
    return true;
}

void PacketSniffer::cleanup_socket() {
    if(socket_fd_ != -1) {
#ifdef _WIN32
        closesocket(socket_fd_);
#else
        close(socket_fd_);
#endif
        socket_fd_ = -1;
    }
}

void PacketSniffer::start() {
    if(!init_socket()) {
        Logger::critical("Failed to initialize sniffer");
        return;
    }
    running_ = true;
    Logger::info("Packet sniffer started");
}

void PacketSniffer::stop() {
    running_ = false;
    cleanup_socket();
    Logger::info("Packet sniffer stopped");
}

void PacketSniffer::run() {
    std::vector<uint8_t> buffer(65536);
    
    while(running_) {
#ifdef _WIN32
        int bytes = recv(socket_fd_, (char*)buffer.data(), buffer.size(), 0);
#else
        ssize_t bytes = recvfrom(socket_fd_, buffer.data(), buffer.size(), 
                                 0, nullptr, nullptr);
#endif
        
        if(bytes > 0) {
            std::vector<uint8_t> packet(buffer.begin(), buffer.begin() + bytes);
            packet_queue_.push(std::move(packet));
        }
    }
}