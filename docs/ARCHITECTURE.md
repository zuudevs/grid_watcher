# System Architecture Documentation

**Grid Watcher IPS - Version 1.0.0**

This document provides an in-depth technical overview of Grid Watcher's architecture, design decisions, and implementation details.

---

## 📋 Table of Contents

- [High-Level Overview](#high-level-overview)
- [Component Architecture](#component-architecture)
- [Data Flow](#data-flow)
- [Concurrency Model](#concurrency-model)
- [Platform Differences](#platform-differences)
- [Performance Characteristics](#performance-characteristics)
- [Security Architecture](#security-architecture)
- [Deployment Models](#deployment-models)

---

## 🏗️ High-Level Overview

Grid Watcher uses a **multi-threaded pipeline architecture** based on the Producer-Consumer pattern. This design ensures high throughput, low latency, and clean separation of concerns.

### System Architecture Diagram

```mermaid
graph TB
    subgraph Network["Network Layer"]
        NIC[Network Interface Card]
        Traffic[Raw Network Traffic]
    end
    
    subgraph Capture["Capture Layer (Thread 1)"]
        RS[Raw Socket<br/>AF_PACKET / AF_INET]
        Sniffer[Packet Sniffer<br/>65KB Buffer]
    end
    
    subgraph Queue1["Thread-Safe Queue 1"]
        PQ[(Packet Queue<br/>std::queue<vector<uint8_t>>)]
    end
    
    subgraph Analysis["Analysis Layer (Thread 2)"]
        Analyzer[Packet Analyzer<br/>Deep Inspection]
        ModbusCheck{Modbus TCP<br/>Port 502?}
        ScanCheck{Port Scan<br/>Pattern?}
        IPExtract[IP/Port Extraction]
    end
    
    subgraph Queue2["Thread-Safe Queue 2"]
        AQ[(Action Queue<br/>std::queue<ActionEvent>)]
    end
    
    subgraph Execute["Execution Layer (Thread 3)"]
        Executor[Action Executor]
        Firewall{Action Type?}
        Block[Block IP<br/>iptables/netsh]
        Log[Log Event]
        Unblock[Unblock IP]
    end
    
    subgraph Export["Export Layer"]
        JSON[JSON Exporter<br/>Atomic Write]
        Dashboard[Web Dashboard<br/>Real-time UI]
    end
    
    Traffic --> NIC
    NIC --> RS
    RS --> Sniffer
    Sniffer -->|Push| PQ
    
    PQ -->|Pop| Analyzer
    Analyzer --> IPExtract
    IPExtract --> ModbusCheck
    IPExtract --> ScanCheck
    
    ModbusCheck -->|Threat| AQ
    ScanCheck -->|Threat| AQ
    
    AQ -->|Pop| Executor
    Executor --> Firewall
    
    Firewall -->|BLOCK_IP| Block
    Firewall -->|LOG_SUSPICIOUS| Log
    Firewall -->|UNBLOCK_IP| Unblock
    
    Executor --> JSON
    JSON -->|Poll Every 1s| Dashboard
    
    style NIC fill:#f9f,stroke:#333,stroke-width:2px
    style Sniffer fill:#bbf,stroke:#333,stroke-width:2px
    style Analyzer fill:#bfb,stroke:#333,stroke-width:2px
    style Executor fill:#fbb,stroke:#333,stroke-width:2px
    style Block fill:#f66,stroke:#333,stroke-width:3px,color:#fff
    style Dashboard fill:#6cf,stroke:#333,stroke-width:2px
```

---

## 🧩 Component Architecture

### 1. Packet Sniffer (Producer)

**Responsibility:** Capture raw network packets from the NIC

**Key Characteristics:**
- **Non-blocking:** Uses raw sockets with minimal processing
- **Buffer Size:** 65536 bytes (1 MTU)
- **Platform-Specific:** Different implementations for Linux/Windows

**Class Diagram:**

```mermaid
classDiagram
    class PacketSniffer {
        -ThreadSafeQueue<vector<uint8_t>>& packet_queue_
        -atomic<bool> running_
        -int socket_fd_
        +PacketSniffer(queue)
        +start() void
        +stop() void
        +run() void
        -init_socket() bool
        -cleanup_socket() void
    }
    
    class ThreadSafeQueue~T~ {
        -queue<T> queue_
        -mutex mtx_
        -condition_variable cv_
        -bool shutdown_
        +push(T&&) void
        +pop() optional<T>
        +shutdown() void
        +size() size_t
    }
    
    PacketSniffer --> ThreadSafeQueue : uses
```

**Pseudo-code:**
```cpp
void PacketSniffer::run() {
    vector<uint8_t> buffer(65536);
    
    while (running_) {
        ssize_t bytes = recv(socket_fd_, buffer.data(), buffer.size(), 0);
        
        if (bytes > 0) {
            vector<uint8_t> packet(buffer.begin(), buffer.begin() + bytes);
            packet_queue_.push(std::move(packet));  // Zero-copy move
        }
    }
}
```

---

### 2. Packet Analyzer (Processor)

**Responsibility:** Perform deep packet inspection and threat detection

**Detection Algorithms:**

#### A. Modbus TCP Detection

**Flow Diagram:**

```mermaid
flowchart TD
    Start([Packet Received])
    CheckPort{Dst Port<br/>== 502?}
    CheckProto{Protocol<br/>== TCP?}
    ExtractFC["Extract Function Code<br/>MBAP byte 7"]
    CheckFC{"FC == 0x05<br/>OR 0x06<br/>OR 0x10?"}
    Block[Generate BLOCK_IP Action]
    Pass[Pass Through]
    
    Start --> CheckPort
    CheckPort -->|No| Pass
    CheckPort -->|Yes| CheckProto
    CheckProto -->|No| Pass
    CheckProto -->|Yes| ExtractFC
    ExtractFC --> CheckFC
    CheckFC -->|Yes| Block
    CheckFC -->|No| Pass
    
    style Block fill:#f66,color:#fff
    style Pass fill:#6f6
```

**Modbus Packet Structure:**
```
┌──────────────────────────────────────┐
│  Ethernet Header (14 bytes)          │
├──────────────────────────────────────┤
│  IP Header (20+ bytes)                │
├──────────────────────────────────────┤
│  TCP Header (20+ bytes)               │
├──────────────────────────────────────┤
│  MBAP Header (7 bytes)                │
│  ┌─────────────────────────────────┐ │
│  │ Transaction ID   (2 bytes)      │ │
│  │ Protocol ID      (2 bytes)      │ │
│  │ Length           (2 bytes)      │ │
│  │ Unit ID          (1 byte)       │ │
│  └─────────────────────────────────┘ │
├──────────────────────────────────────┤
│  Function Code (1 byte) ◄─── CHECK  │
│  Data...                              │
└──────────────────────────────────────┘
```

---

#### B. Port Scan Detection

**Algorithm: Sliding Window with Threshold**

```mermaid
sequenceDiagram
    participant Attacker
    participant Analyzer
    participant ScanMap
    participant ActionQueue
    
    Attacker->>Analyzer: SYN to port 8001
    Analyzer->>ScanMap: Track 192.168.1.100:8001
    Note over ScanMap: Unique ports: 1
    
    Attacker->>Analyzer: SYN to port 8002
    Analyzer->>ScanMap: Track 192.168.1.100:8002
    Note over ScanMap: Unique ports: 2
    
    Note over Attacker,ScanMap: ... (continues)
    
    Attacker->>Analyzer: SYN to port 8010
    Analyzer->>ScanMap: Track 192.168.1.100:8010
    Note over ScanMap: Unique ports: 10
    
    ScanMap->>ScanMap: Threshold exceeded!
    ScanMap->>ActionQueue: BLOCK_IP(192.168.1.100)
    Note over ActionQueue: Action queued for execution
```

**Data Structure:**
```cpp
struct ScanTracker {
    unordered_map<uint16_t, int> port_attempts;  // port -> count
    chrono::steady_clock::time_point last_seen;
};

unordered_map<string, ScanTracker> scan_map_;  // ip -> tracker
```

**Complexity:**
- **Time:** O(1) insert, O(m) check (m = ports attempted)
- **Space:** O(n×m) where n = unique IPs, m = ports per IP

---

### 3. Action Executor (Consumer)

**Responsibility:** Execute threat mitigation actions

**State Machine:**

```mermaid
stateDiagram-v2
    [*] --> Idle
    
    Idle --> Processing : Pop Action from Queue
    
    Processing --> CheckType : Determine Action Type
    
    CheckType --> BlockIP : BLOCK_IP
    CheckType --> UnblockIP : UNBLOCK_IP
    CheckType --> LogSuspicious : LOG_SUSPICIOUS
    
    BlockIP --> CheckBlocked : Is IP already blocked?
    CheckBlocked --> ExecuteBlock : No
    CheckBlocked --> Idle : Yes (skip)
    
    ExecuteBlock --> UpdateFirewall : Run iptables/netsh
    UpdateFirewall --> UpdateJSON : Add to blocked_list
    UpdateJSON --> LogEvent : Write to dashboard_data.json
    LogEvent --> Idle
    
    UnblockIP --> RemoveFirewall : Delete firewall rule
    RemoveFirewall --> UpdateJSON2 : Remove from blocked_list
    UpdateJSON2 --> Idle
    
    LogSuspicious --> LogEvent2 : Write to JSON
    LogEvent2 --> Idle
    
    Idle --> [*] : Shutdown signal
```

---

## 🔄 Data Flow

### Complete Pipeline Flow

```mermaid
graph LR
    subgraph Input
        A[Raw Packet<br/>65KB Buffer]
    end
    
    subgraph Stage1["Stage 1: Capture"]
        B[Copy to Queue<br/>std::move]
    end
    
    subgraph Stage2["Stage 2: Parse"]
        C[Extract Headers]
        D[Parse IP]
        E[Parse TCP/UDP]
    end
    
    subgraph Stage3["Stage 3: Analyze"]
        F{Check Port 502}
        G{Check SYN Pattern}
    end
    
    subgraph Stage4["Stage 4: Decide"]
        H[Generate Action]
        I[Push to Action Queue]
    end
    
    subgraph Stage5["Stage 5: Execute"]
        J[Pop Action]
        K[Execute Command]
        L[Update JSON]
    end
    
    subgraph Output
        M[Dashboard Update]
        N[Firewall Rule]
    end
    
    A --> B
    B --> C
    C --> D
    D --> E
    E --> F
    E --> G
    F --> H
    G --> H
    H --> I
    I --> J
    J --> K
    K --> L
    L --> M
    K --> N
    
    style A fill:#f9f
    style H fill:#ff6
    style K fill:#f66,color:#fff
    style M fill:#6cf
```

---

## ⚙️ Concurrency Model

### Thread Architecture

```mermaid
gantt
    title Grid Watcher Thread Timeline
    dateFormat YYYY-MM-DD
    section Main Thread
    Initialize Components    :a1, 2025-01-01, 1d
    Start Threads           :a2, after a1, 1d
    Monitor Signals         :a3, after a2, 30d
    
    section Sniffer Thread
    Capture Packets         :b1, 2025-01-02, 30d
    
    section Analyzer Thread
    Analyze Packets         :c1, 2025-01-02, 30d
    
    section Executor Thread
    Execute Actions         :d1, 2025-01-02, 30d
```

### Thread Communication

```mermaid
sequenceDiagram
    participant Main
    participant Sniffer
    participant PQueue as Packet Queue
    participant Analyzer
    participant AQueue as Action Queue
    participant Executor
    
    Main->>Sniffer: start()
    Main->>Analyzer: start()
    Main->>Executor: start()
    
    activate Sniffer
    activate Analyzer
    activate Executor
    
    loop Capture Loop
        Sniffer->>PQueue: push(packet)
    end
    
    loop Analysis Loop
        PQueue->>Analyzer: pop()
        Analyzer->>Analyzer: detect threats
        Analyzer->>AQueue: push(action)
    end
    
    loop Execution Loop
        AQueue->>Executor: pop()
        Executor->>Executor: execute action
        Executor->>Executor: update JSON
    end
    
    Main->>Main: Ctrl+C Signal
    Main->>Sniffer: stop()
    Main->>PQueue: shutdown()
    Main->>Analyzer: stop()
    Main->>AQueue: shutdown()
    Main->>Executor: stop()
    
    deactivate Sniffer
    deactivate Analyzer
    deactivate Executor
```

### Synchronization Primitives

| Component | Primitive | Purpose |
|-----------|-----------|---------|
| ThreadSafeQueue | `std::mutex` | Protect queue operations |
| ThreadSafeQueue | `std::condition_variable` | Block on empty queue |
| ActionExecutor | `std::mutex` | Protect blocked IP set |
| Logger | `static std::mutex` | Serialize log writes |
| JsonExporter | `std::mutex` | Atomic JSON writes |

---

## 🖥️ Platform Differences

### Linux vs Windows

```mermaid
graph TB
    subgraph Linux["Linux Implementation"]
        L1[AF_PACKET Socket]
        L2[Full Ethernet Frame<br/>14-byte header]
        L3[Promiscuous Mode<br/>Fully Supported]
        L4[iptables Commands]
        L5[CAP_NET_RAW Capability]
    end
    
    subgraph Windows["Windows Implementation"]
        W1[AF_INET Socket]
        W2[IP Packet Only<br/>No Ethernet header]
        W3[Promiscuous Mode<br/>Limited by OS]
        W4[netsh Commands]
        W5[Administrator Privileges]
    end
    
    subgraph Common["Common Abstraction"]
        C1[PacketSniffer Interface]
        C2[Same Analysis Logic]
        C3[Same Threat Detection]
    end
    
    L1 --> C1
    W1 --> C1
    C1 --> C2
    C2 --> C3
    
    style L1 fill:#6f6
    style W1 fill:#66f,color:#fff
    style C1 fill:#ff6
```

**Header Offset Handling:**
```cpp
static constexpr size_t get_ip_offset() {
#ifdef _WIN32
    return 0;  // Windows: starts at IP header
#else
    return 14; // Linux: skip Ethernet header
#endif
}
```

---

## ⚡ Performance Characteristics

### Throughput Analysis

```mermaid
graph LR
    A[Network<br/>100K pps] -->|Captured| B[Sniffer<br/>95K pps]
    B -->|Queued| C[Analyzer<br/>90K pps]
    C -->|Filtered| D[Executor<br/>5-10 events/s]
    
    style A fill:#f9f
    style B fill:#bbf
    style C fill:#bfb
    style D fill:#fbb
```

**Bottleneck Analysis:**
- **Sniffer:** Limited by kernel socket buffer
- **Analyzer:** CPU-bound (packet parsing)
- **Executor:** I/O-bound (firewall commands)

### Memory Profile

```mermaid
pie title Memory Distribution
    "Packet Buffers" : 30
    "Scan Tracker" : 25
    "Blocked IP Set" : 10
    "Code & Stack" : 20
    "JSON Exporter" : 10
    "Other" : 5
```

**Total:** ~50 MB typical, ~100 MB peak

---

## 🔒 Security Architecture

### Privilege Separation

```mermaid
graph TD
    A[User Space<br/>Non-privileged]
    B[Kernel Space<br/>Privileged]
    
    C[Dashboard<br/>HTTP Server]
    D[Grid Watcher<br/>Raw Sockets]
    
    E[iptables/netsh<br/>Firewall]
    F[Network Stack<br/>Packet Capture]
    
    A --> C
    A --> D
    D --> B
    B --> E
    B --> F
    
    style A fill:#6f6
    style B fill:#f66,color:#fff
    style D fill:#ff6
```

### Attack Surface

| Component | Risk | Mitigation |
|-----------|------|------------|
| Raw Socket | ⚠️ High privilege | Use capabilities, not root |
| Firewall Commands | ⚠️ System modification | Log all commands, require confirmation |
| Dashboard | ⚠️ No authentication | Deploy behind reverse proxy |
| JSON Export | ℹ️ File write | Atomic write, limited path |

---

## 🚀 Deployment Models

### Model 1: Standalone Host-Based IPS

```mermaid
graph TB
    SCADA[SCADA Host]
    GW[Grid Watcher IPS]
    FW[OS Firewall]
    Net[Network]
    
    Net -->|All Traffic| SCADA
    SCADA -->|Monitor| GW
    GW -->|Block| FW
    FW -->|Drop| Net
    
    style SCADA fill:#f9f
    style GW fill:#6f6
    style FW fill:#f66,color:#fff
```

**Pros:**
- No additional hardware
- Direct packet access
- Low latency

**Cons:**
- Single point of failure
- Resource sharing with SCADA

---

### Model 2: Gateway/Jump Server Mode

```mermaid
graph LR
    Internet[Internet]
    GW[Grid Watcher<br/>Gateway]
    SCADA1[SCADA Server 1]
    SCADA2[SCADA Server 2]
    SCADA3[SCADA Server 3]
    
    Internet -->|Traffic| GW
    GW -->|Filtered| SCADA1
    GW -->|Filtered| SCADA2
    GW -->|Filtered| SCADA3
    
    style GW fill:#6f6
    style SCADA1 fill:#f9f
    style SCADA2 fill:#f9f
    style SCADA3 fill:#f9f
```

**Pros:**
- Centralized protection
- Dedicated resources
- Easier management

**Cons:**
- Additional infrastructure
- Potential bottleneck

---

## 📊 Design Principles

### 1. Fail-Safe Over Fail-Open

**Principle:** In case of failure, block traffic rather than allow it.

**Implementation:**
- Default firewall policy: DROP
- Explicit allow rules for known-good traffic
- Watchdog timer for IPS health

---

### 2. Deterministic Behavior

**Principle:** No AI/ML in critical path; predictable, rule-based detection.

**Rationale:**
- SCADA requires high reliability
- False positives unacceptable
- Auditable decision-making

---

### 3. Performance First

**Principle:** Minimize latency and resource usage.

**Optimizations:**
- Zero-copy packet handling
- Lock-free where possible
- Efficient data structures

---

## 🧪 Testing Strategy

### Unit Tests

```mermaid
graph TD
    A[Test Packet Parser]
    B[Test Modbus Detection]
    C[Test Port Scan Logic]
    D[Test Firewall Commands<br/>Mock]
    
    A --> E[CI/CD Pipeline]
    B --> E
    C --> E
    D --> E
    
    style E fill:#6f6
```

### Integration Tests

```mermaid
sequenceDiagram
    participant Test
    participant IPS
    participant Simulator
    
    Test->>IPS: Start IPS
    Test->>Simulator: Send port scan
    Simulator->>IPS: 15 SYN packets
    IPS->>IPS: Detect threat
    IPS->>Test: Check blocked_list
    Test->>Test: Assert IP blocked
```

---

## 📚 Further Reading

- [QUICKSTART.md](QUICKSTART.md) - Getting started guide
- [API.md](API.md) - Detailed API reference
- [ROADMAP.md](ROADMAP.md) - Future development plans

---

**Document Version:** 1.0.0  
**Last Updated:** December 2025  
**Maintained By:** Zuudevs
