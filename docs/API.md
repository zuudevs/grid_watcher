# Grid Watcher IPS - API Reference

Complete technical reference for Grid Watcher's core classes, functions, and interfaces.

---

## 📋 Table of Contents

- [Core Components](#core-components)
- [Packet Sniffer](#packet-sniffer)
- [Packet Analyzer](#packet-analyzer)
- [Action Executor](#action-executor)
- [Utility Classes](#utility-classes)
- [Data Structures](#data-structures)
- [Configuration](#configuration)
- [Integration Examples](#integration-examples)

---

## 🏗️ Core Components

### Architecture Overview

```cpp
// Pipeline Components
PacketSniffer  → ThreadSafeQueue → PacketAnalyzer → ThreadSafeQueue → ActionExecutor
                  (packet_queue)                      (action_queue)
```

---

## 📡 Packet Sniffer

### Class: `PacketSniffer`

**Location:** `include/core/sniffer.hpp`

**Purpose:** Captures raw network packets using platform-specific raw sockets.

### Constructor

```cpp
explicit PacketSniffer(ThreadSafeQueue<std::vector<uint8_t>>& queue);
```

**Parameters:**
- `queue` - Reference to thread-safe queue for captured packets

**Example:**
```cpp
ThreadSafeQueue<std::vector<uint8_t>> packet_queue;
PacketSniffer sniffer(packet_queue);
```

### Public Methods

#### `void start()`

Initializes the raw socket and begins packet capture.

**Thread Safety:** ✅ Thread-safe  
**Requires:** Administrator/Root privileges  
**Throws:** None (logs errors internally)

**Example:**
```cpp
sniffer.start();
// Socket is now capturing packets
```

---

#### `void stop()`

Stops packet capture and closes the socket.

**Thread Safety:** ✅ Thread-safe  
**Blocks:** No

**Example:**
```cpp
sniffer.stop();
// Socket closed, capture stopped
```

---

#### `void run()`

Main capture loop. Should be called from a dedicated thread.

**Thread Safety:** ✅ Thread-safe  
**Blocks:** Yes (until `stop()` is called)  
**Buffer Size:** 65536 bytes per packet

**Example:**
```cpp
std::jthread sniffer_thread([&]{ sniffer.run(); });
// Thread continuously captures packets
```

---

### Platform Differences

| Feature | Linux | Windows |
|---------|-------|---------|
| Socket Type | `AF_PACKET` + `SOCK_RAW` | `AF_INET` + `SOCK_RAW` |
| Header Offset | 14 bytes (Ethernet) | 0 bytes (IP direct) |
| Promiscuous Mode | ✅ Full support | ⚠️ Limited by OS |
| Performance | 🚀 High | ⚡ Medium |

---

## 🔍 Packet Analyzer

### Class: `PacketAnalyzer`

**Location:** `include/core/analyzer.hpp`

**Purpose:** Performs deep packet inspection and threat detection.

### Constructor

```cpp
PacketAnalyzer(ThreadSafeQueue<std::vector<uint8_t>>& pkt_q,
               ThreadSafeQueue<ActionEvent>& act_q);
```

**Parameters:**
- `pkt_q` - Input queue (packets to analyze)
- `act_q` - Output queue (detected threats)

---

### Public Methods

#### `void start()`

Starts the analysis engine.

**Example:**
```cpp
analyzer.start();
Logger::info("Analyzer ready to process packets");
```

---

#### `void run()`

Main analysis loop. Processes packets from the queue.

**Thread Safety:** ✅ Thread-safe  
**Blocks:** Yes (until queue shutdown)

**Example:**
```cpp
std::jthread analyzer_thread([&]{ analyzer.run(); });
```

---

### Detection Methods (Private)

#### `void analyze_packet(const std::vector<uint8_t>& packet)`

Performs comprehensive packet analysis.

**Detection Pipeline:**
1. Extract source IP and destination port
2. Check for Modbus write commands
3. Detect port scanning patterns
4. Push threats to action queue

---

#### `void check_modbus_write(const std::vector<uint8_t>& packet)`

Detects unauthorized Modbus TCP write operations.

**Detects:**
- `0x05` - Write Single Coil
- `0x06` - Write Single Register
- `0x10` - Write Multiple Registers

**Algorithm:**
```cpp
if (dst_port == 502 && protocol == TCP) {
    uint8_t func_code = extract_modbus_function_code(packet);
    if (func_code == 0x05 || func_code == 0x06 || func_code == 0x10) {
        // THREAT DETECTED
        push_action(BLOCK_IP, src_ip, "Unauthorized Modbus write");
    }
}
```

---

#### `void check_port_scan(const std::string& src_ip, uint16_t dst_port)`

Detects port scanning using sliding window algorithm.

**Algorithm:**
- Tracks unique destination ports per source IP
- Threshold: 10 unique ports
- Window: Time-based expiry

**Data Structure:**
```cpp
struct ScanTracker {
    std::unordered_map<uint16_t, int> port_attempts;
    std::chrono::steady_clock::time_point last_seen;
};
std::unordered_map<std::string, ScanTracker> scan_map_;
```

---

#### `bool is_syn_packet(const std::vector<uint8_t>& packet)`

Checks if a packet is a TCP SYN packet.

**Returns:** `true` if SYN flag set and ACK flag not set

---

### Helper Methods

#### `std::string extract_src_ip(const std::vector<uint8_t>& packet)`

Extracts source IP address from packet.

**Returns:** IP address in dotted-decimal notation (e.g., "192.168.1.1")

---

#### `uint16_t extract_dst_port(const std::vector<uint8_t>& packet)`

Extracts destination port from TCP/UDP header.

**Returns:** Port number in host byte order

---

## ⚡ Action Executor

### Class: `ActionExecutor`

**Location:** `include/core/executor.hpp`

**Purpose:** Executes threat mitigation actions (firewall rules, logging).

### Constructor

```cpp
explicit ActionExecutor(ThreadSafeQueue<ActionEvent>& queue);
```

---

### Public Methods

#### `void start()`

Starts the executor and sets system status to "RUNNING".

**Example:**
```cpp
executor.start();
// Now processing actions from queue
```

---

#### `void run()`

Main execution loop. Processes action events.

**Thread Safety:** ✅ Thread-safe  
**JSON Export:** Every 2 seconds or on critical events

---

### Private Methods

#### `void execute_action(const ActionEvent& action)`

Dispatches action based on type.

**Action Types:**
- `BLOCK_IP` - Add firewall rule to block IP
- `UNBLOCK_IP` - Remove firewall rule
- `LOG_SUSPICIOUS` - Log without blocking

---

#### `void block_ip(const std::string& ip)`

Blocks an IP address using OS firewall.

**Linux Command:**
```bash
iptables -A INPUT -s <IP> -j DROP
```

**Windows Command:**
```cmd
netsh advfirewall firewall add rule name="GridWatcher_<IP>" ^
    dir=in interface=any action=block remoteip=<IP>
```

**Example:**
```cpp
executor.block_ip("192.168.1.100");
// IP is now blocked at firewall level
```

---

#### `void unblock_ip(const std::string& ip)`

Removes firewall block for an IP address.

**Linux Command:**
```bash
iptables -D INPUT -s <IP> -j DROP
```

**Windows Command:**
```cmd
netsh advfirewall firewall delete rule name="GridWatcher_<IP>"
```

---

#### `bool is_blocked(const std::string& ip)`

Checks if an IP is currently blocked.

**Returns:** `true` if IP is in blocked set

---

## 🛠️ Utility Classes

### Class: `ThreadSafeQueue<T>`

**Location:** `include/util/ts_queue.hpp`

**Purpose:** Lock-free producer-consumer queue.

#### Methods

```cpp
void push(T&& item);              // Add item to queue
std::optional<T> pop();            // Remove and return item (blocking)
void shutdown();                   // Signal all waiting threads
size_t size() const;              // Current queue size
```

**Example:**
```cpp
ThreadSafeQueue<int> queue;

// Producer
queue.push(42);

// Consumer
auto value = queue.pop();
if (value) {
    std::cout << *value << '\n';
}

// Cleanup
queue.shutdown();
```

---

### Class: `Logger`

**Location:** `include/util/logger.hpp`

**Purpose:** Thread-safe logging with timestamps.

#### Methods

```cpp
static void info(const std::string& msg);
static void warn(const std::string& msg);
static void error(const std::string& msg);
static void critical(const std::string& msg);
```

**Example:**
```cpp
Logger::info("System started");
Logger::warn("High packet loss detected");
Logger::error("Failed to bind socket");
Logger::critical("Security breach detected");
```

**Output Format:**
```
[2025-12-05 10:30:00.123] [INFO] System started
[2025-12-05 10:30:01.456] [WARN] High packet loss detected
```

---

### Class: `JsonExporter`

**Location:** `include/util/json_exporter.hpp`

**Purpose:** Exports system state to JSON for dashboard.

#### Constructor

```cpp
explicit JsonExporter(const std::string& path = "www/dashboard_data.json");
```

#### Methods

```cpp
void set_system_status(const std::string& status);
void increment_blocked();
void add_blocked_ip(const std::string& ip);
void remove_blocked_ip(const std::string& ip);
void set_latest_alert(const std::string& type, const std::string& src_ip, 
                      const std::string& reason);
void increment_threats();
void set_packets_analyzed(int count);
void write();  // Write to disk
```

**Example:**
```cpp
JsonExporter exporter("www/dashboard_data.json");
exporter.set_system_status("RUNNING");
exporter.add_blocked_ip("192.168.1.100");
exporter.increment_threats();
exporter.write();  // Atomic file write
```

---

## 🚀 Launcher Script Generation

### Script: `launcher_gen`

**Location:** `scripts/init.cpp`  
**Purpose:** Generates platform-specific launcher scripts at build time

### Function: `generate_run_file()`

Detects the operating system and creates the appropriate runner script.

**Platforms:**
- **Windows:** Creates `run.bat` in project root
- **Linux:** Creates `run.sh` with executable permissions

#### Windows Script Template

```batch
@echo off
title Grid Watcher Launcher

REM Check binary existence
if not exist "bin\grid_watcher.exe" (
    echo [Error] Binary not found!
    pause
    exit /b
)

REM Launch dashboard in new window
start "GW Dashboard" cmd /k "cd www && python -m http.server 8080"

REM Launch IPS in new window (requires Admin)
start "GW IPS Engine" cmd /k "bin\grid_watcher.exe"
```

#### Linux Script Template

```bash
#!/bin/bash

# Cleanup handler for Ctrl+C
cleanup() {
    echo -e "\n[Info] Shutting down..."
    kill $(jobs -p) 2>/dev/null
    exit
}
trap cleanup SIGINT SIGTERM

# Check binary
if [ ! -f "./bin/grid_watcher" ]; then
    echo "[Error] Binary not found!"
    exit 1
fi

# Start dashboard (background)
(cd www && python3 -m http.server 8080) > /dev/null 2>&1 &

# Start IPS (requires sudo)
sudo ./bin/grid_watcher
```

### Customization Example

To add custom pre-launch checks:

```cpp
// scripts/init.cpp
run_file << R"(
#!/bin/bash

# Custom: Check Python version
if ! command -v python3 &> /dev/null; then
    echo "[Error] Python 3 is required!"
    exit 1
fi

# Custom: Verify network interface
if ! ip link show eth0 &> /dev/null; then
    echo "[Warning] eth0 not found. Using default interface."
fi

# Original launcher logic...
)";
```

**Rebuild Required:**
```bash
cmake --build build --target launcher_gen
./bin/launcher_gen
```

---

## 📊 Data Structures

### `ActionEvent`

**Location:** `include/core/analyzer.hpp`

```cpp
struct ActionEvent {
    ActionType type;        // BLOCK_IP, UNBLOCK_IP, LOG_SUSPICIOUS
    std::string ip_address; // Target IP address
    std::string reason;     // Human-readable reason
};
```

**Example:**
```cpp
ActionEvent event{
    ActionType::BLOCK_IP,
    "192.168.1.100",
    "Port scanning detected"
};
action_queue.push(std::move(event));
```

---

### `ActionType` (Enum)

```cpp
enum class ActionType : uint8_t {
    BLOCK_IP,       // Block source IP address
    LOG_SUSPICIOUS, // Log without blocking
    UNBLOCK_IP      // Remove existing block
};
```

---

### `DashboardAlert`

**Location:** `include/util/json_exporter.hpp`

```cpp
struct DashboardAlert {
    std::string type;      // "PORT_SCAN", "MODBUS_WRITE"
    std::string src_ip;    // Source IP address
    std::string timestamp; // ISO 8601 timestamp
    std::string reason;    // Detection reason
};
```

---

## ⚙️ Configuration

### Compile-Time Configuration

#### Port Scan Threshold

**File:** `src/core/analyzer.cpp`

```cpp
// Line ~95
if(tracker.port_attempts.size() >= 10) {  // Threshold
    // Port scan detected
}
```

**Change to:**
```cpp
constexpr int PORT_SCAN_THRESHOLD = 15;  // More lenient
if(tracker.port_attempts.size() >= PORT_SCAN_THRESHOLD) {
    // Port scan detected
}
```

---

#### Modbus Function Codes

**File:** `src/core/analyzer.cpp`

```cpp
// Line ~85
if(func_code == 0x05 || func_code == 0x06 || func_code == 0x10) {
    // Unauthorized write
}
```

**Add custom codes:**
```cpp
constexpr std::array DANGEROUS_CODES = {0x05, 0x06, 0x0F, 0x10};
if(std::ranges::find(DANGEROUS_CODES, func_code) != DANGEROUS_CODES.end()) {
    // Threat detected
}
```

---

### Runtime Configuration (Future)

Planned features:
- Configuration file (YAML/JSON)
- Dynamic rule updates
- Per-protocol thresholds

---

## 🔌 Integration Examples

### Example 1: Custom Analyzer

```cpp
#include "core/analyzer.hpp"

class CustomAnalyzer : public PacketAnalyzer {
public:
    using PacketAnalyzer::PacketAnalyzer;
    
private:
    void analyze_packet(const std::vector<uint8_t>& packet) override {
        PacketAnalyzer::analyze_packet(packet);  // Call base
        
        // Add custom logic
        if (is_dnp3_packet(packet)) {
            check_dnp3_anomalies(packet);
        }
    }
    
    bool is_dnp3_packet(const std::vector<uint8_t>& packet) {
        uint16_t port = extract_dst_port(packet);
        return port == 20000;  // DNP3 default port
    }
};
```

---

### Example 2: Custom Logger

```cpp
#include "util/logger.hpp"
#include <fstream>

class FileLogger {
    std::ofstream log_file_;
    std::mutex mtx_;
    
public:
    FileLogger(const std::string& path) 
        : log_file_(path, std::ios::app) {}
    
    void log(const std::string& msg) {
        std::lock_guard lock(mtx_);
        log_file_ << Logger::timestamp() << " " << msg << '\n';
        log_file_.flush();
    }
};
```

---

### Example 3: SIEM Integration

```cpp
class SIEMExporter {
    std::string siem_endpoint_;
    
public:
    void export_event(const ActionEvent& event) {
        nlohmann::json j;
        j["timestamp"] = get_iso8601_time();
        j["source_ip"] = event.ip_address;
        j["action"] = action_type_to_string(event.type);
        j["reason"] = event.reason;
        j["severity"] = calculate_severity(event);
        
        send_to_siem(siem_endpoint_, j.dump());
    }
    
private:
    int calculate_severity(const ActionEvent& event) {
        if (event.reason.find("Modbus") != std::string::npos) {
            return 10;  // Critical
        }
        return 7;  // High
    }
};
```

---

## 📈 Performance Characteristics

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| Packet Capture | O(1) | O(n) buffers |
| IP Extraction | O(1) | O(1) |
| Port Scan Detection | O(m) | O(n×m) |
| Firewall Execution | O(1) | O(n) blocked IPs |
| JSON Export | O(n) | O(n) |

*Where n = number of unique IPs, m = ports per IP*

---

## 🔒 Thread Safety Guarantees

| Component | Thread Safety | Synchronization |
|-----------|--------------|-----------------|
| `ThreadSafeQueue` | ✅ Fully safe | Mutex + CV |
| `PacketSniffer` | ✅ Safe | Atomic flags |
| `PacketAnalyzer` | ✅ Safe | Per-thread state |
| `ActionExecutor` | ✅ Safe | Mutex for shared state |
| `Logger` | ✅ Safe | Static mutex |
| `JsonExporter` | ✅ Safe | Member mutex |

---

## 📚 Further Reading

- [ARCHITECTURE.md](ARCHITECTURE.md) - System design details
- [CONTRIBUTING.md](../CONTRIBUTING.md) - Code contribution guidelines
- [QUICKSTART.md](QUICKSTART.md) - Getting started guide

---

## 📞 API Support

For API questions or integration support:
- **Email:** zuudevs@gmail.com
- **GitHub Issues:** Tag with `api` label
- **Documentation:** This file is updated with each release

---

**Last Updated:** December 2025  
**API Version:** 1.0.0  
**Stability:** Stable