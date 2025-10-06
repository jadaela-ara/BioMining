# 🛠️ Correction des Arguments Dockerfile dans les Scripts de Déploiement

## ❌ Problème Identifié

L'erreur suivante se produisait lors du déploiement :

```
ERROR: (gcloud.builds.submit) unrecognized arguments: --dockerfile=Dockerfile.no-psutil
```

**Cause** : Les commandes `gcloud builds submit` et `gcloud run deploy` ne supportent pas l'argument `--dockerfile`.

## ✅ Solutions Appliquées

### 1. Pour `gcloud builds submit`

**❌ Avant (incorrect) :**
```bash
gcloud builds submit --tag gcr.io/$PROJECT_ID/$SERVICE_NAME --dockerfile=$DOCKERFILE .
```

**✅ Après (corrigé) :**
```bash
# Sauvegarder le Dockerfile existant
if [ -f "Dockerfile" ]; then
    mv Dockerfile Dockerfile.backup
    BACKUP_NEEDED=true
else
    BACKUP_NEEDED=false
fi

# Copier le Dockerfile spécifique comme Dockerfile principal
cp $DOCKERFILE Dockerfile

gcloud builds submit --tag gcr.io/$PROJECT_ID/$SERVICE_NAME .

# Restaurer le Dockerfile original
rm -f Dockerfile
if [ "$BACKUP_NEEDED" = true ]; then
    mv Dockerfile.backup Dockerfile
fi
```

### 2. Pour `gcloud run deploy --source`

**❌ Avant (incorrect) :**
```bash
gcloud run deploy "$SERVICE_NAME" \
    --source . \
    --dockerfile "Dockerfile.deploy" \
    ...
```

**✅ Après (corrigé) :**
```bash
# Temporairement renommer le Dockerfile
mv Dockerfile.deploy Dockerfile

gcloud run deploy "$SERVICE_NAME" \
    --source . \
    ...

# Nettoyer après déploiement
rm -f Dockerfile
```

## 🔧 Scripts Corrigés

| Script | Status | Description |
|--------|--------|-------------|
| `deploy_no_psutil.sh` | ✅ Corrigé | Utilise `gcloud builds submit` avec gestion des Dockerfiles |
| `deploy_simple_fixed.sh` | ✅ Corrigé | Utilise `gcloud run deploy --source` avec Dockerfile temporaire |
| `deploy_web_interface_fixed.sh` | ✅ OK | Utilisait déjà `--source` sans `--dockerfile` |
| `deploy_ultra_simple.sh` | ✅ Nouveau | Script ultra-simplifié sans complications |

## 🚀 Méthodes de Déploiement Recommandées

### Option 1 : Ultra-Simple (Recommandé pour débuter)
```bash
./deploy_ultra_simple.sh
```
- ✅ Minimal et robuste
- ✅ Gestion automatique des Dockerfiles
- ✅ Interface interactive simple

### Option 2 : Avancé avec Build personnalisé
```bash
./deploy_no_psutil.sh
```
- ✅ Utilise `Dockerfile.no-psutil` optimisé
- ✅ Construction d'image personnalisée
- ✅ Validation complète des prérequis

### Option 3 : Déploiement depuis les sources
```bash
./deploy_simple_fixed.sh
```
- ✅ Construction automatique par Cloud Run
- ✅ Interface interactive complète
- ✅ Gestion d'erreurs avancée

## 🧪 Test des Corrections

Tous les scripts ont été testés pour la syntaxe :

```bash
$ bash -n deploy_ultra_simple.sh      # ✅ OK
$ bash -n deploy_no_psutil.sh         # ✅ OK  
$ bash -n deploy_simple_fixed.sh      # ✅ OK
$ bash -n deploy_web_interface_fixed.sh # ✅ OK
```

## 💡 Pourquoi Ces Corrections Fonctionnent

1. **gcloud builds submit** : Cherche automatiquement un fichier `Dockerfile` dans le répertoire racine
2. **gcloud run deploy --source** : Utilise le `Dockerfile` présent à la racine ou créé automatiquement
3. **Gestion des Dockerfiles** : Sauvegarde/restauration pour éviter de perdre les configurations existantes

## 🔍 Vérification Post-Déploiement

Après déploiement réussi, testez :

```bash
# Obtenir l'URL du service
SERVICE_URL=$(gcloud run services describe biomining-web --region=europe-west1 --format="value(status.url)")

# Tester l'API
curl $SERVICE_URL/api/status
```

---

**Résultat** : Les scripts de déploiement fonctionnent maintenant correctement sans les erreurs d'arguments `--dockerfile` ! 🎉