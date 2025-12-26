/**
 * @file attack_sim.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Network attack simulator for testing Grid Watcher IPS detection capabilities.
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using sock_t = SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using sock_t = int;
    #define INVALID_SOCKET -1
    #define closesocket close
#endif

/**
 * @brief Simulates a TCP SYN flood attack to trigger port scan detection.
 * 
 * Sends multiple TCP SYN packets to sequential ports on the target IP
 * to mimic a port scanning tool like Nmap.
 * 
 * @param target_ip The IP address of the target system.
 */
void send_syn_flood(const std::string& target_ip) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    
    std::cout << "[SIM] Starting SYN flood to " << target_ip << "\n";
    
    for(int i = 0; i < 15; ++i) {
        sock_t sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock == INVALID_SOCKET) continue;
        
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8000 + i);
        inet_pton(AF_INET, target_ip.c_str(), &addr.sin_addr);
        
        // Non-blocking attempt
#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
#endif
        connect(sock, (sockaddr*)&addr, sizeof(addr));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        closesocket(sock);
        
        std::cout << "[SIM] SYN packet " << (i+1) << "/15 sent\n";
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
}

/**
 * @brief Simulates an unauthorized Modbus TCP write attempt.
 * 
 * Connects to port 502 and sends a Modbus "Write Single Register" (0x06) command,
 * which should be flagged by the IPS as a critical SCADA threat.
 * 
 * @param target_ip The IP address of the target system.
 */
void send_modbus_write_attempt(const std::string& target_ip) {
    std::cout << "[SIM] Simulating Modbus write to " << target_ip << ":502\n";
    std::cout << "[SIM] (Note: Actual Modbus requires full protocol implementation)\n";
    
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    
    sock_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock != INVALID_SOCKET) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(502);
        inet_pton(AF_INET, target_ip.c_str(), &addr.sin_addr);
        
        if(connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
            // Simplified Modbus write frame (0x06 = Write Single Register)
            uint8_t modbus_frame[] = {
                0x00, 0x01,  // Transaction ID
                0x00, 0x00,  // Protocol ID
                0x00, 0x06,  // Length
                0x01,        // Unit ID
                0x06,        // Function code: Write Single Register
                0x00, 0x01,  // Register address
                0x00, 0xFF   // Value
            };
            send(sock, (char*)modbus_frame, sizeof(modbus_frame), 0);
            std::cout << "[SIM] Modbus write frame sent\n";
        }
        closesocket(sock);
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
}

/**
 * @brief Main entry point for the attack simulator.
 * 
 * Orchestrates a sequence of simulated attacks (Port Scan followed by Modbus Write)
 * against a specified target to verify IPS response.
 * 
 * @param argc Argument count.
 * @param argv Argument vector (optional: target IP as first argument).
 * @return int Exit status code.
 */
int main(int argc, char* argv[]) {
    std::string target = "127.0.0.1";
    if(argc > 1) target = argv[1];
    
    std::cout << "===========================================\n";
    std::cout << "  Grid Watcher Attack Simulator\n";
    std::cout << "  Target: " << target << "\n";
    std::cout << "===========================================\n\n";
    
    std::cout << "Waiting 3 seconds for IPS to initialize...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::cout << "\n[1] Simulating Port Scan Attack...\n";
    send_syn_flood(target);
    
    std::cout << "\n[2] Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "\n[3] Simulating Modbus Write Attack...\n";
    send_modbus_write_attempt(target);
    
    std::cout << "\n[SIM] Attack simulation complete\n";
    std::cout << "[SIM] Check Grid Watcher logs for detections\n";
    
    return 0;
}