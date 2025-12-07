# 🚀 BioMining Platform - Guide de Déploiement

## 💡 Quelle Option Choisir?

### 🏠 Déploiement Local (Recommandé pour Développement)

**Avantages:**
- ✅ **Gratuit** - 0€ de frais
- ✅ **Rapide** - Setup en 5 minutes
- ✅ **Simple** - Pas de configuration cloud
- ✅ **Debug facile** - Logs en direct
- ✅ **Modifications instantanées** - Mode auto-reload

**Inconvénients:**
- ❌ Accessible uniquement sur votre réseau local
- ❌ Votre PC doit rester allumé

**👉 [Guide Rapide](./DEMARRAGE_RAPIDE.md)** | **[Guide Complet](./DEPLOIEMENT_LOCAL.md)**

---

### ☁️ Google Cloud Run (Production)

**Avantages:**
- ✅ **Accessible partout** - Via Internet
- ✅ **Scalable** - S'adapte à la charge
- ✅ **Toujours disponible** - 24/7
- ✅ **Sécurisé** - Infrastructure Google

**Inconvénients:**
- ❌ **Coûte de l'argent** - ~50-100€/mois
- ❌ Setup plus complexe - Configuration GCP requise
- ❌ Rebuild nécessaire - 10 min par déploiement

**👉 [Guide Cloud](./DEPLOY_FIX_11.md)**

---

## 🎯 Démarrage Ultra-Rapide (Local)

### Linux / macOS
```bash
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
chmod +x start_local.sh
./start_local.sh
```

### Windows
```batch
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
start_local.bat
```

**Puis ouvrir:** `http://localhost:8000`

---

## 📊 Comparaison Détaillée

| Critère | Local | Cloud Run |
|---------|-------|-----------|
| **Coût** | 🟢 0€/mois | 🔴 ~50-100€/mois |
| **Setup Initial** | 🟢 5 min | 🟡 30 min |
| **Accessibilité** | 🟡 Réseau local | 🟢 Internet mondial |
| **Performance** | 🟢 Excellente | 🟢 Excellente |
| **Maintenance** | 🟢 `git pull` | 🟡 Redéploiement |
| **Logs** | 🟢 Terminal direct | 🟡 Via gcloud |
| **Développement** | 🟢 Idéal | 🟡 Plus lent |
| **Production** | 🟡 Limité | 🟢 Professionnel |

---

## 📚 Documentation Complète

### Déploiement Local
- **[Démarrage Rapide](./DEMARRAGE_RAPIDE.md)** - En 3 commandes
- **[Guide Complet](./DEPLOIEMENT_LOCAL.md)** - Tout savoir sur le déploiement local
- Scripts automatisés: `start_local.sh` (Linux/macOS), `start_local.bat` (Windows)

### Déploiement Cloud
- **[Guide Cloud Run](./DEPLOY_FIX_11.md)** - Déploiement production
- **[Script Deploy](./deploy_python.sh)** - Automatisation complète

### Fixes et Améliorations
- **[Fix #11](./BLOCKCHAIN_FETCHER_FIX_SUMMARY.md)** - Blockchain fetcher enhancement
- **[Tous les Fixes](./TRAINING_FIXES_SUMMARY.md)** - Historique complet
- **[Fix WebSocket](./WEBSOCKET_FIX.md)** - Passage à REST API
- **[Fix Synchronization](./FIX_SYNCHRONIZATION_SUMMARY.md)** - Training sync

---

## 🔧 Technologies

### Backend
- **FastAPI** - Framework web Python moderne
- **Uvicorn** - Serveur ASGI haute performance
- **NumPy** - Calculs numériques
- **Requests** - Client HTTP pour blockchain API

### Frontend
- **Vanilla JavaScript** - Pas de framework lourd
- **Chart.js** - Visualisations
- **CSS3** - Interface moderne

### Déploiement
- **Local**: Python + Uvicorn
- **Cloud**: Docker + Google Cloud Run

---

## 🎓 Pour Commencer

### 1. Choisir Votre Option

**Développement/Tests** → Déploiement Local
**Production/Demo Public** → Google Cloud Run

### 2. Suivre le Guide

- Local: [DEMARRAGE_RAPIDE.md](./DEMARRAGE_RAPIDE.md)
- Cloud: [DEPLOY_FIX_11.md](./DEPLOY_FIX_11.md)

### 3. Tester

```bash
# Test API Health
curl http://localhost:8000/api/health

# Test Training
curl -X POST http://localhost:8000/api/training/historical/start \
  -H "Content-Type: application/json" \
  -d '{"start_height": 870000, "count": 3}'
```

---

## 🆘 Support & Dépannage

### Problèmes Courants

**Port 8000 déjà utilisé:**
```bash
# Trouver et tuer le processus
lsof -i :8000  # Linux/macOS
netstat -ano | findstr :8000  # Windows
```

**Module non trouvé:**
```bash
source venv/bin/activate  # Activer venv
pip install -r requirements.txt  # Réinstaller
```

**Blockchain API timeout:**
- Vérifier connexion internet
- Attendre quelques minutes (rate limiting)
- Essayer un autre bloc

### Logs

**Local:**
- Directement dans le terminal

**Cloud:**
```bash
gcloud logging read "resource.type=cloud_run_revision" --limit 50
```

---

## 🔄 Mises à Jour

### Local
```bash
git pull origin main
./start_local.sh  # Relancer
```

### Cloud
```bash
git pull origin main
./deploy_python.sh  # Redéployer (~10 min)
```

---

## 📈 Prochaines Étapes

Après le déploiement:

1. ✅ Tester l'interface web
2. ✅ Lancer un training test (3-5 blocs)
3. ✅ Vérifier les logs pour Fix #11
4. ✅ Développer de nouvelles features!

---

## 💰 Économies avec Déploiement Local

| Période | Cloud Run | Local | Économie |
|---------|-----------|-------|----------|
| 1 mois | 75€ | 0€ | **75€** |
| 1 an | 900€ | 0€ | **900€** |
| 2 ans | 1800€ | 0€ | **1800€** |

**Note**: Coût Cloud Run estimé pour usage continu 24/7

---

## 🎉 Conclusion

**Déploiement Local** = 🟢 **Recommandé pour:**
- Développement
- Tests
- Démos locales
- Économiser de l'argent

**Google Cloud Run** = 🟢 **Recommandé pour:**
- Production
- Accès public
- Demo professionnelle
- Scalabilité

---

**Commencer maintenant:**
```bash
# Clone & Start!
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
./start_local.sh  # ou start_local.bat sur Windows
```

**Bon mining!** ⛏️🧬
