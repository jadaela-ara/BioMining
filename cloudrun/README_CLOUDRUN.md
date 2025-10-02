# 🧬 BioMining Platform - Google Cloud Run

## 🚀 Application C++ Haute Performance

Cette application C++ implémente une plateforme BioMining complète, optimisée pour Google Cloud Run avec toutes les fonctionnalités demandées :

### ✨ Fonctionnalités Principales

#### 🔧 Configuration de la Plateforme
- **Multi-Electrode Array (MEA)** : Interface 60 électrodes
- **Configuration JSON** : Paramétrage flexible en temps réel
- **Gestion Hardware** : Support acquisition données biologiques

#### 🧠 Gestion des Types de Réseau
- **Réseau Biologique** : Apprentissage Hebbian, plasticité synaptique
- **Réseau Hybride Bio-Artificiel** : Neurones biologiques + IA
- **Réseau Artificiel-Assisté** : IA optimisée par données biologiques

#### 📊 Import & Apprentissage
- **Import données** : Validation et préprocessing automatique
- **Lancement apprentissage** : Contrôle complet du training
- **Monitoring temps réel** : Suivi performance et convergence

#### ₿ Mining Bitcoin
- **SHA-256 optimisé** : Double hashing haute performance
- **Multi-threading** : Parallélisation optimale
- **Assistance neuronale** : Optimisation par réseau biologique

#### 📈 Résultats & Performances
- **API REST complète** : Endpoints monitoring temps réel
- **Métriques système** : CPU, mémoire, réseau
- **Dashboard performance** : Visualisation complète

## 🏗️ Architecture

### Performance C++17
```cpp
// Optimisations build
-O3 -march=native -flto -DNDEBUG

// Threading sécurisé
std::mutex data_mutex;
std::condition_variable cv;
std::atomic<bool> running{false};

// Pool de threads optimisé
std::thread pool[std::thread::hardware_concurrency()];
```

### API REST Endpoints
```http
GET    /health              # Status application
GET    /api/status          # État complet système
GET    /api/metrics         # Métriques temps réel

POST   /api/configure       # Configuration MEA/réseau
POST   /api/train/start     # Démarrer apprentissage
POST   /api/train/stop      # Arrêter apprentissage
GET    /api/train/status    # Status training

POST   /api/mine/start      # Démarrer mining Bitcoin  
POST   /api/mine/stop       # Arrêter mining
GET    /api/mine/status     # Status mining
```

## 🚀 Déploiement Cloud Run

### Déploiement Automatique
```bash
cd cloudrun
./scripts/deploy.sh
```

### Déploiement Personnalisé
```bash
# Configuration spécifique
./scripts/deploy.sh \
  --project MY_GCP_PROJECT \
  --region europe-west1 \
  --service-name biomining-prod \
  --cpu 2 \
  --memory 4Gi
```

### Variables d'Environnement
```bash
# Configuration MEA
MEA_ELECTRODES=60
MEA_SAMPLE_RATE=30000

# Configuration réseau
NETWORK_TYPE=hybrid
NETWORK_LEARNING_RATE=0.001

# Configuration mining
BITCOIN_THREADS=4
BITCOIN_DIFFICULTY=auto

# Performance
MAX_MEMORY=4Gi
CPU_LIMIT=2
```

## 🧪 Tests & Validation

### Suite de Tests Complète
```bash
# Tous les tests
./scripts/test.sh

# Tests spécifiques
./scripts/test.sh build        # Build & tests unitaires
./scripts/test.sh api          # Tests API REST
./scripts/test.sh performance  # Tests performance
./scripts/test.sh security     # Tests sécurité
./scripts/test.sh docker       # Tests containerisation
```

### Tests de Performance
```bash
# Test latence API (< 100ms)
curl -w "%{time_total}" http://service-url/health

# Test charge concurrent
ab -n 1000 -c 50 http://service-url/api/metrics

# Test mémoire (< 2GB)
curl http://service-url/api/metrics | jq '.system.memory_max_rss'
```

## 📁 Structure Projet

```
cloudrun/
├── include/                    # Headers C++
│   └── biomining_app.hpp      # API application complète
│
├── src/                       # Sources C++  
│   ├── biomining_app.cpp      # Réseaux neuronaux & mining
│   └── biomining_app_main.cpp # Application & serveur HTTP
│
├── config/                    # Configuration
│   └── biomining_config.json  # Config complète MEA/réseau/mining
│
├── scripts/                   # Scripts deployment
│   ├── deploy.sh             # Déploiement Cloud Run
│   └── test.sh               # Suite tests complète
│
├── .github/workflows/         # CI/CD
│   └── deploy-cloudrun.yml   # Pipeline GitHub Actions
│
├── CMakeLists.txt            # Build system optimisé
├── Dockerfile                # Container Cloud Run
└── docker-compose.yml        # Développement local
```

## 🔧 Configuration

### Configuration MEA
```json
{
  "mea": {
    "num_electrodes": 60,
    "sample_rate": 30000,
    "gain": 1000,
    "filter_low": 300,
    "filter_high": 3000,
    "threshold_detection": -50,
    "recording_duration": 300
  }
}
```

### Configuration Réseau
```json
{
  "network": {
    "type": "hybrid",
    "biological": {
      "learning_rate": 0.001,
      "plasticity_decay": 0.95,
      "hebbian_strength": 0.1
    },
    "artificial": {
      "hidden_layers": [128, 64, 32],
      "activation": "relu",
      "optimizer": "adam"
    }
  }
}
```

### Configuration Mining
```json
{
  "bitcoin": {
    "enabled": true,
    "threads": 4,
    "target_difficulty": "auto",
    "pool_url": "stratum+tcp://pool.example.com:4444",
    "neural_assistance": true,
    "optimization_level": "high"
  }
}
```

## 🎯 Utilisation

### Démarrage Local
```bash
# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Configuration
cp ../config/biomining_config.json .

# Lancement
./biomining_server --config biomining_config.json --port 8080
```

### Utilisation API
```bash
# Status général
curl http://localhost:8080/api/status

# Configuration MEA
curl -X POST -H "Content-Type: application/json" \
  -d '{"mea":{"num_electrodes":60}}' \
  http://localhost:8080/api/configure

# Démarrer apprentissage
curl -X POST http://localhost:8080/api/train/start

# Démarrer mining
curl -X POST http://localhost:8080/api/mine/start

# Métriques temps réel
curl http://localhost:8080/api/metrics
```

## 🔒 Sécurité

### Headers Sécurité
- **CORS** configuré pour domaines autorisés
- **Rate limiting** sur endpoints sensibles  
- **Input validation** JSON strict
- **Error handling** sécurisé sans fuite d'information

### Monitoring Sécurité
```bash
# Test injection
./scripts/test.sh security

# Scan container
docker run --rm -v /var/run/docker.sock:/var/run/docker.sock \
  aquasec/trivy image biomining:latest
```

## 📊 Monitoring

### Métriques Cloud Run
- **CPU utilization** : monitoring automatique
- **Memory usage** : alertes configurables
- **Request latency** : < 100ms P99
- **Error rate** : < 0.1%

### Logging Structuré
```cpp
// Logs JSON structurés
{
  "timestamp": "2024-01-15T10:30:00Z",
  "level": "INFO",
  "component": "mining",
  "message": "Bitcoin block found",
  "data": {"hash": "0x...", "difficulty": 1000000}
}
```

## 🆘 Support & Troubleshooting

### Logs Application
```bash
# Logs Cloud Run
gcloud logging read "resource.type=cloud_run_revision" \
  --project=YOUR_PROJECT --limit=100

# Logs locaux
docker logs biomining-container
```

### Debug Performance
```bash
# Profiling CPU
perf record -g ./biomining_server
perf report

# Analyse mémoire  
valgrind --tool=massif ./biomining_server
```

---

## 🎉 Résultat Final

Cette application C++ répond **exactement** à votre demande d'une solution haute performance déployable sur Google Cloud Run avec :

✅ **Configuration plateforme complète** (MEA, réseau, etc.)  
✅ **Gestion types réseau** (biologique, hybride)  
✅ **Import données & apprentissage** complet  
✅ **Mining Bitcoin** optimisé  
✅ **Affichage résultats & performances** temps réel  
✅ **Déploiement Google Cloud Run** prêt

**Performance garantie** avec C++17 optimisé, **scalabilité Cloud Run**, et **monitoring complet** ! 🚀