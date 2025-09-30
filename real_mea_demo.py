#!/usr/bin/env python3
"""
Demo de l'Interface MEA Réelle - BioMining Platform
Serveur de démonstration pour montrer les fonctionnalités de l'interface MEA réelle
"""

from flask import Flask, jsonify, render_template_string
import time
import random
from datetime import datetime

app = Flask(__name__)

# Template HTML pour la démo
HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🧬 Interface MEA Réelle - BioMining Platform</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            color: white;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .status-card {
            background: rgba(255,255,255,0.1);
            padding: 20px;
            border-radius: 10px;
            border: 1px solid rgba(255,255,255,0.2);
            backdrop-filter: blur(10px);
        }
        .electrode-grid {
            display: grid;
            grid-template-columns: repeat(10, 1fr);
            gap: 5px;
            background: rgba(0,0,0,0.3);
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 20px;
        }
        .electrode {
            width: 30px;
            height: 30px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 10px;
            color: white;
            transition: all 0.3s ease;
        }
        .electrode.active {
            background: #4CAF50;
            box-shadow: 0 0 10px #4CAF50;
        }
        .electrode.inactive {
            background: #666;
        }
        .electrode.stimulating {
            background: #FF9800;
            animation: pulse 0.5s infinite;
        }
        .spike-detected {
            background: #FF4444 !important;
            animation: spike 0.8s ease-out;
        }
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.2); }
            100% { transform: scale(1); }
        }
        @keyframes spike {
            0% { transform: scale(1); box-shadow: 0 0 5px #FF4444; }
            50% { transform: scale(1.5); box-shadow: 0 0 20px #FF4444; }
            100% { transform: scale(1); box-shadow: 0 0 5px #FF4444; }
        }
        .controls {
            text-align: center;
            margin: 20px 0;
        }
        .btn {
            background: #4CAF50;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            margin: 0 5px;
            transition: background 0.3s;
        }
        .btn:hover {
            background: #45a049;
        }
        .btn.danger {
            background: #f44336;
        }
        .btn.danger:hover {
            background: #da190b;
        }
        .log {
            background: rgba(0,0,0,0.5);
            padding: 15px;
            border-radius: 5px;
            max-height: 200px;
            overflow-y: auto;
            font-family: monospace;
            font-size: 12px;
        }
        .metric {
            display: flex;
            justify-content: space-between;
            margin: 5px 0;
        }
        .value {
            font-weight: bold;
            color: #4CAF50;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🧬 Interface MEA Réelle - BioMining Platform</h1>
            <p>Démonstration de l'interface hardware Multi-Electrode Array</p>
        </div>

        <div class="status-grid">
            <div class="status-card">
                <h3>État du Système</h3>
                <div class="metric">
                    <span>Statut:</span>
                    <span class="value" id="connection-status">Connecté</span>
                </div>
                <div class="metric">
                    <span>Dispositif:</span>
                    <span class="value">Multi Channel Systems MEA2100</span>
                </div>
                <div class="metric">
                    <span>Électrodes actives:</span>
                    <span class="value" id="active-electrodes">60/60</span>
                </div>
                <div class="metric">
                    <span>Fréq. échantillonnage:</span>
                    <span class="value">25,000 Hz</span>
                </div>
            </div>

            <div class="status-card">
                <h3>Acquisition Temps Réel</h3>
                <div class="metric">
                    <span>Spikes détectés:</span>
                    <span class="value" id="spike-count">0</span>
                </div>
                <div class="metric">
                    <span>Qualité signal:</span>
                    <span class="value" id="signal-quality">95.2%</span>
                </div>
                <div class="metric">
                    <span>Débit données:</span>
                    <span class="value" id="data-rate">1.5 MB/s</span>
                </div>
                <div class="metric">
                    <span>Timestamp:</span>
                    <span class="value" id="timestamp"></span>
                </div>
            </div>

            <div class="status-card">
                <h3>Calibration & Tests</h3>
                <div class="metric">
                    <span>Impédance moyenne:</span>
                    <span class="value" id="impedance">1.2 MΩ</span>
                </div>
                <div class="metric">
                    <span>Calibration:</span>
                    <span class="value">Complète</span>
                </div>
                <div class="metric">
                    <span>Dernière stimulation:</span>
                    <span class="value" id="last-stim">Jamais</span>
                </div>
                <div class="metric">
                    <span>Mode acquisition:</span>
                    <span class="value">Continu</span>
                </div>
            </div>
        </div>

        <div class="status-card">
            <h3>Matrice des Électrodes (60x)</h3>
            <div class="electrode-grid" id="electrode-grid">
                <!-- Électrodes générées en JavaScript -->
            </div>
        </div>

        <div class="controls">
            <button class="btn" onclick="startAcquisition()">▶ Démarrer Acquisition</button>
            <button class="btn danger" onclick="stopAcquisition()">⏸ Arrêter</button>
            <button class="btn" onclick="performCalibration()">⚙ Calibration</button>
            <button class="btn" onclick="testStimulation()">⚡ Test Stimulation</button>
            <button class="btn" onclick="resetSystem()">🔄 Reset</button>
        </div>

        <div class="status-card">
            <h3>Journal d'Événements</h3>
            <div class="log" id="event-log"></div>
        </div>
    </div>

    <script>
        let spikeCount = 0;
        let isAcquiring = true;
        let stimulatingElectrode = -1;

        // Initialiser la grille d'électrodes
        function initElectrodes() {
            const grid = document.getElementById('electrode-grid');
            for (let i = 0; i < 60; i++) {
                const electrode = document.createElement('div');
                electrode.className = 'electrode active';
                electrode.id = `electrode-${i}`;
                electrode.textContent = i + 1;
                electrode.title = `Électrode ${i + 1}`;
                grid.appendChild(electrode);
            }
        }

        // Simuler l'activité des spikes
        function simulateSpikes() {
            if (!isAcquiring) return;

            if (Math.random() < 0.15) { // 15% de chance de spike
                const electrodeId = Math.floor(Math.random() * 60);
                const electrode = document.getElementById(`electrode-${electrodeId}`);
                
                electrode.classList.add('spike-detected');
                setTimeout(() => {
                    electrode.classList.remove('spike-detected');
                }, 800);
                
                spikeCount++;
                document.getElementById('spike-count').textContent = spikeCount;
                
                logEvent(`Spike détecté sur électrode ${electrodeId + 1} (${(Math.random() * 200 - 100).toFixed(1)} μV)`);
            }
        }

        // Mettre à jour l'horodatage
        function updateTimestamp() {
            const now = new Date();
            document.getElementById('timestamp').textContent = now.toLocaleTimeString();
            
            // Simuler variation de qualité signal
            const quality = (95 + Math.random() * 4).toFixed(1);
            document.getElementById('signal-quality').textContent = quality + '%';
        }

        // Ajouter un événement au journal
        function logEvent(message) {
            const log = document.getElementById('event-log');
            const timestamp = new Date().toLocaleTimeString();
            log.innerHTML += `[${timestamp}] ${message}\\n`;
            log.scrollTop = log.scrollHeight;
        }

        // Fonctions de contrôle
        function startAcquisition() {
            isAcquiring = true;
            document.getElementById('connection-status').textContent = 'Acquisition Active';
            logEvent('Acquisition temps réel démarrée');
        }

        function stopAcquisition() {
            isAcquiring = false;
            document.getElementById('connection-status').textContent = 'En Attente';
            logEvent('Acquisition arrêtée');
        }

        function performCalibration() {
            logEvent('Calibration démarrée...');
            
            // Simuler calibration sur quelques électrodes
            const electrodes = [5, 15, 25, 35, 45];
            electrodes.forEach((id, index) => {
                setTimeout(() => {
                    const electrode = document.getElementById(`electrode-${id}`);
                    electrode.style.background = '#FFA500';
                    setTimeout(() => {
                        electrode.style.background = '#4CAF50';
                    }, 1000);
                    logEvent(`Électrode ${id + 1} calibrée (${(1.0 + Math.random() * 0.5).toFixed(2)} MΩ)`);
                }, index * 500);
            });
            
            setTimeout(() => {
                logEvent('Calibration terminée avec succès');
            }, 3000);
        }

        function testStimulation() {
            if (stimulatingElectrode >= 0) {
                logEvent('Stimulation déjà en cours...');
                return;
            }
            
            stimulatingElectrode = Math.floor(Math.random() * 60);
            const electrode = document.getElementById(`electrode-${stimulatingElectrode}`);
            
            electrode.classList.add('stimulating');
            logEvent(`Stimulation électrode ${stimulatingElectrode + 1} (2.5V, 100μA, 5ms)`);
            
            setTimeout(() => {
                electrode.classList.remove('stimulating');
                stimulatingElectrode = -1;
                document.getElementById('last-stim').textContent = new Date().toLocaleTimeString();
                logEvent('Stimulation terminée');
            }, 2000);
        }

        function resetSystem() {
            spikeCount = 0;
            document.getElementById('spike-count').textContent = '0';
            document.getElementById('event-log').innerHTML = '';
            logEvent('Système réinitialisé');
            logEvent('Interface MEA Réelle - Prête');
        }

        // Initialisation
        window.onload = function() {
            initElectrodes();
            logEvent('Interface MEA Réelle initialisée');
            logEvent('Dispositif: Multi Channel Systems MEA2100');
            logEvent('60 électrodes détectées et configurées');
            logEvent('Calibration automatique effectuée');
            logEvent('Système prêt pour acquisition');
            
            // Démarrer les mises à jour
            setInterval(simulateSpikes, 800);
            setInterval(updateTimestamp, 1000);
        };
    </script>
</body>
</html>
"""

@app.route('/')
def index():
    """Page principale de démonstration"""
    return render_template_string(HTML_TEMPLATE)

@app.route('/api/status')
def api_status():
    """API pour récupérer le statut du système MEA"""
    return jsonify({
        'status': 'connected',
        'device': 'Multi Channel Systems MEA2100',
        'active_electrodes': 60,
        'sampling_rate': 25000,
        'signal_quality': round(95 + random.random() * 4, 1),
        'spike_count': random.randint(0, 150),
        'impedance_avg': round(1.0 + random.random() * 0.5, 2),
        'timestamp': datetime.now().isoformat(),
        'acquisition_active': True
    })

@app.route('/api/electrodes')
def api_electrodes():
    """API pour récupérer les données des électrodes"""
    electrodes = []
    for i in range(60):
        electrodes.append({
            'id': i,
            'voltage': round(random.uniform(-100, 100), 2),  # μV
            'impedance': round(random.uniform(0.5, 2.0), 2),  # MΩ
            'active': random.random() > 0.05,  # 95% actives
            'quality': round(random.uniform(0.8, 1.0), 2)
        })
    return jsonify({'electrodes': electrodes})

@app.route('/api/spikes')
def api_spikes():
    """API pour récupérer les événements de spikes récents"""
    spikes = []
    for _ in range(random.randint(0, 5)):
        spikes.append({
            'electrode_id': random.randint(0, 59),
            'amplitude': round(random.uniform(-200, -50), 1),  # μV
            'timestamp': time.time() - random.uniform(0, 10),
            'waveform': [round(random.uniform(-100, 100), 1) for _ in range(20)]
        })
    return jsonify({'spikes': spikes})

if __name__ == '__main__':
    print("🧬 Lancement du serveur de démonstration MEA Réelle...")
    print("Interface: http://localhost:5000")
    app.run(host='0.0.0.0', port=5000, debug=False)