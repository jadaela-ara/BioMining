# Guide de Déploiement - Plateforme Bio-Mining Bitcoin

## 🚀 Déploiement Rapide

### Prérequis Système

**Ubuntu/Debian:**
```bash
# Installation automatique des dépendances
make setup-deps

# Ou installation manuelle
sudo apt-get update
sudo apt-get install -y build-essential cmake qt6-base-dev qt6-charts-dev libssl-dev libboost-all-dev
```

**Autres systèmes:**
- Qt6 (base + charts)
- OpenSSL 
- Boost (optionnel)
- CMake 3.16+
- Compilateur C++17+

### Installation Standard

```bash
# Clone du repository
git clone <your-repo-url>
cd bio-crypto-mining-platform

# Build et installation automatiques
make build
sudo make install

# Ou via script de déploiement
./scripts/deploy.sh production
```

### Test de l'Installation

```bash
# Interface CLI
biomining_cli --help
biomining_cli --benchmark --iterations 5

# Interface graphique (si display disponible)
biomining_gui
```

## 🐳 Déploiement Docker

### Build et Lancement Rapide

```bash
# Build de l'image
docker build -t biomining-platform .

# Lancement basique
docker run --rm -it biomining-platform --help

# Avec Docker Compose
docker-compose up biomining-platform
```

### Environnements Docker

```bash
# Développement
docker-compose --profile development up biomining-dev

# Tests automatisés
docker-compose --profile testing up biomining-tests

# Benchmark de performance
docker-compose --profile benchmark up biomining-benchmark

# Monitoring
docker-compose --profile monitoring up biomining-monitor
```

## 🔧 Configuration Environnements

### Développement Local

```bash
./scripts/deploy.sh development --clean
cd build && make -j$(nproc)

# Tests pendant développement
make quick-test

# Lancement direct
./build/bin/biomining_gui
./build/bin/biomining_cli --verbose --iterations 3
```

### Production

```bash
# Déploiement production avec sauvegarde
./scripts/deploy.sh production --backup --clean

# Service systemd automatique créé
sudo systemctl status biomining
sudo systemctl start biomining

# Logs de production
sudo journalctl -u biomining -f
```

### Configuration avancée

```bash
# Variables d'environnement
export INSTALL_PREFIX=/opt/biomining
export BUILD_THREADS=8
export ENABLE_QUANTUM=ON

# Déploiement personnalisé
make build CMAKE_BUILD_TYPE=Release ENABLE_QUANTUM_OPTIMIZATION=ON
```

## 📊 Tests et Validation

### Tests Unitaires

```bash
# All tests
make test

# Tests spécifiques
make test-mea          # Interface MEA
make test-mining       # Moteur Bitcoin
make test-integration  # Tests end-to-end
```

### Benchmarks

```bash
# Benchmark CLI
make benchmark

# Benchmark Docker
docker-compose --profile benchmark up

# Benchmark personnalisé
./build/bin/biomining_cli --benchmark --iterations 50 --output benchmark-results.json
```

### Tests de Performance

```bash
# Tests mémoire (Linux seulement)
make test-memory

# Profiling de performance
make profile

# Analyse statique
make check
```

## 🔍 Monitoring et Maintenance

### Logs et Diagnostic

```bash
# Logs temps réel
tail -f /var/log/biomining/biomining.log

# Interface de monitoring
biomining_cli --monitor

# Diagnostic système
biomining_cli --diagnostic --verbose
```

### Maintenance Préventive

```bash
# Sauvegarde configuration
biomining_cli --export-config backup-$(date +%Y%m%d).json

# Test de santé système
biomining_cli --health-check

# Mise à jour calibration
biomining_cli --calibrate --auto-adjust
```

## 🛠️ Résolution de Problèmes

### Problèmes Courants

**1. Qt6 non trouvé:**
```bash
# Ubuntu/Debian
sudo apt-get install qt6-base-dev qt6-charts-dev

# Vérification
pkg-config --modversion Qt6Core
```

**2. Erreurs de compilation:**
```bash
# Nettoyage complet
make distclean
make build

# Mode debug pour plus d'infos
make build-debug
```

**3. Tests qui échouent:**
```bash
# Tests en mode verbose
cd build && ctest --verbose --output-on-failure

# Tests individuels
./build/test_mea_interface --verbose
```

**4. Permissions Docker:**
```bash
# Ajouter utilisateur au groupe docker
sudo usermod -aG docker $USER
newgrp docker
```

### Debug Avancé

**Valgrind (détection fuites mémoire):**
```bash
valgrind --leak-check=full ./build/bin/biomining_cli --iterations 1
```

**GDB (debugging):**
```bash
gdb ./build/bin/biomining_gui
(gdb) run
(gdb) bt  # backtrace si crash
```

**Logs détaillés:**
```bash
# Mode debug complet
QT_LOGGING_RULES="*.debug=true" ./build/bin/biomining_gui

# Logs spécifiques MEA
QT_LOGGING_RULES="biomining.mea.debug=true" ./build/bin/biomining_cli
```

## 🔐 Sécurité et Production

### Configuration Sécurisée

```bash
# Utilisateur dédié
sudo useradd -r -s /bin/false biomining

# Permissions restrictives
sudo chown -R biomining:biomining /opt/biomining
sudo chmod 750 /opt/biomining/bin/*

# Firewall (si interface réseau future)
sudo ufw allow 8080/tcp  # Interface web monitoring
```

### Sauvegarde et Récupération

```bash
# Sauvegarde complète
tar -czf biomining-backup-$(date +%Y%m%d).tar.gz \
    /opt/biomining \
    /etc/biomining \
    /var/lib/biomining

# Restauration
tar -xzf biomining-backup-YYYYMMDD.tar.gz -C /
sudo systemctl restart biomining
```

### Mise à Jour

```bash
# Sauvegarde avant mise à jour
./scripts/deploy.sh production --backup

# Mise à jour avec nouvelle version
git pull origin main
./scripts/deploy.sh production --force

# Vérification post-mise à jour
biomining_cli --version
sudo systemctl status biomining
```

## 📈 Optimisation Performance

### Réglages Système

```bash
# Optimisations CPU
echo 'performance' | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Limites ressources
ulimit -n 65536  # File descriptors
ulimit -u 32768  # Processes

# Configuration mining optimale
biomining_cli --config-optimize --cpu-cores $(nproc) --memory-gb 8
```

### Monitoring Performance

```bash
# Métriques système
htop
iotop
nvidia-smi  # Si GPU

# Métriques application
biomining_cli --stats --continuous
```

## 🆘 Support et Ressources

### Documentation
- `README.md` - Vue d'ensemble du projet
- `docs/TECHNICAL_ARCHITECTURE.md` - Architecture détaillée
- `examples/` - Exemples d'utilisation
- Inline documentation dans le code

### Commandes Utiles

```bash
# Aide générale
make help

# Status du projet
make status

# Informations version
biomining_cli --version --build-info

# Configuration système
biomining_cli --system-info
```

### Contacts et Support

- **Issues GitHub**: Pour bugs et demandes de fonctionnalités
- **Documentation**: `docs/` et commentaires code
- **Exemples**: `examples/` pour cas d'usage
- **Community**: Forums et discussions développeur

---

*Ce guide couvre les scénarios de déploiement les plus courants. Pour des configurations spécialisées ou des problèmes spécifiques, consultez la documentation technique ou contactez l'équipe de support.*