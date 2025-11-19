#!/usr/bin/env python3
"""
BioMining Platform - Version Simplifiée pour Déploiement Cloud Shell
===================================================================

Version allégée sans dépendances Qt complexes
Interface web pure pour démonstration et tests

Author: BioMining Team  
Date: 2025-10-02
Version: 1.0.0-cloudshell
"""

from flask import Flask, render_template_string, jsonify, request, send_from_directory
import json
import time
import random
import hashlib
import struct
import threading
from datetime import datetime
import os

app = Flask(__name__)

# ====================================================================
# 1. SIMULATEUR MEA SIMPLIFIÉ (Sans Qt6)
# ====================================================================

class SimpleMEAInterface:
    """Interface MEA simplifiée sans dépendances Qt6"""
    
    def __init__(self):
        self.connected = True
        self.electrodes_count = 60
        self.sampling_rate = 25000
        self.is_acquiring = False
        self.data_buffer = []
        
    def get_status(self):
        return {
            "device": "MultiChannel Systems MEA2100 (Simulé)",
            "status": "connected" if self.connected else "disconnected", 
            "electrodes": self.electrodes_count,
            "sampling_rate": self.sampling_rate,
            "acquisition_active": self.is_acquiring,
            "timestamp": datetime.now().isoformat()
        }
    
    def start_acquisition(self):
        self.is_acquiring = True
        return {"success": True, "message": "Acquisition démarrée"}
    
    def stop_acquisition(self):
        self.is_acquiring = False
        return {"success": True, "message": "Acquisition arrêtée"}
    
    def get_electrode_data(self, electrode_id=None):
        """Génère des données simulées d'électrodes"""
        if electrode_id is None:
            # Toutes les électrodes
            data = {}
            for i in range(self.electrodes_count):
                data[f"electrode_{i}"] = {
                    "voltage": round(random.uniform(-100, 100), 2),
                    "impedance": round(random.uniform(0.5, 3.0), 2),
                    "noise_level": round(random.uniform(5, 15), 1)
                }
            return data
        else:
            # Électrode spécifique
            return {
                "electrode_id": electrode_id,
                "voltage": round(random.uniform(-100, 100), 2), 
                "impedance": round(random.uniform(0.5, 3.0), 2),
                "noise_level": round(random.uniform(5, 15), 1),
                "spikes_detected": random.randint(0, 5)
            }

# ====================================================================  
# 2. BITCOIN MINING SIMPLIFIÉ
# ====================================================================

class SimpleBitcoinMiner:
    """Mineur Bitcoin simplifié pour démonstration"""
    
    def __init__(self):
        self.mining_active = False
        self.hashrate = 0
        self.blocks_found = 0
        self.total_hashes = 0
        
        # Bloc Bitcoin Genesis pour tests
        self.genesis_block = {
            "version": 1,
            "previous_hash": "0000000000000000000000000000000000000000000000000000000000000000",
            "merkle_root": "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b", 
            "timestamp": 1231006505,
            "bits": 0x1d00ffff,
            "target_nonce": 2083236893,
            "target_hash": "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
        }
    
    def calculate_bitcoin_hash(self, block_data, nonce):
        """Calcul hash Bitcoin (double SHA-256)"""
        # Construction header du bloc
        header = struct.pack('<I', block_data['version'])
        header += bytes.fromhex(block_data['previous_hash'])[::-1]
        header += bytes.fromhex(block_data['merkle_root'])[::-1] 
        header += struct.pack('<I', block_data['timestamp'])
        header += struct.pack('<I', block_data['bits'])
        header += struct.pack('<I', nonce)
        
        # Double SHA-256
        hash1 = hashlib.sha256(header).digest()
        hash2 = hashlib.sha256(hash1).digest()
        return hash2[::-1].hex()
    
    def start_mining(self):
        """Démarre le mining simulé"""
        self.mining_active = True
        self.hashrate = random.randint(1000, 5000)  # H/s simulé
        return {"success": True, "message": "Mining démarré", "hashrate": self.hashrate}
    
    def stop_mining(self):
        """Arrête le mining"""
        self.mining_active = False
        self.hashrate = 0
        return {"success": True, "message": "Mining arrêté"}
    
    def get_mining_status(self):
        """Status du mining"""
        if self.mining_active:
            self.total_hashes += random.randint(100, 500)
            
            # Simulation découverte de bloc (très rare)
            if random.random() < 0.001:  # 0.1% chance
                self.blocks_found += 1
        
        return {
            "active": self.mining_active,
            "hashrate": self.hashrate,
            "total_hashes": self.total_hashes, 
            "blocks_found": self.blocks_found,
            "current_difficulty": 84381461788.35, 
            "estimated_time_to_block": "~142 years" if self.mining_active else "N/A"
        }
    
    def test_genesis_validation(self):
        """Test validation bloc Genesis"""
        correct_hash = self.calculate_bitcoin_hash(self.genesis_block, self.genesis_block['target_nonce'])
        is_valid = correct_hash == self.genesis_block['target_hash']
        
        return {
            "block": "Genesis Block",
            "nonce": self.genesis_block['target_nonce'],
            "calculated_hash": correct_hash,
            "expected_hash": self.genesis_block['target_hash'],
            "valid": is_valid,
            "test_time": datetime.now().isoformat()
        }

# ====================================================================
# 3. INSTANCES GLOBALES  
# ====================================================================

mea_interface = SimpleMEAInterface()
bitcoin_miner = SimpleBitcoinMiner()

# ====================================================================
# 4. INTERFACE WEB HTML
# ====================================================================

HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🧬 BioMining Platform - Cloud Shell Edition</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            color: white;
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
        }
        
        .header {
            text-align: center;
            margin-bottom: 30px;
            padding: 20px;
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            backdrop-filter: blur(10px);
        }
        
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .card {
            background: rgba(255,255,255,0.15);
            padding: 25px;
            border-radius: 15px;
            border: 1px solid rgba(255,255,255,0.2);
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
        }
        
        .card h3 {
            font-size: 1.4em;
            margin-bottom: 15px;
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            padding-bottom: 10px;
        }
        
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        
        .status-connected { background: #4CAF50; }
        .status-disconnected { background: #f44336; }
        .status-mining { background: #FF9800; animation: pulse 1.5s infinite; }
        
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
        
        .metric {
            display: flex;
            justify-content: space-between;
            margin: 10px 0;
            padding: 8px 0;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        .metric:last-child { border-bottom: none; }
        
        .value {
            font-weight: bold;
            color: #FFD700;
        }
        
        .button {
            background: rgba(255,255,255,0.2);
            color: white;
            border: 1px solid rgba(255,255,255,0.3);
            padding: 12px 20px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            margin: 5px;
            transition: all 0.3s ease;
        }
        
        .button:hover {
            background: rgba(255,255,255,0.3);
            transform: translateY(-2px);
        }
        
        .button.primary {
            background: #4CAF50;
            border-color: #45a049;
        }
        
        .button.danger {
            background: #f44336;
            border-color: #da190b;
        }
        
        .electrode-grid {
            display: grid;
            grid-template-columns: repeat(8, 1fr);
            gap: 3px;
            margin-top: 15px;
        }
        
        .electrode {
            width: 30px;
            height: 30px;
            border: 1px solid rgba(255,255,255,0.3);
            border-radius: 4px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 10px;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        
        .electrode:hover {
            background: rgba(255,255,255,0.2);
            transform: scale(1.1);
        }
        
        .electrode.active {
            background: #4CAF50;
            box-shadow: 0 0 10px #4CAF50;
        }
        
        .log-area {
            background: rgba(0,0,0,0.3);
            padding: 15px;
            border-radius: 8px;
            max-height: 200px;
            overflow-y: auto;
            font-family: monospace;
            font-size: 12px;
            margin-top: 15px;
        }
        
        .footer {
            text-align: center;
            margin-top: 30px;
            padding: 20px;
            border-top: 1px solid rgba(255,255,255,0.2);
        }
        
        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
            }
            
            .header h1 {
                font-size: 2em;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🧬 BioMining Platform</h1>
            <p>Interface Simplifiée - Édition Cloud Shell</p>
            <p><strong>Version 1.0.0</strong> | Déploiement Sans Qt6 | Bio-Crypto Mining</p>
        </div>
        
        <div class="grid">
            <!-- MEA Status -->
            <div class="card">
                <h3>🔬 Interface MEA</h3>
                <div class="metric">
                    <span>Status:</span>
                    <span><span class="status-indicator status-connected" id="mea-status-led"></span><span id="mea-status">Connecté</span></span>
                </div>
                <div class="metric">
                    <span>Électrodes:</span>
                    <span class="value" id="mea-electrodes">60</span>
                </div>
                <div class="metric">
                    <span>Fréquence:</span>
                    <span class="value" id="mea-frequency">25 kHz</span>
                </div>
                <div class="metric">
                    <span>Acquisition:</span>
                    <span class="value" id="mea-acquisition">Arrêtée</span>
                </div>
                
                <button class="button primary" onclick="startAcquisition()">▶️ Démarrer Acquisition</button>
                <button class="button danger" onclick="stopAcquisition()">⏹️ Arrêter</button>
                
                <div class="electrode-grid" id="electrode-grid">
                    <!-- Électrodes générées par JS -->
                </div>
            </div>
            
            <!-- Bitcoin Mining -->
            <div class="card">
                <h3>⛏️ Mining Bitcoin</h3>
                <div class="metric">
                    <span>Status:</span>
                    <span><span class="status-indicator" id="mining-status-led"></span><span id="mining-status">Arrêté</span></span>
                </div>
                <div class="metric">
                    <span>Hashrate:</span>
                    <span class="value" id="hashrate">0 H/s</span>
                </div>
                <div class="metric">
                    <span>Hashes Total:</span>
                    <span class="value" id="total-hashes">0</span>
                </div>
                <div class="metric">
                    <span>Blocs Trouvés:</span>
                    <span class="value" id="blocks-found">0</span>
                </div>
                <div class="metric">
                    <span>Temps Estimé:</span>
                    <span class="value" id="time-estimate">N/A</span>
                </div>
                
                <button class="button primary" onclick="startMining()">⛏️ Démarrer Mining</button>
                <button class="button danger" onclick="stopMining()">⏹️ Arrêter</button>
                <button class="button" onclick="testGenesis()">🧪 Test Genesis</button>
            </div>
            
            <!-- Monitoring -->
            <div class="card">
                <h3>📊 Monitoring Temps Réel</h3>
                <div class="metric">
                    <span>Dernière MAJ:</span>
                    <span class="value" id="last-update">-</span>
                </div>
                <div class="metric">
                    <span>Signaux Actifs:</span>
                    <span class="value" id="active-signals">0</span>
                </div>
                <div class="metric">
                    <span>Spikes Détectés:</span>
                    <span class="value" id="spikes-detected">0</span>
                </div>
                <div class="metric">
                    <span>Qualité Signal:</span>
                    <span class="value" id="signal-quality">95.2%</span>
                </div>
                
                <div class="log-area" id="log-area">
                    <div>🚀 Interface BioMining initialisée</div>
                    <div>✅ Connexion MEA établie (Simulé)</div>
                    <div>📊 Monitoring temps réel actif</div>
                </div>
            </div>
            
            <!-- Configuration -->
            <div class="card">
                <h3>⚙️ Configuration</h3>
                <div class="metric">
                    <span>Mode:</span>
                    <span class="value">Cloud Shell</span>
                </div>
                <div class="metric">
                    <span>Hardware:</span>
                    <span class="value">Simulé</span>
                </div>
                <div class="metric">
                    <span>Qt6 SerialPort:</span>
                    <span class="value" style="color: #4CAF50;">Contourné ✓</span>
                </div>
                <div class="metric">
                    <span>Déploiement:</span>
                    <span class="value" style="color: #4CAF50;">Simplifié ✓</span>
                </div>
                
                <button class="button" onclick="refreshAll()">🔄 Actualiser</button>
                <button class="button" onclick="exportData()">💾 Exporter Données</button>
            </div>
        </div>
        
        <div class="footer">
            <p>🧬 <strong>BioMining Platform</strong> - Révolutionner le Mining avec la Biologie</p>
            <p>Version Cloud Shell Optimisée | Sans Dépendances Qt6 | Interface Web Pure</p>
        </div>
    </div>

    <script>
        // Variables globales
        let updateInterval;
        let acquisitionActive = false;
        let miningActive = false;
        
        // Initialisation
        document.addEventListener('DOMContentLoaded', function() {
            generateElectrodeGrid();
            startRealTimeUpdates();
            log('🎯 Interface utilisateur chargée');
        });
        
        // Génération grille électrodes
        function generateElectrodeGrid() {
            const grid = document.getElementById('electrode-grid');
            grid.innerHTML = '';
            
            for (let i = 0; i < 64; i++) {
                const electrode = document.createElement('div');
                electrode.className = 'electrode';
                electrode.textContent = i;
                electrode.id = `electrode-${i}`;
                electrode.onclick = () => showElectrodeDetails(i);
                grid.appendChild(electrode);
            }
        }
        
        // Mise à jour temps réel
        function startRealTimeUpdates() {
            updateInterval = setInterval(async () => {
                try {
                    await updateMEAStatus();
                    await updateMiningStatus();
                    updateElectrodes();
                    updateTimestamp();
                } catch (error) {
                    console.error('Erreur mise à jour:', error);
                }
            }, 2000);
        }
        
        // Mise à jour MEA
        async function updateMEAStatus() {
            try {
                const response = await fetch('/api/mea/status');
                const data = await response.json();
                
                document.getElementById('mea-status').textContent = data.status === 'connected' ? 'Connecté' : 'Déconnecté';
                document.getElementById('mea-electrodes').textContent = data.electrodes;
                document.getElementById('mea-acquisition').textContent = data.acquisition_active ? 'Active' : 'Arrêtée';
                
                const led = document.getElementById('mea-status-led');
                led.className = `status-indicator ${data.status === 'connected' ? 'status-connected' : 'status-disconnected'}`;
                
            } catch (error) {
                log('❌ Erreur MEA: ' + error.message);
            }
        }
        
        // Mise à jour Mining
        async function updateMiningStatus() {
            try {
                const response = await fetch('/api/mining/status');
                const data = await response.json();
                
                document.getElementById('mining-status').textContent = data.active ? 'Actif' : 'Arrêté';
                document.getElementById('hashrate').textContent = data.hashrate.toLocaleString() + ' H/s';
                document.getElementById('total-hashes').textContent = data.total_hashes.toLocaleString();
                document.getElementById('blocks-found').textContent = data.blocks_found;
                document.getElementById('time-estimate').textContent = data.estimated_time_to_block;
                
                const led = document.getElementById('mining-status-led');
                led.className = `status-indicator ${data.active ? 'status-mining' : 'status-disconnected'}`;
                
            } catch (error) {
                log('❌ Erreur Mining: ' + error.message);
            }
        }
        
        // Mise à jour électrodes
        function updateElectrodes() {
            const activeCount = Math.floor(Math.random() * 15) + 45; // 45-60 actives
            document.getElementById('active-signals').textContent = activeCount;
            document.getElementById('spikes-detected').textContent = Math.floor(Math.random() * 25);
            
            // Animation électrodes
            for (let i = 0; i < 64; i++) {
                const electrode = document.getElementById(`electrode-${i}`);
                if (electrode) {
                    const isActive = Math.random() > 0.3;
                    electrode.className = isActive ? 'electrode active' : 'electrode';
                }
            }
        }
        
        // Fonctions de contrôle
        async function startAcquisition() {
            try {
                const response = await fetch('/api/mea/start', { method: 'POST' });
                const data = await response.json();
                log('▶️ ' + data.message);
                acquisitionActive = true;
            } catch (error) {
                log('❌ Erreur démarrage acquisition: ' + error.message);
            }
        }
        
        async function stopAcquisition() {
            try {
                const response = await fetch('/api/mea/stop', { method: 'POST' });
                const data = await response.json();
                log('⏹️ ' + data.message);
                acquisitionActive = false;
            } catch (error) {
                log('❌ Erreur arrêt acquisition: ' + error.message);
            }
        }
        
        async function startMining() {
            try {
                const response = await fetch('/api/mining/start', { method: 'POST' });
                const data = await response.json();
                log('⛏️ ' + data.message + ` (${data.hashrate} H/s)`);
                miningActive = true;
            } catch (error) {
                log('❌ Erreur démarrage mining: ' + error.message);
            }
        }
        
        async function stopMining() {
            try {
                const response = await fetch('/api/mining/stop', { method: 'POST' });
                const data = await response.json();
                log('⏹️ ' + data.message);
                miningActive = false;
            } catch (error) {
                log('❌ Erreur arrêt mining: ' + error.message);
            }
        }
        
        async function testGenesis() {
            try {
                log('🧪 Test validation bloc Genesis...');
                const response = await fetch('/api/mining/test-genesis');
                const data = await response.json();
                
                if (data.valid) {
                    log('✅ Bloc Genesis validé! Hash: ' + data.calculated_hash.substring(0, 20) + '...');
                } else {
                    log('❌ Échec validation Genesis');
                }
            } catch (error) {
                log('❌ Erreur test Genesis: ' + error.message);
            }
        }
        
        // Fonctions utilitaires
        function refreshAll() {
            log('🔄 Actualisation complète...');
            updateMEAStatus();
            updateMiningStatus();
            updateElectrodes();
        }
        
        function exportData() {
            const data = {
                timestamp: new Date().toISOString(),
                mea_status: document.getElementById('mea-status').textContent,
                mining_active: miningActive,
                hashrate: document.getElementById('hashrate').textContent,
                blocks_found: document.getElementById('blocks-found').textContent
            };
            
            const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'biomining_export_' + new Date().getTime() + '.json';
            a.click();
            URL.revokeObjectURL(url);
            
            log('💾 Données exportées');
        }
        
        function showElectrodeDetails(id) {
            log(`🔍 Électrode ${id}: ${Math.random() > 0.5 ? 'Active' : 'Inactive'} | ${(Math.random() * 100).toFixed(1)} μV`);
        }
        
        function updateTimestamp() {
            document.getElementById('last-update').textContent = new Date().toLocaleTimeString();
        }
        
        function log(message) {
            const logArea = document.getElementById('log-area');
            const timestamp = new Date().toLocaleTimeString();
            const logEntry = document.createElement('div');
            logEntry.textContent = `[${timestamp}] ${message}`;
            
            logArea.appendChild(logEntry);
            logArea.scrollTop = logArea.scrollHeight;
            
            // Limiter à 50 lignes
            while (logArea.children.length > 50) {
                logArea.removeChild(logArea.firstChild);
            }
        }
    </script>
</body>
</html>
"""

# ====================================================================
# 5. ROUTES WEB API
# ====================================================================

@app.route('/')
def index():
    """Page principale"""
    return render_template_string(HTML_TEMPLATE)

@app.route('/api/mea/status')
def mea_status():
    """Status interface MEA"""
    return jsonify(mea_interface.get_status())

@app.route('/api/mea/start', methods=['POST'])  
def mea_start():
    """Démarrer acquisition MEA"""
    return jsonify(mea_interface.start_acquisition())

@app.route('/api/mea/stop', methods=['POST'])
def mea_stop():
    """Arrêter acquisition MEA"""
    return jsonify(mea_interface.stop_acquisition())

@app.route('/api/mea/electrodes')
def mea_electrodes():
    """Données électrodes"""
    electrode_id = request.args.get('id', type=int)
    return jsonify(mea_interface.get_electrode_data(electrode_id))

@app.route('/api/mining/status')
def mining_status():
    """Status mining Bitcoin"""
    return jsonify(bitcoin_miner.get_mining_status())

@app.route('/api/mining/start', methods=['POST'])
def mining_start():
    """Démarrer mining"""
    return jsonify(bitcoin_miner.start_mining())

@app.route('/api/mining/stop', methods=['POST'])  
def mining_stop():
    """Arrêter mining"""
    return jsonify(bitcoin_miner.stop_mining())

@app.route('/api/mining/test-genesis')
def test_genesis():
    """Test validation bloc Genesis"""
    return jsonify(bitcoin_miner.test_genesis_validation())

@app.route('/health')
def health_check():
    """Health check pour monitoring"""
    return jsonify({
        "status": "healthy",
        "timestamp": datetime.now().isoformat(),
        "version": "1.0.0-cloudshell",
        "components": {
            "mea": mea_interface.connected,
            "mining": True,
            "web_interface": True
        }
    })

# ====================================================================
# 6. POINT D'ENTRÉE
# ====================================================================

if __name__ == '__main__':
    print("🧬 BioMining Platform - Version Cloud Shell")
    print("==========================================")
    print("✅ Sans dépendances Qt6 - Interface Web Pure")
    print("🔗 Déploiement simplifié pour Cloud Shell")
    print("📊 MEA + Bitcoin Mining + Interface complète")
    print("")
    
    port = int(os.environ.get('PORT', 5000))
    host = os.environ.get('HOST', '0.0.0.0')
    
    print(f"🚀 Démarrage serveur sur {host}:{port}")
    print(f"🌐 Interface: http://localhost:{port}")
    print("")
    
    app.run(host=host, port=port, debug=False, threaded=True)