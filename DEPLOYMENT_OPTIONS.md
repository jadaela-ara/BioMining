# 🚀 Options de Déploiement C++ Corrigées

## ❌ Problèmes Résolus
1. **Erreur 1**: `Unable to read file [-]: [Errno 2] No such file or directory: '-'`
2. **Erreur 2**: `.options.timeout: unused` dans cloudbuild.yaml

**Causes**: 
- Syntaxe incorrecte avec `gcloud builds submit --config=-` et heredoc
- Configuration `timeout` invalide dans la section `options` de cloudbuild.yaml

## ✅ Solutions Disponibles (3 Options)

### Option 1: Script Corrigé 
**Fichier**: `deploy_cpp_moc_fixed.sh` ✅
- **Correction**: Supprime `timeout` de la section `options`
- Utilise un fichier temporaire `cloudbuild-cpp-moc.yaml` 
- Nettoie automatiquement le fichier temporaire
- Compatible avec `gcloud builds submit`

**Commande**:
```bash
./deploy_cpp_moc_fixed.sh
```

### Option 2: Script Direct (Recommandé) 
**Fichier**: `deploy_cpp_moc_direct.sh` ✅ **NOUVEAU**
- Utilise `gcloud builds submit --tag` (plus simple)
- Évite complètement les problèmes de cloudbuild.yaml
- Même fonctionnalité, syntaxe simplifiée

**Commande**:
```bash
./deploy_cpp_moc_direct.sh
```

### Option 3: Script Docker Local
**Fichier**: `deploy_cpp_moc_simple.sh` ✅ 
- Utilise directement `docker build` et `docker push`
- Puis déploie avec `gcloud run deploy`
- Évite complètement Cloud Build

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

## 🎯 Recommendations

### **RECOMMANDÉ**: `deploy_cpp_moc_direct.sh` 
**Pourquoi**: Syntaxe la plus simple et robuste
- ✅ Évite tous les problèmes de cloudbuild.yaml
- ✅ Utilise `gcloud builds submit --tag` (syntaxe simple)
- ✅ Ressources optimisées (E2_HIGHCPU_8)
- ✅ Timeout étendu (3600s)
- ✅ Tests automatiques après déploiement

### **Alternative**: `deploy_cpp_moc_simple.sh`
**Pourquoi**: Si Docker local est préféré
- ✅ Build Docker local plus contrôlable
- ✅ Évite complètement Cloud Build
- ✅ Même résultat final

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