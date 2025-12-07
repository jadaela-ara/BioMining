# 🏠 Guide de Déploiement Local - BioMining Platform

## 💰 Pourquoi Local?
- ✅ **Gratuit** - Pas de frais Google Cloud Run
- ✅ **Rapide** - Pas de build Docker (~10 min économisés)
- ✅ **Debug facile** - Logs en temps réel
- ✅ **Modifications instantanées** - Mode `--reload`

---

## 📋 Prérequis

### Logiciels Requis

#### 1. Python 3.10+
```bash
# Vérifier la version
python3 --version
# Devrait afficher: Python 3.10.x ou supérieur
```

**Installation si nécessaire:**
- **Windows**: https://www.python.org/downloads/
- **macOS**: `brew install python@3.10`
- **Linux**: `sudo apt install python3.10 python3.10-venv python3-pip`

#### 2. Node.js 18+ (Optionnel - pour le frontend)
```bash
# Vérifier
node --version
npm --version
```

**Installation si nécessaire:**
- **Windows/macOS**: https://nodejs.org/
- **Linux**: `sudo apt install nodejs npm`

#### 3. Git
```bash
git --version
```

---

## 🚀 Installation et Configuration

### Étape 1: Cloner le Repository

```bash
# Choisir un emplacement (exemple: votre dossier personnel)
cd ~
# ou sur Windows: cd C:\Users\VotreNom\

# Créer un dossier pour le projet
mkdir BioMining-Local
cd BioMining-Local

# Cloner depuis GitHub
git clone https://github.com/jadaela-ara/BioMining.git
cd BioMining
```

**Résultat attendu:**
```
Cloning into 'BioMining'...
remote: Enumerating objects: 2453, done.
remote: Counting objects: 100% (2453/2453), done.
✅ Repository cloned successfully
```

### Étape 2: Configuration Python Backend

```bash
# Assurer d'être dans le dossier BioMining
cd ~/BioMining-Local/BioMining
# ou sur Windows: cd C:\Users\VotreNom\BioMining-Local\BioMining

# Créer un environnement virtuel Python
python3 -m venv venv

# Activer l'environnement virtuel
# Sur Linux/macOS:
source venv/bin/activate

# Sur Windows PowerShell:
venv\Scripts\Activate.ps1

# Sur Windows CMD:
venv\Scripts\activate.bat
```

**Vous devriez voir** `(venv)` au début de votre ligne de commande.

```bash
# Installer toutes les dépendances Python
pip install -r requirements.txt
```

**Installation attendue (~2-3 minutes):**
```
Installing collected packages: numpy, fastapi, uvicorn, requests, ...
Successfully installed fastapi-0.104.1 uvicorn-0.24.0 numpy-1.24.3 ...
✅ All dependencies installed
```

### Étape 3: Lancer le Backend

```bash
# Toujours dans le dossier BioMining avec (venv) activé
# Lancer le serveur FastAPI
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --reload
```

**Sortie attendue:**
```
INFO:     Will watch for changes in these directories: ['/home/user/BioMining']
INFO:     Uvicorn running on http://0.0.0.0:8000 (Press CTRL+C to quit)
INFO:     Started reloader process [12345] using StatReload
INFO:     Started server process [12346]
INFO:     Waiting for application startup.
✅ Pure Python Bio-Entropy Mining Platform initialized
INFO:     Application startup complete.
```

**🎉 Le backend est maintenant accessible sur:** `http://localhost:8000`

### Étape 4: Accéder à l'Interface Web

#### Option A: Via le Serveur Python (Recommandé - Plus Simple)

Le serveur FastAPI sert déjà les fichiers statiques!

**Ouvrez votre navigateur et allez sur:**
```
http://localhost:8000
```

✅ **C'est tout!** L'application devrait s'afficher.

#### Option B: Serveur de Développement Séparé (Pour développement frontend)

Si vous voulez modifier le frontend avec rechargement automatique:

**Ouvrir un NOUVEAU terminal** (garder le backend actif):

```bash
cd ~/BioMining-Local/BioMining/web

# Installer les dépendances npm (une seule fois)
npm install

# Lancer le serveur de développement
npm run dev
# ou si pas de script "dev":
python3 -m http.server 3000
```

Puis ouvrez: `http://localhost:3000`

---

## ✅ Vérification du Déploiement

### Test 1: Backend API
```bash
# Dans un nouveau terminal
curl http://localhost:8000/api/health
```

**Réponse attendue:**
```json
{
  "status": "healthy",
  "platform": "Pure Python Bio-Entropy Mining",
  "version": "1.0.0"
}
```

### Test 2: Interface Web

Ouvrez `http://localhost:8000` dans votre navigateur.

**Vous devriez voir:**
- ✅ Dashboard Bio-Mining
- ✅ Onglets: Dashboard, Training, Mining, System
- ✅ Console logs: "🚀 BioMining Pure Python Platform initialized"

### Test 3: Training Historical

1. Aller sur l'onglet **"Training"**
2. Remplir le formulaire:
   - Start Height: `870000`
   - Block Count: `5`
3. Cliquer sur **"Start Training"**

**Dans les logs du backend, vous verrez:**
```
INFO:     🎓 Starting historical training session
INFO:     🔍 Fetching block at height 870000 (fetch #1)
INFO:     ✅ Successfully fetched block 870000: hash=0000..., nonce=0xa15f5978
INFO:     ✅ New unique block: height=870000
INFO:     🔄 Training on block 1/5: height 870000
```

---

## 🔧 Configuration Avancée

### Changer le Port du Backend

Par défaut, le backend tourne sur le port `8000`. Pour changer:

```bash
# Utiliser un autre port (ex: 5000)
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 5000 --reload
```

Puis accédez à: `http://localhost:5000`

### Mode Production (Sans Auto-Reload)

Pour de meilleures performances (sans rechargement automatique):

```bash
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --workers 4
```

### Configuration CORS (Si Frontend sur Port Différent)

Si vous servez le frontend depuis un port différent (ex: 3000), le CORS est déjà configuré dans `server.py`:

```python
# Déjà présent dans web/api/server.py
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # En production, spécifier les origines exactes
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
```

---

## 📊 Surveillance et Logs

### Voir les Logs en Temps Réel

Les logs s'affichent directement dans le terminal où vous avez lancé le backend:

```
INFO:     🔍 Fetching block at height 870000 (fetch #1)
INFO:     ✅ Successfully fetched block 870000
INFO:     ✅ New unique block: height=870000
INFO:     🔄 Training on block 1/5: height 870000
INFO:     ✅ Neural loss: 0.350000
```

### Niveau de Log

Pour plus de détails, modifiez dans `server.py`:

```python
# Changer de INFO à DEBUG
logging.basicConfig(level=logging.DEBUG, ...)
```

---

## 🛑 Arrêter le Serveur

Dans le terminal où le backend tourne:
```bash
# Appuyer sur:
Ctrl + C

# Devrait afficher:
INFO:     Shutting down
INFO:     Finished server process
```

---

## 🔄 Mise à Jour du Code

Quand vous voulez récupérer les dernières modifications:

```bash
# Dans le dossier BioMining
cd ~/BioMining-Local/BioMining

# Arrêter le serveur (Ctrl+C si en cours)

# Récupérer les derniers commits
git pull origin main

# Réinstaller les dépendances (au cas où)
source venv/bin/activate  # ou venv\Scripts\activate sur Windows
pip install -r requirements.txt

# Relancer le serveur
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --reload
```

---

## 🐛 Dépannage

### Problème: "Port already in use"

**Erreur:**
```
ERROR:    [Errno 98] Address already in use
```

**Solution:**
```bash
# Trouver le processus sur le port 8000
# Linux/macOS:
lsof -i :8000
kill -9 <PID>

# Windows:
netstat -ano | findstr :8000
taskkill /PID <PID> /F

# Ou utiliser un autre port:
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8001 --reload
```

### Problème: "Module 'requests' not found"

**Solution:**
```bash
# Assurez-vous que venv est activé
source venv/bin/activate  # ou venv\Scripts\activate

# Réinstaller les dépendances
pip install -r requirements.txt
```

### Problème: "Can't connect to blockchain.info"

**Erreur dans les logs:**
```
ERROR: blockchain.info API error: Connection timeout
```

**Solutions:**
1. Vérifier votre connexion internet
2. Attendre quelques minutes (rate limiting temporaire)
3. Essayer un bloc plus récent (ex: 880000+)

### Problème: Page blanche dans le navigateur

**Solutions:**
1. Vérifier que le backend tourne (`http://localhost:8000/api/health`)
2. Ouvrir la console du navigateur (F12) pour voir les erreurs
3. Vider le cache du navigateur (Ctrl+Shift+R)
4. Vérifier les logs du backend pour les erreurs

---

## 📱 Accès depuis un Autre Appareil (Même Réseau)

Pour accéder à l'interface depuis un téléphone/tablette sur le même WiFi:

```bash
# Trouver votre IP locale
# Linux/macOS:
ifconfig | grep "inet "
# Windows:
ipconfig

# Exemple d'IP: 192.168.1.100

# Le serveur tourne déjà sur 0.0.0.0:8000, donc accessible via:
http://192.168.1.100:8000
```

---

## ⚡ Scripts de Démarrage Rapide

### Linux/macOS

Créez un fichier `start.sh`:

```bash
#!/bin/bash
cd ~/BioMining-Local/BioMining
source venv/bin/activate
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --reload
```

Rendez-le exécutable:
```bash
chmod +x start.sh
./start.sh
```

### Windows

Créez un fichier `start.bat`:

```batch
@echo off
cd C:\Users\VotreNom\BioMining-Local\BioMining
call venv\Scripts\activate.bat
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --reload
pause
```

Double-cliquez dessus pour lancer.

---

## 📊 Comparaison Local vs Cloud Run

| Aspect | Local | Google Cloud Run |
|--------|-------|------------------|
| **Coût** | 🟢 Gratuit | 🔴 ~$50-100/mois |
| **Setup** | 🟢 5 minutes | 🟡 10 minutes + config |
| **Performance** | 🟢 Excellent (votre PC) | 🟢 Excellent (servers Google) |
| **Accessibilité** | 🟡 Réseau local uniquement | 🟢 Internet mondial |
| **Maintenance** | 🟢 Simple (git pull) | 🟡 Redéploiement nécessaire |
| **Démarrage** | 🟢 Instantané | 🟡 Cold start possible |
| **Logs** | 🟢 Terminal direct | 🟡 Via gcloud ou console |
| **Scalabilité** | 🔴 1 instance | 🟢 Auto-scale |

---

## ✅ Résumé - Commandes Essentielles

### Démarrage Quotidien

```bash
# 1. Aller dans le dossier
cd ~/BioMining-Local/BioMining

# 2. Activer l'environnement virtuel
source venv/bin/activate  # Linux/macOS
# ou: venv\Scripts\activate  # Windows

# 3. Lancer le serveur
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --reload

# 4. Ouvrir le navigateur
# http://localhost:8000
```

### Mise à Jour

```bash
# Arrêter le serveur (Ctrl+C)
git pull origin main
pip install -r requirements.txt
# Relancer le serveur
```

---

## 🎯 Prochaines Étapes

Maintenant que votre plateforme est locale:

1. ✅ **Tester le Fix #11**: Lancer un training et vérifier les logs
2. ✅ **Développer**: Modifier le code et voir les changements en direct
3. ✅ **Économiser**: Plus de frais Cloud Run!

---

## 🆘 Support

Si vous rencontrez des problèmes:

1. **Vérifier les logs** du terminal backend
2. **Ouvrir la console** du navigateur (F12)
3. **Vérifier les commits** récents: `git log --oneline -5`

---

**Plateforme**: BioMining Pure Python
**Déploiement**: Local (Gratuit)
**Backend**: http://localhost:8000
**Status**: ✅ Prêt à l'emploi
