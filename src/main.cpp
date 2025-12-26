/**
 * @file src/main.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Main entry point for the Grid Watcher SCADA Intrusion Prevention System.
 * @version 1.0.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "gridwatcher.hpp"
#include <thread>
#include <csignal>
#include <atomic>

/**
 * @brief Global atomic flag to control the application main loop.
 * 
 * This flag is set to true when a termination signal is received, 
 * triggering a graceful shutdown of all worker threads.
 */
std::atomic<bool> g_shutdown{false};

/**
 * @brief Handles system signals (SIGINT, SIGTERM) for graceful termination.
 * 
 * @param signal The signal number received (unused in implementation).
 */
void signal_handler(int) {
    Logger::info("Shutdown signal received");
    g_shutdown = true;
}

/**
 * @brief Application entry point.
 * 
 * Initializes the IPS pipeline components (Sniffer, Analyzer, Executor),
 * launches worker threads, and monitors the shutdown flag.
 * 
 * @return int Exit status code (0 for success).
 */
int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    Logger::info("========================================");
    Logger::info("  Grid Watcher IPS v1.0");
    Logger::info("  SCADA Intrusion Prevention System");
#ifdef _WIN32
    Logger::info("  Platform: Windows (Compatibility Mode)");
#else
    Logger::info("  Platform: Linux (Production)");
#endif
    Logger::info("========================================");
    
    // Pipeline queues
    ThreadSafeQueue<std::vector<uint8_t>> packet_queue;
    ThreadSafeQueue<ActionEvent> action_queue;
    
    // Initialize components
    PacketSniffer sniffer(packet_queue);
    PacketAnalyzer analyzer(packet_queue, action_queue);
    ActionExecutor executor(action_queue);
    
    // Start pipeline
    sniffer.start();
    analyzer.start();
    executor.start();
    
    // Launch worker threads
    std::jthread sniffer_thread([&]{ sniffer.run(); });
    std::jthread analyzer_thread([&]{ analyzer.run(); });
    std::jthread executor_thread([&]{ executor.run(); });
    
    Logger::info("All systems operational. Monitoring traffic...");
    
    // Wait for shutdown
    while(!g_shutdown) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Graceful shutdown
    Logger::info("Initiating shutdown sequence...");
    sniffer.stop();
    packet_queue.shutdown();
    
    analyzer.stop();
    action_queue.shutdown();
    
    executor.stop();
    
    Logger::info("Grid Watcher IPS terminated");
    return 0;
}