# 🚀 Guide de Démarrage - BioMining Platform

## 🎯 Bienvenue !

Ce guide vous permet de démarrer rapidement avec la plateforme BioMining, de l'installation à votre premier mining hybride bio-cryptographique.

## ⏱️ Démarrage Express (5 minutes)

### Option 1: Docker - Le Plus Rapide 🐳

```bash
# Lancement immédiat avec Docker
docker run -it --privileged -p 5000:5000 -p 8080:8080 \
    biomining/platform:latest

# Accès interface web
open http://localhost:5000
```

### Option 2: Installation Automatique (Ubuntu/Debian)

```bash
# Script d'installation one-liner
curl -fsSL https://install.biomining.io | bash

# Ou téléchargement manuel
wget https://github.com/jadaela-ara/BioMining/archive/main.zip
unzip main.zip && cd BioMining-main
chmod +x install.sh && ./install.sh

# Lancement
biomining_gui
```

---

## 📦 Installation Détaillée

### Prérequis Système

#### Configuration Minimale
- **OS**: Ubuntu 20.04+, macOS 10.15+, Windows 10+
- **CPU**: 2+ cores
- **RAM**: 2GB
- **Stockage**: 5GB libre
- **Réseau**: Connexion internet

#### Configuration Recommandée  
- **OS**: Ubuntu 22.04 LTS
- **CPU**: 4+ cores, 3GHz+
- **RAM**: 8GB+
- **MEA Hardware**: Dispositif professionnel (MCS, Blackrock, etc.)

### Installation par Plateforme

#### 🐧 Ubuntu/Debian (Recommandé)

```bash
# 1. Mise à jour système
sudo apt-get update && sudo apt-get upgrade -y

# 2. Installation dépendances
sudo apt-get install -y build-essential cmake git \
    qt6-base-dev qt6-serialport-dev qt6-charts-dev \
    libssl-dev libxkbcommon-dev

# 3. Clone et compilation
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
make -j$(nproc)

# 4. Test installation
./bin/biomining_gui --version
```

#### 🍎 macOS

```bash
# 1. Installation Homebrew (si nécessaire)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 2. Installation dépendances
brew install cmake qt6 openssl git

# 3. Configuration environment
export Qt6_DIR=$(brew --prefix qt6)
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)

# 4. Clone et compilation
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
make -j$(sysctl -n hw.ncpu)

# 5. Lancement
./bin/biomining_gui
```

#### 🪟 Windows (WSL2 Recommandé)

```powershell
# Option 1: WSL2 (recommandé)
wsl --install -d Ubuntu-22.04
# Puis suivre instructions Ubuntu dans WSL2

# Option 2: Native Windows (MSYS2)
# 1. Installer MSYS2 depuis https://www.msys2.org/
# 2. Dans terminal MSYS2:
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-qt6 mingw-w64-x86_64-openssl git

# 3. Compilation
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make -j4
```

---

## 🧬 Configuration MEA Hardware

### Détection Automatique

```bash
# Scan automatique des dispositifs MEA
sudo ./setup_mea_device.sh --auto-detect

# Output exemple:
# 🔍 Scanning for MEA devices...
# ✅ Found: Multi Channel Systems MEA2100 on /dev/ttyUSB0
# ✅ Found: Blackrock CerePlex on 192.168.1.100:3333
# 
# Use: ./biomining_gui --device mcs --port /dev/ttyUSB0
```

### Configuration Rapide par Type

#### Multi Channel Systems (MCS)
```bash
# Setup MCS MEA2100
./biomining_gui --quick-setup mcs \
    --device-path /dev/ttyUSB0 \
    --sample-rate 25000

# Ou configuration réseau
./biomining_gui --quick-setup mcs \
    --network-host 192.168.1.50 \
    --network-port 3333
```

#### Blackrock Microsystems  
```bash
# Setup Blackrock CerePlex
./biomining_gui --quick-setup blackrock \
    --device-path /dev/ttyUSB0 \
    --baud-rate 921600 \
    --electrodes 96
```

#### Mode Demo (Sans Hardware)
```bash
# Démonstration avec MEA simulé
./biomining_gui --demo-mode \
    --simulate-spikes \
    --spike-rate 120 \
    --noise-level 0.1

# Interface web de démo
./biomining_gui --web-demo --port 5000
```

---

## 🎮 Premier Lancement - Interface GUI

### Écran de Bienvenue

Au premier démarrage, vous verrez:

```
╔══════════════════════════════════════════════╗
║          🧬⛏️ BioMining Platform              ║
║                                              ║
║  Bienvenue dans le mining bio-cryptographique║
║                                              ║
║  [🔧 Configuration Rapide]                   ║
║  [📡 Détecter MEA]                          ║
║  [🎯 Mode Démo]                             ║  
║  [📚 Documentation]                          ║
║                                              ║
║  Version: 1.1.0 | Status: Ready             ║
╚══════════════════════════════════════════════╝
```

### Assistant de Configuration (3 étapes)

#### Étape 1: Détection Hardware
- Scan automatique des ports série
- Détection des dispositifs MEA réseau  
- Test de communication basique
- Sélection du dispositif principal

#### Étape 2: Configuration MEA
- Paramètres d'acquisition (fréquence, gain)
- Test d'impédance des électrodes
- Calibration automatique
- Validation qualité du signal

#### Étape 3: Préférences Mining
- Activation du mining hybride
- Réglage du poids bio-entropique (0-50%)
- Configuration des threads CPU
- Test de performance initial

### Interface Principale

```
┌─ BioMining Platform ─────────────────────────────────────────────────┐
│ [File] [MEA] [Mining] [Tools] [Help]                     🟢 Connected │
├─────────────────────────────────────────────────────────────────────┤
│ ┌─ MEA Control ─┐ ┌─ Electrode Grid ─────┐ ┌─ Mining Stats ─┐        │
│ │               │ │                      │ │                │        │
│ │ 🔴 Connect    │ │  [Grid 60 électr.]   │ │ ⛏️  127.3 H/s   │        │
│ │ ▶️ Start       │ │                      │ │ 🧬 Bio: 30%    │        │
│ │ ⚙️ Calibrate   │ │  Real-time visual    │ │ 💎 Blocks: 3   │        │
│ │               │ │                      │ │                │        │
│ │ Quality: 92%  │ │  Spike detection     │ │ 🔥 Spikes: 180/min     │
│ └───────────────┘ └──────────────────────┘ └───────────────┘        │
├─────────────────────────────────────────────────────────────────────┤
│ [Logs] MEA connected | Calibration OK | Mining started...            │
└─────────────────────────────────────────────────────────────────────┘
```

---

## ⚡ Premier Test de Mining

### Lancement Rapide

```bash
# Démarrage complet automatique
./biomining_gui --auto-start \
    --mea-auto-detect \
    --mining-hybrid \
    --bio-weight 0.25

# Ou mode pas-à-pas
./biomining_gui --step-by-step-setup
```

### Séquence de Test Complète

#### 1. Connexion MEA
```bash
# Dans l'interface ou CLI
mea.connect()
# ✅ MEA connected: Multi Channel Systems MEA2100
# ✅ 60 electrodes detected
# ✅ Signal quality: 94.2%
```

#### 2. Calibration Système
```bash
mea.calibrate()
# 🔧 Starting full system calibration...
# 🔧 Electrode impedance test: 58/60 OK  
# 🔧 Signal amplification: Optimal
# ✅ Calibration complete (2m 34s)
```

#### 3. Test Acquisition
```bash
mea.startAcquisition()
# 📊 Data acquisition started
# 📊 Sampling: 25,000 Hz
# 📊 Buffer: Normal (< 50%)
# 🔥 Spikes detected: 142/min
```

#### 4. Démarrage Mining Hybride
```bash
mining.start(bioWeight=0.3)
# ⛏️ Hybrid mining started
# ⛏️ Threads: 4 CPU cores
# ⛏️ Bio-entropy: 30% weight
# 📈 Hash rate: 89.2 → 127.8 H/s (+43%)
```

### Monitoring en Temps Réel

#### Dashboard Console
```bash
# Affichage temps réel
./biomining_gui --console-mode --update-interval 1s

# Output:
╔═══ BIOMINING LIVE DASHBOARD ═══╗
║                                 ║
║ 🧬 MEA Status:     🟢 Optimal   ║  
║ ⚡ Electrodes:     58/60 Active ║
║ 🔥 Spike Rate:     180/min      ║
║ 📊 Signal Quality: 94.2%        ║
║                                 ║
║ ⛏️ Mining Rate:    127.8 H/s    ║
║ 🎯 Bio Contrib.:   38.6 H/s (30%)║
║ 💎 Blocks Found:   3           ║
║ ⏱️ Uptime:        1h 23m       ║
║                                 ║
║ 💻 CPU Usage:      45%         ║
║ 🌡️ Temperature:    52°C        ║
╚═════════════════════════════════╝
```

#### Interface Web Live
```bash
# Démarrage serveur web intégré
./biomining_gui --web-interface --port 8080

# Accès monitoring
open http://localhost:8080/dashboard
```

---

## 🧪 Premiers Expériences

### Test de Stimulation Électrode

```bash
# Interface graphique: MEA → Test Stimulation
# Ou CLI:
mea.stimulate(electrode=15, voltage=2.5, duration=5.0)

# ⚡ Stimulation electrode 15: 2.5V, 5ms
# ⚡ Safety check: PASS
# ⚡ Response detected: +12.3μV spike after 8ms
# ✅ Stimulation successful
```

### Optimisation Bio-Entropie

```bash
# Test différents poids bio-entropiques  
for weight in 0.1 0.2 0.3 0.4 0.5; do
    echo "Testing bio-weight: $weight"
    ./benchmark_mining --bio-weight $weight --duration 60s
done

# Résultats exemple:
# Bio-weight: 0.1 → Hash: 95.2 H/s
# Bio-weight: 0.2 → Hash: 112.5 H/s  
# Bio-weight: 0.3 → Hash: 127.8 H/s ⭐ Optimal
# Bio-weight: 0.4 → Hash: 119.3 H/s
# Bio-weight: 0.5 → Hash: 108.7 H/s
```

### Analyse des Patterns Biologiques

```bash
# Analyse des spikes  
./spike_analyzer --duration 300s --export-csv spikes.csv

# Visualisation
./biomining_gui --spike-viewer --file spikes.csv

# Stats générées:
# 📊 Total spikes: 847
# 📊 Electrodes actives: 42/60 (70%)
# 📊 Fréquence moyenne: 2.82 Hz
# 📊 Amplitude moyenne: -67.3 μV
# 📊 Patterns détectés: Burst (15), Oscillation (3)
```

---

## 📊 Monitoring et Métriques

### Métriques de Base

#### Performance Mining
```bash
# Métriques mining temps réel
curl http://localhost:8080/api/mining/stats | jq

{
  "hash_rate_total": 127.8,
  "hash_rate_bio": 38.6,
  "hash_rate_standard": 89.2,
  "bio_contribution_percent": 30.2,
  "blocks_found": 3,
  "shares_accepted": 1247,
  "efficiency_gain": 43.2,
  "uptime_seconds": 4980
}
```

#### Qualité MEA
```bash  
# Status MEA détaillé
curl http://localhost:8080/api/mea/status | jq

{
  "connection": "connected",
  "device": "Multi Channel Systems MEA2100",
  "electrodes_active": 58,
  "electrodes_total": 60,
  "signal_quality_avg": 94.2,
  "impedance_avg_mohm": 1.15,
  "sampling_rate": 25000,
  "spike_rate_per_min": 180,
  "data_throughput_mbps": 1.85
}
```

### Alertes Automatiques

#### Configuration Alertes
```bash
# Alertes par email
./biomining_gui --alert-config \
    --email admin@lab.com \
    --smtp smtp.lab.com:587 \
    --alert-quality-below 80 \
    --alert-hashrate-drop 20

# Alertes système (notifications desktop)
./biomining_gui --desktop-alerts \
    --alert-spike-burst \
    --alert-block-found \
    --alert-hardware-error
```

#### Types d'Alertes
- 🔴 **Critique**: Perte connexion MEA, erreur hardware
- 🟡 **Avertissement**: Qualité signal < 80%, électrode défaillante  
- 🔵 **Information**: Bloc trouvé, record hash rate
- 🟢 **Succès**: Calibration OK, optimisation réussie

---

## 🔧 Configuration Avancée

### Profils de Performance

#### Profil Recherche (Max Qualité Bio)
```bash
./biomining_gui --profile research \
    --mea-max-quality \
    --sampling-rate 30000 \
    --all-electrodes \
    --bio-weight 0.4 \
    --detailed-logging
    
# Performance: ~95 H/s, Qualité Max
```

#### Profil Production (Max Hash Rate)
```bash  
./biomining_gui --profile production \
    --mea-optimized \
    --sampling-rate 25000 \
    --selective-electrodes \
    --bio-weight 0.25 \
    --performance-mode
    
# Performance: ~140 H/s, Efficace
```

#### Profil Économie (Min Consommation)
```bash
./biomining_gui --profile eco \
    --mea-power-save \
    --sampling-rate 20000 \
    --reduce-threads 2 \
    --bio-weight 0.15 \
    --sleep-mode-idle
    
# Performance: ~85 H/s, 18W seulement  
```

### Configuration Système

#### Variables d'Environnement
```bash
# Configuration globale
export BIOMINING_CONFIG_DIR="$HOME/.biomining"
export BIOMINING_DATA_DIR="/data/biomining" 
export BIOMINING_LOG_LEVEL="INFO"
export BIOMINING_PLUGIN_DIR="/usr/local/lib/biomining/plugins"

# MEA Hardware  
export BIOMINING_MEA_TYPE="MCS_MEA2100"
export BIOMINING_MEA_DEVICE="/dev/ttyUSB0"
export BIOMINING_MEA_BAUD="115200"

# Mining
export BIOMINING_THREADS="4"
export BIOMINING_BIO_WEIGHT="0.3"  
export BIOMINING_POOL_URL="stratum+tcp://pool.example.com:4444"

# Debug
export BIOMINING_DEBUG_MEA="1"
export BIOMINING_PROFILE_ENABLED="1"
```

#### Fichier Configuration Principal
```bash
# ~/.biomining/config.json
{
  "system": {
    "log_level": "INFO",
    "data_retention_days": 30,
    "auto_backup": true,
    "update_check": true
  },
  "mea": {
    "auto_detect": true,
    "preferred_device": "MCS_MEA2100",
    "calibration_interval_hours": 24,
    "quality_threshold": 0.8
  },
  "mining": {
    "auto_start": false,
    "bio_entropy_enabled": true,
    "adaptive_weight": true,
    "performance_monitoring": true
  },
  "gui": {
    "theme": "dark",
    "update_interval_ms": 1000,  
    "show_advanced_panels": true,
    "remember_window_state": true
  }
}
```

---

## 🎯 Prochaines Étapes

### Exploration Approfondie

1. **📚 Documentation Complète**
   ```bash
   # Lire la doc utilisateur détaillée
   firefox docs/USER_GUIDE.md
   
   # API référence pour développeurs
   firefox docs/API_REFERENCE.md
   
   # Guide développeur avancé
   firefox docs/DEVELOPER_GUIDE.md
   ```

2. **🧪 Expérimentation Avancée**
   ```bash
   # Exemples complets
   ./bin/example_real_mea --interactive
   ./bin/example_complete_workflow --step-by-step
   ./bin/example_custom_benchmark --duration 1800
   ```

3. **🔧 Développement Custom**
   ```bash
   # Créer plugin MEA personnalisé
   ./scripts/create_mea_plugin.sh MyCustomMEA
   
   # Algorithme mining personnalisé
   ./scripts/create_mining_algorithm.sh MyAlgorithm
   ```

### Ressources Communauté

#### Support & Questions
- 💬 **Discord**: [BioMining Community](https://discord.gg/biomining)
- 📧 **Email**: support@biomining.io
- 🐛 **Issues**: [GitHub Issues](https://github.com/jadaela-ara/BioMining/issues)
- 📚 **Wiki**: [Documentation Complète](https://docs.biomining.io)

#### Contribution  
- 🤝 **Contributing**: [Guide de Contribution](CONTRIBUTING.md)
- 🔧 **Development**: [Developer Setup](DEVELOPER_GUIDE.md)
- 🧪 **Testing**: [Testing Guidelines](TESTING.md)
- 📖 **Documentation**: [Docs Writing Guide](DOCS_GUIDE.md)

### Formation Avancée

#### Webinaires & Tutorials
- 🎥 **YouTube**: [BioMining Tutorials](https://youtube.com/@biomining)
- 📺 **Webinaires**: Sessions live mensuelles
- 🎓 **Formation**: Cours complets disponibles
- 📖 **Publications**: Articles scientifiques

#### Laboratoire Virtuel
```bash
# Environnement de test complet
docker run -it --name biomining-lab \
    -p 5000:5000 -p 8080:8080 -p 3333:3333 \
    biomining/laboratory:latest

# Accès interface lab
open http://localhost:5000/laboratory
```

---

## ✅ Checklist de Démarrage

### Installation ✓
- [ ] Système d'exploitation compatible
- [ ] Dépendances installées (Qt6, OpenSSL, CMake)
- [ ] BioMining Platform compilé avec succès
- [ ] Tests de base passent (`make test`)

### Configuration MEA ✓  
- [ ] Hardware MEA détecté (ou mode démo activé)
- [ ] Connexion établie avec succès
- [ ] Calibration système complète
- [ ] Qualité signal > 80%

### Mining Hybride ✓
- [ ] Mining standard fonctionnel
- [ ] Bio-entropie activée et configurée
- [ ] Hash rate hybride > mining standard  
- [ ] Monitoring et métriques actives

### Interface & Monitoring ✓
- [ ] Interface GUI responsive
- [ ] Dashboard web accessible
- [ ] Logs et diagnostics configurés
- [ ] Alertes système opérationnelles

---

**🎉 Félicitations !**

Vous avez maintenant une plateforme BioMining complètement opérationnelle. Votre voyage dans le mining bio-cryptographique peut commencer !

**🧬⛏️ Prêt à Révolutionner le Mining ?**

---

**Besoin d'aide ?** Notre communauté est là pour vous accompagner à chaque étape. N'hésitez pas à nous contacter !

📧 **Contact**: [getting-started@biomining.io](mailto:getting-started@biomining.io)  
💬 **Discord**: [Rejoindre la Communauté](https://discord.gg/biomining)  
📚 **Docs**: [Documentation Complète](https://docs.biomining.io)