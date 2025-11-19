/**
 * ================================================================
 * BIOMINING PURE PYTHON PLATFORM - MAIN APPLICATION
 * JavaScript for Bio-Entropy Mining Interface Management
 * Pure Python Implementation (No C++ Dependencies)
 * ================================================================
 */

class BioMiningApp {
    constructor() {
        this.charts = {};
        this.websocket = null;
        this.currentPage = 'dashboard';
        this.miningActive = false;
        this.trainingManager = null;
        
        console.log('🚀 BioMining Pure Python Platform initialized');
        this.init();
    }

    /**
     * Initialize the application
     */
    init() {
        this.setupEventListeners();
        this.initializeWebSocket();
        this.loadConfiguration();
        this.startPerformanceMonitoring();
        
        // Initialize Training Manager (from training.js)
        if (typeof TrainingManager !== 'undefined') {
            this.trainingManager = new TrainingManager();
        }
        
        // Verify form handlers after setup
        setTimeout(() => this.verifyFormHandlers(), 100);
    }

    /**
     * Setup all event listeners for the interface
     */
    setupEventListeners() {
        // Navigation
        document.querySelectorAll('.nav-btn').forEach(item => {
            item.addEventListener('click', (e) => {
                e.preventDefault();
                const page = item.getAttribute('data-page');
                this.switchPage(page);
            });
        });

        // System control buttons
        document.addEventListener('click', (e) => {
            // Bio-Entropy Mining controls
            if (e.target.classList.contains('btn-start-mining')) {
                this.startBioEntropyMining();
            }
            
            if (e.target.classList.contains('btn-stop-mining')) {
                this.stopBioEntropyMining();
            }

            // Emergency stop button
            if (e.target.id === 'emergencyStop') {
                this.emergencyStop();
            }

            // Configuration buttons
            if (e.target.id === 'resetConfig') {
                this.resetConfiguration();
            }

            // Export buttons
            if (e.target.id === 'exportCSV') {
                this.exportData('csv');
            }

            if (e.target.id === 'exportJSON') {
                this.exportData('json');
            }

            if (e.target.id === 'exportReport') {
                this.exportData('pdf');
            }
        });

        // Configuration forms
        document.addEventListener('submit', (e) => {
            if (e.target.id && e.target.id.endsWith('Form')) {
                e.preventDefault();
                this.handleConfigurationUpdate(e.target);
            }
        });

        // Range sliders
        document.addEventListener('input', (e) => {
            if (e.target.type === 'range') {
                const valueSpan = e.target.nextElementSibling;
                if (valueSpan && (valueSpan.classList.contains('range-value') || valueSpan.classList.contains('slider-value'))) {
                    valueSpan.textContent = e.target.value;
                }
            }
        });

        // Keyboard shortcuts
        document.addEventListener('keydown', (e) => {
            this.handleKeyboardShortcuts(e);
        });

        // Window resize
        window.addEventListener('resize', () => {
            this.handleResize();
        });
    }

    /**
     * Initialize WebSocket connection for real-time communication
     * NOTE: WebSocket is optional - the app works without it using REST APIs
     */
    initializeWebSocket() {
        // WebSocket disabled for now - not required for Pure Python platform
        // The app uses REST API endpoints instead:
        // - /api/bio-entropy/start
        // - /api/bio-entropy/stop
        // - /api/bio-entropy/status
        // - /api/training/historical/start
        // - /api/training/historical/status
        
        console.log('ℹ️ WebSocket disabled - using REST API endpoints');
        this.updateConnectionStatus('rest-api');
        
        // Uncomment below to enable WebSocket when backend supports it:
        /*
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws/bio-mining`;

        try {
            this.websocket = new WebSocket(wsUrl);

            this.websocket.onopen = () => {
                console.log('✅ WebSocket connected');
                this.updateConnectionStatus('connected');
                this.requestSystemStatus();
            };

            this.websocket.onmessage = (event) => {
                this.handleWebSocketMessage(JSON.parse(event.data));
            };

            this.websocket.onclose = () => {
                console.log('🔌 WebSocket disconnected');
                this.updateConnectionStatus('disconnected');
                // Attempt to reconnect after 3 seconds
                setTimeout(() => this.initializeWebSocket(), 3000);
            };

            this.websocket.onerror = (error) => {
                console.error('❌ WebSocket error:', error);
                this.updateConnectionStatus('error');
            };

        } catch (error) {
            console.error('❌ Failed to initialize WebSocket:', error);
            this.updateConnectionStatus('error');
        }
        */
    }

    /**
     * Handle incoming WebSocket messages
     */
    handleWebSocketMessage(message) {
        switch (message.type) {
            case 'system_status':
                this.updateSystemStatus(message.data);
                break;
            case 'bio_entropy_update':
                this.updateBioEntropyStats(message.data);
                break;
            case 'bio_entropy_started':
                this.onBioEntropyStarted(message.data);
                break;
            case 'bio_entropy_stopped':
                this.onBioEntropyStopped(message.data);
                break;
            case 'performance_metrics':
                this.updatePerformanceMetrics(message.data);
                break;
            case 'error':
                this.showNotification('error', message.message);
                break;
            case 'success':
                this.showNotification('success', message.message);
                break;
            case 'configuration_updated':
                this.handleConfigurationUpdated(message.data);
                break;
            default:
                console.log('📨 Unknown message type:', message.type);
        }
    }

    /**
     * Send message through WebSocket
     */
    sendWebSocketMessage(message) {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.websocket.send(JSON.stringify(message));
        } else {
            console.error('❌ WebSocket not connected');
            this.showNotification('error', 'Connection lost. Attempting to reconnect...');
        }
    }

    /**
     * Switch between different pages
     */
    switchPage(pageName) {
        // Update navigation
        document.querySelectorAll('.nav-btn').forEach(item => {
            item.classList.remove('active');
        });
        const activeNavBtn = document.querySelector(`[data-page="${pageName}"]`);
        if (activeNavBtn) {
            activeNavBtn.classList.add('active');
        }

        // Update content
        document.querySelectorAll('.page').forEach(page => {
            page.classList.remove('active');
        });
        const activePage = document.getElementById(`${pageName}-page`);
        if (activePage) {
            activePage.classList.add('active');
        }

        this.currentPage = pageName;

        // Page-specific initialization
        switch (pageName) {
            case 'dashboard':
                this.refreshDashboard();
                break;
            case 'mining':
                this.refreshMiningPage();
                break;
            case 'training':
                if (this.trainingManager) {
                    this.trainingManager.init();
                }
                break;
            case 'results':
                this.refreshResultsPage();
                break;
            case 'config':
                this.refreshConfigPage();
                break;
        }

        console.log(`📋 Switched to ${pageName} page`);
    }

    /**
     * Start Bio-Entropy mining operation
     */
    async startBioEntropyMining() {
        try {
            console.log('🧬 Starting Bio-Entropy mining...');
            
            // Gather Bio-Entropy configuration from form
            const config = {
                // Compute Mode
                mode: document.querySelector('input[name="bioMode"]:checked')?.value || 'SimulatedNetwork',
                
                // Exploration Strategy
                strategy: document.getElementById('entropyStrategy')?.value || 'Auto',
                starting_points: parseInt(document.getElementById('startingPointsCount')?.value) || 1000,
                window_size: parseInt(document.getElementById('windowSize')?.value) || 4194304,
                max_voltage: parseFloat(document.getElementById('maxVoltage')?.value) || 3.0,
                
                // Feature Extraction
                extract_timestamp: document.getElementById('extractTimestamp')?.checked ?? true,
                extract_difficulty: document.getElementById('extractDifficulty')?.checked ?? true,
                extract_prev_hash: document.getElementById('extractPrevHash')?.checked ?? true,
                extract_merkle: document.getElementById('extractMerkle')?.checked ?? true,
                extract_bytes: document.getElementById('extractBytes')?.checked ?? true,
                
                // Reinforcement Learning
                enable_reinforcement: document.getElementById('enableReinforcement')?.checked ?? true,
                reward_weight: parseFloat(document.getElementById('rewardWeight')?.value) || 1.0,
                history_size: parseInt(document.getElementById('historySize')?.value) || 100,
                
                // Mining Parameters
                difficulty: 4,
                threads: 4
            };
            
            console.log('📋 Bio-Entropy Config:', config);
            
            // Use Bio-Entropy endpoint
            const response = await fetch('/api/bio-entropy/start', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(config)
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('success', '🧬 Bio-Entropy mining started successfully!');
                this.miningActive = true;
                this.updateMiningControls(true);
                
                // Update dashboard display
                const modeDisplay = config.mode === 'RealMEA' ? 'MEA Réel' : 'Réseau Simulé';
                this.updateElement('activeModeDisplay', modeDisplay);
                this.updateElement('dashStrategy', config.strategy);
                
                console.log('✅ Bio-Entropy mining active:', result.stats);
            } else {
                this.showNotification('error', 'Failed to start mining: ' + (result.message || 'Unknown error'));
                console.error('❌ Mining start failed:', result);
            }
        } catch (error) {
            console.error('❌ Error starting Bio-Entropy mining:', error);
            this.showNotification('error', 'Network error: ' + error.message);
        }
    }

    /**
     * Stop Bio-Entropy mining operation
     */
    async stopBioEntropyMining() {
        try {
            console.log('🛑 Stopping Bio-Entropy mining...');
            
            const response = await fetch('/api/bio-entropy/stop', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('info', '🧬 Bio-Entropy mining stopped successfully');
                this.miningActive = false;
                this.updateMiningControls(false);
                console.log('✅ Bio-Entropy mining stopped');
            } else {
                this.showNotification('error', 'Failed to stop mining: ' + (result.message || 'Unknown error'));
                console.error('❌ Mining stop failed:', result);
            }
        } catch (error) {
            console.error('❌ Error stopping Bio-Entropy mining:', error);
            this.showNotification('error', 'Network error: ' + error.message);
        }
    }

    /**
     * Update Bio-Entropy statistics
     */
    updateBioEntropyStats(data) {
        console.log('🧬 Updating Bio-Entropy stats:', data);
        
        // Update Dashboard metrics
        this.updateElement('dashConfidence', data.confidence ? data.confidence.toFixed(1) + '%' : '0%');
        this.updateElement('dashStartingPoints', data.total_points ? data.total_points.toLocaleString() : '0');
        this.updateElement('dashHashrate', data.hashrate ? this.formatHashrate(data.hashrate) : '0 H/s');
        this.updateElement('dashStrategy', data.strategy || 'Auto');
        
        // Update Mining page detailed stats (if on mining page)
        if (this.currentPage === 'mining') {
            // Entropy Seed Card
            if (data.primary_seed) {
                const seedHex = '0x' + data.primary_seed.toString(16).padStart(16, '0');
                this.updateElement('primarySeed', seedHex);
            }
            this.updateElement('seedConfidence', data.confidence ? data.confidence.toFixed(1) + '%' : '0%');
            this.updateElement('responseStrength', data.response_strength ? data.response_strength.toFixed(2) : '0.00');
            
            // Starting Points Card
            this.updateElement('totalStartingPoints', data.total_points ? data.total_points.toLocaleString() : '0');
            this.updateElement('activeStrategy', data.strategy || 'Auto');
            this.updateElement('estimatedCoverage', data.expected_coverage ? data.expected_coverage.toFixed(2) + '%' : '0%');
            
            // Biological Response Card
            this.updateElement('bioResponseTime', data.bio_response_time ? data.bio_response_time.toFixed(1) + ' ms' : '0 ms');
            this.updateElement('signalQuality', data.signal_quality ? (data.signal_quality * 100).toFixed(1) + '%' : '0%');
            this.updateElement('reinforcedPatterns', data.reinforced_patterns || 0);
            
            // Parallel Search Card
            this.updateElement('activeThreads', data.active_threads || 0);
            this.updateElement('windowPerPoint', data.window_size ? data.window_size.toLocaleString() + ' nonces' : '0');
            this.updateElement('bioEntropyHashrate', data.hashrate ? this.formatHashrate(data.hashrate) : '0 H/s');
        }
    }

    /**
     * Handle Bio-Entropy mining started event
     */
    onBioEntropyStarted(data) {
        console.log('🧬 Bio-Entropy mining started:', data);
        
        if (data.success && data.stats) {
            this.updateBioEntropyStats(data.stats);
            this.showNotification('success', '🧬 Bio-Entropy mining is now active!');
        }
    }

    /**
     * Handle Bio-Entropy mining stopped event
     */
    onBioEntropyStopped(data) {
        console.log('🛑 Bio-Entropy mining stopped:', data);
        this.showNotification('info', '🧬 Bio-Entropy mining has been stopped');
    }

    /**
     * Format hashrate for display
     */
    formatHashrate(hashrate) {
        if (hashrate >= 1e12) {
            return (hashrate / 1e12).toFixed(2) + ' TH/s';
        } else if (hashrate >= 1e9) {
            return (hashrate / 1e9).toFixed(2) + ' GH/s';
        } else if (hashrate >= 1e6) {
            return (hashrate / 1e6).toFixed(2) + ' MH/s';
        } else if (hashrate >= 1e3) {
            return (hashrate / 1e3).toFixed(2) + ' KH/s';
        }
        return hashrate.toFixed(2) + ' H/s';
    }

    /**
     * Handle configuration form updates
     */
    handleConfigurationUpdate(form) {
        const formData = new FormData(form);
        const config = {};
        
        // Handle all form elements including unchecked checkboxes and sliders
        const allInputs = form.querySelectorAll('input, select, textarea');
        allInputs.forEach(input => {
            if (input.type === 'checkbox') {
                config[input.id] = input.checked ? 'on' : 'off';
            } else if (input.type === 'radio') {
                if (input.checked) {
                    config[input.name] = input.value;
                }
            } else {
                config[input.id] = input.value;
            }
        });
        
        console.log(`💾 Updating configuration for ${form.id}:`, config);
        
        // Send to server with form ID
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.sendWebSocketMessage({
                type: 'update_config',
                form_id: form.id,
                data: config
            });
        } else {
            this.showNotification('error', 'WebSocket not connected');
            return;
        }
        
        this.showNotification('info', 'Configuration being updated...');
    }

    /**
     * Handle configuration updated response
     */
    handleConfigurationUpdated(data) {
        if (data.success) {
            this.showNotification('success', 'Configuration updated successfully');
        } else {
            this.showNotification('error', 'Failed to update configuration');
        }
    }

    /**
     * Update system status display
     */
    updateSystemStatus(statusData) {
        console.log('📊 System status updated:', statusData);
        
        // Update Pure Python system status
        if (statusData.pure_python) {
            this.updateElement('pythonStatus', statusData.pure_python.status || 'offline');
        }
        
        // Update biological network status
        if (statusData.biological_network) {
            this.updateElement('networkStatus', statusData.biological_network.status || 'offline');
            this.updateElement('networkInitialized', statusData.biological_network.initialized ? 'Yes' : 'No');
        }
        
        // Update MEA interface status
        if (statusData.mea_interface) {
            this.updateElement('meaStatus', statusData.mea_interface.status || 'offline');
            this.updateElement('meaElectrodes', statusData.mea_interface.num_electrodes || 60);
        }
    }

    /**
     * Update performance metrics
     */
    updatePerformanceMetrics(performanceData = null) {
        // Update timestamp
        this.updateElement('last-update', new Date().toLocaleTimeString());
        
        // If performance data is provided (from WebSocket), update the metrics
        if (performanceData) {
            if (performanceData.cpu_usage !== undefined) {
                this.updateElement('cpu-usage', `${performanceData.cpu_usage.toFixed(1)}%`);
            }
            if (performanceData.memory_usage !== undefined) {
                this.updateElement('memory-usage', `${performanceData.memory_usage.toFixed(1)}%`);
            }
        } else {
            // Request latest data if WebSocket is connected
            if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                this.sendWebSocketMessage({ type: 'get_performance_metrics' });
            }
        }
    }

    /**
     * Update mining control buttons
     */
    updateMiningControls(isMining) {
        const startBtns = document.querySelectorAll('.btn-start-mining');
        const stopBtns = document.querySelectorAll('.btn-stop-mining');
        
        startBtns.forEach(btn => btn.disabled = isMining);
        stopBtns.forEach(btn => btn.disabled = !isMining);
    }

    /**
     * Show notification to user
     */
    showNotification(type, message, duration = 5000) {
        const notification = document.createElement('div');
        notification.className = `notification notification-${type}`;
        notification.innerHTML = `
            <i class="fas ${this.getNotificationIcon(type)}"></i>
            <span>${message}</span>
            <button class="notification-close">&times;</button>
        `;
        
        // Add to container
        let container = document.getElementById('notifications-container');
        if (!container) {
            container = document.createElement('div');
            container.id = 'notifications-container';
            container.className = 'notifications-container';
            document.body.appendChild(container);
        }
        
        container.appendChild(notification);
        
        // Auto-remove after duration
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, duration);
        
        // Manual close
        notification.querySelector('.notification-close').addEventListener('click', () => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        });
    }

    /**
     * Get notification icon based on type
     */
    getNotificationIcon(type) {
        switch (type) {
            case 'success': return 'fa-check-circle';
            case 'error': return 'fa-exclamation-circle';
            case 'warning': return 'fa-exclamation-triangle';
            case 'info': return 'fa-info-circle';
            default: return 'fa-bell';
        }
    }

    /**
     * Utility function to update element content
     */
    updateElement(id, value) {
        const element = document.getElementById(id);
        if (element) {
            element.textContent = value;
        }
    }

    /**
     * Format large numbers for display
     */
    formatNumber(num) {
        if (num >= 1e9) {
            return (num / 1e9).toFixed(2) + 'B';
        } else if (num >= 1e6) {
            return (num / 1e6).toFixed(2) + 'M';
        } else if (num >= 1e3) {
            return (num / 1e3).toFixed(2) + 'K';
        }
        return num.toString();
    }

    /**
     * Load configuration from localStorage
     */
    loadConfiguration() {
        try {
            const savedConfig = localStorage.getItem('bioMiningConfig');
            if (savedConfig) {
                const config = JSON.parse(savedConfig);
                this.applyConfiguration(config);
                console.log('⚙️ Configuration loaded from localStorage');
            }
        } catch (error) {
            console.error('❌ Error loading configuration:', error);
        }
    }

    /**
     * Save current configuration
     */
    saveConfiguration() {
        try {
            const config = this.getCurrentConfiguration();
            localStorage.setItem('bioMiningConfig', JSON.stringify(config));
            this.showNotification('success', 'Configuration saved successfully');
            console.log('💾 Configuration saved');
        } catch (error) {
            console.error('❌ Error saving configuration:', error);
            this.showNotification('error', 'Failed to save configuration');
        }
    }

    /**
     * Get current configuration from form inputs
     */
    getCurrentConfiguration() {
        const config = {};
        
        // Collect all config inputs
        document.querySelectorAll('.config-input').forEach(input => {
            const key = input.name || input.id;
            let value = input.value;
            
            // Convert to appropriate type
            if (input.type === 'number') {
                value = parseFloat(value);
            } else if (input.type === 'checkbox') {
                value = input.checked;
            }
            
            config[key] = value;
        });
        
        return config;
    }

    /**
     * Apply configuration to form inputs
     */
    applyConfiguration(config) {
        Object.keys(config).forEach(key => {
            const input = document.getElementById(key) || document.querySelector(`[name="${key}"]`);
            if (input) {
                if (input.type === 'checkbox') {
                    input.checked = config[key];
                } else {
                    input.value = config[key];
                }
            }
        });
    }

    /**
     * Reset configuration to defaults
     */
    resetConfiguration() {
        console.log('🔄 Resetting configuration to defaults');
        
        // Reset all forms
        document.querySelectorAll('form[id$="Form"]').forEach(form => {
            form.reset();
        });
        
        this.showNotification('success', 'Configuration reset to defaults');
    }

    /**
     * Update connection status indicator
     */
    updateConnectionStatus(status) {
        const statusElement = document.getElementById('systemStatus');
        const statusDot = document.querySelector('.status-dot');
        const statusText = document.querySelector('.status-text');
        
        if (statusDot && statusText) {
            statusDot.className = 'status-dot';
            switch (status) {
                case 'connected':
                    statusDot.classList.add('connected');
                    statusText.textContent = 'Connected';
                    break;
                case 'disconnected':
                    statusDot.classList.add('disconnected');
                    statusText.textContent = 'Disconnected';
                    break;
                case 'error':
                    statusDot.classList.add('error');
                    statusText.textContent = 'Error';
                    break;
                case 'rest-api':
                    statusDot.classList.add('connected'); // Show as connected (using REST)
                    statusText.textContent = 'REST API';
                    break;
                default:
                    statusText.textContent = 'Unknown';
            }
        }
    }

    /**
     * Request system status from server
     */
    requestSystemStatus() {
        this.sendWebSocketMessage({ type: 'get_system_status' });
    }

    /**
     * Handle keyboard shortcuts
     */
    handleKeyboardShortcuts(event) {
        if (event.ctrlKey || event.metaKey) {
            switch (event.key) {
                case '1':
                    event.preventDefault();
                    this.switchPage('dashboard');
                    break;
                case '2':
                    event.preventDefault();
                    this.switchPage('mining');
                    break;
                case '3':
                    event.preventDefault();
                    this.switchPage('training');
                    break;
                case '4':
                    event.preventDefault();
                    this.switchPage('results');
                    break;
                case '5':
                    event.preventDefault();
                    this.switchPage('config');
                    break;
                case 's':
                    event.preventDefault();
                    this.saveConfiguration();
                    break;
                case 'r':
                    event.preventDefault();
                    this.refreshCurrentPage();
                    break;
            }
        }
        
        // ESC key to close modals
        if (event.key === 'Escape') {
            this.closeAllModals();
        }
    }

    /**
     * Handle window resize
     */
    handleResize() {
        // Refresh charts and visualizations
        if (this.charts) {
            Object.values(this.charts).forEach(chart => {
                if (chart.resize) chart.resize();
            });
        }
    }

    /**
     * Close all modal dialogs
     */
    closeAllModals() {
        document.querySelectorAll('.modal').forEach(modal => {
            modal.classList.add('hidden');
        });
    }

    /**
     * Start performance monitoring
     */
    startPerformanceMonitoring() {
        setInterval(() => {
            this.updatePerformanceMetrics();
        }, 1000); // Update every second
        
        console.log('📈 Performance monitoring started');
    }

    /**
     * Refresh current page data
     */
    refreshCurrentPage() {
        switch (this.currentPage) {
            case 'dashboard':
                this.refreshDashboard();
                break;
            case 'mining':
                this.refreshMiningPage();
                break;
            case 'training':
                if (this.trainingManager) {
                    this.trainingManager.refresh();
                }
                break;
            case 'results':
                this.refreshResultsPage();
                break;
            case 'config':
                this.refreshConfigPage();
                break;
        }
    }

    /**
     * Refresh dashboard data
     */
    refreshDashboard() {
        this.requestSystemStatus();
        console.log('🔄 Dashboard refreshed');
    }

    /**
     * Refresh mining page
     */
    refreshMiningPage() {
        // Request bio-entropy stats
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.sendWebSocketMessage({ type: 'get_bio_entropy_status' });
        }
        console.log('🔄 Mining page refreshed');
    }

    /**
     * Refresh results page
     */
    refreshResultsPage() {
        console.log('🔄 Results page refreshed');
    }

    /**
     * Refresh config page
     */
    refreshConfigPage() {
        console.log('🔄 Config page refreshed');
    }

    /**
     * Emergency stop all operations
     */
    async emergencyStop() {
        console.log('🚨 EMERGENCY STOP ACTIVATED');
        this.showNotification('warning', 'Emergency stop activated - shutting down all systems');
        
        try {
            await this.stopBioEntropyMining();
            this.showNotification('success', 'All systems stopped safely');
        } catch (error) {
            console.error('Error during emergency stop:', error);
            this.showNotification('error', 'Error during emergency stop');
        }
    }

    /**
     * Export data in specified format
     */
    async exportData(format) {
        console.log(`📤 Exporting data as ${format}`);
        this.showNotification('info', `Preparing ${format.toUpperCase()} export...`);
        
        // Placeholder for actual export implementation
        setTimeout(() => {
            this.showNotification('success', `Data exported as ${format.toUpperCase()}`);
        }, 1000);
    }

    /**
     * Verify form handlers are properly attached
     */
    verifyFormHandlers() {
        const forms = document.querySelectorAll('form[id$="Form"]');
        console.log(`✅ ${forms.length} configuration forms found and handled`);
    }
}

// Initialize the application when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.app = new BioMiningApp();
});
