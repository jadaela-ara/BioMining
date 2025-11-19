# 🚀 Guide de Configuration pour Google Cloud Shell

## 🔧 Installation des Dépendances

### Option 1 : Script Automatique (Recommandé)

```bash
# Dans votre répertoire BioMining
./install_deps_cloudshell.sh
```

### Option 2 : Installation Manuelle

Si le script ne fonctionne pas, installez manuellement :

```bash
# Mise à jour
sudo apt-get update

# Dépendances essentielles
sudo apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-charts-dev \
    libssl-dev \
    pkg-config

# Dépendances optionnelles mais recommandées
sudo apt-get install -y \
    libboost-all-dev \
    qt6-tools-dev \
    valgrind
```

## 🏗️ Compilation

### Étapes de Base

```bash
# 1. Créer le répertoire de build
mkdir -p build && cd build

# 2. Configuration avec CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

# 3. Compilation
make -j$(nproc)

# 4. Tests (optionnel)
make test
```

### Compilation Rapide (Debug)

```bash
# Pour un développement plus rapide
mkdir -p build_debug && cd build_debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
make -j$(nproc)
```

## 🔍 Dépannage

### Problème : "Qt6 not found"

```bash
# Installer Qt6 complet
sudo apt-get install -y qt6-base-dev qt6-charts-dev qt6-tools-dev

# Vérifier l'installation
qmake6 --version
```

### Problème : "OpenSSL not found"

```bash
# Installer OpenSSL
sudo apt-get install -y libssl-dev libcrypto++-dev

# Vérifier
openssl version
```

### Problème : "Boost not found" (Warning)

```bash
# Boost est optionnel, mais recommandé
sudo apt-get install -y libboost-all-dev
```

### Problème : "make: command not found"

```bash
# Installer build-essential
sudo apt-get install -y build-essential cmake
```

## 🎯 Commandes Utiles

### Nettoyage

```bash
# Nettoyer le build
rm -rf build/
mkdir -p build

# Ou via Make (si disponible)
make clean
```

### Compilation Sélective

```bash
# Compiler seulement la bibliothèque core
make biomining_core

# Compiler seulement les tests
make test_mea_interface

# Compiler tout
make -j$(nproc)
```

### Vérifications

```bash
# Vérifier les dépendances installées
dpkg -l | grep -E "(qt6|cmake|build-essential|libssl)"

# Vérifier les versions
gcc --version
cmake --version
qmake6 --version
```

## ⚡ Optimisations Cloud Shell

### Utilisation Efficace des Ressources

```bash
# Utiliser tous les cœurs disponibles
make -j$(nproc)

# Ou limiter pour éviter la surcharge
make -j2
```

### Compilation Incrémentale

```bash
# Après des modifications, compilation rapide
cd build && make

# Reconfiguration seulement si nécessaire
# (éviter cmake .. à chaque fois)
```

## 🐛 Erreurs Communes et Solutions

### Error: "gtest/gtest.h: No such file or directory"

**Solution** : Les fichiers de test ont été convertis vers Qt Test. Assurez-vous d'avoir la dernière version :

```bash
git pull origin main
```

### Error: "'BiologicalNetwork' was not declared"

**Solution** : Problèmes de namespace. Vérifiez que vous avez les dernières corrections :

```bash
git pull origin main
./fix_biomining_cloudshell.sh  # Si nécessaire
```

### Error: "No rule to make target 'setup-deps'"

**Solution** : Utilisez le nouveau script :

```bash
./install_deps_cloudshell.sh
```

## 📋 Checklist de Vérification

Avant de compiler, vérifiez :

- [ ] Dépendances installées : `./install_deps_cloudshell.sh`
- [ ] Code à jour : `git pull origin main`
- [ ] Répertoire build créé : `mkdir -p build`
- [ ] Configuration CMake : `cmake .. -DBUILD_TESTS=ON`
- [ ] Compilation : `make -j$(nproc)`

## 🎉 Résultat Attendu

Si tout fonctionne correctement, vous devriez voir :

```
[100%] Built target biomining_core
[100%] Built target biomining_gui
[100%] Built target test_mea_interface
... 
Build completed successfully!
```

---

💡 **Conseil** : Gardez ce guide ouvert pendant le développement pour référence rapide !