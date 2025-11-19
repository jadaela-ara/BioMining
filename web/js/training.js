/**
 * Training Page Handler
 * Manages Historical Bitcoin Training interface
 */

class TrainingManager {
    constructor() {
        this.trainingActive = false;
        this.currentSession = null;
        this.initializeElements();
        this.attachEventListeners();
        this.loadTrainingSessions();
    }
    
    initializeElements() {
        // Configuration inputs
        this.startHeightInput = document.getElementById('trainingStartHeight');
        this.blockCountInput = document.getElementById('trainingBlockCount');
        this.validateEveryInput = document.getElementById('trainingValidateEvery');
        this.validationCountInput = document.getElementById('trainingValidationCount');
        
        // Buttons
        this.startBtn = document.getElementById('startTraining');
        this.stopBtn = document.getElementById('stopTraining');
        
        // Status elements
        this.statusText = document.getElementById('trainingStatus');
        this.blocksTrainedText = document.getElementById('blocksTrained');
        this.currentLossText = document.getElementById('currentLoss');
        this.improvementText = document.getElementById('improvementPercent');
        
        // Progress
        this.progressBar = document.getElementById('trainingProgressBar');
        this.progressText = document.getElementById('trainingProgressText');
        this.trainingLog = document.getElementById('trainingLog');
        
        // Results
        this.beforeDistanceText = document.getElementById('beforeDistance');
        this.beforeSuccessRateText = document.getElementById('beforeSuccessRate');
        this.afterDistanceText = document.getElementById('afterDistance');
        this.afterSuccessRateText = document.getElementById('afterSuccessRate');
        this.overallImprovementText = document.getElementById('overallImprovement');
        
        // History table
        this.historyTable = document.getElementById('trainingHistoryTable').querySelector('tbody');
    }
    
    attachEventListeners() {
        this.startBtn.addEventListener('click', () => this.startTraining());
        this.stopBtn.addEventListener('click', () => this.stopTraining());
    }
    
    async startTraining() {
        const config = {
            start_height: parseInt(this.startHeightInput.value),
            count: parseInt(this.blockCountInput.value),
            validate_every: parseInt(this.validateEveryInput.value),
            validation_count: parseInt(this.validationCountInput.value)
        };
        
        // Validation
        if (config.count > 1000) {
            this.addLog('❌ Error: Maximum 1000 blocks per session', 'error');
            return;
        }
        
        if (config.start_height < 1) {
            this.addLog('❌ Error: Invalid start height', 'error');
            return;
        }
        
        try {
            this.addLog(`🎓 Starting training on ${config.count} blocks from height ${config.start_height}...`);
            
            const response = await fetch('/api/training/historical/start', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(config)
            });
            
            const data = await response.json();
            
            if (data.success) {
                this.trainingActive = true;
                this.startBtn.disabled = true;
                this.stopBtn.disabled = false;
                this.statusText.textContent = 'Training...';
                this.statusText.className = 'status-value training';
                this.addLog('✅ Training started successfully');
                
                // Reset progress
                this.updateProgress(0, config.count);
            } else {
                this.addLog('❌ Failed to start training: ' + (data.error || 'Unknown error'), 'error');
            }
        } catch (error) {
            this.addLog('❌ Error starting training: ' + error.message, 'error');
            console.error('Training start error:', error);
        }
    }
    
    async stopTraining() {
        try {
            const response = await fetch('/api/training/historical/stop', {method: 'POST'});
            const data = await response.json();
            
            if (data.success) {
                this.trainingActive = false;
                this.startBtn.disabled = false;
                this.stopBtn.disabled = true;
                this.statusText.textContent = 'Stopped';
                this.statusText.className = 'status-value stopped';
                this.addLog('🛑 Training stopped');
            }
        } catch (error) {
            this.addLog('❌ Error stopping training: ' + error.message, 'error');
        }
    }
    
    updateProgress(current, total) {
        const percent = (current / total) * 100;
        this.progressBar.style.width = percent + '%';
        this.progressText.textContent = `${current} / ${total} blocks`;
        this.blocksTrainedText.textContent = current;
    }
    
    addLog(message, type = 'info') {
        const entry = document.createElement('p');
        entry.className = `log-entry log-${type}`;
        entry.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;
        this.trainingLog.appendChild(entry);
        this.trainingLog.scrollTop = this.trainingLog.scrollHeight;
        
        // Keep only last 100 entries
        while (this.trainingLog.children.length > 100) {
            this.trainingLog.removeChild(this.trainingLog.firstChild);
        }
    }
    
    handleTrainingUpdate(data) {
        // Update from WebSocket
        if (data.block_height) {
            this.addLog(`📦 Block ${data.block_height}: loss=${data.neural_loss?.toFixed(6) || 'N/A'}`);
        }
        
        if (data.validation_result) {
            const result = data.validation_result;
            this.addLog(`📊 Validation: distance=${result.neural_distance_percent?.toFixed(2)}% ${result.success ? '✅' : '❌'}`);
        }
        
        if (data.progress) {
            this.updateProgress(data.progress.current, data.progress.total);
        }
        
        if (data.current_loss !== undefined) {
            this.currentLossText.textContent = data.current_loss.toFixed(6);
        }
    }
    
    handleTrainingComplete(data) {
        this.trainingActive = false;
        this.startBtn.disabled = false;
        this.stopBtn.disabled = true;
        this.statusText.textContent = 'Complete';
        this.statusText.className = 'status-value complete';
        
        this.addLog('🎉 Training complete!', 'success');
        this.addLog(`   Improvement: ${data.improvement_percent?.toFixed(1)}%`);
        this.addLog(`   Success rate: ${(data.success_rate_after * 100).toFixed(1)}%`);
        
        // Update results display
        if (data.session_id) {
            this.loadSessionResults(data.session_id);
        }
        
        // Reload sessions list
        this.loadTrainingSessions();
    }
    
    handleTrainingError(data) {
        this.trainingActive = false;
        this.startBtn.disabled = false;
        this.stopBtn.disabled = true;
        this.statusText.textContent = 'Error';
        this.statusText.className = 'status-value error';
        
        this.addLog('❌ Training error: ' + data.error, 'error');
    }
    
    async loadTrainingSessions() {
        try {
            const response = await fetch('/api/training/historical/sessions');
            const data = await response.json();
            
            if (data.sessions && data.sessions.length > 0) {
                this.historyTable.innerHTML = '';
                data.sessions.forEach(session => {
                    this.addSessionToTable(session);
                });
            } else {
                this.historyTable.innerHTML = '<tr><td colspan="7" class="no-data">No training sessions yet</td></tr>';
            }
        } catch (error) {
            console.error('Error loading sessions:', error);
        }
    }
    
    addSessionToTable(session) {
        const row = document.createElement('tr');
        
        // Extract info from filename (training_session_START_COUNT_TIMESTAMP.json)
        const match = session.filename.match(/training_session_(\d+)_(\d+)_?(\d*)/);
        const startBlock = match ? match[1] : 'N/A';
        const blockCount = match ? match[2] : 'N/A';
        
        const date = new Date(session.modified * 1000);
        
        row.innerHTML = `
            <td>${session.filename}</td>
            <td>${startBlock}</td>
            <td>${blockCount}</td>
            <td class="pending">--</td>
            <td class="pending">--</td>
            <td>${date.toLocaleString()}</td>
            <td>
                <button class="btn-sm btn-view" onclick="trainingManager.viewSession('${session.filename}')">
                    <i class="fas fa-eye"></i> View
                </button>
            </td>
        `;
        
        this.historyTable.appendChild(row);
    }
    
    async viewSession(filename) {
        try {
            const response = await fetch(`/api/training/session/${filename}`);
            const session = await response.json();
            
            // Display session results
            this.displaySessionResults(session);
        } catch (error) {
            this.addLog('❌ Error loading session: ' + error.message, 'error');
        }
    }
    
    displaySessionResults(session) {
        // Update before/after display
        this.beforeDistanceText.textContent = session.avg_neural_distance_before?.toFixed(2) + '%';
        this.beforeSuccessRateText.textContent = (session.success_rate_before * 100).toFixed(1) + '%';
        
        this.afterDistanceText.textContent = session.avg_neural_distance_after?.toFixed(2) + '%';
        this.afterSuccessRateText.textContent = (session.success_rate_after * 100).toFixed(1) + '%';
        
        const improvement = session.improvement_percent;
        this.overallImprovementText.textContent = (improvement >= 0 ? '+' : '') + improvement.toFixed(1) + '%';
        this.overallImprovementText.className = 'improvement-value highlight ' + (improvement >= 0 ? 'positive' : 'negative');
        
        this.improvementText.textContent = (improvement >= 0 ? '+' : '') + improvement.toFixed(1) + '%';
        
        this.addLog(`📊 Loaded session: ${session.session_id}`, 'success');
        this.addLog(`   Blocks trained: ${session.blocks_trained}`);
        this.addLog(`   Improvement: ${improvement.toFixed(1)}%`);
    }
    
    async loadSessionResults(sessionId) {
        // Try to find and load the session
        const sessions = await this.loadTrainingSessions();
        // Implementation depends on session storage format
    }
}

// Initialize when page loads
let trainingManager;

document.addEventListener('DOMContentLoaded', () => {
    // Initialize training manager when training page is visible
    const trainingPage = document.getElementById('training-page');
    if (trainingPage) {
        trainingManager = new TrainingManager();
    }
});

// Export for WebSocket integration
if (typeof window !== 'undefined') {
    window.TrainingManager = TrainingManager;
}
