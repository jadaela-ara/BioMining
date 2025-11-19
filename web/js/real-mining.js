/**
 * Real Bitcoin Mining Manager
 * Manages real Bitcoin mining (Testnet/Mainnet) with pool connections
 */

class RealMiningManager {
    constructor() {
        this.miningActive = false;
        this.currentSession = null;
        this.stats = {
            hashrate: 0,
            jobsReceived: 0,
            sharesFound: 0,
            sharesAccepted: 0,
            sharesRejected: 0,
            blocksFound: 0,
            bioNoncesUsed: 0,
            poolConnected: false,
            difficulty: 0
        };
        
        this.initializeElements();
        this.attachEventListeners();
        this.loadSavedConfig();
    }
    
    initializeElements() {
        // Configuration
        this.networkSelect = document.getElementById('miningNetwork');
        this.poolSelect = document.getElementById('miningPool');
        this.customPoolConfig = document.getElementById('customPoolConfig');
        this.poolHostInput = document.getElementById('poolHost');
        this.poolPortInput = document.getElementById('poolPort');
        this.workerNameInput = document.getElementById('workerName');
        this.scanDepthInput = document.getElementById('scanDepth');
        this.neuralPredictionsInput = document.getElementById('neuralPredictions');
        this.bioWeightInput = document.getElementById('bioWeight');
        this.bioWeightValue = document.getElementById('bioWeightValue');
        this.useBioEntropyCheck = document.getElementById('useBioEntropy');
        
        // Buttons
        this.startBtn = document.getElementById('startRealMining');
        this.stopBtn = document.getElementById('stopRealMining');
        this.testBtn = document.getElementById('testConnection');
        
        // Status
        this.badge = document.getElementById('realMiningBadge');
        
        // Stats
        this.hashrateText = document.getElementById('realHashrate');
        this.poolStatusText = document.getElementById('poolStatus');
        this.poolDifficultyText = document.getElementById('poolDifficulty');
        this.jobsReceivedText = document.getElementById('jobsReceived');
        this.sharesFoundText = document.getElementById('sharesFound');
        this.sharesAcceptedText = document.getElementById('sharesAccepted');
        this.sharesRejectedText = document.getElementById('sharesRejected');
        this.acceptanceRateText = document.getElementById('acceptanceRate');
        this.realBlocksFoundText = document.getElementById('realBlocksFound');
        this.bioNoncesUsedText = document.getElementById('bioNoncesUsed');
        
        // Log
        this.logContainer = document.getElementById('realMiningLog');
    }
    
    attachEventListeners() {
        // Network/Pool selection
        this.poolSelect.addEventListener('change', () => this.handlePoolChange());
        this.networkSelect.addEventListener('change', () => this.handleNetworkChange());
        
        // Bio weight slider
        this.bioWeightInput.addEventListener('input', (e) => {
            this.bioWeightValue.textContent = e.target.value;
        });
        
        // Buttons
        this.startBtn.addEventListener('click', () => this.startMining());
        this.stopBtn.addEventListener('click', () => this.stopMining());
        this.testBtn.addEventListener('click', () => this.testConnection());
    }
    
    handlePoolChange() {
        if (this.poolSelect.value === 'custom') {
            this.customPoolConfig.style.display = 'block';
        } else {
            this.customPoolConfig.style.display = 'none';
        }
    }
    
    handleNetworkChange() {
        const network = this.networkSelect.value;
        
        // Update pool options based on network
        this.poolSelect.innerHTML = '';
        
        if (network === 'testnet') {
            this.poolSelect.innerHTML = `
                <option value="bitcoin.com">Bitcoin.com Testnet</option>
                <option value="solo">CKPool Solo Testnet</option>
                <option value="custom">Pool Personnalisé</option>
            `;
            this.addLog('ℹ️ Testnet sélectionné: Difficulté basse, trouvez des blocs!', 'info');
        } else {
            this.poolSelect.innerHTML = `
                <option value="slushpool">Slush Pool</option>
                <option value="f2pool">F2Pool</option>
                <option value="antpool">AntPool</option>
                <option value="custom">Pool Personnalisé</option>
            `;
            this.addLog('⚠️ Mainnet: Mining CPU n\'est PAS rentable!', 'warning');
        }
    }
    
    loadSavedConfig() {
        // Load from localStorage if available
        const saved = localStorage.getItem('realMiningConfig');
        if (saved) {
            try {
                const config = JSON.parse(saved);
                if (config.workerName) this.workerNameInput.value = config.workerName;
                if (config.scanDepth) this.scanDepthInput.value = config.scanDepth;
                if (config.neuralPredictions) this.neuralPredictionsInput.value = config.neuralPredictions;
                if (config.bioWeight) {
                    this.bioWeightInput.value = config.bioWeight;
                    this.bioWeightValue.textContent = config.bioWeight;
                }
            } catch (e) {
                console.error('Error loading saved config:', e);
            }
        }
    }
    
    saveConfig() {
        const config = {
            network: this.networkSelect.value,
            pool: this.poolSelect.value,
            workerName: this.workerNameInput.value,
            scanDepth: parseInt(this.scanDepthInput.value),
            neuralPredictions: parseInt(this.neuralPredictionsInput.value),
            bioWeight: parseFloat(this.bioWeightInput.value),
            useBioEntropy: this.useBioEntropyCheck.checked
        };
        
        localStorage.setItem('realMiningConfig', JSON.stringify(config));
        return config;
    }
    
    async startMining() {
        // Validation
        if (!this.workerNameInput.value) {
            this.addLog('❌ Erreur: Worker name requis', 'error');
            return;
        }
        
        const config = this.saveConfig();
        
        // Add custom pool if selected
        if (config.pool === 'custom') {
            config.customHost = this.poolHostInput.value;
            config.customPort = parseInt(this.poolPortInput.value);
            
            if (!config.customHost || !config.customPort) {
                this.addLog('❌ Erreur: Host et port requis pour pool personnalisé', 'error');
                return;
            }
        }
        
        try {
            this.addLog(`🚀 Démarrage mining ${config.network}...`);
            this.addLog(`   Pool: ${config.pool}`);
            this.addLog(`   Worker: ${config.workerName}`);
            if (config.useBioEntropy) {
                this.addLog(`   🧬 Bio-Entropy activé (weight: ${config.bioWeight})`);
            }
            
            const response = await fetch('/api/real-mining/start', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(config)
            });
            
            const data = await response.json();
            
            if (data.success) {
                this.miningActive = true;
                this.startBtn.disabled = true;
                this.stopBtn.disabled = false;
                this.updateBadge('active', 'Mining Actif');
                this.addLog('✅ Mining démarré avec succès!', 'success');
                
                // Start polling for updates
                this.startStatsPolling();
            } else {
                this.addLog(`❌ Échec démarrage: ${data.error || 'Erreur inconnue'}`, 'error');
            }
        } catch (error) {
            this.addLog(`❌ Erreur: ${error.message}`, 'error');
            console.error('Mining start error:', error);
        }
    }
    
    async stopMining() {
        try {
            this.addLog('🛑 Arrêt du mining...');
            
            const response = await fetch('/api/real-mining/stop', {method: 'POST'});
            const data = await response.json();
            
            if (data.success) {
                this.miningActive = false;
                this.startBtn.disabled = false;
                this.stopBtn.disabled = true;
                this.updateBadge('inactive', 'Inactif');
                this.addLog('✅ Mining arrêté', 'info');
                
                // Stop polling
                if (this.statsInterval) {
                    clearInterval(this.statsInterval);
                }
                
                // Show final stats
                this.addLog(`📊 Statistiques finales:`, 'info');
                this.addLog(`   Shares trouvés: ${this.stats.sharesFound}`, 'info');
                this.addLog(`   Shares acceptés: ${this.stats.sharesAccepted}`, 'success');
                this.addLog(`   Blocs trouvés: ${this.stats.blocksFound}`, this.stats.blocksFound > 0 ? 'success' : 'info');
            }
        } catch (error) {
            this.addLog(`❌ Erreur arrêt: ${error.message}`, 'error');
        }
    }
    
    async testConnection() {
        this.addLog('🔌 Test de connexion au pool...');
        
        const config = this.saveConfig();
        if (!config.workerName) {
            this.addLog('❌ Worker name requis pour le test', 'error');
            return;
        }
        
        try {
            const response = await fetch('/api/real-mining/test-connection', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(config)
            });
            
            const data = await response.json();
            
            if (data.success) {
                this.addLog('✅ Connexion réussie!', 'success');
                this.addLog(`   Latence: ${data.latency}ms`, 'info');
                this.addLog(`   Pool: ${data.pool_info}`, 'info');
            } else {
                this.addLog(`❌ Échec connexion: ${data.error}`, 'error');
            }
        } catch (error) {
            this.addLog(`❌ Erreur test: ${error.message}`, 'error');
        }
    }
    
    startStatsPolling() {
        // Poll stats every 2 seconds
        this.statsInterval = setInterval(() => this.updateStats(), 2000);
    }
    
    async updateStats() {
        if (!this.miningActive) return;
        
        try {
            const response = await fetch('/api/real-mining/stats');
            const data = await response.json();
            
            if (data.stats) {
                this.stats = data.stats;
                this.updateStatsDisplay();
            }
        } catch (error) {
            console.error('Stats update error:', error);
        }
    }
    
    updateStatsDisplay() {
        // Hashrate
        this.hashrateText.textContent = this.formatHashrate(this.stats.hashrate);
        
        // Pool status
        this.poolStatusText.textContent = this.stats.poolConnected ? 'Connecté' : 'Déconnecté';
        this.poolStatusText.className = this.stats.poolConnected ? 'stat-value-real success' : 'stat-value-real error';
        
        // Difficulty
        this.poolDifficultyText.textContent = this.stats.difficulty.toFixed(2);
        
        // Jobs
        this.jobsReceivedText.textContent = this.stats.jobsReceived;
        
        // Shares
        this.sharesFoundText.textContent = this.stats.sharesFound;
        this.sharesAcceptedText.textContent = this.stats.sharesAccepted;
        this.sharesRejectedText.textContent = this.stats.sharesRejected;
        
        // Acceptance rate
        const acceptanceRate = this.stats.sharesFound > 0 
            ? (this.stats.sharesAccepted / this.stats.sharesFound * 100).toFixed(1)
            : 0;
        this.acceptanceRateText.textContent = acceptanceRate + '%';
        
        // Blocks
        this.realBlocksFoundText.textContent = this.stats.blocksFound;
        if (this.stats.blocksFound > 0) {
            this.realBlocksFoundText.classList.add('gold', 'pulse');
        }
        
        // Bio nonces
        this.bioNoncesUsedText.textContent = this.stats.bioNoncesUsed;
    }
    
    formatHashrate(hashrate) {
        if (hashrate >= 1000000000) {
            return (hashrate / 1000000000).toFixed(2) + ' GH/s';
        } else if (hashrate >= 1000000) {
            return (hashrate / 1000000).toFixed(2) + ' MH/s';
        } else if (hashrate >= 1000) {
            return (hashrate / 1000).toFixed(2) + ' KH/s';
        } else {
            return hashrate.toFixed(2) + ' H/s';
        }
    }
    
    updateBadge(status, text) {
        const dot = this.badge.querySelector('.badge-dot');
        const textEl = this.badge.querySelector('.badge-text');
        
        dot.className = 'badge-dot ' + status;
        textEl.textContent = text;
    }
    
    addLog(message, type = 'info') {
        const entry = document.createElement('p');
        entry.className = `log-entry log-${type}`;
        
        const timestamp = new Date().toLocaleTimeString();
        entry.textContent = `[${timestamp}] ${message}`;
        
        this.logContainer.appendChild(entry);
        
        // Auto-scroll to bottom
        this.logContainer.scrollTop = this.logContainer.scrollHeight;
        
        // Keep only last 100 entries
        while (this.logContainer.children.length > 100) {
            this.logContainer.removeChild(this.logContainer.firstChild);
        }
    }
    
    handleMiningEvent(event) {
        switch (event.type) {
            case 'job_received':
                this.addLog(`📦 Nouveau job reçu (${event.data.job_id.substring(0, 8)}...)`, 'info');
                break;
            
            case 'share_found':
                this.addLog(`✨ Share trouvé! Nonce: ${event.data.nonce}`, 'success');
                break;
            
            case 'share_accepted':
                this.addLog(`✅ Share accepté par le pool!`, 'success');
                break;
            
            case 'share_rejected':
                this.addLog(`❌ Share rejeté: ${event.data.reason}`, 'error');
                break;
            
            case 'block_found':
                this.addLog(`🎉🎉🎉 BLOC TROUVÉ! 🎉🎉🎉`, 'success');
                this.addLog(`   Hash: ${event.data.hash}`, 'success');
                break;
            
            case 'difficulty_change':
                this.addLog(`🎯 Difficulté changée: ${event.data.difficulty}`, 'info');
                break;
            
            case 'connection_lost':
                this.addLog(`⚠️ Connexion perdue, reconnexion...`, 'warning');
                break;
            
            case 'connection_restored':
                this.addLog(`✅ Connexion rétablie`, 'success');
                break;
        }
    }
}

// Initialize on page load
let realMiningManager;
document.addEventListener('DOMContentLoaded', () => {
    realMiningManager = new RealMiningManager();
});

// Export for WebSocket handler
if (typeof window !== 'undefined') {
    window.realMiningManager = realMiningManager;
}
