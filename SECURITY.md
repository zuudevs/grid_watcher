# Security Policy

## 🔒 Overview

Grid Watcher IPS is a security-critical application designed to protect SCADA infrastructure. We take security vulnerabilities seriously and appreciate responsible disclosure from security researchers and users.

---

## 🛡️ Supported Versions

We provide security updates for the following versions:

| Version | Supported          | Status |
| ------- | ------------------ | ------ |
| 1.0.x   | ✅ Yes             | Active |
| < 1.0   | ❌ No              | EOL    |

---

## 🚨 Reporting a Vulnerability

### **DO NOT** Open Public Issues

If you discover a security vulnerability, **DO NOT** create a public GitHub issue. Public disclosure could put active deployments at risk.

### Reporting Process

**Primary Contact:**
- **Email:** zuudevs@gmail.com
- **Subject Line:** `[SECURITY] Grid Watcher Vulnerability Report`

**Please Include:**
1. **Description** - Detailed explanation of the vulnerability
2. **Impact Assessment** - Potential security impact and affected components
3. **Reproduction Steps** - Step-by-step instructions to reproduce the issue
4. **Proof of Concept** - If available (code snippets, screenshots, logs)
5. **Suggested Fix** - If you have a solution in mind
6. **Disclosure Timeline** - Your preferred disclosure timeline

### What to Expect

- **Initial Response:** Within 48 hours
- **Vulnerability Confirmation:** Within 7 days
- **Fix Timeline:** Critical issues - 14 days; High severity - 30 days
- **Public Disclosure:** Coordinated disclosure after fix is released
- **Credit:** Security researchers will be credited in release notes (if desired)

---

## 🔐 Security Best Practices

### Deployment Security

1. **Privilege Management**
   - Run Grid Watcher with minimal required privileges
   - Use dedicated service accounts (avoid running as root continuously)
   - Implement principle of least privilege

2. **Network Isolation**
   - Deploy in isolated network segments
   - Use network segmentation for SCADA networks
   - Monitor firewall rules created by Grid Watcher

3. **Access Control**
   - Restrict access to the dashboard (implement authentication if exposed)
   - Use HTTPS for remote dashboard access
   - Regularly review blocked IP lists

4. **Monitoring**
   - Enable system logging
   - Monitor Grid Watcher's own logs for anomalies
   - Set up alerts for critical events

### Development Security

1. **Code Review**
   - All security-related changes require peer review
   - Use static analysis tools (clang-tidy, cppcheck)
   - Follow secure coding guidelines

2. **Dependency Management**
   - Minimize external dependencies
   - Regularly update compiler and libraries
   - Audit third-party code

3. **Testing**
   - Conduct security testing before releases
   - Perform fuzz testing on packet parsing logic
   - Test privilege escalation scenarios

---

## 🛠️ Known Security Considerations

### By Design

1. **Raw Socket Access**
   - Requires elevated privileges (root/Administrator)
   - **Mitigation:** Use capability-based security (Linux: `CAP_NET_RAW`)

2. **Firewall Modification**
   - Automatically modifies system firewall rules
   - **Mitigation:** Log all firewall changes; implement manual approval mode

3. **Dashboard Access**
   - Default setup has no authentication
   - **Mitigation:** Deploy behind reverse proxy with authentication

4. **Packet Capture**
   - Captures all network traffic (potential privacy concern)
   - **Mitigation:** Deploy only on isolated SCADA networks

### Platform-Specific

**Linux:**
- Requires `CAP_NET_RAW` and `CAP_NET_ADMIN` capabilities
- iptables rules persist across restarts (manual cleanup may be needed)

**Windows:**
- Requires Administrator privileges
- Windows Firewall rules are permanent (uninstall script recommended)

---

## 📋 Security Checklist for Deployments

Before deploying Grid Watcher in production:

- [ ] Review and understand firewall rule changes
- [ ] Configure network interfaces correctly
- [ ] Set up logging and monitoring
- [ ] Implement dashboard authentication
- [ ] Test in staging environment
- [ ] Document incident response procedures
- [ ] Train operators on security features
- [ ] Establish backup and recovery procedures
- [ ] Review compliance requirements (IEC 62443, NIST, etc.)

---

## 🔍 Security Audit History

| Date       | Type          | Findings | Status |
|------------|---------------|----------|--------|
| 2025-12-05 | Initial Release | N/A     | ✅ Complete |

---

## 🏆 Hall of Fame

We recognize and thank security researchers who have responsibly disclosed vulnerabilities:

*Currently empty - be the first contributor!*

---

## 📚 Additional Resources

- [IEC 62443 - Industrial Security Standards](https://www.isa.org/standards-and-publications/isa-standards/isa-iec-62443-series-of-standards)
- [NIST Cybersecurity Framework](https://www.nist.gov/cyberframework)
- [OWASP Secure Coding Practices](https://owasp.org/www-project-secure-coding-practices-quick-reference-guide/)

---

## 📞 Contact

For security-related inquiries:
- **Email:** zuudevs@gmail.com
- **PGP Key:** Available upon request

For general questions, use [GitHub Discussions](https://github.com/zuudevs/grid_watcher/discussions).

---

**Last Updated:** December 2025  
**Version:** 1.0