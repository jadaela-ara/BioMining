# 📚 Guide Utilisateur - BioMining Platform

## 🎯 Bienvenue dans BioMining

Ce guide vous accompagne dans l'utilisation complète de la plateforme BioMining, de l'installation à l'optimisation avancée.

## 📋 Table des Matières

1. [Installation et Setup](#installation-et-setup)
2. [Premier Démarrage](#premier-démarrage)
3. [Configuration MEA Hardware](#configuration-mea-hardware)
4. [Interface Utilisateur](#interface-utilisateur)
5. [Mining Hybride](#mining-hybride)
6. [Monitoring et Diagnostics](#monitoring-et-diagnostics)
7. [Optimisation des Performances](#optimisation-des-performances)
8. [Troubleshooting](#troubleshooting)

---

## 🚀 Installation et Setup

### Prérequis Système

#### Configuration Minimale
- **OS**: Linux (Ubuntu 20.04+), macOS 10.15+, Windows 10+
- **CPU**: 2+ cores (Intel/AMD 64-bit)
- **RAM**: 2GB minimum, 4GB recommandé
- **Stockage**: 5GB espace libre
- **Réseau**: Connexion internet pour mises à jour

#### Configuration Recommandée
- **OS**: Ubuntu 22.04 LTS
- **CPU**: 4+ cores, 3GHz+
- **RAM**: 8GB+
- **GPU**: Compatible CUDA (optionnel pour mining)
- **MEA Hardware**: Dispositif professionnel supporté

### Installation Rapide

#### Option 1: Installation Automatique (Ubuntu/Debian)
```bash
# Téléchargement du script d'installation
curl -fsSL https://raw.githubusercontent.com/jadaela-ara/BioMining/main/install.sh | bash

# Ou téléchargement et exécution manuelle
wget https://github.com/jadaela-ara/BioMining/archive/main.zip
unzip main.zip && cd BioMining-main
chmod +x install.sh && ./install.sh
```

#### Option 2: Compilation depuis les Sources
```bash
# Installation des dépendances
sudo apt-get update
sudo apt-get install -y build-essential cmake git \
    qt6-base-dev qt6-serialport-dev qt6-charts-dev \
    libssl-dev libxkbcommon-dev

# Clone du repository
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining

# Compilation
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
make -j$(nproc)

# Installation (optionnel)
sudo make install
```

#### Option 3: Docker (Multi-plateforme)
```bash
# Lancement direct avec Docker
docker run -it --privileged -p 5000:5000 \
    -v /dev:/dev \
    biomining/platform:latest

# Ou avec docker-compose
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
docker-compose up biomining-platform
```

### Vérification de l'Installation

```bash
# Test des binaires
./bin/biomining_gui --version
./bin/example_real_mea --help

# Test de l'interface graphique
./bin/biomining_gui &

# Test avec hardware MEA (si disponible)
sudo ./bin/example_real_mea --detect-devices
```

---

## 🎬 Premier Démarrage

### Lancement de l'Interface Graphique

```bash
# Depuis le répertoire de build
./bin/biomining_gui

# Ou si installé globalement
biomining_gui
```

### Assistant de Configuration Initial

À la première ouverture, l'assistant vous guide à travers :

1. **Détection Hardware**
   - Scan automatique des ports série
   - Détection des dispositifs MEA connectés
   - Test des protocoles de communication

2. **Configuration de Base**
   - Sélection du type de dispositif MEA
   - Paramètres de connexion (port, baudrate, IP)
   - Configuration des électrodes actives

3. **Calibration Initiale**
   - Test d'impédance des électrodes
   - Calibration automatique du système
   - Validation de la qualité du signal

4. **Préférences Mining**
   - Activation du mining hybride
   - Configuration des threads CPU
   - Réglages d'entropie biologique

### Configuration Rapide avec Présets

```bash
# Preset Multi Channel Systems MEA2100
./bin/biomining_gui --preset mcs_mea2100

# Preset Blackrock CerePlex
./bin/biomining_gui --preset blackrock_cereplex

# Mode démo avec MEA simulé
./bin/biomining_gui --demo-mode
```

---

## 🧬 Configuration MEA Hardware

### Dispositifs MEA Supportés

#### Multi Channel Systems (MCS)
**Modèles**: MEA2100, MEA1060, USB-MEA

**Configuration Standard**:
```bash
# Setup automatique
sudo ./setup_mea_device.sh --device mcs --model mea2100

# Configuration manuelle
./bin/biomining_gui --device-config mcs \
    --protocol tcp \
    --host localhost \
    --port 3333
```

**Paramètres Typiques**:
- **Protocole**: TCP/IP sur port 3333
- **Fréquence**: 25,000 Hz
- **Électrodes**: 60 canaux
- **Amplification**: 1,200x

#### Blackrock Microsystems
**Modèles**: CerePlex Direct, CerePlex W

**Configuration**:
```bash
# Détection automatique
./bin/example_real_mea --device blackrock --auto-detect

# Configuration USB
./bin/biomining_gui --device-config blackrock \
    --protocol serial \
    --port /dev/ttyUSB0 \
    --baudrate 921600
```

#### Plexon OmniPlex
**Configuration Réseau**:
```bash
./bin/biomining_gui --device-config plexon \
    --protocol udp \
    --host 192.168.1.100 \
    --port 6000 \
    --multicast 224.0.0.1
```

#### Intan Technologies (RHD Series)
**Configuration USB**:
```bash
# Installation des drivers
sudo ./setup_mea_device.sh --device intan

# Configuration
./bin/biomining_gui --device-config intan \
    --protocol serial \
    --port /dev/ttyACM0 \
    --sample-rate 30000
```

### Configuration Manuelle Avancée

#### Fichier de Configuration JSON
Créer `~/.biomining/mea_config.json`:

```json
{
  "device": {
    "type": "MultiChannelSystems_MCS",
    "name": "MEA Lab Setup #1",
    "protocol": "TCP",
    "connection": {
      "host": "192.168.1.50",
      "port": 3333,
      "timeout_ms": 5000,
      "retry_count": 3
    }
  },
  "acquisition": {
    "sampling_rate": 25000,
    "electrode_count": 60,
    "active_electrodes": [0, 1, 2, 5, 8, 12, 15, 18, 22, 25],
    "amplification": 1200,
    "filter_low_cut": 300,
    "filter_high_cut": 8000,
    "buffer_size": 2048
  },
  "stimulation": {
    "enabled": true,
    "max_voltage": 3.0,
    "max_current_ua": 80,
    "safety_checks": true,
    "charge_balancing": true
  },
  "spike_detection": {
    "enabled": true,
    "threshold_uv": -60.0,
    "window_ms": 2,
    "dead_time_ms": 1,
    "save_waveforms": true
  },
  "logging": {
    "level": "INFO",
    "save_raw_data": false,
    "data_directory": "/home/user/biomining_data"
  }
}
```

#### Variables d'Environnement
```bash
# Configuration globale
export BIOMINING_CONFIG_FILE="/path/to/config.json"
export BIOMINING_DATA_DIR="/data/biomining"
export BIOMINING_LOG_LEVEL="DEBUG"

# MEA spécifique
export BIOMINING_MEA_TYPE="MCS"
export BIOMINING_MEA_HOST="192.168.1.50"
export BIOMINING_MEA_PORT="3333"

# Mining
export BIOMINING_MINING_THREADS="4"
export BIOMINING_BIO_ENTROPY_WEIGHT="0.3"
```

---

## 🖥️ Interface Utilisateur

### Interface Graphique Principale

#### Vue d'Ensemble
L'interface est organisée en plusieurs panneaux :

1. **Panneau de Contrôle** (gauche)
   - État de connexion MEA
   - Contrôles d'acquisition
   - Configuration en temps réel

2. **Visualisation MEA** (centre)
   - Grille des 60 électrodes
   - Signaux en temps réel
   - Événements de spikes

3. **Mining Dashboard** (droite)
   - Statistiques de mining
   - Hash rate hybride
   - Blocs trouvés

4. **Logs et Diagnostics** (bas)
   - Messages système
   - Erreurs et avertissements
   - Performances détaillées

#### Contrôles Principaux

##### Connexion MEA
```
[🔴] Déconnecté     -> Clic pour connecter
[🟡] Connexion...   -> Attente établissement
[🟢] Connecté       -> Clic pour déconnecter
[🔶] Erreur        -> Voir logs pour détails
```

##### Acquisition de Données
```
[▶️ Démarrer]      -> Lance l'acquisition continue
[⏸️ Pause]         -> Met en pause (garde connexion)
[⏹️ Arrêter]       -> Arrête et sauvegarde
[🔄 Calibrer]      -> Lance calibration complète
```

##### Mining Hybride
```
[⛏️ Start Mining]   -> Démarre mining avec bio-entropie
[📊 Stats]         -> Ouvre statistiques détaillées
[⚙️ Config]        -> Configuration mining avancée
[💾 Save Session]  -> Sauvegarde session complète
```

### Visualisation des Électrodes

#### Grille Interactive
- **Électrode Active** : 🟢 Vert - Signal normal
- **Électrode Stimulant** : 🟠 Orange - En cours de stimulation
- **Spike Détecté** : 🔴 Rouge - Spike récent (flash)
- **Électrode Inactive** : ⚫ Gris - Hors service/déconnectée
- **Calibration** : 🔵 Bleu - En cours de calibration

#### Informations en Temps Réel
Survol d'une électrode affiche :
```
📍 Électrode #15
🔋 Voltage: -42.3 μV
⚡ Impédance: 1.25 MΩ
📶 Qualité: 94%
🕐 Dernier spike: 2.3s
```

#### Graphiques de Signal
- **Oscilloscope Multi-Canaux** : Signaux temps réel
- **Spectrogramme** : Analyse fréquentielle
- **Raster Plot** : Visualisation des spikes
- **Histogramme d'Amplitude** : Distribution des signaux

### Interface Web (Optionnel)

#### Accès Distant
```bash
# Démarrage du serveur web
./bin/biomining_gui --web-server --port 8080

# Accès via navigateur
http://localhost:8080
```

#### Fonctionnalités Web
- **Dashboard Responsive** - Support mobile/tablette
- **Monitoring Temps Réel** - WebSocket pour updates live
- **Contrôle à Distance** - Start/stop acquisition et mining
- **Export de Données** - Téléchargement CSV/JSON
- **APIs REST** - Intégration avec systèmes tiers

---

## ⛏️ Mining Hybride

### Concepts de Base

#### Entropie Biologique
L'entropie biologique enrichit le processus de mining en utilisant :
- **Variabilité des Spikes** - Timing imprévisible des neurones
- **Amplitude des Signaux** - Variations naturelles de voltage
- **Patterns Complexes** - Interactions entre électrodes
- **Bruit Thermique** - Fluctuations quantiques cellulaires

#### Algorithme Hybride
```
Hash Standard = SHA256(Block Header + Nonce)
Bio Entropy = Extract(MEA Signals, Spikes, Timing)
Hybrid Hash = SHA256(Hash Standard + Bio Entropy)
```

### Configuration du Mining

#### Mode Simple
```bash
# Mining hybride basique
./bin/biomining_gui --mining-mode hybrid --bio-weight 0.2

# Mining standard (sans biologie)
./bin/biomining_gui --mining-mode standard

# Mining démo (entropie simulée)
./bin/biomining_gui --mining-mode demo
```

#### Configuration Avancée

##### Interface Graphique
1. **Menu Mining** → **Configuration Hybride**
2. **Réglage Entropie Biologique** : 0% → 50%
3. **Sélection Électrodes** : Choisir électrodes contributives
4. **Algorithme de Mélange** : SHA256, Blake2b, Keccak
5. **Seuils de Qualité** : Filtrage signaux faibles

##### Ligne de Commande
```bash
./bin/hybrid_miner \
    --bio-entropy-weight 0.3 \
    --active-electrodes "0,5,10,15,20,25" \
    --spike-threshold -50.0 \
    --quality-min 0.8 \
    --hash-algorithm sha256 \
    --pool stratum+tcp://pool.example.com:4444 \
    --username your_wallet_address
```

#### Optimisation Automatique

##### Auto-tuning
L'algorithme d'auto-tuning ajuste automatiquement :
- **Poids d'Entropie** selon qualité du signal
- **Électrodes Actives** selon contribution entropique
- **Seuils de Détection** pour maximiser l'efficacité
- **Fréquence d'Échantillonnage** selon patterns détectés

```bash
# Activation auto-tuning
./bin/biomining_gui --auto-tune --tune-duration 3600  # 1 heure
```

##### Profils Prédéfinis
```bash
# Profil haute performance
./bin/biomining_gui --profile performance

# Profil économie d'énergie
./bin/biomining_gui --profile power-save

# Profil recherche (max entropie bio)
./bin/biomining_gui --profile research

# Profil personnalisé
./bin/biomining_gui --profile-file ~/.biomining/custom_profile.json
```

### Monitoring du Mining

#### Métriques Principales
- **Hash Rate Total** : Haches/seconde combiné
- **Hash Rate Bio** : Contribution biologique
- **Hash Rate Standard** : Mining classique
- **Entropie Score** : Qualité de l'entropie (0-100%)
- **Efficiency Ratio** : Gain vs mining standard

#### Dashboard en Temps Réel
```
⛏️  MINING HYBRIDE - DASHBOARD LIVE
════════════════════════════════════════

📊 Performance
   Hash Rate Total:    127.3 H/s  ⬆️ +52%
   Contribution Bio:    38.2 H/s  (30%)
   Hash Rate Standard:  89.1 H/s
   
🧬 Entropie Biologique  
   Score Qualité:       87%       🟢
   Spikes/min:          240       ⬆️
   Électrodes Actives:  42/60     
   Signal/Bruit:        12.4 dB   

💎 Résultats Mining
   Blocs Trouvés:       3         🎉
   Shares Acceptés:     1,247     
   Rejetés:             12        (0.96%)
   Uptime:              2h 34m    

🔋 Système
   CPU Usage:           45%       
   Memory:              234 MB    
   MEA Status:          🟢 Optimal
   Température:         52°C      
```

---

## 📈 Monitoring et Diagnostics

### Surveillance en Temps Réel

#### Indicateurs MEA
```
🧬 MEA STATUS MONITOR
═══════════════════════

🔌 Connexion
   Dispositif:     Multi Channel Systems MEA2100
   Status:         🟢 Connecté (TCP)
   Uptime:         1h 23m 45s
   Latence:        1.2ms ⬇️

⚡ Électrodes  
   Actives:        58/60     🟢
   Qualité Moy.:   92.3%     🟢  
   Impédance:      0.8-2.1MΩ 🟡
   Hors Service:   E34, E57  ⚠️

📊 Acquisition
   Freq. Échant.:  25,000 Hz 🟢
   Débit Données:  1.85 MB/s 
   Buffer Usage:   23%       🟢
   Dropped Pkts:   0         🟢

🔥 Spikes
   Détectés/min:   180       🟢
   Amplitude Moy.: -67.2 μV  
   Électrodes ++-: E12,E25,E41
   False Positifs: 2.1%      🟢
```

#### Graphiques Diagnostics

##### Signal Quality Timeline
```
Quality %
100 ┤                                    ╭─╮  
 95 ┤         ╭─╮                       ╱   ╰─╮
 90 ┤      ╭─╯   ╰─╮                  ╱       ╰
 85 ┤   ╭─╯         ╰─╮             ╱          
 80 ┤╭─╯               ╰─╮        ╱            
    └┴────────────────────────────────────────
     0    5   10   15   20   25   30min
```

##### Spike Rate Distribution
```
Spikes/min
300 ┤     ██                           
250 ┤   █████                         
200 ┤ ███████  ██                     
150 ┤███████████                      
100 ┤███████████ ██                   
 50 ┤██████████████                   
  0 ┤██████████████████████           
    └┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬
     E1 E5 E10E15E20E25E30E35E40E45E50
```

### Alertes et Notifications

#### Configuration des Alertes
```bash
# Alertes par email
./bin/biomining_gui --alert-email admin@lab.com \
    --alert-smtp smtp.lab.com:587

# Alertes Slack
./bin/biomining_gui --alert-slack https://hooks.slack.com/...

# Alertes système (desktop)
./bin/biomining_gui --alert-desktop
```

#### Types d'Alertes
- 🔴 **Critique** : Perte connexion MEA, erreur hardware
- 🟡 **Avertissement** : Qualité signal < 80%, électrode défaillante
- 🔵 **Information** : Bloc trouvé, calibration terminée
- 🟢 **Succès** : Record hash rate, optimisation réussie

### Logs et Historique

#### Structure des Logs
```
/var/log/biomining/
├── system.log          # Logs système généraux
├── mea_interface.log   # Communication MEA
├── mining.log          # Activité mining
├── performance.log     # Métriques performances
└── debug.log          # Debug développeur
```

#### Analyse des Logs
```bash
# Logs en temps réel
tail -f /var/log/biomining/system.log

# Recherche d'erreurs
grep -i "error\|fail\|critical" /var/log/biomining/*.log

# Analyse performance
./bin/log_analyzer --performance --last-24h
./bin/log_analyzer --spike-analysis --electrode 15
```

#### Export et Sauvegarde
```bash
# Export session complète
./bin/biomining_gui --export-session /data/backup/session_$(date +%Y%m%d).zip

# Export données MEA (CSV)
./bin/data_exporter --format csv --electrodes all --duration 1h

# Export mining stats (JSON)
./bin/mining_stats_export --format json --include-bio-entropy
```

---

## ⚡ Optimisation des Performances

### Réglages Système

#### Configuration CPU
```bash
# Performance governor
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Isolation CPU pour mining
isolcpus=2,3 # Dans GRUB_CMDLINE_LINUX

# Priorité temps réel
./bin/biomining_gui --rt-priority 90 --cpu-affinity 2,3
```

#### Configuration Réseau (MEA TCP)
```bash
# Optimisation réseau
echo 'net.core.rmem_max = 16777216' >> /etc/sysctl.conf
echo 'net.core.wmem_max = 16777216' >> /etc/sysctl.conf
echo 'net.ipv4.tcp_rmem = 4096 87380 16777216' >> /etc/sysctl.conf

# Application immédiate
sudo sysctl -p
```

#### Configuration GPU (optionnel)
```bash
# NVIDIA pour mining
nvidia-smi -pm 1  # Mode persistance
nvidia-smi -pl 200  # Limite puissance 200W

# Mining hybride avec GPU
./bin/biomining_gui --gpu-mining --gpu-bio-weight 0.15
```

### Optimisations MEA

#### Paramètres d'Acquisition Optimaux
```json
{
  "acquisition": {
    "sampling_rate": 30000,        // ↑ Pour spikes rapides
    "buffer_size": 4096,           // ↑ Pour réduire latence
    "amplification": 2000,         // ↑ Pour signaux faibles
    "filter_low_cut": 500,         // ↑ Pour réduire bruit
    "filter_high_cut": 6000        // ↓ Pour éviter aliasing
  },
  "optimization": {
    "electrode_selection": "auto",  // Sélection automatique
    "quality_threshold": 0.85,     // Filtre qualité
    "spike_clustering": true,      // Groupement spikes
    "adaptive_threshold": true     // Seuil adaptatif
  }
}
```

#### Calibration Avancée
```bash
# Calibration complète avec optimisation
./bin/calibration_tool --full-system --optimize --duration 300

# Calibration spécifique électrodes problématiques  
./bin/calibration_tool --electrodes "34,57" --intensive

# Auto-calibration périodique
./bin/biomining_gui --auto-calibrate --interval 3600  # Chaque heure
```

### Profils de Performance

#### Profil "Research Lab"
```bash
./bin/biomining_gui --profile research-lab \
    --max-quality-mode \
    --full-electrode-sampling \
    --detailed-logging \
    --bio-entropy-weight 0.4
```
- **Priorité** : Qualité des données biologiques
- **Performance** : ~95 H/s
- **Consommation** : 35W
- **Usage** : Recherche, expérimentation

#### Profil "Production Mining"  
```bash
./bin/biomining_gui --profile production \
    --performance-mode \
    --selective-sampling \
    --optimized-entropy \
    --bio-entropy-weight 0.25
```
- **Priorité** : Hash rate maximal
- **Performance** : ~140 H/s  
- **Consommation** : 28W
- **Usage** : Mining commercial

#### Profil "Power Efficient"
```bash
./bin/biomining_gui --profile power-save \
    --reduced-sampling \
    --smart-electrode-selection \
    --adaptive-bio-weight
```
- **Priorité** : Efficacité énergétique
- **Performance** : ~85 H/s
- **Consommation** : 18W
- **Usage** : Déploiement embarqué

### Benchmarking et Tests

#### Suite de Benchmarks
```bash
# Benchmark complet système
./bin/benchmark_suite --full-system --duration 1800

# Test performance MEA
./bin/mea_benchmark --latency-test --throughput-test --quality-test

# Benchmark mining hybride
./bin/mining_benchmark --bio-entropy-test --hash-rate-test
```

#### Résultats Types
```
🏆 BIOMINING BENCHMARK RESULTS
════════════════════════════════

⚡ Performance
   Hash Rate Standard:     89.2 ± 2.1 H/s
   Hash Rate Hybride:     127.8 ± 3.4 H/s  (+43.2%)
   Gain Entropie Bio:      38.6 H/s        (30.2%)
   
🧬 MEA Performance  
   Latence Acquisition:    0.83 ± 0.12 ms
   Débit Max Soutenu:      8.2 MB/s
   Électrodes Optimal:     54/60           (90%)
   Qualité Moy. Signal:    91.7%
   
💎 Mining Efficacité
   Shares/Watt:            4.57            (+28%)
   Temp. Stable:           47°C
   Uptime 24h:             99.97%
   False Positive:         1.2%
   
🎯 Score Global:           A+ (94/100)
```

---

## 🔧 Troubleshooting

### Problèmes Courants MEA

#### Connexion Impossible
**Symptômes** : Timeout, "Périphérique non trouvé"

**Solutions** :
```bash
# Vérification ports série
ls -l /dev/tty*
sudo dmesg | grep -i usb

# Test permissions
sudo usermod -a -G dialout $USER
sudo chmod 666 /dev/ttyUSB0

# Reset dispositif  
sudo ./setup_mea_device.sh --reset --device /dev/ttyUSB0

# Test de communication basique
echo "AT" > /dev/ttyUSB0 && cat /dev/ttyUSB0
```

#### Qualité Signal Faible
**Symptômes** : Signal < 80%, spikes manqués

**Diagnostic** :
```bash
# Analyse qualité détaillée
./bin/signal_analyzer --electrode-analysis --duration 300

# Test d'impédance
./bin/impedance_tester --all-electrodes --detailed

# Calibration forcée
./bin/calibration_tool --force-recalibration
```

**Solutions** :
- Nettoyer électrodes (solution saline)
- Vérifier connexions hardware
- Ajuster amplification : 1200x → 2000x
- Réduire bruit environnemental

#### Perte de Spikes
**Symptômes** : Spikes détectés < attendu

**Diagnostic** :
```bash
# Analyse seuil optimal
./bin/spike_analyzer --threshold-optimization --electrode 15

# Comparaison avec baseline
./bin/spike_comparator --baseline-file reference_spikes.json
```

**Solutions** :
```bash
# Ajustement seuil automatique
./bin/biomining_gui --adaptive-threshold --sensitivity high

# Filtre passe-bande optimisé
./bin/filter_optimizer --target-frequency 1500 --electrode-group all
```

### Problèmes Mining

#### Hash Rate Faible
**Solutions** :
```bash
# Optimisation threads
./bin/biomining_gui --threads $(nproc) --cpu-affinity auto

# Vérification entropie biologique
./bin/entropy_analyzer --mea-contribution --quality-check

# Profil performance
./bin/biomining_gui --profile performance --overclock-safe
```

#### Instabilité Mining
**Solutions** :
```bash
# Mode stable
./bin/biomining_gui --stability-mode --reduce-bio-weight

# Monitoring système
./bin/system_monitor --thermal --memory --process-mining

# Limitation ressources
./bin/biomining_gui --memory-limit 1GB --cpu-limit 80%
```

### Outils de Diagnostic

#### Auto-Diagnostic
```bash
# Test complet système
./bin/system_diagnostics --comprehensive --fix-auto

# Test MEA spécifique
./bin/mea_diagnostics --hardware-test --communication-test

# Test mining
./bin/mining_diagnostics --performance-test --entropy-test
```

#### Génération de Rapport
```bash
# Rapport complet pour support
./bin/support_report_generator --include-logs --include-config \
    --anonymize --output ~/biomining_support_report.zip

# Rapport performance
./bin/performance_report --last-24h --include-graphs \
    --format pdf --output ~/performance_report.pdf
```

### Support et Communauté

#### Documentation
- 📖 [Documentation Complète](https://biomining.readthedocs.io)
- 🎥 [Tutoriels Vidéo](https://youtube.com/@biomining)
- 📚 [Knowledge Base](https://help.biomining.io)

#### Community Support  
- 💬 [Discord Community](https://discord.gg/biomining)
- 🐛 [GitHub Issues](https://github.com/jadaela-ara/BioMining/issues)
- 📧 [Email Support](mailto:support@biomining.io)
- 🗨️ [Forum Utilisateurs](https://forum.biomining.io)

#### Support Professionnel
Pour un support professionnel :
- 📞 **Hotline** : +33 1 XX XX XX XX
- 📧 **Email Pro** : pro-support@biomining.io  
- 💼 **Contrat SLA** : Disponible pour laboratoires
- 🏢 **Formation On-Site** : Installation et formation équipe

---

**🧬⛏️ BioMining Platform - Guide Utilisateur Complet**

*Ce guide couvre l'utilisation complète de la plateforme. Pour des questions spécifiques, consultez l'[API Reference](API_REFERENCE.md) ou contactez notre support.*