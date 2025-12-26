# Build Instructions

This document provides comprehensive instructions for building Grid Watcher IPS from source on various platforms and configurations.

---

## 📋 Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Build](#quick-build)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Build Configurations](#build-configurations)
- [Troubleshooting](#troubleshooting)
- [Advanced Build Options](#advanced-build-options)

---

## 🔧 Prerequisites

### Required Tools

| Tool | Minimum Version | Recommended |
|------|----------------|-------------|
| **C++ Compiler** | GCC 13+ / Clang 16+ / MSVC 19.35+ | Latest stable |
| **CMake** | 3.20 | 3.27+ |
| **Git** | 2.30+ | Latest |
| **Python** | 3.8+ | 3.11+ (for dashboard) |

### Platform Requirements

**Linux (Recommended for Production):**
- Kernel 5.x or higher
- `libstdc++` with C++23 support
- `iptables` (for firewall integration)
- Root/sudo access for raw sockets

**Windows (Development Mode):**
- Windows 10/11 (x64)
- Windows SDK 10.0.19041.0+
- Administrator privileges
- Visual Studio 2022 or Build Tools

### Installing Prerequisites

#### Ubuntu/Debian
```bash
# Update package index
sudo apt update

# Install build essentials
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    iptables

# Install modern GCC (if not available)
sudo apt install -y gcc-13 g++-13
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

# Verify installation
gcc --version  # Should show 13.x or higher
cmake --version  # Should show 3.20 or higher
```

#### Fedora/RHEL
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    git \
    python3 \
    iptables

# For GCC 13
sudo dnf install -y gcc-13 g++-13
```

#### Arch Linux
```bash
sudo pacman -S \
    gcc \
    cmake \
    ninja \
    git \
    python \
    iptables
```

#### Windows (with Chocolatey)
```powershell
# Install Chocolatey first (https://chocolatey.org/)

# Install tools
choco install -y visualstudio2022buildtools
choco install -y cmake
choco install -y git
choco install -y python

# Or install Visual Studio Community with C++ workload
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ninja git python@3.11
```

---

## ⚡ Quick Build

### Clone Repository

```bash
git clone https://github.com/zuudevs/grid_watcher.git
cd grid_watcher
```

### Default Build (Release)

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Verify
./bin/grid_watcher --version
```

### Run

```bash
# Linux (requires sudo)
sudo ./bin/grid_watcher

# Windows (run as Administrator)
.\bin\grid_watcher.exe
```

---

## 🖥️ Platform-Specific Instructions

### Linux (Production Build)

#### Standard Build
```bash
# Configure with specific compiler
CC=gcc-13 CXX=g++-13 cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja

# Build with all cores
cmake --build build -j$(nproc)

# Install (optional)
sudo cmake --install build --prefix /usr/local
```

#### Distribution-Specific Optimizations

**Ubuntu 22.04/24.04:**
```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=native -O3" \
    -G Ninja

cmake --build build -j$(nproc)
```

**RHEL/CentOS:**
```bash
# Enable Developer Toolset if needed
scl enable gcc-toolset-13 bash

cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja

cmake --build build
```

---

### Windows (MSVC)

#### Visual Studio 2022

**Using CMake GUI:**
1. Open CMake GUI
2. Set source: `C:/path/to/grid_watcher`
3. Set build: `C:/path/to/grid_watcher/build`
4. Click "Configure" → Select "Visual Studio 17 2022"
5. Click "Generate"
6. Click "Open Project"
7. Build in Visual Studio (Ctrl+Shift+B)

**Using Command Line:**
```cmd
REM Run from Developer Command Prompt

REM Configure
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

REM Build
cmake --build build --config Release

REM Run (requires Administrator)
.\bin\grid_watcher.exe
```

#### MinGW-w64 (GCC on Windows)

```bash
# Configure
cmake -S . -B build \
    -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc

# Build
cmake --build build
```

---

### macOS (Experimental)

```bash
# Configure
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja

# Build
cmake --build build -j$(sysctl -n hw.ncpu)

# Note: Raw socket capture requires root
sudo ./bin/grid_watcher
```

---

## 🎛️ Build Configurations

### Debug Build (Development)

```bash
cmake -S . -B build-debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g -O0 -fsanitize=address,undefined"

cmake --build build-debug
```

**Enables:**
- Debug symbols (`-g`)
- No optimization (`-O0`)
- Address Sanitizer
- Undefined Behavior Sanitizer

### Release Build (Production)

```bash
cmake -S . -B build-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG"

cmake --build build-release
```

**Enables:**
- Maximum optimization (`-O3`)
- Assertions disabled (`DNDEBUG`)
- Link-Time Optimization (if supported)

### RelWithDebInfo (Profiling)

```bash
cmake -S . -B build-profile \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build build-profile
```

**Use for:**
- Performance profiling
- Production debugging
- Benchmarking

### MinSizeRel (Embedded Systems)

```bash
cmake -S . -B build-minsize \
    -DCMAKE_BUILD_TYPE=MinSizeRel

cmake --build build-minsize
```

---

## 🔍 Build Verification

### Run Tests

```bash
# Build attack simulator
cmake --build build --target attack_sim

# Test IPS response
cd build
sudo ./bin/grid_watcher &  # Run in background
./bin/attack_sim 127.0.0.1
```

### Check Binary

```bash
# Linux
ldd ./bin/grid_watcher
file ./bin/grid_watcher

# Show symbols
nm -C ./bin/grid_watcher | grep -i "main"

# Check size
ls -lh ./bin/grid_watcher
```

### Verify Compiler Version

```bash
# Check what was used
cmake -S . -B build --trace-expand | grep CXX_COMPILER
```

---

## 🐛 Troubleshooting

### Common Issues

#### 1. "C++23 features not available"

**Solution:**
```bash
# Verify compiler version
g++ --version  # Must be 13+
clang++ --version  # Must be 16+

# Force specific compiler
CC=gcc-13 CXX=g++-13 cmake -S . -B build
```

#### 2. "Cannot find <format> header"

**Cause:** Incomplete C++23 standard library

**Solution:**
```bash
# Ubuntu: Install libc++-dev
sudo apt install -y libc++-17-dev

# Or use GCC 13 libstdc++
sudo apt install -y libstdc++-13-dev
```

#### 3. "Raw socket permission denied"

**Cause:** Insufficient privileges

**Solution:**
```bash
# Run with sudo
sudo ./bin/grid_watcher

# Or grant CAP_NET_RAW capability (Linux)
sudo setcap cap_net_raw,cap_net_admin+eip ./bin/grid_watcher
./bin/grid_watcher  # Now runs without sudo
```

#### 4. "Winsock initialization failed" (Windows)

**Cause:** Missing ws2_32.lib linking

**Solution:**
```bash
# Ensure CMakeLists.txt has:
# target_link_libraries(grid_watcher ws2_32)

# Clean and rebuild
cmake --build build --clean-first
```

#### 5. "std::format" linker errors

**Cause:** Format library not linked

**Solution:**
```bash
# GCC requires explicit linking
cmake -S . -B build -DCMAKE_CXX_FLAGS="-lfmt"
```

### Build System Issues

#### CMake Cache Problems

```bash
# Clear cache completely
rm -rf build/
cmake -S . -B build

# Or just clear cache
cmake --build build --target clean
rm build/CMakeCache.txt
```

#### Ninja Not Found

```bash
# Install Ninja
sudo apt install ninja-build  # Ubuntu
brew install ninja  # macOS

# Or use Make instead
cmake -S . -B build -G "Unix Makefiles"
```

---

## 🚀 Advanced Build Options

### Static Linking

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_EXE_LINKER_FLAGS="-static"

cmake --build build
```

### Cross-Compilation

```bash
# Example: ARM64
cmake -S . -B build-arm64 \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++

cmake --build build-arm64
```

### Custom Install Prefix

```bash
cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX=/opt/grid_watcher

cmake --build build
sudo cmake --install build
```

### Build with Clang

```bash
CC=clang-16 CXX=clang++-16 cmake -S . -B build-clang \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja

cmake --build build-clang
```

### Enable Link-Time Optimization

```bash
cmake -S . -B build-lto \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON

cmake --build build-lto
```

---

## 📊 Build Performance

### Parallel Builds

```bash
# Use all cores
cmake --build build -j$(nproc)  # Linux
cmake --build build -j$(sysctl -n hw.ncpu)  # macOS
cmake --build build -j%NUMBER_OF_PROCESSORS%  # Windows

# Limit cores (e.g., 4)
cmake --build build -j4
```

### Build Time Optimization

```bash
# Use Ninja (faster than Make)
cmake -S . -B build -G Ninja

# Use ccache
export CC="ccache gcc"
export CXX="ccache g++"
cmake -S . -B build

# Use precompiled headers (if CMake 3.16+)
# Add to CMakeLists.txt:
# target_precompile_headers(grid_watcher PRIVATE <vector> <string>)
```

---

## 📦 Packaging

### Create Distribution Package

```bash
# Build release
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Create package
cd build
cpack -G TGZ  # Linux
cpack -G ZIP  # Windows
```

---

## 🔗 Next Steps

After successful build:

1. **Run Tests:** See [Testing Guidelines](CONTRIBUTING.md#testing-guidelines)
2. **Start Dashboard:** `cd www && python -m http.server 8080`
3. **Read Docs:** [QUICKSTART.md](docs/QUICKSTART.md)

---

## 📞 Getting Help

If you encounter build issues:

1. Check [Troubleshooting](#troubleshooting) section
2. Search [GitHub Issues](https://github.com/zuudevs/grid_watcher/issues)
3. Ask in [GitHub Discussions](https://github.com/zuudevs/grid_watcher/discussions)
4. Contact: zuudevs@gmail.com

---

**Last Updated:** December 2025  
**CMake Version:** 3.20+  
**Compiler Support:** GCC 13+, Clang 16+, MSVC 19.35+