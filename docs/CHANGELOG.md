# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2025-12-05
### Added
- Core IPS engine (Sniffer, Analyzer, Executor).
- Support for Modbus TCP function code detection (0x05, 0x06, 0x10).
- Port scan detection algorithm (SYN flood pattern).
- Web-based Dashboard using HTML5/JS.
- Automatic firewall integration (iptables/netsh).
- JSON Exporter for logging events.

### Security
- Implemented thread-safe queue for packet processing.
- Added raw socket restriction (Admin/Root required).

## [1.0.1] - 2025-12-26
### Refactor
- Consolidated all headers into `gridwatcher.hpp`.

### Added
- Launcher Generation Script: automatically creates `run.sh` and `run.bat`.
- Added comprehensive Doxygen-based documentation for all core components and utilities.