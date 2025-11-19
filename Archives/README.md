# Archives - BioMining Platform

Ce répertoire contient les fichiers archivés de l'ancien système BioMining avec intégration C++.

## Contenu

### Documentation C++
- Fichiers de documentation sur l'intégration C++/Qt
- Guides de compilation et déploiement C++
- Documentation sur les bindings pybind11

### Fichiers de Build C++
- CMakeLists.txt - Configuration CMake
- Makefile - Build système
- Scripts de compilation (compile_*.sh)

### Code Source C++
- src/ - Code source C++ du système
- include/ - Headers C++
- python_bindings/ - Bindings pybind11
- examples/ - Exemples C++

### Scripts de Déploiement C++
- deploy_cpp_*.sh - Scripts de déploiement avec C++
- Dockerfiles variés pour différentes configurations C++

### Fichiers de Configuration
- Configurations Docker Compose
- Fichiers de service Cloud Run
- Requirements Python pour version C++

### Tests et Utilitaires C++
- Tests de bindings C++
- Tests d'alignement et validation
- Scripts de diagnostic

## Pourquoi Archivé?

La plateforme a été simplifiée pour utiliser uniquement Python pur, éliminant les dépendances C++/Qt complexes. Cela facilite:
- Le déploiement
- La maintenance
- Le développement
- L'exécution sur différentes plateformes

## Version Actuelle

La version actuelle de la plateforme (dans le répertoire racine) utilise:
- **Pure Python** - Aucune dépendance C++
- **Real Bitcoin Mining** - Mining réel avec Stratum
- **Bio-Entropy** - Génération d'entropie biologique
- **Web Interface** - Interface web complète

## Déploiement Actuel

Utilisez le script `deploy_python.sh` à la racine du projet pour déployer la version Python pure.

```bash
./deploy_python.sh
```

## Restauration

Si vous avez besoin de restaurer l'ancienne version C++:
1. Copiez les fichiers nécessaires depuis Archives/ vers la racine
2. Utilisez un des scripts `deploy_cpp_*.sh`
3. Assurez-vous d'avoir les dépendances C++ installées

## Date d'Archivage

19 Novembre 2025

---

**Note**: Ces fichiers sont conservés pour référence historique. La plateforme Python pure est recommandée pour tous les nouveaux déploiements.
