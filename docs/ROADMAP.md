# Grid Watcher IPS - Development Roadmap

This document outlines the planned features, improvements, and long-term vision for Grid Watcher IPS.

---

## 🎯 Vision Statement

**Mission:** Build the most comprehensive, performant, and user-friendly open-source IPS specifically designed for industrial control systems and SCADA infrastructure.

**Core Values:**
- 🛡️ Security First
- ⚡ Performance & Efficiency
- 🔧 Developer-Friendly
- 📖 Documentation Excellence
- 🌍 Open Source Community

---

## 📅 Release Timeline

### Version 1.0.0 (Current) ✅
**Released:** December 2025  
**Status:** Stable

**Features:**
- ✅ Multi-threaded packet capture (Linux/Windows)
- ✅ Modbus TCP protocol analysis
- ✅ Port scan detection
- ✅ Automatic IP blocking (iptables/netsh)
- ✅ Real-time web dashboard
- ✅ JSON logging and export
- ✅ Cross-platform support

---

### Version 1.1.0 (Q1 2026) 🚧
**Target:** February 2026  
**Focus:** Enhanced Detection & Configuration

#### Planned Features

**1. Configuration File Support** 🔧
- YAML-based configuration
- Runtime rule updates without restart
- Per-protocol threshold customization

```yaml
# config.yaml
detection:
  port_scan:
    threshold: 15
    window_seconds: 30
  
  modbus:
    protected_function_codes: [0x05, 0x06, 0x0F, 0x10]
    monitor_only_mode: false
```

**Status:** 🟡 Planning  
**Assignee:** TBD  
**Issue:** [#5](https://github.com/zuudevs/grid_watcher/issues/5)

---

**2. Rate-Based Anomaly Detection** 📊
- Detect excessive packet rates from single source
- Configurable thresholds (pps, Mbps)
- Time-window based analysis

**Algorithm:**
```cpp
if (packets_per_second > THRESHOLD) {
    classify_as_dos_attack();
}
```

**Status:** 🟡 Research  
**Assignee:** Community  
**Issue:** [#12](https://github.com/zuudevs/grid_watcher/issues/12)

---

**3. IPv6 Support** 🌐
- Full IPv6 packet parsing
- IPv6 address blocking
- Dual-stack operation

**Status:** 🟢 Ready to implement  
**Assignee:** TBD  
**Issue:** [#8](https://github.com/zuudevs/grid_watcher/issues/8)

---

**4. Persistent Blocklist** 💾
- SQLite database for blocked IPs
- Survives application restart
- Expiration policies

**Schema:**
```sql
CREATE TABLE blocked_ips (
    ip_address TEXT PRIMARY KEY,
    blocked_at TIMESTAMP,
    reason TEXT,
    expires_at TIMESTAMP
);
```

**Status:** 🟢 Design complete  
**Issue:** [#15](https://github.com/zuudevs/grid_watcher/issues/15)

---

### Version 1.2.0 (Q2 2026) 🚀
**Target:** May 2026  
**Focus:** Additional SCADA Protocols

#### Planned Features

**1. DNP3 Protocol Support** 🏭
- Deep packet inspection for DNP3
- Function code validation
- Unauthorized write detection

**DNP3 Ports:**
- TCP: 20000
- UDP: 20000

**Status:** 🟡 Research Phase  
**Complexity:** High  
**Issue:** [#20](https://github.com/zuudevs/grid_watcher/issues/20)

---

**2. IEC 60870-5-104 Support** ⚡
- European standard for power systems
- ASDU type monitoring
- Command validation

**IEC 104 Port:** TCP 2404

**Status:** 🔴 Not started  
**Issue:** [#22](https://github.com/zuudevs/grid_watcher/issues/22)

---

**3. OPC UA Security** 🔐
- OPC UA session monitoring
- Certificate validation
- Anomaly detection in method calls

**Status:** 🔴 Research needed  
**Issue:** [#25](https://github.com/zuudevs/grid_watcher/issues/25)

---

### Version 1.3.0 (Q3 2026) 📡
**Target:** August 2026  
**Focus:** Advanced Features & Integration

#### Planned Features

**1. Machine Learning Anomaly Detection** 🤖
- Baseline normal traffic patterns
- Detect deviations using ML
- Auto-tuning thresholds

**Technology Stack:**
- TensorFlow Lite C++ API
- Lightweight model (~5MB)
- Real-time inference (<10ms)

**Status:** 🔴 Experimental  
**Issue:** [#30](https://github.com/zuudevs/grid_watcher/issues/30)

---

**2. SIEM Integration** 📤
- Syslog export
- Splunk connector
- ELK Stack support

**Supported Formats:**
- CEF (Common Event Format)
- LEEF (Log Event Extended Format)
- JSON over HTTP

**Status:** 🟡 Design Phase  
**Issue:** [#32](https://github.com/zuudevs/grid_watcher/issues/32)

---

**3. Distributed Deployment** 🌐
- Multi-sensor architecture
- Central management console
- Aggregated threat intelligence

```
┌─────────┐     ┌─────────┐     ┌─────────┐
│ Sensor1 │────►│ Central │◄────│ Sensor2 │
└─────────┘     │ Manager │     └─────────┘
                └─────────┘
                     │
                     ▼
                ┌─────────┐
                │Dashboard│
                └─────────┘
```

**Status:** 🔴 Design Phase  
**Issue:** [#35](https://github.com/zuudevs/grid_watcher/issues/35)

---

### Version 2.0.0 (Q1 2027) 🎉
**Target:** February 2027  
**Focus:** Major Architecture Overhaul

#### Planned Features

**1. Plugin Architecture** 🔌
- Dynamic protocol loading
- Third-party plugin support
- Hot-reload capabilities

**Plugin Interface:**
```cpp
class IProtocolPlugin {
public:
    virtual ~IProtocolPlugin() = default;
    virtual void analyze(const Packet& pkt) = 0;
    virtual std::string name() const = 0;
    virtual uint16_t port() const = 0;
};
```

**Status:** 🔴 Concept Phase

---

**2. Enhanced Dashboard** 💎
- React/TypeScript rewrite
- Real-time WebSocket updates
- Advanced analytics and graphs

**Features:**
- Live packet capture viewer
- Interactive threat timeline
- Exportable reports (PDF/Excel)

**Status:** 🟡 Mockups in progress

---

**3. Performance Optimizations** ⚡
- eBPF/XDP integration (Linux)
- Zero-copy packet processing
- DPDK support for high-throughput

**Target Performance:**
- 1M+ packets per second
- <100μs detection latency
- <10% CPU usage

**Status:** 🔴 Research Phase

---

## 🎨 Feature Requests from Community

Vote on these features in [GitHub Discussions](https://github.com/zuudevs/grid_watcher/discussions):

### High Priority (Community Votes > 10)

1. **Whitelist Management** (15 votes)
   - Exclude trusted IPs from detection
   - CIDR range support
   - Time-based whitelisting

2. **Email Notifications** (12 votes)
   - SMTP integration
   - Customizable alert templates
   - Rate limiting to avoid spam

3. **Mobile App** (11 votes)
   - iOS/Android companion app
   - Push notifications
   - Remote control capabilities

### Medium Priority (5-10 votes)

4. **Packet Capture Export** (8 votes)
   - PCAP file generation
   - Wireshark integration
   - Forensic analysis support

5. **Geolocation Blocking** (7 votes)
   - Block by country
   - GeoIP database integration
   - Visualize attack sources on map

6. **VPN Detection** (6 votes)
   - Identify VPN/Proxy traffic
   - Tor exit node blocking
   - Anonymizer detection

---

## 🐛 Known Issues & Technical Debt

### Critical
- [ ] **Windows Compatibility Mode Limitations** [#40]
  - Promiscuous mode not fully functional
  - Solution: Implement WinPcap/Npcap driver

### High
- [ ] **Memory Leak in Long-Running Sessions** [#42]
  - Scan tracker not cleaning up expired entries
  - Solution: Add periodic cleanup timer

- [ ] **Race Condition in Queue Shutdown** [#43]
  - Potential deadlock on rapid start/stop
  - Solution: Use `std::stop_token` (C++20)

### Medium
- [ ] **Dashboard Refresh Lag** [#45]
  - 1-second polling creates UI jank
  - Solution: Implement WebSocket updates

### Low
- [ ] **Log Rotation Not Implemented** [#48]
  - Logs can grow indefinitely
  - Solution: Add `logrotate` configuration

---

## 🧪 Experimental Ideas

These are exploratory concepts that may or may not make it into releases:

### 1. Hardware Acceleration
- FPGA-based packet filtering
- GPU-accelerated pattern matching
- ASIC integration for industrial appliances

### 2. Blockchain-Based Threat Intelligence
- Decentralized threat database
- Immutable audit logs
- Smart contract-based rule updates

### 3. Quantum-Resistant Cryptography
- Post-quantum algorithms for secure communication
- Future-proof dashboard authentication

---

## 📊 Performance Goals

| Metric | v1.0 (Current) | v1.5 (Target) | v2.0 (Goal) |
|--------|----------------|---------------|-------------|
| **Packet Rate** | 50K pps | 200K pps | 1M+ pps |
| **Memory Usage** | 50 MB | 100 MB | 200 MB |
| **CPU Usage (Idle)** | 5% | 3% | <1% |
| **Detection Latency** | <1ms | <500μs | <100μs |
| **Protocols Supported** | 1 (Modbus) | 3 | 10+ |
| **Platform Support** | 2 (Linux/Win) | 3 (+macOS) | 5 (+BSD/ARM) |

---

## 🤝 How to Contribute to the Roadmap

### Propose New Features

1. **Open a Discussion**
   - Use [GitHub Discussions](https://github.com/zuudevs/grid_watcher/discussions)
   - Tag with `feature-request`
   - Explain use case and benefits

2. **Create RFC (Request for Comments)**
   - For major features
   - Include technical design
   - Gather community feedback

3. **Vote on Existing Proposals**
   - 👍 Upvote features you want
   - 💬 Comment with your perspective
   - 🤝 Volunteer to implement

### Claim a Feature

If you want to implement something from this roadmap:

1. Comment on the associated issue
2. Fork the repository
3. Create a feature branch
4. Submit a Pull Request

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

---

## 📢 Communication Channels

**Stay Updated:**
- ⭐ Star the repo for release notifications
- 👀 Watch releases on GitHub
- 📧 Subscribe to mailing list (coming soon)
- 🐦 Follow [@zuudevs](https://twitter.com/zuudevs) on Twitter

**Get Involved:**
- 💬 [GitHub Discussions](https://github.com/zuudevs/grid_watcher/discussions)
- 📝 [Issue Tracker](https://github.com/zuudevs/grid_watcher/issues)
- 📧 Email: zuudevs@gmail.com

---

## 🏆 Hall of Fame

Contributors who implement major roadmap features will be recognized here:

*Coming soon - be the first!*

---

## 📝 Version History

| Version | Release Date | Status | Notes |
|---------|-------------|--------|-------|
| 1.0.0 | Dec 2025 | ✅ Released | Initial release |
| 1.1.0 | Feb 2026 | 🟡 Planned | Enhanced detection |
| 1.2.0 | May 2026 | 🟡 Planned | Additional protocols |
| 1.3.0 | Aug 2026 | 🟡 Planned | Advanced features |
| 2.0.0 | Feb 2027 | 🔴 Concept | Major overhaul |

---

## 🎯 Success Metrics

We'll measure success by:

- **Adoption:** 1,000 active installations by end of 2026
- **Community:** 50+ contributors
- **Performance:** 1M pps processing capability
- **Coverage:** 90% of common SCADA protocols
- **Security:** Zero critical vulnerabilities
- **Documentation:** 100% API coverage

---

## 💡 Long-Term Vision (2027+)

- **Industry Standard:** Become the de-facto open-source IPS for SCADA
- **Certification:** IEC 62443 compliance certification
- **Enterprise:** Commercial support offerings
- **Ecosystem:** Thriving plugin marketplace
- **Education:** Training courses and certifications

---

## 🙏 Thank You

This roadmap is shaped by community feedback. Your input matters!

---

**This roadmap is subject to change based on community priorities and contributions.**

**Last Updated:** December 2025  
**Next Review:** March 2026  
**Maintained by:** Zuudevs & Community

---

**Ready to help build the future?** See [CONTRIBUTING.md](../CONTRIBUTING.md) to get started! 🚀