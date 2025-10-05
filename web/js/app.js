/**
 * ================================================================
 * HYBRID BITCOIN MINING PLATFORM - MAIN APPLICATION
 * Advanced JavaScript for Triple System Interface Management
 * (SHA-256 + BiologicalNetwork + RealMEAInterface)
 * ================================================================
 */

class HybridBitcoinMiningApp {
    constructor() {
        this.init();
        this.websocket = null;
        this.currentPanel = 'dashboard';
        this.systems = {
            sha256: { status: 'offline', hashrate: 0, blocks: 0 },
            biological: { status: 'offline', neurons: 0, synapses: 0, learning_rate: 0 },
            mea: { status: 'offline', electrodes: 60, active_electrodes: 0, stimulation: false }
        };
        this.miningStats = {
            totalHashes: 0,
            validNonces: 0,
            successRate: 0,
            avgResponseTime: 0
        };
        this.meaElectrodes = this.initializeElectrodes();
        this.charts = {};
        
        console.log('🚀 Hybrid Bitcoin Mining Platform initialized');
    }

    /**
     * Initialize the application
     */
    init() {
        this.setupEventListeners();
        this.initializeWebSocket();
        this.loadConfiguration();
        this.startPerformanceMonitoring();
        this.setupMEAVisualization();
    }

    /**
     * Setup all event listeners for the interface
     */
    setupEventListeners() {
        // Navigation
        document.querySelectorAll('.nav-item').forEach(item => {
            item.addEventListener('click', (e) => {
                e.preventDefault();
                const panel = item.getAttribute('data-panel');
                this.switchPanel(panel);
            });
        });

        // System control buttons
        document.addEventListener('click', (e) => {
            if (e.target.classList.contains('btn-start-system')) {
                const system = e.target.getAttribute('data-system');
                this.startSystem(system);
            }
            
            if (e.target.classList.contains('btn-stop-system')) {
                const system = e.target.getAttribute('data-system');
                this.stopSystem(system);
            }

            if (e.target.classList.contains('btn-start-mining')) {
                this.startMining();
            }

            if (e.target.classList.contains('btn-stop-mining')) {
                this.stopMining();
            }

            if (e.target.classList.contains('btn-start-training')) {
                this.startTraining();
            }

            if (e.target.classList.contains('btn-electrode')) {
                const electrodeId = parseInt(e.target.getAttribute('data-electrode'));
                this.toggleElectrode(electrodeId);
            }
        });

        // Configuration forms
        document.addEventListener('change', (e) => {
            if (e.target.classList.contains('config-input')) {
                this.updateConfiguration(e.target);
            }
        });

        // File uploads
        document.addEventListener('change', (e) => {
            if (e.target.classList.contains('file-upload')) {
                this.handleFileUpload(e.target);
            }
        });

        // Drag and drop for file uploads
        document.addEventListener('dragover', (e) => {
            e.preventDefault();
            if (e.target.classList.contains('upload-area')) {
                e.target.classList.add('drag-over');
            }
        });

        document.addEventListener('dragleave', (e) => {
            if (e.target.classList.contains('upload-area')) {
                e.target.classList.remove('drag-over');
            }
        });

        document.addEventListener('drop', (e) => {
            e.preventDefault();
            if (e.target.classList.contains('upload-area')) {
                e.target.classList.remove('drag-over');
                this.handleFileDrop(e.target, e.dataTransfer.files);
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
     */
    initializeWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws/hybrid-mining`;

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
    }

    /**
     * Handle incoming WebSocket messages
     */
    handleWebSocketMessage(message) {
        switch (message.type) {
            case 'system_status':
                this.updateSystemStatus(message.data);
                break;
            case 'mining_update':
                this.updateMiningStats(message.data);
                break;
            case 'electrode_data':
                this.updateElectrodeData(message.data);
                break;
            case 'training_progress':
                this.updateTrainingProgress(message.data);
                break;
            case 'biological_activity':
                this.updateBiologicalActivity(message.data);
                break;
            case 'error':
                this.showNotification('error', message.message);
                break;
            case 'success':
                this.showNotification('success', message.message);
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
     * Switch between different panels
     */
    switchPanel(panelName) {
        // Update navigation
        document.querySelectorAll('.nav-item').forEach(item => {
            item.classList.remove('active');
        });
        document.querySelector(`[data-panel="${panelName}"]`).classList.add('active');

        // Update content
        document.querySelectorAll('.panel-section').forEach(section => {
            section.classList.add('hidden');
        });
        document.getElementById(`${panelName}-section`).classList.remove('hidden');

        this.currentPanel = panelName;

        // Panel-specific initialization
        switch (panelName) {
            case 'dashboard':
                this.refreshDashboard();
                break;
            case 'mea-control':
                this.refreshMEAVisualization();
                break;
            case 'mining':
                this.refreshMiningCharts();
                break;
            case 'results':
                this.refreshResultsCharts();
                break;
        }

        console.log(`📋 Switched to ${panelName} panel`);
    }

    /**
     * Initialize MEA electrodes array
     */
    initializeElectrodes() {
        const electrodes = [];
        for (let i = 1; i <= 60; i++) {
            electrodes.push({
                id: i,
                active: false,
                stimulating: false,
                recording: false,
                voltage: 0,
                impedance: Math.random() * 1000 + 500, // kΩ
                lastActivity: null
            });
        }
        return electrodes;
    }

    /**
     * Setup MEA visualization grid
     */
    setupMEAVisualization() {
        const meaGrid = document.getElementById('mea-grid');
        if (!meaGrid) return;

        meaGrid.innerHTML = '';
        
        // Create 8x8 grid (64 positions, but only 60 electrodes)
        for (let i = 0; i < 64; i++) {
            const electrode = document.createElement('div');
            
            if (i < 60) { // Active electrode positions
                const electrodeData = this.meaElectrodes[i];
                electrode.className = 'electrode';
                electrode.setAttribute('data-electrode', electrodeData.id);
                electrode.innerHTML = `
                    <span>${electrodeData.id}</span>
                    <div class="electrode-tooltip">
                        Electrode ${electrodeData.id}<br>
                        Impedance: ${electrodeData.impedance.toFixed(1)}kΩ<br>
                        Status: ${electrodeData.active ? 'Active' : 'Inactive'}
                    </div>
                `;
                
                // Add click listener
                electrode.addEventListener('click', () => {
                    this.toggleElectrode(electrodeData.id);
                });
                
            } else { // Empty positions
                electrode.className = 'electrode-empty';
            }
            
            meaGrid.appendChild(electrode);
        }
    }

    /**
     * Toggle electrode state
     */
    toggleElectrode(electrodeId) {
        const electrode = this.meaElectrodes.find(e => e.id === electrodeId);
        if (electrode) {
            electrode.active = !electrode.active;
            this.updateElectrodeDisplay(electrodeId);
            
            // Send command to backend
            this.sendWebSocketMessage({
                type: 'electrode_control',
                electrode_id: electrodeId,
                active: electrode.active
            });
            
            console.log(`🔌 Electrode ${electrodeId} ${electrode.active ? 'activated' : 'deactivated'}`);
        }
    }

    /**
     * Update electrode visual display
     */
    updateElectrodeDisplay(electrodeId) {
        const electrodeElement = document.querySelector(`[data-electrode="${electrodeId}"]`);
        if (!electrodeElement) return;

        const electrode = this.meaElectrodes.find(e => e.id === electrodeId);
        
        // Update classes
        electrodeElement.classList.remove('active', 'stimulating', 'recording');
        
        if (electrode.active) {
            electrodeElement.classList.add('active');
        }
        if (electrode.stimulating) {
            electrodeElement.classList.add('stimulating');
        }
        if (electrode.recording) {
            electrodeElement.classList.add('recording');
        }

        // Update tooltip
        const tooltip = electrodeElement.querySelector('.electrode-tooltip');
        if (tooltip) {
            tooltip.innerHTML = `
                Electrode ${electrode.id}<br>
                Impedance: ${electrode.impedance.toFixed(1)}kΩ<br>
                Voltage: ${electrode.voltage.toFixed(2)}mV<br>
                Status: ${electrode.active ? 'Active' : 'Inactive'}
            `;
        }
    }

    /**
     * Start a specific system
     */
    async startSystem(systemName) {
        try {
            this.showNotification('info', `Starting ${systemName} system...`);
            
            this.sendWebSocketMessage({
                type: 'start_system',
                system: systemName
            });
            
            // Update button states
            const startBtn = document.querySelector(`.btn-start-system[data-system="${systemName}"]`);
            const stopBtn = document.querySelector(`.btn-stop-system[data-system="${systemName}"]`);
            
            if (startBtn) startBtn.disabled = true;
            if (stopBtn) stopBtn.disabled = false;
            
            console.log(`🚀 Starting ${systemName} system`);
            
        } catch (error) {
            console.error(`❌ Error starting ${systemName} system:`, error);
            this.showNotification('error', `Failed to start ${systemName} system`);
        }
    }

    /**
     * Stop a specific system
     */
    async stopSystem(systemName) {
        try {
            this.showNotification('info', `Stopping ${systemName} system...`);
            
            this.sendWebSocketMessage({
                type: 'stop_system',
                system: systemName
            });
            
            // Update button states
            const startBtn = document.querySelector(`.btn-start-system[data-system="${systemName}"]`);
            const stopBtn = document.querySelector(`.btn-stop-system[data-system="${systemName}"]`);
            
            if (startBtn) startBtn.disabled = false;
            if (stopBtn) stopBtn.disabled = true;
            
            console.log(`🛑 Stopping ${systemName} system`);
            
        } catch (error) {
            console.error(`❌ Error stopping ${systemName} system:`, error);
            this.showNotification('error', `Failed to stop ${systemName} system`);
        }
    }

    /**
     * Start mining operation
     */
    startMining() {
        this.sendWebSocketMessage({
            type: 'start_mining',
            method: 'triple_system' // Use all three systems
        });
        
        // Update UI
        document.getElementById('start-mining-btn').disabled = true;
        document.getElementById('stop-mining-btn').disabled = false;
        
        this.showNotification('success', 'Mining started with triple system optimization');
        console.log('⛏️ Mining started');
    }

    /**
     * Stop mining operation
     */
    stopMining() {
        this.sendWebSocketMessage({
            type: 'stop_mining'
        });
        
        // Update UI
        document.getElementById('start-mining-btn').disabled = false;
        document.getElementById('stop-mining-btn').disabled = true;
        
        this.showNotification('info', 'Mining stopped');
        console.log('🛑 Mining stopped');
    }

    /**
     * Start training process
     */
    startTraining() {
        const trainingConfig = this.getTrainingConfiguration();
        
        this.sendWebSocketMessage({
            type: 'start_training',
            config: trainingConfig
        });
        
        // Update UI
        document.getElementById('start-training-btn').disabled = true;
        document.getElementById('stop-training-btn').disabled = false;
        
        this.showNotification('success', 'Training started');
        console.log('🧠 Training started with config:', trainingConfig);
    }

    /**
     * Get current training configuration
     */
    getTrainingConfiguration() {
        return {
            biological_epochs: parseInt(document.getElementById('biological-epochs')?.value || 1000),
            mea_stimulation_frequency: parseFloat(document.getElementById('mea-frequency')?.value || 10.0),
            learning_rate: parseFloat(document.getElementById('learning-rate')?.value || 0.001),
            batch_size: parseInt(document.getElementById('batch-size')?.value || 32),
            target_accuracy: parseFloat(document.getElementById('target-accuracy')?.value || 0.85)
        };
    }

    /**
     * Update system status display
     */
    updateSystemStatus(statusData) {
        Object.keys(statusData).forEach(system => {
            const status = statusData[system];
            this.systems[system] = { ...this.systems[system], ...status };
            
            // Update status indicators
            const indicator = document.getElementById(`${system}-status`);
            if (indicator) {
                indicator.className = `status-indicator ${status.status}`;
                indicator.textContent = status.status.toUpperCase();
            }
            
            // Update system metrics
            this.updateSystemMetrics(system, status);
        });
        
        console.log('📊 System status updated');
    }

    /**
     * Update system-specific metrics
     */
    updateSystemMetrics(system, status) {
        switch (system) {
            case 'sha256':
                this.updateElement('sha256-hashrate', `${(status.hashrate / 1e6).toFixed(2)} MH/s`);
                this.updateElement('sha256-blocks', status.blocks_mined || 0);
                break;
                
            case 'biological':
                this.updateElement('biological-neurons', status.active_neurons || 0);
                this.updateElement('biological-synapses', status.active_synapses || 0);
                this.updateElement('biological-learning-rate', `${(status.learning_rate || 0).toFixed(4)}`);
                break;
                
            case 'mea':
                this.updateElement('mea-active-electrodes', status.active_electrodes || 0);
                this.updateElement('mea-total-electrodes', 60);
                this.updateElement('mea-stimulation', status.stimulation ? 'Active' : 'Inactive');
                break;
        }
    }

    /**
     * Update mining statistics
     */
    updateMiningStats(miningData) {
        this.miningStats = { ...this.miningStats, ...miningData };
        
        // Update dashboard metrics
        this.updateElement('total-hashes', this.formatNumber(this.miningStats.totalHashes));
        this.updateElement('valid-nonces', this.miningStats.validNonces);
        this.updateElement('success-rate', `${(this.miningStats.successRate * 100).toFixed(2)}%`);
        this.updateElement('avg-response-time', `${this.miningStats.avgResponseTime.toFixed(2)}ms`);
        
        // Update charts if visible
        if (this.currentPanel === 'mining' || this.currentPanel === 'results') {
            this.updateMiningCharts(miningData);
        }
        
        console.log('⛏️ Mining stats updated');
    }

    /**
     * Update electrode data from MEA interface
     */
    updateElectrodeData(electrodeData) {
        electrodeData.forEach(data => {
            const electrode = this.meaElectrodes.find(e => e.id === data.electrode_id);
            if (electrode) {
                electrode.voltage = data.voltage;
                electrode.impedance = data.impedance;
                electrode.active = data.active;
                electrode.stimulating = data.stimulating;
                electrode.recording = data.recording;
                electrode.lastActivity = new Date();
                
                this.updateElectrodeDisplay(electrode.id);
            }
        });
        
        // Update active electrodes count
        const activeCount = this.meaElectrodes.filter(e => e.active).length;
        this.systems.mea.active_electrodes = activeCount;
        this.updateElement('mea-active-electrodes', activeCount);
    }

    /**
     * Update training progress
     */
    updateTrainingProgress(progressData) {
        const { epoch, total_epochs, loss, accuracy, system } = progressData;
        
        // Update progress bars
        const progressBar = document.getElementById(`${system}-training-progress`);
        if (progressBar) {
            const percentage = (epoch / total_epochs) * 100;
            progressBar.querySelector('.progress-fill').style.width = `${percentage}%`;
            progressBar.querySelector('.progress-text').textContent = 
                `Epoch ${epoch}/${total_epochs} - Loss: ${loss.toFixed(4)} - Accuracy: ${(accuracy * 100).toFixed(1)}%`;
        }
        
        console.log(`🧠 Training progress (${system}): ${epoch}/${total_epochs}`);
    }

    /**
     * Update biological activity display
     */
    updateBiologicalActivity(activityData) {
        const { neuron_activity, synaptic_strength, firing_patterns } = activityData;
        
        // Update real-time activity display
        const activityDisplay = document.getElementById('biological-activity');
        if (activityDisplay) {
            activityDisplay.innerHTML = `
                <div class="activity-metric">
                    <span class="metric-label">Neuron Activity:</span>
                    <span class="metric-value">${neuron_activity.toFixed(2)}</span>
                </div>
                <div class="activity-metric">
                    <span class="metric-label">Synaptic Strength:</span>
                    <span class="metric-value">${synaptic_strength.toFixed(4)}</span>
                </div>
                <div class="activity-metric">
                    <span class="metric-label">Firing Rate:</span>
                    <span class="metric-value">${firing_patterns.rate.toFixed(1)} Hz</span>
                </div>
            `;
        }
    }

    /**
     * Handle file upload
     */
    async handleFileUpload(input) {
        const file = input.files[0];
        if (!file) return;
        
        const formData = new FormData();
        formData.append('file', file);
        formData.append('type', input.getAttribute('data-type') || 'training');
        
        try {
            const response = await fetch('/api/upload', {
                method: 'POST',
                body: formData
            });
            
            if (response.ok) {
                const result = await response.json();
                this.showNotification('success', `File uploaded successfully: ${file.name}`);
                console.log('📁 File uploaded:', result);
            } else {
                throw new Error('Upload failed');
            }
        } catch (error) {
            console.error('❌ Upload error:', error);
            this.showNotification('error', `Failed to upload file: ${file.name}`);
        }
    }

    /**
     * Handle file drop
     */
    handleFileDrop(dropArea, files) {
        Array.from(files).forEach(file => {
            // Create temporary input to trigger upload
            const input = document.createElement('input');
            input.type = 'file';
            input.files = files;
            input.setAttribute('data-type', dropArea.getAttribute('data-type'));
            this.handleFileUpload(input);
        });
    }

    /**
     * Handle keyboard shortcuts
     */
    handleKeyboardShortcuts(event) {
        if (event.ctrlKey || event.metaKey) {
            switch (event.key) {
                case '1':
                    event.preventDefault();
                    this.switchPanel('dashboard');
                    break;
                case '2':
                    event.preventDefault();
                    this.switchPanel('configuration');
                    break;
                case '3':
                    event.preventDefault();
                    this.switchPanel('networks');
                    break;
                case '4':
                    event.preventDefault();
                    this.switchPanel('mea-control');
                    break;
                case '5':
                    event.preventDefault();
                    this.switchPanel('training');
                    break;
                case '6':
                    event.preventDefault();
                    this.switchPanel('mining');
                    break;
                case '7':
                    event.preventDefault();
                    this.switchPanel('results');
                    break;
                case 's':
                    event.preventDefault();
                    this.saveConfiguration();
                    break;
                case 'r':
                    event.preventDefault();
                    this.refreshCurrentPanel();
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
        
        // Adjust MEA grid if needed
        this.refreshMEAVisualization();
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
     * Load configuration from localStorage or server
     */
    loadConfiguration() {
        try {
            const savedConfig = localStorage.getItem('hybridMiningConfig');
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
            localStorage.setItem('hybridMiningConfig', JSON.stringify(config));
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
     * Update connection status indicator
     */
    updateConnectionStatus(status) {
        const indicator = document.getElementById('connection-status');
        if (indicator) {
            indicator.className = `status-indicator ${status}`;
            indicator.textContent = status.toUpperCase();
        }
    }

    /**
     * Request system status from server
     */
    requestSystemStatus() {
        this.sendWebSocketMessage({ type: 'get_system_status' });
    }

    /**
     * Refresh current panel data
     */
    refreshCurrentPanel() {
        switch (this.currentPanel) {
            case 'dashboard':
                this.refreshDashboard();
                break;
            case 'mea-control':
                this.refreshMEAVisualization();
                break;
            case 'mining':
                this.refreshMiningCharts();
                break;
            case 'results':
                this.refreshResultsCharts();
                break;
        }
    }

    /**
     * Refresh dashboard data
     */
    refreshDashboard() {
        this.requestSystemStatus();
        // Additional dashboard-specific refreshes
        console.log('🔄 Dashboard refreshed');
    }

    /**
     * Refresh MEA visualization
     */
    refreshMEAVisualization() {
        // Update all electrode displays
        this.meaElectrodes.forEach(electrode => {
            this.updateElectrodeDisplay(electrode.id);
        });
        console.log('🔄 MEA visualization refreshed');
    }

    /**
     * Refresh mining charts (placeholder for chart library integration)
     */
    refreshMiningCharts() {
        // This would integrate with Chart.js or similar library
        console.log('📊 Mining charts refreshed');
    }

    /**
     * Refresh results charts (placeholder for chart library integration)
     */
    refreshResultsCharts() {
        // This would integrate with Chart.js or similar library
        console.log('📊 Results charts refreshed');
    }

    /**
     * Update mining charts with new data
     */
    updateMiningCharts(data) {
        // Chart update logic would go here
        console.log('📊 Mining charts updated with new data');
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
     * Update performance metrics
     */
    updatePerformanceMetrics() {
        // Update timestamp
        this.updateElement('last-update', new Date().toLocaleTimeString());
        
        // Request latest data if WebSocket is connected
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.sendWebSocketMessage({ type: 'get_performance_metrics' });
        }
    }
}

/**
 * Initialize application when DOM is loaded
 */
document.addEventListener('DOMContentLoaded', () => {
    window.hybridMiningApp = new HybridBitcoinMiningApp();
});

/**
 * Export for potential module usage
 */
if (typeof module !== 'undefined' && module.exports) {
    module.exports = HybridBitcoinMiningApp;
}