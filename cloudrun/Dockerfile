# BioMining Platform - Dockerfile pour Google Cloud Run
# Optimisé pour performance C++ et déploiement cloud-native

# ====================================================================
# STAGE 1: BUILD ENVIRONMENT
# ====================================================================

FROM ubuntu:22.04 AS builder

LABEL maintainer="BioMining Team"
LABEL description="BioMining Platform - High Performance C++ Build"

# Éviter les interactions pendant l'installation
ENV DEBIAN_FRONTEND=noninteractive

# Installer les dépendances de build
RUN apt-get update && apt-get install -y \
    # Outils de build essentiels
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    # Compilateur moderne
    gcc-11 \
    g++-11 \
    # Bibliothèques système
    libssl-dev \
    libcrypto++-dev \
    # Outils réseau et utilitaires
    curl \
    wget \
    git \
    # Outils de profiling (optionnel)
    valgrind \
    gdb \
    # Nettoyage
    && rm -rf /var/lib/apt/lists/*

# Configurer les compilateurs par défaut
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100

# Variables d'environnement pour le build
ENV CC=gcc-11
ENV CXX=g++-11
ENV CMAKE_BUILD_TYPE=Release
ENV CMAKE_GENERATOR=Ninja

# Créer répertoire de travail
WORKDIR /build

# Copier les sources
COPY . .

# ====================================================================
# BUILD OPTIMISÉ
# ====================================================================

# Configuration CMake avec optimisations Cloud Run
RUN cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG -march=x86-64 -mtune=generic -flto" \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,--strip-all -Wl,--gc-sections" \
    -DBUILD_TESTS=OFF \
    -B build-release \
    .

# Build parallèle
RUN cmake --build build-release --parallel $(nproc)

# Vérifier que l'exécutable fonctionne
RUN ./build-release/biomining_server --help

# ====================================================================
# STAGE 2: RUNTIME ENVIRONMENT (DISTROLESS)
# ====================================================================

FROM gcr.io/distroless/cc-debian11:latest AS runtime

# Métadonnées du conteneur
LABEL org.opencontainers.image.title="BioMining Platform"
LABEL org.opencontainers.image.description="High-Performance C++ Application for Bio-Crypto Mining"
LABEL org.opencontainers.image.version="1.0.0"
LABEL org.opencontainers.image.vendor="BioMining Team"
LABEL org.opencontainers.image.licenses="MIT"

# Variables d'environnement Cloud Run
ENV PORT=8080
ENV HOST=0.0.0.0
ENV BIOMINING_LOG_LEVEL=info
ENV BIOMINING_CONFIG_PATH=/config/biomining_config.json

# Créer les répertoires nécessaires
WORKDIR /app

# Copier l'exécutable optimisé depuis le builder
COPY --from=builder /build/build-release/biomining_server /app/

# Copier les fichiers de configuration
COPY --from=builder /build/config/ /config/

# Créer répertoires pour les données
RUN mkdir -p /data /logs

# Exposer le port Cloud Run
EXPOSE ${PORT}

# Health check pour Cloud Run
HEALTHCHECK --interval=30s --timeout=10s --start-period=30s --retries=3 \
    CMD curl -f http://localhost:${PORT}/health || exit 1

# Point d'entrée avec configuration flexible
ENTRYPOINT ["/app/biomining_server"]
CMD ["--config", "/config/biomining_config.json"]

# ====================================================================
# METADATA ET ANNOTATIONS
# ====================================================================

# Annotations Cloud Run recommandées
LABEL run.googleapis.com/cpu-throttling="false"
LABEL run.googleapis.com/execution-environment="gen2"
LABEL run.googleapis.com/network-interfaces="[{\"name\":\"default\",\"subnetwork\":\"default\"}]"

# Optimisations spécifiques
ENV MALLOC_ARENA_MAX=2
ENV MALLOC_MMAP_THRESHOLD_=131072
ENV MALLOC_TRIM_THRESHOLD_=131072
ENV MALLOC_TOP_PAD_=131072

# Variables pour debugging (désactivées en production)
ENV BIOMINING_DEBUG=0
ENV BIOMINING_VERBOSE=0

# ====================================================================
# ALTERNATIVE: BUILD STAGE AVEC UBUNTU POUR DEBUGGING
# ====================================================================

FROM ubuntu:22.04 AS debug-runtime

# Installation minimale pour le runtime
RUN apt-get update && apt-get install -y \
    libssl3 \
    libstdc++6 \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Copier l'application
WORKDIR /app
COPY --from=builder /build/build-release/biomining_server /app/
COPY --from=builder /build/config/ /config/

# Variables d'environnement
ENV PORT=8080
ENV BIOMINING_CONFIG_PATH=/config/biomining_config.json

EXPOSE ${PORT}

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=30s --retries=3 \
    CMD curl -f http://localhost:${PORT}/health || exit 1

ENTRYPOINT ["/app/biomining_server"]
CMD ["--config", "/config/biomining_config.json"]