#!/bin/bash

# Script de déploiement automatisé pour la Plateforme Bio-Mining Bitcoin
# Usage: ./deploy.sh [environment] [options]

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERSION=$(cat "$PROJECT_ROOT/VERSION" 2>/dev/null || echo "1.0.0")
BUILD_ID=$(date +"%Y%m%d-%H%M%S")

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Fonctions utilitaires
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Affichage de l'aide
show_help() {
    cat << EOF
Script de Déploiement - Plateforme Bio-Mining Bitcoin v$VERSION

USAGE:
    $0 [ENVIRONMENT] [OPTIONS]

ENVIRONMENTS:
    development     Déploiement développement local
    testing         Déploiement pour tests automatisés
    staging         Déploiement staging/pré-production
    production      Déploiement production
    docker          Déploiement containerisé Docker

OPTIONS:
    --build-only        Compilation seulement (pas d'installation)
    --skip-tests        Ignorer l'exécution des tests
    --force            Forcer la réinstallation
    --clean            Nettoyage complet avant build
    --backup           Créer une sauvegarde avant déploiement
    --docker-push      Push des images Docker vers registry
    --help             Afficher cette aide

EXAMPLES:
    $0 development
    $0 production --backup --clean
    $0 docker --docker-push
    $0 testing --skip-tests

VARIABLES D'ENVIRONNEMENT:
    INSTALL_PREFIX     Préfixe d'installation (défaut: /opt/biomining)
    BUILD_THREADS      Nombre de threads de compilation (défaut: nproc)
    DOCKER_REGISTRY    Registry Docker pour push d'images
    BACKUP_DIR         Répertoire de sauvegarde (défaut: ./backups)
EOF
}

# Configuration par défaut
ENVIRONMENT=""
BUILD_ONLY=false
SKIP_TESTS=false
FORCE_INSTALL=false
CLEAN_BUILD=false
CREATE_BACKUP=false
DOCKER_PUSH=false

INSTALL_PREFIX=${INSTALL_PREFIX:-"/opt/biomining"}
BUILD_THREADS=${BUILD_THREADS:-$(nproc)}
DOCKER_REGISTRY=${DOCKER_REGISTRY:-""}
BACKUP_DIR=${BACKUP_DIR:-"$PROJECT_ROOT/backups"}

# Parse des arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        development|testing|staging|production|docker)
            ENVIRONMENT="$1"
            shift
            ;;
        --build-only)
            BUILD_ONLY=true
            shift
            ;;
        --skip-tests)
            SKIP_TESTS=true
            shift
            ;;
        --force)
            FORCE_INSTALL=true
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --backup)
            CREATE_BACKUP=true
            shift
            ;;
        --docker-push)
            DOCKER_PUSH=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            log_error "Option inconnue: $1"
            show_help
            exit 1
            ;;
    esac
done

# Vérification environnement obligatoire
if [[ -z "$ENVIRONMENT" ]]; then
    log_error "Environnement requis. Utilisez --help pour l'aide."
    exit 1
fi

log_info "=== Déploiement Plateforme Bio-Mining v$VERSION ==="
log_info "Environnement: $ENVIRONMENT"
log_info "Build ID: $BUILD_ID"
log_info "Installation: $INSTALL_PREFIX"

# Vérification des prérequis
check_prerequisites() {
    log_info "Vérification des prérequis..."
    
    # Commandes requises
    local required_commands=("cmake" "make" "git")
    
    if [[ "$ENVIRONMENT" == "docker" ]]; then
        required_commands+=("docker" "docker-compose")
    else
        required_commands+=("qmake6" "pkg-config")
    fi
    
    for cmd in "${required_commands[@]}"; do
        if ! command -v "$cmd" &> /dev/null; then
            log_error "Commande requise manquante: $cmd"
            exit 1
        fi
    done
    
    # Vérification Qt6 (hors Docker)
    if [[ "$ENVIRONMENT" != "docker" ]]; then
        if ! pkg-config --exists Qt6Core; then
            log_error "Qt6 non trouvé. Installez avec: sudo apt install qt6-base-dev"
            exit 1
        fi
    fi
    
    log_success "Prérequis vérifiés"
}

# Création de sauvegarde
create_backup() {
    if [[ "$CREATE_BACKUP" == "true" && -d "$INSTALL_PREFIX" ]]; then
        log_info "Création sauvegarde..."
        
        mkdir -p "$BACKUP_DIR"
        local backup_file="$BACKUP_DIR/biomining-backup-$BUILD_ID.tar.gz"
        
        tar -czf "$backup_file" -C "$(dirname "$INSTALL_PREFIX")" "$(basename "$INSTALL_PREFIX")" || {
            log_warning "Sauvegarde échouée, mais on continue..."
        }
        
        if [[ -f "$backup_file" ]]; then
            log_success "Sauvegarde créée: $backup_file"
        fi
    fi
}

# Nettoyage du build
clean_build() {
    if [[ "$CLEAN_BUILD" == "true" ]]; then
        log_info "Nettoyage complet..."
        cd "$PROJECT_ROOT"
        
        rm -rf build/
        find . -name "*.user" -delete 2>/dev/null || true
        find . -name "*.autosave" -delete 2>/dev/null || true
        
        log_success "Nettoyage terminé"
    fi
}

# Configuration CMake par environnement
configure_cmake() {
    log_info "Configuration CMake pour $ENVIRONMENT..."
    
    cd "$PROJECT_ROOT"
    mkdir -p build
    cd build
    
    local cmake_args=()
    cmake_args+=("-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX")
    cmake_args+=("-DBUILD_EXAMPLES=ON")
    
    case "$ENVIRONMENT" in
        development)
            cmake_args+=("-DCMAKE_BUILD_TYPE=Debug")
            cmake_args+=("-DBUILD_TESTS=ON")
            cmake_args+=("-DENABLE_PROFILING=ON")
            ;;
        testing)
            cmake_args+=("-DCMAKE_BUILD_TYPE=Debug")
            cmake_args+=("-DBUILD_TESTS=ON")
            cmake_args+=("-DENABLE_PROFILING=OFF")
            ;;
        staging|production)
            cmake_args+=("-DCMAKE_BUILD_TYPE=Release")
            cmake_args+=("-DBUILD_TESTS=OFF")
            cmake_args+=("-DENABLE_PROFILING=OFF")
            ;;
        docker)
            # Configuration gérée par Dockerfile
            return 0
            ;;
    esac
    
    cmake "${cmake_args[@]}" ..
    log_success "Configuration CMake terminée"
}

# Compilation
build_project() {
    log_info "Compilation du projet..."
    
    if [[ "$ENVIRONMENT" == "docker" ]]; then
        build_docker
        return
    fi
    
    cd "$PROJECT_ROOT/build"
    make -j"$BUILD_THREADS"
    
    log_success "Compilation terminée"
}

# Build Docker
build_docker() {
    log_info "Construction des images Docker..."
    
    cd "$PROJECT_ROOT"
    
    # Build image principale
    docker build -t "biomining-platform:$VERSION" -t "biomining-platform:latest" .
    
    # Build image de développement
    docker build --target development -t "biomining-platform:dev" .
    
    # Build image de test
    docker build --target testing -t "biomining-platform:test" .
    
    # Push vers registry si demandé
    if [[ "$DOCKER_PUSH" == "true" && -n "$DOCKER_REGISTRY" ]]; then
        log_info "Push vers registry Docker..."
        
        docker tag "biomining-platform:$VERSION" "$DOCKER_REGISTRY/biomining-platform:$VERSION"
        docker tag "biomining-platform:latest" "$DOCKER_REGISTRY/biomining-platform:latest"
        
        docker push "$DOCKER_REGISTRY/biomining-platform:$VERSION"
        docker push "$DOCKER_REGISTRY/biomining-platform:latest"
    fi
    
    log_success "Images Docker construites"
}

# Tests
run_tests() {
    if [[ "$SKIP_TESTS" == "true" ]]; then
        log_warning "Tests ignorés sur demande"
        return
    fi
    
    log_info "Exécution des tests..."
    
    if [[ "$ENVIRONMENT" == "docker" ]]; then
        docker run --rm "biomining-platform:test"
    else
        cd "$PROJECT_ROOT/build"
        ctest --output-on-failure --parallel "$BUILD_THREADS"
    fi
    
    log_success "Tests réussis"
}

# Installation
install_project() {
    if [[ "$BUILD_ONLY" == "true" ]]; then
        log_info "Mode build-only, installation ignorée"
        return
    fi
    
    if [[ "$ENVIRONMENT" == "docker" ]]; then
        log_info "Mode Docker, installation via containers"
        return
    fi
    
    log_info "Installation du projet..."
    
    cd "$PROJECT_ROOT/build"
    
    # Installation système ou utilisateur
    if [[ "$INSTALL_PREFIX" == "/usr/local" || "$INSTALL_PREFIX" =~ ^/opt/.* ]]; then
        sudo make install
    else
        make install
    fi
    
    # Configuration des permissions
    if [[ "$ENVIRONMENT" == "production" ]]; then
        configure_production_permissions
    fi
    
    log_success "Installation terminée dans $INSTALL_PREFIX"
}

# Configuration permissions production
configure_production_permissions() {
    log_info "Configuration permissions production..."
    
    # Création utilisateur/groupe biomining
    if ! id biomining &>/dev/null; then
        sudo useradd -r -s /bin/false biomining
    fi
    
    # Permissions sur les répertoires
    sudo mkdir -p /var/log/biomining /var/lib/biomining
    sudo chown -R biomining:biomining /var/log/biomining /var/lib/biomining
    sudo chmod 755 /var/log/biomining /var/lib/biomining
    
    # Service systemd
    create_systemd_service
}

# Création service systemd
create_systemd_service() {
    log_info "Création service systemd..."
    
    sudo tee /etc/systemd/system/biomining.service > /dev/null << EOF
[Unit]
Description=Bio-Mining Bitcoin Platform
After=network.target

[Service]
Type=simple
User=biomining
Group=biomining
ExecStart=$INSTALL_PREFIX/bin/biomining_cli --automatic --config /etc/biomining/production.json
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal
SyslogIdentifier=biomining

[Install]
WantedBy=multi-user.target
EOF
    
    sudo systemctl daemon-reload
    sudo systemctl enable biomining
    
    log_success "Service systemd créé"
}

# Post-installation
post_install() {
    log_info "Configuration post-installation..."
    
    # Création des répertoires de configuration
    case "$ENVIRONMENT" in
        development|testing)
            mkdir -p "$HOME/.config/biomining"
            ;;
        staging|production)
            sudo mkdir -p /etc/biomining
            # Copie des configurations par défaut
            if [[ -d "$PROJECT_ROOT/config" ]]; then
                sudo cp -r "$PROJECT_ROOT/config/"* /etc/biomining/
            fi
            ;;
    esac
    
    log_success "Configuration post-installation terminée"
}

# Fonction principale
main() {
    log_info "Démarrage déploiement $ENVIRONMENT..."
    
    check_prerequisites
    create_backup
    clean_build
    configure_cmake
    build_project
    run_tests
    install_project
    post_install
    
    log_success "=== Déploiement $ENVIRONMENT terminé avec succès! ==="
    
    # Informations finales
    case "$ENVIRONMENT" in
        docker)
            log_info "Utilisez: docker-compose up pour démarrer les services"
            ;;
        development|testing)
            log_info "Binaires disponibles dans: $PROJECT_ROOT/build/bin/"
            ;;
        staging|production)
            log_info "Installation système dans: $INSTALL_PREFIX"
            log_info "Service: sudo systemctl start biomining"
            ;;
    esac
}

# Point d'entrée
main "$@"