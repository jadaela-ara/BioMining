# 🚀 Options de Déploiement C++ Corrigées

## ❌ Problème Résolu
**Erreur**: `Unable to read file [-]: [Errno 2] No such file or directory: '-'`

**Cause**: Syntaxe incorrecte avec `gcloud builds submit --config=-` et heredoc

## ✅ Solutions Disponibles

### Option 1: Script Corrigé (Recommandé)
**Fichier**: `deploy_cpp_moc_fixed.sh` ✅
- Utilise un fichier temporaire `cloudbuild-cpp-moc.yaml` 
- Nettoie automatiquement le fichier temporaire
- Compatible avec `gcloud builds submit`

**Commande**:
```bash
./deploy_cpp_moc_fixed.sh
```

### Option 2: Script Simplifié (Alternative)
**Fichier**: `deploy_cpp_moc_simple.sh` ✅ 
- Utilise directement `docker build` et `docker push`
- Puis déploie avec `gcloud run deploy`
- Évite complètement les problèmes de Cloud Build

**Commande**:
```bash
./deploy_cpp_moc_simple.sh
```

## 🔧 Correction Appliquée

### Avant (Problématique):
```bash
gcloud builds submit \
    --config=- \
    . << EOF
steps:
...
EOF
```

### Après (Corrigé):
```bash
# Crée un fichier temporaire
cat > "cloudbuild-cpp-moc.yaml" << EOF
steps:
...
EOF

# Utilise le fichier temporaire
gcloud builds submit --config="cloudbuild-cpp-moc.yaml" .

# Nettoie
rm -f "cloudbuild-cpp-moc.yaml"
```

## 🎯 Recommendation

**Utilisez `deploy_cpp_moc_fixed.sh`** - il offre :
- ✅ Correction de l'erreur heredoc
- ✅ Build multi-étape avec Cloud Build
- ✅ Ressources optimisées (E2_HIGHCPU_8)
- ✅ Timeout étendu (3600s)
- ✅ Tests automatiques après déploiement

## 🧪 Test de Déploiement

Après déploiement réussi :
```bash
# Vérifier C++ disponible
curl https://YOUR-SERVICE-URL/api/bindings

# Tester BiologicalNetwork real C++
curl -X POST https://YOUR-SERVICE-URL/api/configure-biological-network \
     -H 'Content-Type: application/json' \
     -d '{"learning_rate": 0.01, "epochs": 100}'
```

**Résultat attendu**: `"cpp_available": true` et méthodes C++ réelles ! 🎉