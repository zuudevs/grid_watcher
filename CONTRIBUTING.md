# Contributing to Grid Watcher IPS

Thank you for your interest in contributing to Grid Watcher! This document provides guidelines and best practices for contributing to the project.

---

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Pull Request Process](#pull-request-process)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)
- [Communication](#communication)

---

## 🤝 Code of Conduct

This project adheres to the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to zuudevs@gmail.com.

---

## 🚀 Getting Started

### Prerequisites

1. **Development Environment**
   - C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022+)
   - CMake 3.20 or higher
   - Git for version control

2. **Knowledge Requirements**
   - Modern C++ (C++17/20/23 features)
   - Network programming (sockets, protocols)
   - Multi-threading and concurrency
   - Basic SCADA/ICS security concepts

### Setting Up Your Development Environment

```bash
# Fork and clone the repository
git clone https://github.com/YOUR_USERNAME/grid_watcher.git
cd grid_watcher

# Add upstream remote
git remote add upstream https://github.com/zuudevs/grid_watcher.git

# Create a development build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

---

## 🔄 Development Workflow

### 1. Create a Feature Branch

```bash
# Update your fork
git checkout main
git pull upstream main

# Create a new branch
git checkout -b feature/your-feature-name
# or
git checkout -b bugfix/issue-number-description
```

### 2. Make Your Changes

- Write clean, readable code
- Follow the coding standards (see below)
- Add tests for new features
- Update documentation as needed

### 3. Commit Your Changes

```bash
git add .
git commit -m "feat: add description of your changes"
```

**Commit Message Format:**
```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

**Example:**
```
feat(analyzer): add DNP3 protocol detection

Implement deep packet inspection for DNP3 protocol commonly
used in SCADA systems. Includes function code validation and
unauthorized write detection.

Closes #123
```

### 4. Push and Create Pull Request

```bash
git push origin feature/your-feature-name
```

Then create a Pull Request on GitHub.

---

## 💻 Coding Standards

### Modern C++ Guidelines

#### 1. **Use Modern C++ Features**

```cpp
// ✅ GOOD - Use auto for type deduction
auto packet = create_packet();

// ❌ BAD - Explicit unnecessary types
std::vector<uint8_t> packet = create_packet();

// ✅ GOOD - Use range-based for loops
for (const auto& item : container) {
    process(item);
}

// ❌ BAD - Index-based iteration when not needed
for (size_t i = 0; i < container.size(); ++i) {
    process(container[i]);
}
```

#### 2. **RAII (Resource Acquisition Is Initialization)**

```cpp
// ✅ GOOD - RAII with smart pointers
class ResourceManager {
    std::unique_ptr<Resource> resource_;
public:
    ResourceManager() : resource_(std::make_unique<Resource>()) {}
    // Destructor automatically cleans up
};

// ❌ BAD - Manual memory management
class BadManager {
    Resource* resource_;
public:
    BadManager() : resource_(new Resource()) {}
    ~BadManager() { delete resource_; } // Error-prone
};
```

#### 3. **No Naked Pointers**

```cpp
// ✅ GOOD - Use smart pointers
std::unique_ptr<Analyzer> analyzer = std::make_unique<Analyzer>();
std::shared_ptr<Config> config = std::make_shared<Config>();

// ❌ BAD - Raw pointer ownership
Analyzer* analyzer = new Analyzer(); // Who owns this? When to delete?
```

#### 4. **Const Correctness**

```cpp
// ✅ GOOD - Use const where possible
void process_packet(const std::vector<uint8_t>& packet) {
    // packet is read-only
}

std::string get_ip() const { return ip_; } // Const member function

// ❌ BAD - Missing const
void process_packet(std::vector<uint8_t>& packet) { // Implies modification
}
```

#### 5. **Use std::string_view for Read-Only Strings**

```cpp
// ✅ GOOD - Avoid unnecessary copies
void log_message(std::string_view message) {
    std::cout << message << '\n';
}

// ❌ BAD - Unnecessary copy
void log_message(const std::string& message) { // Copy if string literal
}
```

#### 6. **Prefer std::array over C-arrays**

```cpp
// ✅ GOOD - Type-safe, bounds-checked
std::array<uint8_t, 1024> buffer{};

// ❌ BAD - No bounds checking
uint8_t buffer[1024];
```

#### 7. **Thread Safety**

```cpp
// ✅ GOOD - Use RAII for mutex locks
void safe_function() {
    std::lock_guard<std::mutex> lock(mutex_);
    // Critical section
} // Automatic unlock

// ❌ BAD - Manual lock/unlock
void unsafe_function() {
    mutex_.lock();
    // Critical section
    mutex_.unlock(); // Forgotten if exception occurs
}
```

### Code Style

- **Indentation:** 4 spaces (no tabs)
- **Line Length:** Maximum 100 characters
- **Naming Conventions:**
  - Classes: `PascalCase` (e.g., `PacketAnalyzer`)
  - Functions: `snake_case` (e.g., `process_packet`)
  - Variables: `snake_case` (e.g., `packet_count`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER_SIZE`)
  - Member variables: `snake_case_` with trailing underscore (e.g., `socket_fd_`)
  - Private members: Leading underscore optional

- **Braces:** Opening brace on same line
  ```cpp
  if (condition) {
      // code
  }
  ```

- **Comments:** Use Doxygen style for public APIs
  ```cpp
  /**
   * @brief Analyzes a network packet for threats
   * @param packet Raw packet data
   * @return true if threat detected, false otherwise
   */
  bool analyze_packet(const std::vector<uint8_t>& packet);
  ```

### Static Analysis

Run these tools before submitting:

```bash
# Clang-Tidy
clang-tidy src/**/*.cpp -- -std=c++23

# Cppcheck
cppcheck --enable=all --std=c++23 src/

# Clang-Format (auto-format)
clang-format -i src/**/*.cpp include/**/*.hpp
```

---

## 🔍 Pull Request Process

### Before Submitting

1. **Test Your Changes**
   - All existing tests pass
   - New tests added for new features
   - Manual testing completed

2. **Update Documentation**
   - Update README.md if needed
   - Add/update API documentation
   - Update CHANGELOG.md

3. **Code Quality**
   - No compiler warnings
   - Static analysis clean
   - Code formatted correctly

### PR Requirements

Your Pull Request should:

1. **Have a Clear Title**
   - `feat: Add DNP3 protocol support`
   - `fix: Resolve memory leak in packet analyzer`

2. **Include a Description**
   - What problem does this solve?
   - How does it solve it?
   - Any breaking changes?
   - Related issues (e.g., "Closes #42")

3. **Be Reviewable**
   - Small, focused changes (< 500 lines preferred)
   - One logical change per PR
   - Clear commit history

### Review Process

1. Automated checks must pass (CI/CD)
2. Code review by at least one maintainer
3. All discussions resolved
4. Approved by maintainer
5. Merged by maintainer

---

## 🧪 Testing Guidelines

### Unit Tests

```cpp
// test/test_analyzer.cpp
#include <catch2/catch_test_macros.hpp>
#include "core/analyzer.hpp"

TEST_CASE("PacketAnalyzer detects Modbus write", "[analyzer]") {
    // Arrange
    std::vector<uint8_t> modbus_packet = create_test_packet();
    
    // Act
    bool detected = analyzer.detect_modbus_write(modbus_packet);
    
    // Assert
    REQUIRE(detected == true);
}
```

### Integration Tests

```bash
# Run attack simulator
./bin/attack_sim 127.0.0.1

# Verify IPS response
./bin/grid_watcher --test-mode
```

### Performance Tests

- Benchmark packet processing throughput
- Memory leak detection (Valgrind)
- CPU profiling under load

---

## 📝 Documentation

### Code Documentation

- Public APIs must have Doxygen comments
- Complex algorithms need explanatory comments
- TODO comments should reference issues

### User Documentation

Update relevant documentation files:
- `README.md` - High-level changes
- `docs/API.md` - New APIs
- `docs/QUICKSTART.md` - Usage changes
- `docs/ARCHITECTURE.md` - Design changes

---

## 💬 Communication

### Asking Questions

- **GitHub Discussions:** General questions, feature ideas
- **GitHub Issues:** Bug reports, feature requests
- **Email:** Security vulnerabilities only

### Reporting Bugs

Use the bug report template and include:
- Operating system and version
- Compiler version
- Steps to reproduce
- Expected vs actual behavior
- Relevant logs

### Suggesting Features

Open a discussion or issue with:
- Use case description
- Proposed solution
- Alternatives considered
- Willingness to implement

---

## 🏆 Recognition

Contributors will be recognized in:
- CHANGELOG.md (for each release)
- GitHub contributors page
- Special mentions for significant contributions

---

## 📚 Resources

- [Modern C++ Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [C++23 Features](https://en.cppreference.com/w/cpp/23)
- [CMake Best Practices](https://cliutils.gitlab.io/modern-cmake/)

---

## 🙏 Thank You!

Every contribution, no matter how small, helps improve Grid Watcher. We appreciate your time and effort!

---

**Questions?** Contact the maintainers at zuudevs@gmail.com or open a GitHub Discussion.

**Happy Coding! 🚀**