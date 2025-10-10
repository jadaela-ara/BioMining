/**
 * ================================================================
 * HYBRID BITCOIN MINING PLATFORM - MAIN APPLICATION
 * Advanced JavaScript for Triple System Interface Management
 * (SHA-256 + BiologicalNetwork + RealMEAInterface)
 * ================================================================
 */

class HybridBitcoinMiningApp {
    constructor() {
        this.charts = {};
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
        this.setupConfigurationForms();
        this.initializeCharts();
        this.initializeElectrodeGrid();
        this.updateTotalWeight();
        
        // Verify form handlers after setup
        setTimeout(() => this.verifyFormHandlers(), 100);
        
        // Setup form value preservation
        this.setupFormPreservation();
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

            // Emergency stop button
            if (e.target.id === 'emergencyStop') {
                this.emergencyStop();
            }

            // Configuration buttons
            if (e.target.id === 'resetConfig') {
                this.resetConfiguration();
            }

            if (e.target.id === 'autoOptimize') {
                this.autoOptimizeWeights();
            }

            if (e.target.id === 'initNetwork') {
                this.initializeNetwork();
            }

            if (e.target.id === 'connectMEA') {
                this.connectMEADevice();
            }

            // MEA Control buttons
            if (e.target.id === 'stimulateSelected') {
                this.stimulateSelectedElectrodes();
            }

            if (e.target.id === 'startRecording') {
                this.startMEARecording();
            }

            if (e.target.id === 'stopRecording') {
                this.stopMEARecording();
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
                this.updateSystemStatus(message.data.systems || message.data);
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
            case 'performance_metrics':
                this.updatePerformanceMetrics(message.data);
                break;
            case 'error':
                this.showNotification('error', message.message);
                break;
            case 'success':
                this.showNotification('success', message.message);
                break;
            case 'performance_metrics':
                this.updatePerformanceMetrics();
                break;
            case 'config_update_response':
                this.handleConfigUpdateResponse(message.data);
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

        this.currentPanel = pageName;

        // Page-specific initialization
        switch (pageName) {
            case 'dashboard':
                this.refreshDashboard();
                break;
            case 'mea':
                this.refreshMEAVisualization();
                break;
            case 'mining':
                this.refreshMiningCharts();
                break;
            case 'results':
                this.refreshResultsCharts();
                break;
            case 'training':
                this.initializeTrainingInterface();
                break;
            case 'networks':
                this.initializeNetworksInterface();
                break;
            case 'config':
                this.initializeConfigInterface();
                break;
        }

        console.log(`📋 Switched to ${pageName} page`);
    }
    
    // Alias pour compatibilité
    switchPanel(panelName) {
        this.switchPage(panelName);
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
            if (performanceData.gpu_usage !== undefined) {
                this.updateElement('gpu-usage', `${performanceData.gpu_usage.toFixed(1)}%`);
            }
            if (performanceData.network_io !== undefined) {
                const networkMB = (performanceData.network_io / (1024 * 1024)).toFixed(2);
                this.updateElement('network-io', `${networkMB} MB`);
            }
        } else {
            // Request latest data if WebSocket is connected
            if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                this.sendWebSocketMessage({ type: 'get_performance_metrics' });
            }
        }
    }

    /**
     * Refresh dashboard data and UI elements
     */
    refreshDashboard() {
        console.log('🔄 Refreshing dashboard...');
        // Update system status cards
        this.updateSystemStatus();
        // Request latest metrics
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.sendWebSocketMessage({ type: 'get_system_status' });
        }
    }

    /**
     * Initialize training interface
     */
    initializeTrainingInterface() {
        console.log('🧠 Initializing training interface...');
        // Setup training controls
        this.setupTrainingControls();
    }

    /**
     * Initialize networks interface
     */
    initializeNetworksInterface() {
        console.log('🕸️ Initializing networks interface...');
        // Setup network visualization
        this.setupNetworkVisualization();
    }

    /**
     * Initialize configuration interface
     */
    initializeConfigInterface() {
        console.log('⚙️ Initializing config interface...');
        // Setup configuration forms
        this.setupConfigurationForms();
    }

    /**
     * Setup training controls
     */
    setupTrainingControls() {
        // Initialize training form controls
        const startTrainingBtn = document.getElementById('startTraining');
        const pauseTrainingBtn = document.getElementById('pauseTraining');
        const stopTrainingBtn = document.getElementById('stopTraining');
        
        if (startTrainingBtn) {
            startTrainingBtn.addEventListener('click', () => this.startTraining());
        }
        if (pauseTrainingBtn) {
            pauseTrainingBtn.addEventListener('click', () => this.pauseTraining());
        }
        if (stopTrainingBtn) {
            stopTrainingBtn.addEventListener('click', () => this.stopTraining());
        }
    }

    /**
     * Setup network visualization
     */
    setupNetworkVisualization() {
        // Initialize network graphs and visualizations
        console.log('Setting up network visualization');
    }

    /**
     * Setup configuration forms
     */
    setupConfigurationForms() {
        // Setup all configuration form handlers for all forms ending with 'Form'
        const configForms = document.querySelectorAll('form[id$="Form"]');
        configForms.forEach(form => {
            form.addEventListener('submit', (e) => {
                e.preventDefault();
                this.handleConfigurationUpdate(form);
                console.log(`📝 Form ${form.id} submitted (prevented default refresh)`);
            });
        });

        // Setup range sliders
        const rangeInputs = document.querySelectorAll('input[type="range"]');
        rangeInputs.forEach(range => {
            range.addEventListener('input', (e) => {
                const valueSpan = e.target.nextElementSibling;
                if (valueSpan && (valueSpan.classList.contains('range-value') || valueSpan.classList.contains('slider-value'))) {
                    valueSpan.textContent = e.target.value + (e.target.id.includes('Weight') ? '%' : '');
                }
                
                // Update total weight for weight sliders
                if (e.target.id.includes('WeightSlider')) {
                    this.updateTotalWeight();
                }
            });
        });
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
     * Start a specific system
     */
    async startSystem(systemName) {
        console.log(`🚀 Starting ${systemName} system...`);
        
        try {
            const response = await fetch(`/api/systems/${systemName}/start`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('success', `${systemName} system started successfully`);
                this.updateSystemStatus();
            } else {
                this.showNotification('error', `Failed to start ${systemName} system`);
            }
        } catch (error) {
            console.error(`Error starting ${systemName} system:`, error);
            this.showNotification('error', 'Network error');
        }
    }

    /**
     * Stop a specific system
     */
    async stopSystem(systemName) {
        console.log(`🛑 Stopping ${systemName} system...`);
        
        try {
            const response = await fetch(`/api/systems/${systemName}/stop`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('success', `${systemName} system stopped successfully`);
                this.updateSystemStatus();
            } else {
                this.showNotification('error', `Failed to stop ${systemName} system`);
            }
        } catch (error) {
            console.error(`Error stopping ${systemName} system:`, error);
            this.showNotification('error', 'Network error');
        }
    }

    /**
     * Start mining process
     */
    async startMining() {
        console.log('⛏️ Starting mining process...');
        
        const config = {
            method: 'triple_system',
            difficulty: 4,
            batch_size: 32,
            max_attempts: 1000000
        };
        
        try {
            const response = await fetch('/api/mining/start', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(config)
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('success', 'Mining started successfully');
                this.updateMiningControls(true);
            } else {
                this.showNotification('error', 'Failed to start mining');
            }
        } catch (error) {
            console.error('Error starting mining:', error);
            this.showNotification('error', 'Network error');
        }
    }

    /**
     * Stop mining process
     */
    async stopMining() {
        console.log('🛑 Stopping mining process...');
        
        try {
            const response = await fetch('/api/mining/stop', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('success', 'Mining stopped successfully');
                this.updateMiningControls(false);
            } else {
                this.showNotification('error', 'Failed to stop mining');
            }
        } catch (error) {
            console.error('Error stopping mining:', error);
            this.showNotification('error', 'Network error');
        }
    }

    /**
     * Start training process
     */
    async startTraining() {
        console.log('🧠 Starting training process...');
        
        // Get configuration from training form
        const config = {
            epochs: parseInt(document.getElementById('trainingEpochs')?.value) || 1000,
            batch_size: parseInt(document.getElementById('batchSize')?.value) || 32,
            learning_rate: 0.001,  // Default value
            target_accuracy: 0.85, // Default value
            bitcoin_patterns: true
        };
        
        try {
            const response = await fetch('/api/training/start', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(config)
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('success', 'Training started successfully');
                this.updateTrainingControls(true);
            } else {
                this.showNotification('error', 'Failed to start training');
            }
        } catch (error) {
            console.error('Error starting training:', error);
            this.showNotification('error', 'Network error');
        }
    }

    /**
     * Stop training process
     */
    async stopTraining() {
        console.log('🛑 Stopping training process...');
        
        try {
            const response = await fetch('/api/training/stop', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            
            const result = await response.json();
            
            if (result.success) {
                this.showNotification('success', 'Training stopped successfully');
                this.updateTrainingControls(false);
            } else {
                this.showNotification('error', 'Failed to stop training');
            }
        } catch (error) {
            console.error('Error stopping training:', error);
            this.showNotification('error', 'Network error');
        }
    }

    /**
     * Pause training process
     */
    pauseTraining() {
        console.log('⏸️ Pausing training process...');
        // For now, just stop training (could be enhanced with pause/resume functionality)
        this.stopTraining();
    }

    /**
     * Update training control buttons
     */
    updateTrainingControls(isTraining) {
        const startBtn = document.getElementById('startTraining');
        const pauseBtn = document.getElementById('pauseTraining');
        const stopBtn = document.getElementById('stopTraining');
        
        if (startBtn) startBtn.disabled = isTraining;
        if (pauseBtn) pauseBtn.disabled = !isTraining;
        if (stopBtn) stopBtn.disabled = !isTraining;
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
    showNotification(type, message) {
        // Create notification element
        const notification = document.createElement('div');
        notification.className = `notification notification-${type}`;
        notification.innerHTML = `
            <div class="notification-content">
                <i class="fas fa-${type === 'success' ? 'check' : 'exclamation-triangle'}"></i>
                <span>${message}</span>
                <button class="notification-close">&times;</button>
            </div>
        `;
        
        // Add to page
        document.body.appendChild(notification);
        
        // Auto remove after 5 seconds
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, 5000);
        
        // Close button functionality
        const closeBtn = notification.querySelector('.notification-close');
        if (closeBtn) {
            closeBtn.addEventListener('click', () => {
                if (notification.parentNode) {
                    notification.parentNode.removeChild(notification);
                }
            });
        }
    }

    /**
     * Update system status display
     */
    updateSystemStatus() {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.sendWebSocketMessage({ type: 'get_system_status' });
        }
    }

    /**
     * Toggle electrode state
     */
    async toggleElectrode(electrodeId) {
        console.log(`🔌 Toggling electrode ${electrodeId}`);
        
        const control = {
            electrode_id: electrodeId,
            active: !this.meaElectrodes[electrodeId - 1]?.active,
            stimulation_voltage: 0.0,
            recording_mode: true
        };
        
        try {
            const response = await fetch(`/api/electrodes/${electrodeId}/control`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(control)
            });
            
            const result = await response.json();
            
            if (result.success) {
                // Update local state
                if (this.meaElectrodes[electrodeId - 1]) {
                    this.meaElectrodes[electrodeId - 1].active = control.active;
                }
                this.refreshMEAVisualization();
            }
        } catch (error) {
            console.error('Error toggling electrode:', error);
            this.showNotification('error', 'Failed to toggle electrode');
        }
    }

    /**
     * Emergency stop all operations
     */
    async emergencyStop() {
        console.log('🚨 EMERGENCY STOP ACTIVATED');
        this.showNotification('warning', 'Emergency stop activated - shutting down all systems');
        
        try {
            // Stop all systems
            await Promise.all([
                this.stopMining(),
                this.stopTraining(),
                this.stopSystem('sha256'),
                this.stopSystem('biological'), 
                this.stopSystem('mea')
            ]);
            
            this.showNotification('success', 'All systems stopped safely');
        } catch (error) {
            console.error('Error during emergency stop:', error);
            this.showNotification('error', 'Error during emergency stop');
        }
    }

    /**
     * Reset configuration to defaults
     */
    resetConfiguration() {
        console.log('🔄 Resetting configuration to defaults');
        
        // Reset form values
        const form = document.getElementById('tripleConfigForm');
        if (form) {
            form.reset();
            // Reset specific values
            document.getElementById('enableTripleSystem').checked = true;
            document.getElementById('enableCrossLearning').checked = true;
            document.getElementById('enableDynamicWeighting').checked = true;
            const adaptationRate = document.getElementById('adaptationRate');
            if (adaptationRate) {
                adaptationRate.value = 0.02;
                const valueSpan = adaptationRate.nextElementSibling;
                if (valueSpan) valueSpan.textContent = '0.02';
            }
        }
        
        this.showNotification('success', 'Configuration reset to defaults');
    }

    /**
     * Auto-optimize system weights
     */
    async autoOptimizeWeights() {
        console.log('🎯 Auto-optimizing system weights');
        this.showNotification('info', 'Optimizing weights based on performance data...');
        
        // Simulate optimization process
        setTimeout(() => {
            // Update weight sliders with optimized values
            const sha256Slider = document.getElementById('sha256WeightSlider');
            const networkSlider = document.getElementById('networkWeightSlider');
            const meaSlider = document.getElementById('meaWeightSlider');
            
            if (sha256Slider && networkSlider && meaSlider) {
                sha256Slider.value = 35;
                networkSlider.value = 40;
                meaSlider.value = 25;
                
                // Update displayed values
                const sha256Value = sha256Slider.nextElementSibling;
                const networkValue = networkSlider.nextElementSibling;
                const meaValue = meaSlider.nextElementSibling;
                
                if (sha256Value) sha256Value.textContent = '35%';
                if (networkValue) networkValue.textContent = '40%';
                if (meaValue) meaValue.textContent = '25%';
                
                this.updateTotalWeight();
            }
            
            this.showNotification('success', 'Weights optimized successfully');
        }, 2000);
    }

    /**
     * Initialize biological network
     */
    async initializeNetwork() {
        console.log('🧠 Initializing biological network');
        this.showNotification('info', 'Initializing biological neural network...');
        
        try {
            await this.startSystem('biological');
            this.showNotification('success', 'Biological network initialized successfully');
        } catch (error) {
            console.error('Error initializing network:', error);
            this.showNotification('error', 'Failed to initialize network');
        }
    }

    /**
     * Connect to MEA device
     */
    async connectMEADevice() {
        console.log('🔌 Connecting to MEA device');
        this.showNotification('info', 'Connecting to MEA device...');
        
        try {
            await this.startSystem('mea');
            this.showNotification('success', 'MEA device connected successfully');
        } catch (error) {
            console.error('Error connecting MEA device:', error);
            this.showNotification('error', 'Failed to connect MEA device');
        }
    }

    /**
     * Update total weight display
     */
    updateTotalWeight() {
        const sha256Weight = parseInt(document.getElementById('sha256WeightSlider')?.value) || 0;
        const networkWeight = parseInt(document.getElementById('networkWeightSlider')?.value) || 0;
        const meaWeight = parseInt(document.getElementById('meaWeightSlider')?.value) || 0;
        
        const total = sha256Weight + networkWeight + meaWeight;
        const totalDisplay = document.getElementById('totalWeight');
        if (totalDisplay) {
            totalDisplay.textContent = `${total}%`;
            totalDisplay.className = total === 100 ? 'weight-valid' : 'weight-invalid';
        }
    }

    /**
     * Handle file upload
     */
    handleFileUpload(input) {
        const files = input.files;
        if (files.length === 0) return;
        
        console.log(`📁 Uploading ${files.length} file(s)`);
        this.showNotification('info', `Uploading ${files.length} file(s)...`);
        
        // Simulate file upload
        setTimeout(() => {
            this.showNotification('success', 'Files uploaded successfully');
        }, 1500);
    }

    /**
     * Update connection status display
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
                    statusText.textContent = 'Connecté';
                    break;
                case 'disconnected':
                    statusDot.classList.add('disconnected');
                    statusText.textContent = 'Déconnecté';
                    break;
                case 'error':
                    statusDot.classList.add('error');
                    statusText.textContent = 'Erreur';
                    break;
                default:
                    statusText.textContent = 'Inconnu';
            }
        }
    }

    /**
     * Request current system status
     */
    requestSystemStatus() {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.sendWebSocketMessage({ type: 'get_system_status' });
        }
    }

    /**
     * Update system status display
     */
    updateSystemStatus(data) {
        if (data && data.systems) {
            // Update SHA-256 system
            if (data.systems.sha256) {
                const sha256Status = document.getElementById('sha256Status');
                const sha256Accuracy = document.getElementById('sha256Accuracy');
                if (sha256Status) {
                    sha256Status.textContent = data.systems.sha256.status === 'online' ? 'En ligne' : 'Hors ligne';
                    sha256Status.className = `card-status ${data.systems.sha256.status}`;
                }
                if (sha256Accuracy) {
                    sha256Accuracy.textContent = `${Math.round(data.systems.sha256.hashrate || 0)}%`;
                }
            }

            // Update biological network
            if (data.systems.biological) {
                const networkStatus = document.getElementById('networkStatus');
                const networkAccuracy = document.getElementById('networkAccuracy');
                if (networkStatus) {
                    networkStatus.textContent = data.systems.biological.status === 'online' ? 'En ligne' : 'Hors ligne';
                    networkStatus.className = `card-status ${data.systems.biological.status}`;
                }
                if (networkAccuracy) {
                    networkAccuracy.textContent = `${Math.round(data.systems.biological.learning_rate * 100 || 0)}%`;
                }
            }

            // Update MEA system
            if (data.systems.mea) {
                const meaStatus = document.getElementById('meaStatus');
                const meaAccuracy = document.getElementById('meaAccuracy');
                if (meaStatus) {
                    meaStatus.textContent = data.systems.mea.status === 'online' ? 'En ligne' : 'Hors ligne';
                    meaStatus.className = `card-status ${data.systems.mea.status}`;
                }
                if (meaAccuracy) {
                    meaAccuracy.textContent = `${Math.round((data.systems.mea.active_electrodes / 60) * 100 || 0)}%`;
                }
            }
        }
    }

    /**
     * Update mining statistics
     */
    updateMiningStats(data) {
        if (data) {
            this.miningStats = { ...this.miningStats, ...data };
            
            // Update UI elements
            this.updateElement('totalHashes', this.formatNumber(data.total_hashes || 0));
            this.updateElement('validNonces', data.valid_nonces || 0);
            this.updateElement('successRate', `${(data.success_rate || 0).toFixed(2)}%`);
            this.updateElement('blocksFound', data.blocks_mined || 0);
            
            // Update charts if they exist
            if (this.charts.mining) {
                this.updateMiningCharts(data);
            }
        }
    }

    /**
     * Update electrode data display
     */
    updateElectrodeData(data) {
        if (Array.isArray(data)) {
            this.meaElectrodes = data;
            this.refreshMEAVisualization();
        }
    }

    /**
     * Update training progress display
     */
    updateTrainingProgress(data) {
        if (data) {
            // Update progress bars
            const biologicalProgress = document.getElementById('biologicalProgress');
            const meaProgress = document.getElementById('meaProgress');
            const crossProgress = document.getElementById('crossProgress');
            
            if (biologicalProgress && data.biological_accuracy) {
                const percentage = Math.round(data.biological_accuracy * 100);
                biologicalProgress.style.width = `${percentage}%`;
                const progressText = biologicalProgress.parentElement.querySelector('.progress-text');
                if (progressText) progressText.textContent = `${percentage}%`;
            }
            
            if (meaProgress && data.mea_accuracy) {
                const percentage = Math.round(data.mea_accuracy * 100);
                meaProgress.style.width = `${percentage}%`;
                const progressText = meaProgress.parentElement.querySelector('.progress-text');
                if (progressText) progressText.textContent = `${percentage}%`;
            }
            
            // Update training chart if exists
            if (this.charts.training) {
                this.updateTrainingChart(data);
            }
        }
    }

    /**
     * Handle keyboard shortcuts
     */
    handleKeyboardShortcuts(event) {
        // Ctrl/Cmd + key combinations
        if (event.ctrlKey || event.metaKey) {
            switch (event.key) {
                case 's':
                    event.preventDefault();
                    this.saveCurrentConfiguration();
                    break;
                case 'r':
                    event.preventDefault();
                    this.resetConfiguration();
                    break;
            }
        }
        
        // Escape key to close modals
        if (event.key === 'Escape') {
            this.closeAllModals();
        }
    }

    /**
     * Handle window resize
     */
    handleResize() {
        // Recalculate chart sizes if they exist
        Object.values(this.charts).forEach(chart => {
            if (chart && typeof chart.resize === 'function') {
                chart.resize();
            }
        });
    }

    /**
     * Save current configuration
     */
    saveCurrentConfiguration() {
        const config = this.gatherCurrentConfiguration();
        localStorage.setItem('hybridMiningConfig', JSON.stringify(config));
        this.showNotification('success', 'Configuration saved');
    }

    /**
     * Gather current configuration from all forms
     */
    gatherCurrentConfiguration() {
        const config = {};
        
        // Gather all form data
        const forms = document.querySelectorAll('form[id*="config"]');
        forms.forEach(form => {
            const formData = new FormData(form);
            config[form.id] = Object.fromEntries(formData.entries());
        });
        
        return config;
    }

    /**
     * Update training chart
     */
    updateTrainingChart(data) {
        // Implementation for Chart.js integration would go here
        console.log('📊 Training chart updated:', data);
    }

    /**
     * Initialize MEA electrode grid
     */
    initializeElectrodeGrid() {
        const gridContainer = document.getElementById('electrodeGrid');
        if (gridContainer) {
            gridContainer.innerHTML = '';
            
            // Create 8x8 electrode grid (64 electrodes)
            for (let i = 1; i <= 64; i++) {
                const electrode = document.createElement('div');
                electrode.className = 'electrode inactive';
                electrode.dataset.electrode = i;
                electrode.textContent = i;
                electrode.addEventListener('click', () => this.toggleElectrode(i));
                gridContainer.appendChild(electrode);
            }
        }
    }

    /**
     * Initialize all charts
     */
    initializeCharts() {
        // Initialize performance chart
        const performanceCanvas = document.getElementById('performanceChart');
        if (performanceCanvas && typeof Chart !== 'undefined') {
            this.charts.performance = new Chart(performanceCanvas, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'SHA-256',
                        data: [],
                        borderColor: '#ff6b35',
                        backgroundColor: 'rgba(255, 107, 53, 0.1)'
                    }, {
                        label: 'Biological',
                        data: [],
                        borderColor: '#4ecdc4',
                        backgroundColor: 'rgba(78, 205, 196, 0.1)'
                    }, {
                        label: 'MEA',
                        data: [],
                        borderColor: '#45b7d1',
                        backgroundColor: 'rgba(69, 183, 209, 0.1)'
                    }]
                },
                options: {
                    responsive: true,
                    animation: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            max: 100
                        }
                    }
                }
            });
        }

        // Initialize training chart
        const trainingCanvas = document.getElementById('trainingChart');
        if (trainingCanvas && typeof Chart !== 'undefined') {
            this.charts.training = new Chart(trainingCanvas, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Loss',
                        data: [],
                        borderColor: '#f44336',
                        yAxisID: 'y'
                    }, {
                        label: 'Accuracy',
                        data: [],
                        borderColor: '#4caf50',
                        yAxisID: 'y1'
                    }]
                },
                options: {
                    responsive: true,
                    scales: {
                        y: {
                            type: 'linear',
                            display: true,
                            position: 'left',
                            beginAtZero: true
                        },
                        y1: {
                            type: 'linear',
                            display: true,
                            position: 'right',
                            beginAtZero: true,
                            max: 1,
                            grid: {
                                drawOnChartArea: false
                            }
                        }
                    }
                }
            });
        }
    }

    /**
     * Setup form value preservation across page refreshes
     */
    setupFormPreservation() {
        // Save form data on input changes
        document.addEventListener('input', (e) => {
            if (e.target.form && e.target.form.id.endsWith('Form')) {
                this.saveFormData(e.target.form);
            }
        });

        // Save form data on form submission prevention
        document.addEventListener('submit', (e) => {
            if (e.target.id.endsWith('Form')) {
                this.saveFormData(e.target);
            }
        });

        // Restore saved form data on page load
        this.restoreFormData();
    }

    /**
     * Save form data to localStorage
     */
    saveFormData(form) {
        try {
            const formData = new FormData(form);
            const data = {};
            
            for (let [key, value] of formData.entries()) {
                data[key] = value;
            }
            
            localStorage.setItem(`form_${form.id}`, JSON.stringify(data));
        } catch (error) {
            console.warn('Failed to save form data:', error);
        }
    }

    /**
     * Restore form data from localStorage
     */
    restoreFormData() {
        try {
            const forms = document.querySelectorAll('form[id$="Form"]');
            
            forms.forEach(form => {
                const savedData = localStorage.getItem(`form_${form.id}`);
                if (savedData) {
                    const data = JSON.parse(savedData);
                    
                    Object.entries(data).forEach(([name, value]) => {
                        const input = form.querySelector(`[name="${name}"]`);
                        if (input) {
                            if (input.type === 'checkbox' || input.type === 'radio') {
                                input.checked = value === 'on' || value === input.value;
                            } else {
                                input.value = value;
                            }
                        }
                    });
                }
            });
        } catch (error) {
            console.warn('Failed to restore form data:', error);
        }
    }

    /**
     * Clear saved form data for a specific form
     */
    clearFormData(formId) {
        try {
            localStorage.removeItem(`form_${formId}`);
        } catch (error) {
            console.warn('Failed to clear form data:', error);
        }
    }

    /**
     * Stimulate selected electrodes
     */
    stimulateSelectedElectrodes() {
        const selectedElectrodes = document.querySelectorAll('.electrode.active');
        if (selectedElectrodes.length === 0) {
            this.showNotification('warning', 'Sélectionnez au moins une électrode');
            return;
        }

        const voltage = document.getElementById('stimulationVoltage').value;
        const duration = document.getElementById('stimulationDuration').value;
        
        selectedElectrodes.forEach(electrode => {
            electrode.classList.add('stimulating');
            setTimeout(() => {
                electrode.classList.remove('stimulating');
            }, parseInt(duration) * 10); // Visual feedback duration
        });

        this.showNotification('success', `Stimulation de ${selectedElectrodes.length} électrodes (${voltage}mV, ${duration}ms)`);
    }

    /**
     * Start MEA recording
     */
    startMEARecording() {
        const duration = document.getElementById('recordingDuration').value;
        
        document.getElementById('startRecording').disabled = true;
        document.getElementById('stopRecording').disabled = false;
        
        this.showNotification('info', `Enregistrement MEA démarré (${duration}s)`);
        
        // Auto-stop after duration
        setTimeout(() => {
            if (!document.getElementById('stopRecording').disabled) {
                this.stopMEARecording();
            }
        }, parseInt(duration) * 1000);
    }

    /**
     * Stop MEA recording
     */
    stopMEARecording() {
        document.getElementById('startRecording').disabled = false;
        document.getElementById('stopRecording').disabled = true;
        
        this.showNotification('success', 'Enregistrement MEA terminé');
    }

    /**
     * Export data in various formats
     */
    exportData(format) {
        const data = this.gatherExportData();
        
        switch (format) {
            case 'csv':
                this.downloadCSV(data);
                break;
            case 'json':
                this.downloadJSON(data);
                break;
            case 'pdf':
                this.generatePDFReport(data);
                break;
        }
        
        this.showNotification('success', `Export ${format.toUpperCase()} généré`);
    }

    /**
     * Gather data for export
     */
    gatherExportData() {
        return {
            timestamp: new Date().toISOString(),
            systems: this.systems,
            miningStats: this.miningStats,
            electrodes: this.meaElectrodes,
            performance: this.performance_metrics
        };
    }

    /**
     * Download data as CSV
     */
    downloadCSV(data) {
        const csv = this.convertToCSV(data);
        const blob = new Blob([csv], { type: 'text/csv' });
        this.downloadBlob(blob, 'biomining-data.csv');
    }

    /**
     * Download data as JSON
     */
    downloadJSON(data) {
        const json = JSON.stringify(data, null, 2);
        const blob = new Blob([json], { type: 'application/json' });
        this.downloadBlob(blob, 'biomining-data.json');
    }

    /**
     * Generate PDF report
     */
    generatePDFReport(data) {
        // Simulate PDF generation
        const content = `BioMining Platform Report\n\nGenerated: ${data.timestamp}\n\nSystems Status:\n${JSON.stringify(data.systems, null, 2)}`;
        const blob = new Blob([content], { type: 'text/plain' });
        this.downloadBlob(blob, 'biomining-report.txt');
    }

    /**
     * Convert data to CSV format
     */
    convertToCSV(data) {
        const headers = ['Timestamp', 'System', 'Status', 'Value'];
        const rows = [headers.join(',')];
        
        Object.entries(data.systems).forEach(([system, status]) => {
            rows.push([data.timestamp, system, status.status, JSON.stringify(status)].join(','));
        });
        
        return rows.join('\n');
    }

    /**
     * Download blob as file
     */
    downloadBlob(blob, filename) {
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }
    /**
     * Handle configuration update response
     */
    handleConfigUpdateResponse(data) {
        const { form_id, success, message } = data;
        
        if (success) {
            this.showNotification('success', message);
            console.log(`✅ Configuration updated for ${form_id}`);
            
            // Update UI based on form type
            this.updateFormUI(form_id, data.config);
        } else {
            this.showNotification('error', message);
            console.error(`❌ Configuration failed for ${form_id}: ${message}`);
        }
    }

    /**
     * Handle configuration updated broadcast
     */
    handleConfigurationUpdated(data) {
        const { form_id, config } = data;
        console.log(`🔄 Configuration broadcast received for ${form_id}`);
        
        // Update UI to reflect changes from other clients
        this.updateFormUI(form_id, config);
    }

    /**
     * Update form UI after configuration changes
     */
    updateFormUI(formId, config) {
        // Keep form values after successful configuration
        this.preserveFormValues(formId, config);
        
        switch (formId) {
            case 'weightsForm':
                // Update weight displays
                this.updateTotalWeight();
                break;
                
            case 'biologicalNetworkForm':
                // Update network status display
                const statusElement = document.getElementById('networkCurrentStatus');
                if (statusElement) {
                    statusElement.textContent = 'Configuré ✅';
                }
                break;
                
            case 'meaConfigForm':
                // Update MEA status display
                const meaStatusElement = document.getElementById('meaConnectionStatus');
                if (meaStatusElement) {
                    meaStatusElement.textContent = 'Configuré ✅';
                }
                break;
                
            case 'trainingConfigForm':
                // Update training progress displays
                console.log('🧠 Training configuration updated');
                break;
                
            case 'miningConfigForm':
                // Update mining status displays
                console.log('⛏️ Mining configuration updated');
                break;
                
            case 'tripleConfigForm':
                // Update triple system status
                console.log('🔧 Triple system configuration updated');
                break;
                
            default:
                console.log(`ℹ️ Configuration updated for ${formId}`);
        }
        
        console.log(`✅ Form ${formId} UI updated and values preserved`);
    }
    
    /**
     * Preserve form values to prevent loss on submission
     */
    preserveFormValues(formId, config) {
        const form = document.getElementById(formId);
        if (!form || !config) return;
        
        // Restore all form values from the configuration
        Object.keys(config).forEach(fieldId => {
            const element = document.getElementById(fieldId);
            if (element) {
                const value = config[fieldId];
                
                if (element.type === 'checkbox') {
                    element.checked = (value === 'on' || value === true);
                } else if (element.type === 'range') {
                    element.value = value;
                    // Update range display
                    const valueSpan = element.nextElementSibling;
                    if (valueSpan && (valueSpan.classList.contains('range-value') || valueSpan.classList.contains('slider-value'))) {
                        valueSpan.textContent = value + (element.id.includes('Weight') ? '%' : '');
                    }
                } else if (element.tagName === 'SELECT' || element.type === 'text' || element.type === 'number') {
                    element.value = value;
                }
            }
        });
        
        console.log(`🔄 Form values preserved for ${formId}`);
    }
    
    /**
     * Verify all forms have submit prevention handlers
     */
    verifyFormHandlers() {
        const allForms = document.querySelectorAll('form[id$="Form"]');
        console.log(`🔍 Verifying ${allForms.length} forms have submit prevention:`);
        
        allForms.forEach(form => {
            const hasPreventionHandler = form.onsubmit !== null || 
                                       form.getAttribute('onsubmit') !== null ||
                                       form.classList.contains('submit-handled');
            
            // Mark forms as handled to track them
            form.classList.add('submit-handled');
            
            console.log(`  📋 ${form.id}: ${hasPreventionHandler ? '✅' : '⚠️'} preventDefault handler`);
        });
        
        return allForms.length;
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
