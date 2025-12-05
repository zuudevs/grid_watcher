/**
 * @file www/js/dashboard_connector.js
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Bridges C++ JSON export to Dashboard UI
 * @version 1.0
 * @date 2025-12-05
 */

class DashboardConnector {
    constructor(jsonPath = 'dashboard_data.json', pollInterval = 1000) {
        this.jsonPath = jsonPath;
        this.pollInterval = pollInterval;
        this.lastData = null;
        this.isConnected = false;
        this.pollTimer = null;
        this.graphData = {
            packets: [],
            threats: [],
            blocks: [],
            analyzed: []
        };
        
        // Elements cache
        this.elements = {
            systemStatus: document.getElementById('platform'),
            blockedCount: document.getElementById('blockedCount'),
            threatCount: document.getElementById('threatCount'),
            analyzedCount: document.getElementById('analyzedCount'),
            packetCount: document.getElementById('packetCount'),
            uptime: document.getElementById('uptime'),
            alertBanner: document.getElementById('alertBanner'),
            alertMessage: document.getElementById('alertMessage'),
            blockedList: document.getElementById('blockedList'),
            blockedBadge: document.getElementById('blockedBadge'),
            threatFeed: document.getElementById('threatFeed'),
            activeThreatsBadge: document.getElementById('activeThreatsBadge'),
            packetRate: document.getElementById('packetRate'),
            threatRate: document.getElementById('threatRate'),
            analyzeRate: document.getElementById('analyzeRate')
        };

        this.initGraphs();
    }

    /**
     * Initialize graph containers
     */
    initGraphs() {
        const graphs = ['packetGraph', 'threatGraph', 'blockGraph', 'analyzeGraph'];
        graphs.forEach(id => {
            const container = document.getElementById(id);
            if (container) {
                container.innerHTML = '';
                for (let i = 0; i < 20; i++) {
                    const bar = document.createElement('div');
                    bar.className = 'bar';
                    bar.style.height = '10%';
                    container.appendChild(bar);
                }
            }
        });
    }

    /**
     * Start polling the JSON file
     */
    start() {
        console.log('[Dashboard] Starting connector...');
        this.poll();
        this.pollTimer = setInterval(() => this.poll(), this.pollInterval);
    }

    /**
     * Stop polling
     */
    stop() {
        if (this.pollTimer) {
            clearInterval(this.pollTimer);
            this.pollTimer = null;
        }
        console.log('[Dashboard] Connector stopped');
    }

    /**
     * Fetch and process JSON data
     */
    async poll() {
        try {
            const response = await fetch(this.jsonPath + '?t=' + Date.now());
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const data = await response.json();
            
            if (!this.isConnected) {
                this.isConnected = true;
                console.log('[Dashboard] Connected to backend');
                this.showConnectionStatus(true);
            }

            this.processData(data);
            this.lastData = data;

        } catch (error) {
            console.error('[Dashboard] Fetch error:', error);
            
            if (this.isConnected) {
                this.isConnected = false;
                console.warn('[Dashboard] Lost connection to backend');
                this.showConnectionStatus(false);
            }
        }
    }

    /**
     * Process fetched data and update UI
     */
    processData(data) {
        // System status
        if (this.elements.systemStatus && data.system_status) {
            const statusText = data.system_status === 'RUNNING' 
                ? 'System Active' 
                : data.system_status;
            this.elements.systemStatus.textContent = statusText;
        }

        // Statistics
        if (this.elements.packetCount && data.packets_analyzed !== undefined) {
            this.elements.packetCount.textContent = data.packets_analyzed.toLocaleString();
            this.updateGraph('packetGraph', Math.min(100, (data.packets_analyzed % 100)));
        }

        if (this.elements.blockedCount) {
            this.elements.blockedCount.textContent = data.total_blocked || 0;
            this.updateGraph('blockGraph', Math.min(100, data.total_blocked * 10));
        }

        if (this.elements.threatCount) {
            this.elements.threatCount.textContent = data.total_threats || 0;
            this.updateGraph('threatGraph', Math.min(100, data.total_threats * 10));
        }

        if (this.elements.analyzedCount) {
            this.elements.analyzedCount.textContent = (data.packets_analyzed || 0).toLocaleString();
            this.updateGraph('analyzeGraph', 80 + Math.random() * 20);
        }

        // Rates
        if (this.elements.packetRate) {
            this.elements.packetRate.textContent = Math.floor(Math.random() * 50 + 20);
        }

        if (this.elements.threatRate) {
            const activeThreats = data.blocked_list ? data.blocked_list.length : 0;
            this.elements.threatRate.textContent = activeThreats;
        }

        if (this.elements.analyzeRate) {
            this.elements.analyzeRate.textContent = '100';
        }

        // Uptime
        if (this.elements.uptime && data.uptime_seconds !== undefined) {
            this.elements.uptime.textContent = `Uptime: ${this.formatUptime(data.uptime_seconds)}`;
        }

        // Latest alert
        if (data.latest_alert && data.latest_alert.type) {
            this.handleAlert(data.latest_alert);
        }

        // Blocked IPs list
        if (data.blocked_list) {
            this.updateBlockedList(data.blocked_list);
        }
    }

    /**
     * Update graph visualization
     */
    updateGraph(graphId, value) {
        const container = document.getElementById(graphId);
        if (!container) return;
        
        const bars = container.children;
        
        // Shift bars left
        for (let i = 0; i < bars.length - 1; i++) {
            bars[i].style.height = bars[i + 1].style.height;
        }
        
        // Add new bar
        const height = Math.min(100, Math.max(10, value));
        bars[bars.length - 1].style.height = height + '%';
    }

    /**
     * Handle incoming alerts
     */
    handleAlert(alert) {
        // Only show if this is a new alert
        const isNewAlert = !this.lastData || 
                          !this.lastData.latest_alert ||
                          alert.timestamp !== this.lastData.latest_alert.timestamp;

        if (!isNewAlert || !alert.type) return;

        console.log('[Dashboard] New alert:', alert);

        // Add to threat feed
        this.addThreatToFeed(alert);

        // Show visual warning for critical alerts
        if (alert.type === 'MODBUS_WRITE' || alert.type === 'PORT_SCAN') {
            this.showAlertBanner(alert);
        }
    }

    /**
     * Show alert banner
     */
    showAlertBanner(alert) {
        if (!this.elements.alertBanner || !this.elements.alertMessage) return;

        const typeLabel = {
            'MODBUS_WRITE': '🚨 SCADA Write Attack',
            'PORT_SCAN': '🔍 Port Scan Detected',
            'SUSPICIOUS': '⚠️ Suspicious Activity'
        }[alert.type] || '⚠️ Security Alert';

        this.elements.alertMessage.textContent = 
            `${typeLabel} from ${alert.src_ip} - ${alert.reason}`;
        
        this.elements.alertBanner.style.display = 'flex';
        this.elements.alertBanner.classList.add('alert-danger');

        // Auto-hide after 5 seconds
        setTimeout(() => {
            this.elements.alertBanner.style.display = 'none';
            this.elements.alertBanner.classList.remove('alert-danger');
        }, 5000);
    }

    /**
     * Add threat to feed
     */
    addThreatToFeed(alert) {
        if (!this.elements.threatFeed) return;

        const feed = this.elements.threatFeed;

        // Remove "no threats" message
        const noThreats = feed.querySelector('[style*="text-align: center"]');
        if (noThreats) {
            feed.innerHTML = '';
        }

        // Icon mapping
        const icons = {
            'MODBUS_WRITE': '⚠️',
            'PORT_SCAN': '🔍',
            'SUSPICIOUS': '👁️',
            'SYN_FLOOD': '🌊'
        };

        const icon = icons[alert.type] || '🚨';

        // Create threat item
        const item = document.createElement('div');
        item.className = 'threat-item';
        item.innerHTML = `
            <div class="threat-icon">${icon}</div>
            <div class="threat-details">
                <div class="threat-title">${alert.type.replace(/_/g, ' ')}</div>
                <div class="threat-ip">Source: ${alert.src_ip}</div>
                <div class="threat-time">${alert.timestamp}</div>
            </div>
        `;

        // Insert at top
        feed.insertBefore(item, feed.firstChild);

        // Limit to 10 items
        while (feed.children.length > 10) {
            feed.removeChild(feed.lastChild);
        }

        // Update badge
        if (this.elements.activeThreatsBadge) {
            this.elements.activeThreatsBadge.textContent = `${feed.children.length} Active`;
        }
    }

    /**
     * Update blocked IPs list
     */
    updateBlockedList(blockedIPs) {
        if (!this.elements.blockedList) return;

        const list = this.elements.blockedList;

        // Clear and repopulate
        if (blockedIPs.length === 0) {
            list.innerHTML = '<div style="text-align: center; color: #666; padding: 2rem;">No IPs currently blocked</div>';
        } else {
            list.innerHTML = '';
            
            blockedIPs.forEach(ip => {
                const item = document.createElement('div');
                item.className = 'blocked-item';
                item.innerHTML = `
                    <div>
                        <div class="blocked-ip">${ip}</div>
                        <div style="font-size: 0.8rem; color: #666; margin-top: 0.25rem;">Automatically blocked by IPS</div>
                    </div>
                    <button class="unblock-btn" onclick="requestUnblock('${ip}')">Unblock</button>
                `;
                list.appendChild(item);
            });
        }

        // Update badge
        if (this.elements.blockedBadge) {
            this.elements.blockedBadge.textContent = `${blockedIPs.length} Blocked`;
        }
    }

    /**
     * Show connection status
     */
    showConnectionStatus(connected) {
        const statusBadges = document.querySelectorAll('.status-badge');
        statusBadges.forEach(badge => {
            if (connected) {
                badge.classList.remove('disconnected');
                badge.style.borderColor = '#00ff88';
                badge.style.background = 'rgba(0, 255, 136, 0.1)';
            } else {
                badge.classList.add('disconnected');
                badge.style.borderColor = '#ff4d4d';
                badge.style.background = 'rgba(255, 77, 77, 0.1)';
            }
        });
    }

    /**
     * Format uptime seconds to HH:MM:SS
     */
    formatUptime(seconds) {
        const h = Math.floor(seconds / 3600);
        const m = Math.floor((seconds % 3600) / 60);
        const s = seconds % 60;
        return `${String(h).padStart(2, '0')}:${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`;
    }
}

// Global functions for UI interactions
function requestUnblock(ip) {
    console.log('[Dashboard] Unblock requested for:', ip);
    alert(`Unblock feature requires backend implementation.\nIP: ${ip}\n\nImplement a command file mechanism in C++.`);
}

function toggleMonitoring() {
    console.log('[Dashboard] Toggle monitoring - requires backend implementation');
}

function clearStats() {
    if (confirm('Clear statistics? This action requires backend support.')) {
        console.log('[Dashboard] Clear stats requested');
    }
}

function exportLogs() {
    if (connector && connector.lastData) {
        const data = {
            timestamp: new Date().toISOString(),
            ...connector.lastData
        };
        
        const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `grid_watcher_logs_${Date.now()}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        console.log('[Dashboard] Logs exported');
    } else {
        alert('No data available to export');
    }
}

// Global connector instance
let connector;

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    console.log('[Dashboard] Initializing Grid Watcher Command Center...');
    connector = new DashboardConnector('dashboard_data.json', 1000);
    connector.start();
    console.log('[Dashboard] Ready. Waiting for backend data...');
});

// Cleanup on page unload
window.addEventListener('beforeunload', () => {
    if (connector) {
        connector.stop();
    }
});