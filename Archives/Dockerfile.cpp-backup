# Dockerfile pour la Plateforme Bio-Mining Bitcoin
# Construction multi-stage pour optimiser la taille de l'image finale

# === STAGE 1: Build Environment ===
FROM ubuntu:22.04 AS builder

# Éviter les prompts interactifs
ENV DEBIAN_FRONTEND=noninteractive

# Installation des dépendances de build
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-charts-dev \
    libssl-dev \
    libboost-all-dev \
    pkg-config \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Création de l'utilisateur de build
RUN useradd -m -s /bin/bash builder
WORKDIR /home/builder

# Copie des sources
COPY --chown=builder:builder . /home/builder/biomining

# Switch vers l'utilisateur de build
USER builder

# Configuration et compilation
WORKDIR /home/builder/biomining
RUN mkdir -p build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=ON \
        -DBUILD_EXAMPLES=ON \
        -DENABLE_QUANTUM_OPTIMIZATION=OFF \
        -DCMAKE_INSTALL_PREFIX=/opt/biomining && \
    make -j$(nproc)

# Tests (optionnel - peut être désactivé pour builds plus rapides)
RUN cd build && ctest --output-on-failure || true

# Installation dans le préfixe
USER root
RUN cd /home/builder/biomining/build && make install

# === STAGE 2: Runtime Environment ===
FROM ubuntu:22.04 AS runtime

# Éviter les prompts interactifs
ENV DEBIAN_FRONTEND=noninteractive

# Installation des dépendances runtime seulement
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-charts-dev \
    libssl3 \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-thread1.74.0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Création de l'utilisateur runtime
RUN useradd -m -s /bin/bash biomining && \
    mkdir -p /opt/biomining && \
    chown -R biomining:biomining /opt/biomining

# Copie des binaires depuis le stage builder
COPY --from=builder /opt/biomining /opt/biomining

# Configuration de l'environnement
ENV PATH="/opt/biomining/bin:${PATH}"
ENV LD_LIBRARY_PATH="/opt/biomining/lib:${LD_LIBRARY_PATH}"

# Création des répertoires de données
RUN mkdir -p /data/biomining/{config,logs,calibration,results} && \
    chown -R biomining:biomining /data/biomining

# Switch vers l'utilisateur runtime
USER biomining
WORKDIR /data/biomining

# Configuration par défaut
COPY --chown=biomining:biomining config/docker_default.json /data/biomining/config/default.json

# Labels de métadonnées
LABEL org.opencontainers.image.title="Bio-Mining Bitcoin Platform"
LABEL org.opencontainers.image.description="Plateforme hybride bio-informatique pour mining Bitcoin"
LABEL org.opencontainers.image.version="1.0.0"
LABEL org.opencontainers.image.vendor="BioLab Research"
LABEL maintainer="research@biolab.example.com"

# Ports exposés (pour future interface web)
EXPOSE 8080 8443

# Volumes pour persistance
VOLUME ["/data/biomining/config", "/data/biomining/logs", "/data/biomining/results"]

# Point d'entrée par défaut
ENTRYPOINT ["biomining_cli"]
CMD ["--help"]

# === STAGE 3: Development Environment (optionnel) ===
FROM builder AS development

USER root

# Installation d'outils de développement additionnels
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    cppcheck \
    doxygen \
    graphviz \
    vim \
    nano \
    htop \
    strace \
    && rm -rf /var/lib/apt/lists/*

USER builder
WORKDIR /home/builder/biomining

# Compilation en mode debug pour développement
RUN cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
        -DBUILD_EXAMPLES=ON \
        -DENABLE_PROFILING=ON && \
    make -j$(nproc)

# Point d'entrée développement
ENTRYPOINT ["/bin/bash"]

# === STAGE 4: Testing Environment ===
FROM runtime AS testing

USER root

# Installation d'outils de test
RUN apt-get update && apt-get install -y \
    valgrind \
    gdb \
    strace \
    && rm -rf /var/lib/apt/lists/*

# Copie des binaires de test depuis builder
COPY --from=builder /home/builder/biomining/build/test_* /opt/biomining/bin/
COPY --from=builder /home/builder/biomining/build/bin/examples/ /opt/biomining/bin/examples/

USER biomining

# Script de test automatisé
COPY --chown=biomining:biomining scripts/docker_test.sh /opt/biomining/bin/run_tests.sh
RUN chmod +x /opt/biomining/bin/run_tests.sh

ENTRYPOINT ["/opt/biomining/bin/run_tests.sh"]