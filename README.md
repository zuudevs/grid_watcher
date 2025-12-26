# Grid Watcher IPS 🛡️

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++ Version](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://isocpp.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey.svg)]()
[![SCADA](https://img.shields.io/badge/SCADA-Modbus%20TCP-orange.svg)]()
[![GitHub Repo stars](https://img.shields.io/github/stars/zuudevs/grid_watcher?style=social)](https://github.com/zuudevs/grid_watcher)

> **Modern C++23 Intrusion Prevention System designed specifically for SCADA infrastructure protection**

Grid Watcher is a lightweight, high-performance network security solution that provides real-time threat detection and automated response capabilities for industrial control systems. Built with modern C++23, it combines deep packet inspection, protocol analysis, and active defense mechanisms to protect critical infrastructure.

---

## 🎯 Key Features

### 🏭 **SCADA-Specific Protection**
- **Modbus TCP Protocol Analysis** - Detects unauthorized write commands (`0x05`, `0x06`, `0x10`)
- **Industrial Protocol Awareness** - Understands SCADA communication patterns
- **Zero False Positives** - Precision-tuned for industrial environments

### 🔍 **Advanced Threat Detection**
- **Port Scan Detection** - Identifies horizontal and vertical scanning patterns
- **SYN Flood Recognition** - Detects TCP-based reconnaissance attempts
- **Real-time Deep Packet Inspection** - Zero-copy packet analysis for minimal overhead

### ⚡ **Active Defense Mechanisms**
- **Automatic IP Blocking** - Instant firewall integration (`iptables`/`netsh`)
- **Multi-threaded Pipeline** - Non-blocking packet capture and analysis
- **Sub-millisecond Response Time** - Critical for time-sensitive SCADA operations

### 📊 **Real-time Monitoring Dashboard**
- **Web-based Command Center** - HTML5/JavaScript interface
- **Live Threat Visualization** - Real-time attack feed and metrics
- **System Health Monitoring** - Uptime, throughput, and performance stats

---

## 🚀 Quick Start

```bash
# Clone the repository
git clone https://github.com/zuudevs/grid_watcher.git
cd grid_watcher

# Build the project (automatically generates run.sh/run.bat)
cmake -S . -B build
cmake --build build --config Release

# Launch everything with one command!
# Linux:
./run.sh

# Windows:
run.bat

# Or manually:
sudo ./bin/grid_watcher  # IPS Engine
cd www && python -m http.server 8080  # Dashboard
```

**Dashboard Access:** `http://localhost:8080`

---

## 💻 Architecture Overview

Grid Watcher uses a **Producer-Consumer Pipeline** with three independent threads:

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐
│   Packet    │ ───► │   Analyzer   │ ───► │  Executor   │
│   Sniffer   │      │   (DPI)      │      │  (Firewall) │
└─────────────┘      └──────────────┘      └─────────────┘
      │                     │                      │
   Raw Socket         Threat Logic          OS Firewall
   (AF_PACKET)        Detection              Commands
```

For detailed architecture documentation, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

---

## 📦 Requirements

### System Requirements
- **Operating System:** Linux (Production) / Windows (Development Mode)
- **Privileges:** Root (Linux) / Administrator (Windows)
- **Network:** Raw socket capability

### Build Requirements
- **C++ Compiler:** GCC 13+, Clang 16+, or MSVC 2022+
- **Build System:** CMake 3.20+
- **Standard:** C++23 with concepts and ranges support

### Runtime Dependencies
- **Linux:** `iptables` (for firewall integration)
- **Windows:** Windows Firewall with Advanced Security
- **Dashboard:** Python 3.x (for local HTTP server)

---

## 🔧 Building from Source

Detailed build instructions are available in [BUILD.md](BUILD.md).

### Quick Build

**Linux:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

**Windows (MSVC):**
```cmd
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

---

## 📖 Documentation

| Document | Description |
|----------|-------------|
| [QUICKSTART.md](docs/QUICKSTART.md) | 5-minute tutorial and basic usage |
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | System design and technical details |
| [API.md](docs/API.md) | Class and function reference |
| [BUILD.md](BUILD.md) | Comprehensive build instructions |
| [CONTRIBUTING.md](CONTRIBUTING.md) | How to contribute to the project |
| [ROADMAP.md](docs/ROADMAP.md) | Future features and development plans |

---

## 🎮 Usage Examples

### Basic Monitoring
```bash
# Start the IPS engine
sudo ./bin/grid_watcher

# In another terminal, start the dashboard
cd www && python -m http.server 8080
```

### Attack Simulation (Testing)
```bash
# Run the built-in attack simulator
./bin/attack_sim 192.168.1.100
```

### Custom Configuration
```cpp
// Configure port scan threshold
constexpr int PORT_SCAN_THRESHOLD = 10;

// Configure Modbus protection
std::vector<uint8_t> protected_function_codes = {0x05, 0x06, 0x10};
```

---

## 🔒 Security Considerations

- **Raw Socket Access:** Requires elevated privileges
- **Firewall Modifications:** Automatically modifies system firewall rules
- **Network Visibility:** Captures all network traffic on the interface
- **Production Deployment:** Test thoroughly in staging environment first

For security policy and vulnerability reporting, see [SECURITY.md](SECURITY.md).

---

## 🤝 Contributing

We welcome contributions! Please read our [Contributing Guidelines](CONTRIBUTING.md) and [Code of Conduct](CODE_OF_CONDUCT.md) before submitting pull requests.

### Development Workflow
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📊 Performance

| Metric | Performance |
|--------|-------------|
| Packet Processing Rate | ~50,000 pps |
| Memory Footprint | <50 MB |
| CPU Usage | <5% (idle), <15% (high load) |
| Detection Latency | <1ms |
| Platform Overhead | Minimal (raw sockets) |

*Benchmarked on Intel i7-12700K, 32GB RAM, Linux 6.x kernel*

---

## ⚠️ Disclaimer

This software is designed for **authorized security testing and legitimate network monitoring** of SCADA infrastructure. Users are responsible for ensuring compliance with applicable laws and regulations. The authors assume no liability for misuse or damages resulting from the use of this software.

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```
Copyright (c) 2025 Rafi Indra Pramudhito Zuhayr (Zuudevs)
```

---

## 👨‍💻 Author

**Rafi Indra Pramudhito Zuhayr** (Zuudevs)
- GitHub: [@zuudevs](https://github.com/zuudevs)
- Email: zuudevs@gmail.com

---

## 🌟 Acknowledgments

- SCADA security research community
- Industrial Control Systems security standards (IEC 62443)
- Modbus protocol specification contributors

---

## 📈 Project Status

**Current Version:** 1.0.0  
**Status:** Active Development  
**Last Updated:** December 2025

For planned features and roadmap, see [ROADMAP.md](docs/ROADMAP.md).

---

<p align="center">
  <strong>⚡ Protect Your Infrastructure with Modern C++ ⚡</strong>
</p>

<p align="center">
  <a href="https://github.com/zuudevs/grid_watcher/issues">Report Bug</a> •
  <a href="https://github.com/zuudevs/grid_watcher/issues">Request Feature</a> •
  <a href="docs/QUICKSTART.md">Documentation</a>
</p>
