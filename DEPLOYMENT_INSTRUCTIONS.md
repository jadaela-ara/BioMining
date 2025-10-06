# 🚀 Instructions de Déploiement - Biomining Web Interface

Ce guide vous explique comment déployer votre application web de minage Bitcoin biologique sur Google Cloud Run.

## 📋 Prérequis

1. **Google Cloud CLI installé** : 
   - Téléchargez depuis : https://cloud.google.com/sdk/docs/install
   - Vérifiez : `gcloud --version`

2. **Authentification** :
   ```bash
   gcloud auth login
   gcloud config set project VOTRE_PROJECT_ID
   ```

3. **Projet Google Cloud** avec :
   - Facturation activée
   - APIs Cloud Run et Cloud Build activées (le script les activera automatiquement)

## 🛠️ Méthodes de Déploiement

### Option 1 : Déploiement Simple (Recommandé)
Le script le plus facile à utiliser avec configuration interactive :

```bash
./deploy_simple_fixed.sh
```

**Avantages :**
- ✅ Configuration interactive
- ✅ Validation automatique des prérequis  
- ✅ Gestion d'erreurs robuste
- ✅ Instructions de dépannage intégrées

### Option 2 : Déploiement Avancé
Pour les utilisateurs expérimentés avec plus de contrôle :

```bash
# Configurez d'abord le projet (optionnel)
export GOOGLE_CLOUD_PROJECT=votre-projet-id

./deploy_web_interface_fixed.sh
```

### Option 3 : Déploiement avec Dockerfile Personnalisé
Utilise le Dockerfile optimisé sans psutil :

```bash
./deploy_no_psutil.sh
```

## 🔧 Configuration des Variables d'Environnement

Configurez votre projet avant le déploiement (optionnel) :

```bash
# Projet GCP
export GOOGLE_CLOUD_PROJECT=votre-projet-id

# Région (optionnel, défaut: europe-west1)
export GOOGLE_CLOUD_REGION=europe-west1
```

## 📊 Vérification du Déploiement

Une fois déployé, votre application sera accessible via :

- **🏠 Interface Web** : `https://votre-service-url/`
- **📊 API Status** : `https://votre-service-url/api/status`
- **🔌 WebSocket** : `wss://votre-service-url/ws/hybrid-mining`

### Test de Santé
```bash
curl https://votre-service-url/api/status
```

## ❌ Résolution des Problèmes Courants

### Erreur : "PROJECT_ID not found"
```bash
# Listez vos projets
gcloud projects list

# Configurez le bon projet
gcloud config set project votre-projet-id
```

### Erreur : "Authentication required"
```bash
gcloud auth login
gcloud auth configure-docker
```

### Erreur : "Billing account required"
1. Allez sur https://console.cloud.google.com/billing
2. Associez un compte de facturation à votre projet

### Erreur : "Quota exceeded"
1. Vérifiez vos quotas : https://console.cloud.google.com/iam-admin/quotas
2. Demandez une augmentation si nécessaire
3. Essayez une autre région :
   ```bash
   export GOOGLE_CLOUD_REGION=us-central1
   ./deploy_simple_fixed.sh
   ```

### Erreur de Build : "Source code not found"
Vérifiez que vous êtes dans le bon répertoire :
```bash
ls -la  # Doit contenir web/, requirements-no-psutil.txt, etc.
```

## 📝 Logs et Monitoring

### Consulter les logs de déploiement
```bash
gcloud logging read 'resource.type="cloud_run_revision"' --limit 20
```

### Consulter les logs de l'application
```bash
gcloud run services logs read biomining-web --region=europe-west1
```

### Monitoring dans la Console GCP
1. Allez sur https://console.cloud.google.com/run
2. Cliquez sur votre service "biomining-web"
3. Consultez l'onglet "Logs" et "Métriques"

## 🔄 Mise à Jour de l'Application

Pour mettre à jour votre application après des modifications :

```bash
# Méthode simple
./deploy_simple_fixed.sh

# Ou spécifiez explicitement le projet
export GOOGLE_CLOUD_PROJECT=votre-projet-id
./deploy_web_interface_fixed.sh
```

## 🗑️ Suppression du Service

Pour supprimer complètement le service :

```bash
gcloud run services delete biomining-web --region=europe-west1
```

## 💡 Conseils de Performance

1. **Optimisation des coûts** : Le service s'arrête automatiquement quand il n'y a pas de trafic
2. **Scaling** : Configure automatiquement de 0 à 10 instances selon le trafic
3. **Région** : Choisissez une région proche de vos utilisateurs

## 📞 Support

Si vous rencontrez des problèmes :

1. Vérifiez ce guide de dépannage
2. Consultez les logs avec les commandes ci-dessus
3. Vérifiez la documentation Cloud Run : https://cloud.google.com/run/docs

---

🧠⚡ **Votre interface révolutionnaire de minage Bitcoin biologique sera bientôt en ligne !** ⚡🧠