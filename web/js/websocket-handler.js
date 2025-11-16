/**
 * ================================================================
 * WEBSOCKET HANDLER FOR REAL-TIME COMMUNICATION
 * Advanced WebSocket management for Triple System Platform
 * ================================================================
 */

class WebSocketHandler {
    constructor(app) {
        this.app = app;
        this.socket = null;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 10;
        this.reconnectDelay = 1000;
        this.heartbeatInterval = null;
        this.messageQueue = [];
        this.eventHandlers = new Map();
        this.connectionState = 'disconnected';
        
        this.init();
    }

    /**
     * Initialize WebSocket connection
     */
    init() {
        this.connect();
        this.setupHeartbeat();
    }

    /**
     * Establish WebSocket connection
     */
    connect() {
        try {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            const host = window.location.host;
            const wsUrl = `${protocol}//${host}/ws/hybrid-mining`;

            console.log(`🔌 Connecting to WebSocket: ${wsUrl}`);
            
            this.socket = new WebSocket(wsUrl);
            this.setupEventListeners();
            
        } catch (error) {
            console.error('❌ WebSocket connection failed:', error);
            this.handleConnectionError();
        }
    }

    /**
     * Setup WebSocket event listeners
     */
    setupEventListeners() {
        this.socket.onopen = (event) => {
            this.handleOpen(event);
        };

        this.socket.onmessage = (event) => {
            this.handleMessage(event);
        };

        this.socket.onclose = (event) => {
            this.handleClose(event);
        };

        this.socket.onerror = (event) => {
            this.handleError(event);
        };
    }

    /**
     * Handle WebSocket open event
     */
    handleOpen(event) {
        console.log('✅ WebSocket connected successfully');
        
        this.connectionState = 'connected';
        this.reconnectAttempts = 0;
        this.reconnectDelay = 1000;
        
        // Update UI
        this.updateConnectionStatus('connected');
        
        // Send authentication if required
        this.authenticate();
        
        // Process queued messages
        this.processMessageQueue();
        
        // Start heartbeat
        this.startHeartbeat();
        
        // Notify application
        this.app.onWebSocketConnected();
    }

    /**
     * Handle incoming WebSocket messages
     */
    handleMessage(event) {
        try {
            const message = JSON.parse(event.data);
            
            // Log message for debugging
            if (message.type !== 'heartbeat' && message.type !== 'pong') {
                console.log('📨 WebSocket message received:', message.type);
            }
            
            // Route message to appropriate handler
            this.routeMessage(message);
            
        } catch (error) {
            console.error('❌ Error parsing WebSocket message:', error);
        }
    }

    /**
     * Handle WebSocket close event
     */
    handleClose(event) {
        console.log(`🔌 WebSocket connection closed (Code: ${event.code})`);
        
        this.connectionState = 'disconnected';
        this.stopHeartbeat();
        
        // Update UI
        this.updateConnectionStatus('disconnected');
        
        // Attempt to reconnect unless intentionally closed
        if (event.code !== 1000 && this.reconnectAttempts < this.maxReconnectAttempts) {
            this.scheduleReconnect();
        }
        
        // Notify application
        this.app.onWebSocketDisconnected();
    }

    /**
     * Handle WebSocket error event
     */
    handleError(event) {
        console.error('❌ WebSocket error:', event);
        
        this.connectionState = 'error';
        this.updateConnectionStatus('error');
        
        // Notify application
        this.app.onWebSocketError(event);
    }

    /**
     * Route incoming messages to appropriate handlers
     */
    routeMessage(message) {
        switch (message.type) {
            // System status updates
            case 'system_status':
                this.handleSystemStatus(message.data);
                break;
                
            // Mining updates
            case 'mining_update':
                this.handleMiningUpdate(message.data);
                break;
                
            // MEA electrode data
            case 'electrode_data':
                this.handleElectrodeData(message.data);
                break;
                
            // Training progress
            case 'training_progress':
                this.handleTrainingProgress(message.data);
                break;
                
            // Training complete
            case 'training_complete':
                this.handleTrainingComplete(message.data);
                break;
                
            // Training error
            case 'training_error':
                this.handleTrainingError(message.data);
                break;
                
            // Biological network activity
            case 'biological_activity':
                this.handleBiologicalActivity(message.data);
                break;
                
            // Real-time performance metrics
            case 'performance_metrics':
                this.handlePerformanceMetrics(message.data);
                break;
                
            // Configuration updates
            case 'config_update':
                this.handleConfigUpdate(message.data);
                break;
                
            // Error messages
            case 'error':
                this.handleErrorMessage(message.data);
                break;
                
            // Success messages
            case 'success':
                this.handleSuccessMessage(message.data);
                break;
                
            // Heartbeat response
            case 'pong':
                this.handlePong();
                break;
                
            // Authentication response
            case 'auth_response':
                this.handleAuthResponse(message.data);
                break;
                
            // File upload progress
            case 'upload_progress':
                this.handleUploadProgress(message.data);
                break;
                
            // Custom events
            default:
                this.handleCustomMessage(message);
        }
    }

    /**
     * Handle system status updates
     */
    handleSystemStatus(data) {
        this.app.updateSystemStatus(data);
    }

    /**
     * Handle mining updates
     */
    handleMiningUpdate(data) {
        this.app.updateMiningStats(data);
        
        // Real-time mining notifications
        if (data.nonce_found) {
            this.app.showNotification('success', `Valid nonce found: ${data.nonce}`);
        }
        
        if (data.block_mined) {
            this.app.showNotification('success', `Block mined successfully! Hash: ${data.block_hash}`);
        }
    }

    /**
     * Handle electrode data updates
     */
    handleElectrodeData(data) {
        this.app.updateElectrodeData(data);
        
        // Check for abnormal electrode conditions
        data.forEach(electrode => {
            if (electrode.voltage > 100) { // High voltage threshold
                this.app.showNotification('warning', 
                    `High voltage detected on electrode ${electrode.electrode_id}: ${electrode.voltage}mV`);
            }
            
            if (electrode.impedance > 2000) { // High impedance threshold
                this.app.showNotification('warning', 
                    `High impedance on electrode ${electrode.electrode_id}: ${electrode.impedance}kΩ`);
            }
        });
    }

    /**
     * Handle training progress updates
     */
    handleTrainingProgress(data) {
        this.app.updateTrainingProgress(data);
        
        // Milestone notifications
        if (data.epoch % 100 === 0) {
            this.app.showNotification('info', 
                `Training milestone: Epoch ${data.epoch}/${data.total_epochs} completed`);
        }
        
        // Training completion
        if (data.epoch === data.total_epochs) {
            this.app.showNotification('success', 
                `Training completed! Final accuracy: ${(data.accuracy * 100).toFixed(2)}%`);
        }
    }

    /**
     * Handle biological activity updates
     */
    handleBiologicalActivity(data) {
        this.app.updateBiologicalActivity(data);
        
        // Detect significant biological events
        if (data.spike_detected) {
            this.app.showNotification('info', 'Biological spike pattern detected');
        }
        
        if (data.learning_event) {
            this.app.showNotification('success', 
                `Hebbian learning event: ${data.synapses_strengthened} synapses strengthened`);
        }
    }

    /**
     * Handle performance metrics
     */
    handlePerformanceMetrics(data) {
        // Update performance displays
        this.app.updateElement('cpu-usage', `${data.cpu_usage.toFixed(1)}%`);
        this.app.updateElement('memory-usage', `${data.memory_usage.toFixed(1)}%`);
        this.app.updateElement('gpu-usage', `${data.gpu_usage.toFixed(1)}%`);
        this.app.updateElement('network-io', `${this.formatBytes(data.network_io)}/s`);
        
        // Performance warnings
        if (data.cpu_usage > 90) {
            this.app.showNotification('warning', 'High CPU usage detected');
        }
        
        if (data.memory_usage > 85) {
            this.app.showNotification('warning', 'High memory usage detected');
        }
    }

    /**
     * Handle configuration updates
     */
    handleConfigUpdate(data) {
        this.app.applyConfiguration(data.config);
        this.app.showNotification('info', 'Configuration updated remotely');
    }

    /**
     * Handle error messages
     */
    handleErrorMessage(data) {
        this.app.showNotification('error', data.message || 'An error occurred');
        console.error('🔴 Server error:', data);
    }

    /**
     * Handle success messages
     */
    handleSuccessMessage(data) {
        this.app.showNotification('success', data.message || 'Operation successful');
        console.log('✅ Server success:', data);
    }

    /**
     * Handle heartbeat response
     */
    handlePong() {
        // Connection is alive
        this.lastPongTime = Date.now();
    }

    /**
     * Handle authentication response
     */
    handleAuthResponse(data) {
        if (data.success) {
            console.log('🔐 Authentication successful');
            this.app.showNotification('success', 'Connected and authenticated');
        } else {
            console.error('🔐 Authentication failed');
            this.app.showNotification('error', 'Authentication failed');
        }
    }

    /**
     * Handle file upload progress
     */
    handleUploadProgress(data) {
        const { filename, progress, status } = data;
        
        // Update progress bar if exists
        const progressBar = document.querySelector(`[data-upload="${filename}"] .progress-fill`);
        if (progressBar) {
            progressBar.style.width = `${progress}%`;
        }
        
        // Show completion notification
        if (status === 'completed') {
            this.app.showNotification('success', `File upload completed: ${filename}`);
        } else if (status === 'failed') {
            this.app.showNotification('error', `File upload failed: ${filename}`);
        }
    }

    /**
     * Handle custom messages
     */
    handleCustomMessage(message) {
        // Check for registered event handlers
        if (this.eventHandlers.has(message.type)) {
            const handler = this.eventHandlers.get(message.type);
            handler(message.data);
        } else {
            console.log('📨 Unhandled message type:', message.type);
        }
    }

    /**
     * Send message through WebSocket
     */
    send(message) {
        if (this.isConnected()) {
            try {
                this.socket.send(JSON.stringify(message));
                console.log('📤 Message sent:', message.type);
            } catch (error) {
                console.error('❌ Failed to send message:', error);
                this.queueMessage(message);
            }
        } else {
            console.log('📋 Queuing message (not connected):', message.type);
            this.queueMessage(message);
        }
    }

    /**
     * Queue message for later sending
     */
    queueMessage(message) {
        this.messageQueue.push({
            message,
            timestamp: Date.now()
        });
        
        // Limit queue size
        if (this.messageQueue.length > 100) {
            this.messageQueue.shift();
        }
    }

    /**
     * Process queued messages
     */
    processMessageQueue() {
        const currentTime = Date.now();
        
        while (this.messageQueue.length > 0) {
            const queuedItem = this.messageQueue.shift();
            
            // Skip messages older than 30 seconds
            if (currentTime - queuedItem.timestamp > 30000) {
                continue;
            }
            
            this.send(queuedItem.message);
        }
    }

    /**
     * Check if WebSocket is connected
     */
    isConnected() {
        return this.socket && this.socket.readyState === WebSocket.OPEN;
    }

    /**
     * Schedule reconnection attempt
     */
    scheduleReconnect() {
        this.reconnectAttempts++;
        const delay = Math.min(this.reconnectDelay * this.reconnectAttempts, 30000);
        
        console.log(`🔄 Scheduling reconnect attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts} in ${delay}ms`);
        
        setTimeout(() => {
            if (this.connectionState !== 'connected') {
                this.connect();
            }
        }, delay);
    }

    /**
     * Handle connection error
     */
    handleConnectionError() {
        this.connectionState = 'error';
        this.updateConnectionStatus('error');
        
        if (this.reconnectAttempts < this.maxReconnectAttempts) {
            this.scheduleReconnect();
        } else {
            console.error('❌ Max reconnection attempts reached');
            this.app.showNotification('error', 'Connection failed. Please refresh the page.');
        }
    }

    /**
     * Setup heartbeat mechanism
     */
    setupHeartbeat() {
        this.heartbeatInterval = setInterval(() => {
            if (this.isConnected()) {
                this.send({ type: 'ping', timestamp: Date.now() });
            }
        }, 30000); // Send ping every 30 seconds
    }

    /**
     * Start heartbeat
     */
    startHeartbeat() {
        this.lastPongTime = Date.now();
        
        // Check for stale connections
        this.heartbeatCheckInterval = setInterval(() => {
            if (this.isConnected() && Date.now() - this.lastPongTime > 60000) {
                console.log('💔 Heartbeat timeout - connection may be stale');
                this.socket.close();
            }
        }, 10000);
    }

    /**
     * Stop heartbeat
     */
    stopHeartbeat() {
        if (this.heartbeatCheckInterval) {
            clearInterval(this.heartbeatCheckInterval);
            this.heartbeatCheckInterval = null;
        }
    }

    /**
     * Authenticate with server
     */
    authenticate() {
        const token = localStorage.getItem('authToken') || 'default-token';
        this.send({
            type: 'authenticate',
            token: token,
            timestamp: Date.now()
        });
    }

    /**
     * Update connection status in UI
     */
    updateConnectionStatus(status) {
        const indicator = document.getElementById('connection-status');
        if (indicator) {
            indicator.className = `status-indicator ${status}`;
            
            const statusText = {
                connected: 'ONLINE',
                disconnected: 'OFFLINE',
                error: 'ERROR',
                connecting: 'CONNECTING'
            };
            
            indicator.textContent = statusText[status] || status.toUpperCase();
        }
    }

    /**
     * Register custom event handler
     */
    on(eventType, handler) {
        this.eventHandlers.set(eventType, handler);
    }

    /**
     * Unregister event handler
     */
    off(eventType) {
        this.eventHandlers.delete(eventType);
    }

    /**
     * Format bytes for display
     */
    formatBytes(bytes) {
        if (bytes === 0) return '0 B';
        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }

    /**
     * Close WebSocket connection
     */
    close() {
        if (this.heartbeatInterval) {
            clearInterval(this.heartbeatInterval);
        }
        
        if (this.heartbeatCheckInterval) {
            clearInterval(this.heartbeatCheckInterval);
        }
        
        if (this.socket) {
            this.socket.close(1000, 'Client disconnecting');
        }
        
        console.log('🔌 WebSocket connection closed by client');
    }

    /**
     * Get connection statistics
     */
    getStats() {
        return {
            connectionState: this.connectionState,
            reconnectAttempts: this.reconnectAttempts,
            queuedMessages: this.messageQueue.length,
            lastPongTime: this.lastPongTime,
            isConnected: this.isConnected()
        };
    }
    
    /**
     * Handle training complete event
     */
    handleTrainingComplete(data) {
        console.log('🎉 Training complete:', data);
        if (window.trainingManager) {
            window.trainingManager.handleTrainingComplete(data);
        }
    }
    
    /**
     * Handle training error event
     */
    handleTrainingError(data) {
        console.error('❌ Training error:', data);
        if (window.trainingManager) {
            window.trainingManager.handleTrainingError(data);
        }
    }
}

// Export for module usage
if (typeof module !== 'undefined' && module.exports) {
    module.exports = WebSocketHandler;
}