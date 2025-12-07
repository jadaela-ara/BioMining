# ⚡ Démarrage Rapide - BioMining Local

## 🎯 Objectif
Lancer la plateforme BioMining sur votre PC en 5 minutes, **sans frais**.

---

## 🚀 En 3 Commandes

### Linux / macOS

```bash
# 1. Cloner le repository
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining

# 2. Rendre le script exécutable
chmod +x start_local.sh

# 3. Lancer!
./start_local.sh
```

### Windows

```batch
REM 1. Cloner le repository
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining

REM 2. Lancer!
start_local.bat
```

**C'est tout!** 🎉

---

## 📱 Accéder à l'Interface

Ouvrez votre navigateur sur:
```
http://localhost:8000
```

---

## ✅ Vérification Rapide

### Test 1: API Health
```bash
curl http://localhost:8000/api/health
```

**Réponse attendue:**
```json
{"status": "healthy", "platform": "Pure Python Bio-Entropy Mining"}
```

### Test 2: Interface Web

1. Ouvrir `http://localhost:8000`
2. Vous devriez voir le **Dashboard BioMining**
3. Aller dans l'onglet **"Training"**
4. Lancer un training test:
   - Start Height: `870000`
   - Block Count: `3`
   - Cliquer **"Start Training"**

---

## 🔄 Mises à Jour

Pour récupérer les dernières modifications:

```bash
# Arrêter le serveur (Ctrl+C)
git pull origin main
# Relancer
./start_local.sh  # ou start_local.bat sur Windows
```

---

## 🛑 Arrêter le Serveur

Dans le terminal où le serveur tourne:
```
Ctrl + C
```

---

## 🐛 Problèmes?

### "Port 8000 already in use"
```bash
# Tuer le processus existant
# Linux/macOS:
lsof -i :8000
kill -9 <PID>

# Windows:
netstat -ano | findstr :8000
taskkill /PID <PID> /F
```

### "Module not found"
```bash
# Réinstaller les dépendances
source venv/bin/activate  # ou venv\Scripts\activate sur Windows
pip install -r requirements.txt
```

---

## 📖 Documentation Complète

Pour plus de détails, voir:
- **Guide complet**: `DEPLOIEMENT_LOCAL.md`
- **Fix #11**: `BLOCKCHAIN_FETCHER_FIX_SUMMARY.md`
- **Tous les fixes**: `TRAINING_FIXES_SUMMARY.md`

---

## 💰 Économies

| Déploiement | Coût Mensuel |
|-------------|--------------|
| Google Cloud Run | ~$50-100 💸 |
| **Local (PC)** | **$0** 🎉 |

**Économie annuelle**: **~$600-1200!**

---

## 🎉 C'est Parti!

Votre plateforme BioMining est maintenant:
- ✅ **Gratuite** (0€)
- ✅ **Locale** (sur votre PC)
- ✅ **Rapide** (pas de latence réseau)
- ✅ **Prête** (avec Fix #11)

**Bon mining!** ⛏️🧬
