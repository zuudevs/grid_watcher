# Grid Watcher IPS - Quick Start Guide

Get up and running with Grid Watcher in under 5 minutes! This guide will walk you through installation, basic usage, and your first threat detection.

---

## 🎯 Prerequisites Checklist

Before starting, ensure you have:

- ✅ Linux (Ubuntu/Debian/RHEL) or Windows 10/11
- ✅ Administrator/Root access
- ✅ C++23 compiler (GCC 13+ or MSVC 2022+)
- ✅ CMake 3.20+
- ✅ Python 3.8+ (for dashboard)

---

## 🚀 Step 1: Installation (2 minutes)

### Option A: Quick Install (Linux)

```bash
# Clone repository
git clone https://github.com/zuudevs/grid_watcher.git
cd grid_watcher

# Build (takes ~1 minute)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Verify installation
ls -lh bin/grid_watcher
```

### Option B: Quick Install (Windows)

```cmd
REM Clone repository
git clone https://github.com/zuudevs/grid_watcher.git
cd grid_watcher

REM Build
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release

REM Verify
dir bin\grid_watcher.exe
```

---

## 🛡️ Step 2: Start the IPS (30 seconds)

### Linux
```bash
# Run with sudo (required for raw sockets)
sudo ./bin/grid_watcher
```

**Expected Output:**
```
========================================
  Grid Watcher IPS v1.0
  SCADA Intrusion Prevention System
  Platform: Linux (Production)
========================================
[2025-12-05 10:30:00.000] [INFO] Packet capture socket initialized
[2025-12-05 10:30:00.001] [INFO] Packet sniffer started
[2025-12-05 10:30:00.002] [INFO] Packet analyzer started
[2025-12-05 10:30:00.003] [INFO] Action executor started
[2025-12-05 10:30:00.004] [INFO] All systems operational. Monitoring traffic...
```

### Windows (PowerShell as Administrator)
```powershell
# Run as Administrator
.\bin\grid_watcher.exe
```

**Expected Output:**
```
========================================
  Grid Watcher IPS v1.0
  SCADA Intrusion Prevention System
  Platform: Windows (Compatibility Mode)
========================================
[2025-12-05 10:30:00.000] [WARN] Running in Windows Compatibility Mode. TCP capture may be limited by OS.
[2025-12-05 10:30:00.001] [INFO] Packet capture socket initialized
[2025-12-05 10:30:00.002] [INFO] All systems operational. Monitoring traffic...
```

---

## 📊 Step 3: Launch Dashboard (30 seconds)

Open a **new terminal** (keep the IPS running):

```bash
cd www
python -m http.server 8080
```

**Access Dashboard:**
1. Open browser: `http://localhost:8080`
2. You should see the Grid Watcher Command Center
3. System Status should show "System Active"

---

## 🎮 Step 4: Test with Attack Simulation (2 minutes)

Open a **third terminal** to run the attack simulator:

```bash
cd grid_watcher
./bin/attack_sim 127.0.0.1
```

**What Happens:**
1. Simulator sends 15 SYN packets to different ports (port scan)
2. Simulator attempts Modbus write command on port 502
3. IPS detects threats and blocks the IP

**Watch the IPS Console:**
```
[2025-12-05 10:32:15.123] [WARN] Port scan detected from 127.0.0.1
[2025-12-05 10:32:15.124] [CRIT] BLOCKING IP: 127.0.0.1
[2025-12-05 10:32:17.456] [WARN] SCADA Write Attempt from 127.0.0.1 (Function Code: 0x06)
```

**Check Dashboard:**
- Threats Detected: +2
- IPs Blocked: 127.0.0.1
- Live Threat Feed: Shows "PORT_SCAN" and "MODBUS_WRITE" alerts

---

## ✅ Step 5: Verify Firewall Block

### Linux - Check iptables
```bash
sudo iptables -L INPUT -v
```

**Expected Output:**
```
Chain INPUT (policy ACCEPT)
target     prot opt source               destination
DROP       all  --  127.0.0.1            anywhere
```

### Windows - Check Firewall
```powershell
# Run as Administrator
netsh advfirewall firewall show rule name=all | Select-String "GridWatcher"
```

**Expected Output:**
```
Rule Name:              GridWatcher_127.0.0.1
----------------------------------------------
Enabled:                Yes
Direction:              In
Profiles:               Domain,Private,Public
Action:                 Block
```

---

## 🧪 Step 6: Your First Custom Detection

Let's create a simple script to test the IPS with your own traffic.

### Create Test Script: `test_detection.py`

```python
#!/usr/bin/env python3
import socket
import time

def test_port_scan(target_ip):
    """Simulate a port scan"""
    print(f"[TEST] Scanning {target_ip}...")
    
    for port in range(8000, 8015):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(0.1)
            sock.connect((target_ip, port))
            sock.close()
        except:
            pass
        time.sleep(0.1)
        print(f"[TEST] Attempted port {port}")
    
    print("[TEST] Port scan complete. Check IPS console!")

if __name__ == "__main__":
    test_port_scan("127.0.0.1")
```

### Run Test
```bash
chmod +x test_detection.py
python3 test_detection.py
```

**IPS Should Detect:**
```
[WARN] Port scan detected from 127.0.0.1
[CRIT] BLOCKING IP: 127.0.0.1
```

---

## 🎓 Understanding the Results

### What Just Happened?

1. **Packet Capture:** IPS captured raw network packets
2. **Analysis:** Detected 10+ unique ports from same source IP
3. **Threat Classification:** Identified as port scanning attack
4. **Active Defense:** Executed firewall rule to block source IP
5. **Logging:** Recorded event to JSON for dashboard

### Dashboard Metrics Explained

| Metric | Meaning |
|--------|---------|
| **Packets Captured** | Total raw packets captured from network |
| **Threats Detected** | Number of security events identified |
| **IPs Blocked** | Unique IP addresses blocked by firewall |
| **Packets Analyzed** | Packets that passed deep inspection |

---

## 🔧 Common First-Time Issues

### Issue 1: "Permission Denied" (Linux)

**Problem:** Raw sockets require root privileges

**Solution:**
```bash
# Run with sudo
sudo ./bin/grid_watcher

# Or grant capabilities (one-time setup)
sudo setcap cap_net_raw,cap_net_admin+eip ./bin/grid_watcher
./bin/grid_watcher  # Now works without sudo
```

### Issue 2: Dashboard Shows "No Data"

**Problem:** Browser can't fetch `dashboard_data.json`

**Solution:**
```bash
# Check if IPS is writing JSON
ls -lh www/dashboard_data.json
cat www/dashboard_data.json

# Restart HTTP server
cd www
python -m http.server 8080
```

### Issue 3: "Address Already in Use" (Dashboard)

**Problem:** Port 8080 already taken

**Solution:**
```bash
# Use different port
python -m http.server 8888

# Access: http://localhost:8888
```

### Issue 4: Windows Firewall Blocks Localhost

**Problem:** Windows firewall blocks 127.0.0.1 after test

**Solution:**
```powershell
# Unblock localhost (as Administrator)
netsh advfirewall firewall delete rule name="GridWatcher_127.0.0.1"
```

---

## 🎯 Next Steps

Now that you have Grid Watcher running, explore these features:

### 1. Production Deployment
- Read [ARCHITECTURE.md](ARCHITECTURE.md) for system design
- Configure for your network environment
- Set up monitoring and alerting

### 2. Advanced Configuration
- Adjust port scan threshold (default: 10 ports)
- Customize Modbus function codes
- Integrate with SIEM systems

### 3. Protocol Support
- Currently supports: Modbus TCP (port 502)
- Coming soon: DNP3, IEC 104 (see [ROADMAP.md](ROADMAP.md))

### 4. Custom Rules
Study the codebase:
```bash
# Analyzer logic
cat src/core/analyzer.cpp

# Detection rules
grep -n "check_modbus_write" src/core/analyzer.cpp
```

---

## 📚 Essential Documentation

| Document | When to Read |
|----------|-------------|
| [BUILD.md](../BUILD.md) | Build issues or custom compilation |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Understanding internal design |
| [API.md](API.md) | Integrating or extending Grid Watcher |
| [CONTRIBUTING.md](../CONTRIBUTING.md) | Contributing code or features |

---

## 🛠️ Development Workflow

Want to add custom detection logic?

### 1. Study Existing Code

```cpp
// src/core/analyzer.cpp
void PacketAnalyzer::check_modbus_write(const std::vector<uint8_t>& packet) {
    uint16_t dst_port = extract_dst_port(packet);
    if(dst_port != 502) return; // Modbus TCP port
    
    // Your custom logic here
    uint8_t func_code = /* extract from packet */;
    
    if(/* your condition */) {
        action_queue_.push(ActionEvent{
            ActionType::BLOCK_IP,
            src_ip,
            "Custom threat detected"
        });
    }
}
```

### 2. Rebuild and Test

```bash
cmake --build build
# Script run.sh/run.bat automatically regenerated!

# Launch with updated code
./run.sh  # Linux
run.bat   # Windows
```

### 3. Share Your Improvements

See [CONTRIBUTING.md](../CONTRIBUTING.md) for pull request guidelines.

---

## 💡 Pro Tips

1. **Use tmux/screen** for managing multiple terminals:
   ```bash
   tmux new -s gridwatcher
   # Split panes: Ctrl+B then "
   ```

2. **Monitor Real-time Logs:**
   ```bash
   sudo ./bin/grid_watcher | tee grid_watcher.log
   ```

3. **Dashboard Refresh Rate:**
   - Default: 1 second polling
   - Edit `www/js/dashboard_connector.js` to adjust

4. **Production Considerations:**
   - Always test in staging environment first
   - Monitor system resources (CPU, memory)
   - Set up log rotation
   - Document your firewall rules

---

## 🚨 Emergency Stop

If you need to stop everything:

```bash
# Stop IPS
Ctrl+C  # In IPS terminal

# Cleanup iptables (Linux)
sudo iptables -F INPUT  # Flush all INPUT rules

# Cleanup Windows Firewall
netsh advfirewall firewall delete rule name=all program=any
```

---

## ❓ Getting Help

- **Documentation:** Start with [README.md](../README.md)
- **GitHub Issues:** Report bugs or request features
- **Email:** zuudevs@gmail.com
- **Community:** GitHub Discussions (coming soon)

---

## 🎉 Success!

You've successfully:
- ✅ Installed Grid Watcher IPS
- ✅ Detected your first threats
- ✅ Blocked malicious IPs
- ✅ Visualized attacks in real-time

**Welcome to Grid Watcher! 🛡️**

---

**Next Recommended Read:** [ARCHITECTURE.md](ARCHITECTURE.md) - Understand how it all works under the hood.

**Ready to contribute?** See [CONTRIBUTING.md](../CONTRIBUTING.md)

---

*Last Updated: December 2025*  
*Estimated Time: 5 minutes*  
*Difficulty: Beginner-Friendly*