# Makefile pour la Plateforme Bio-Mining Bitcoin
# Simplifie les tâches de build communes

# Variables
BUILD_DIR = build
INSTALL_PREFIX = /usr/local
CMAKE_BUILD_TYPE = Release
THREADS = $(shell nproc)

# Détection du système
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
endif
ifeq ($(UNAME_S),Darwin)
    PLATFORM = macos
endif

# Couleurs pour l'affichage
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

.PHONY: all build clean install uninstall test benchmark docs help setup-deps check-deps

# Target par défaut
all: build

# Affichage de l'aide
help:
	@echo "$(BLUE)Plateforme Hybride Bio-Mining Bitcoin - Makefile$(NC)"
	@echo ""
	@echo "$(YELLOW)Targets disponibles:$(NC)"
	@echo "  $(GREEN)build$(NC)          - Compilation complète du projet"
	@echo "  $(GREEN)build-debug$(NC)    - Compilation en mode debug"
	@echo "  $(GREEN)build-release$(NC)  - Compilation en mode release (défaut)"
	@echo "  $(GREEN)build-gui$(NC)      - Compilation interface graphique seulement"
	@echo "  $(GREEN)build-cli$(NC)      - Compilation interface CLI seulement"
	@echo "  $(GREEN)build-tests$(NC)    - Compilation des tests unitaires"
	@echo ""
	@echo "  $(GREEN)test$(NC)           - Exécution des tests unitaires"
	@echo "  $(GREEN)test-mea$(NC)       - Tests de l'interface MEA seulement"
	@echo "  $(GREEN)test-mining$(NC)    - Tests du moteur de mining seulement"
	@echo "  $(GREEN)test-integration$(NC) - Tests d'intégration seulement"
	@echo ""
	@echo "  $(GREEN)benchmark$(NC)      - Exécution des benchmarks de performance"
	@echo "  $(GREEN)install$(NC)        - Installation du système"
	@echo "  $(GREEN)uninstall$(NC)      - Désinstallation du système"
	@echo ""
	@echo "  $(GREEN)docs$(NC)           - Génération de la documentation"
	@echo "  $(GREEN)package$(NC)        - Création des packages d'installation"
	@echo "  $(GREEN)clean$(NC)          - Nettoyage des fichiers de build"
	@echo "  $(GREEN)distclean$(NC)      - Nettoyage complet"
	@echo ""
	@echo "  $(GREEN)setup-deps$(NC)     - Installation des dépendances (Ubuntu/Debian)"
	@echo "  $(GREEN)check-deps$(NC)     - Vérification des dépendances"
	@echo "  $(GREEN)run-gui$(NC)        - Lancement de l'interface graphique"
	@echo "  $(GREEN)run-cli$(NC)        - Lancement de l'interface CLI"
	@echo ""
	@echo "$(YELLOW)Variables d'environnement:$(NC)"
	@echo "  BUILD_TYPE={Debug|Release}     - Type de build (défaut: Release)"
	@echo "  INSTALL_PREFIX=<path>          - Préfixe d'installation (défaut: /usr/local)"
	@echo "  ENABLE_TESTS={ON|OFF}          - Compilation des tests (défaut: ON)"
	@echo "  ENABLE_QUANTUM={ON|OFF}        - Optimisations quantiques (défaut: OFF)"
	@echo ""
	@echo "$(YELLOW)Examples:$(NC)"
	@echo "  make build BUILD_TYPE=Debug"
	@echo "  make install INSTALL_PREFIX=/opt/biomining"
	@echo "  make test"

# Installation des dépendances (Ubuntu/Debian)
setup-deps:
	@echo "$(YELLOW)Installation des dépendances pour $(PLATFORM)...$(NC)"
ifeq ($(PLATFORM),linux)
	sudo apt-get update
	sudo apt-get install -y \
		build-essential \
		cmake \
		qt6-base-dev \
		qt6-charts-dev \
		libssl-dev \
		libboost-all-dev \
		pkg-config \
		doxygen \
		graphviz \
		cppcheck \
		valgrind
	@echo "$(GREEN)Dépendances installées avec succès!$(NC)"
else
	@echo "$(RED)Installation automatique des dépendances non supportée pour $(PLATFORM)$(NC)"
	@echo "Veuillez installer manuellement: Qt6, OpenSSL, Boost, CMake"
endif

# Vérification des dépendances
check-deps:
	@echo "$(YELLOW)Vérification des dépendances...$(NC)"
	@command -v cmake >/dev/null 2>&1 || { echo "$(RED)CMake non trouvé$(NC)"; exit 1; }
	@command -v qmake6 >/dev/null 2>&1 || command -v qmake >/dev/null 2>&1 || { echo "$(RED)Qt6 non trouvé$(NC)"; exit 1; }
	@pkg-config --exists openssl || { echo "$(RED)OpenSSL non trouvé$(NC)"; exit 1; }
	@echo "$(GREEN)Toutes les dépendances sont présentes!$(NC)"

# Configuration CMake de base
configure:
	@echo "$(YELLOW)Configuration du projet...$(NC)"
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DBUILD_TESTS=${ENABLE_TESTS:-ON} \
		-DBUILD_EXAMPLES=${ENABLE_EXAMPLES:-ON} \
		-DENABLE_QUANTUM_OPTIMIZATION=${ENABLE_QUANTUM:-OFF}

# Build principal
build: check-deps configure
	@echo "$(YELLOW)Compilation du projet ($(CMAKE_BUILD_TYPE))...$(NC)"
	cd $(BUILD_DIR) && make -j$(THREADS)
	@echo "$(GREEN)Compilation terminée avec succès!$(NC)"

# Build debug
build-debug:
	$(MAKE) build CMAKE_BUILD_TYPE=Debug

# Build release  
build-release:
	$(MAKE) build CMAKE_BUILD_TYPE=Release

# Build GUI seulement
build-gui: configure
	@echo "$(YELLOW)Compilation interface graphique...$(NC)"
	cd $(BUILD_DIR) && make biomining_gui -j$(THREADS)
	@echo "$(GREEN)Interface graphique compilée!$(NC)"

# Build CLI seulement
build-cli: configure
	@echo "$(YELLOW)Compilation interface CLI...$(NC)"
	cd $(BUILD_DIR) && make biomining_cli -j$(THREADS)
	@echo "$(GREEN)Interface CLI compilée!$(NC)"

# Build tests
build-tests: configure
	@echo "$(YELLOW)Compilation des tests...$(NC)"
	cd $(BUILD_DIR) && make -j$(THREADS) test_mea_interface test_bitcoin_miner test_integration
	@echo "$(GREEN)Tests compilés!$(NC)"

# Exécution des tests
test: build-tests
	@echo "$(YELLOW)Exécution des tests unitaires...$(NC)"
	cd $(BUILD_DIR) && ctest --output-on-failure --parallel $(THREADS)
	@echo "$(GREEN)Tests terminés!$(NC)"

# Tests spécifiques
test-mea: build-tests
	@echo "$(YELLOW)Tests interface MEA...$(NC)"
	cd $(BUILD_DIR) && ./test_mea_interface

test-mining: build-tests  
	@echo "$(YELLOW)Tests moteur de mining...$(NC)"
	cd $(BUILD_DIR) && ./test_bitcoin_miner

test-integration: build-tests
	@echo "$(YELLOW)Tests d'intégration...$(NC)"
	cd $(BUILD_DIR) && ./test_integration

# Benchmark de performance
benchmark: build
	@echo "$(YELLOW)Exécution des benchmarks...$(NC)"
	cd $(BUILD_DIR) && ./bin/biomining_cli --benchmark --iterations 20
	@echo "$(GREEN)Benchmarks terminés!$(NC)"

# Installation
install: build
	@echo "$(YELLOW)Installation dans $(INSTALL_PREFIX)...$(NC)"
	cd $(BUILD_DIR) && sudo make install
	@echo "$(GREEN)Installation terminée!$(NC)"

# Désinstallation
uninstall:
	@echo "$(YELLOW)Désinstallation...$(NC)"
	cd $(BUILD_DIR) && sudo make uninstall 2>/dev/null || echo "$(YELLOW)Pas de target uninstall disponible$(NC)"
	@echo "$(GREEN)Désinstallation terminée!$(NC)"

# Documentation
docs:
	@echo "$(YELLOW)Génération de la documentation...$(NC)"
ifeq (,$(wildcard $(BUILD_DIR)))
	$(MAKE) configure
endif
	cd $(BUILD_DIR) && make docs
	@echo "$(GREEN)Documentation générée dans $(BUILD_DIR)/docs/$(NC)"

# Package
package: build
	@echo "$(YELLOW)Création des packages...$(NC)"
	cd $(BUILD_DIR) && make package
	@echo "$(GREEN)Packages créés dans $(BUILD_DIR)/$(NC)"

# Nettoyage
clean:
	@echo "$(YELLOW)Nettoyage des fichiers de build...$(NC)"
	rm -rf $(BUILD_DIR)
	@echo "$(GREEN)Nettoyage terminé!$(NC)"

distclean: clean
	@echo "$(YELLOW)Nettoyage complet...$(NC)"
	find . -name "*.user" -delete
	find . -name ".DS_Store" -delete
	find . -name "*.autosave" -delete
	rm -f *.json *.log *.tmp
	@echo "$(GREEN)Nettoyage complet terminé!$(NC)"

# Lancement des applications
run-gui: build-gui
	@echo "$(YELLOW)Lancement de l'interface graphique...$(NC)"
	cd $(BUILD_DIR) && ./bin/biomining_gui

run-cli: build-cli
	@echo "$(YELLOW)Lancement de l'interface CLI...$(NC)"
	cd $(BUILD_DIR) && ./bin/biomining_cli --help

run-cli-demo: build-cli
	@echo "$(YELLOW)Démonstration CLI...$(NC)"
	cd $(BUILD_DIR) && ./bin/biomining_cli --verbose --iterations 3

# Analyse statique du code
check: configure
	@echo "$(YELLOW)Analyse statique du code...$(NC)"
	cd $(BUILD_DIR) && make check
	@echo "$(GREEN)Analyse terminée!$(NC)"

# Formatage du code
format:
	@echo "$(YELLOW)Formatage du code...$(NC)"
	find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i -style=file
	@echo "$(GREEN)Code formaté!$(NC)"

# Tests mémoire (avec Valgrind)
test-memory: build-tests
ifeq ($(PLATFORM),linux)
	@echo "$(YELLOW)Tests de mémoire avec Valgrind...$(NC)"
	cd $(BUILD_DIR) && valgrind --leak-check=full --error-exitcode=1 ./test_mea_interface
	cd $(BUILD_DIR) && valgrind --leak-check=full --error-exitcode=1 ./test_bitcoin_miner
	@echo "$(GREEN)Tests mémoire terminés!$(NC)"
else
	@echo "$(YELLOW)Tests mémoire disponibles seulement sur Linux$(NC)"
endif

# Profiling de performance
profile: build
ifeq ($(PLATFORM),linux)
	@echo "$(YELLOW)Profiling de performance...$(NC)"
	cd $(BUILD_DIR) && perf record -g ./bin/biomining_cli --benchmark --iterations 10
	cd $(BUILD_DIR) && perf report
else
	@echo "$(YELLOW)Profiling disponible seulement sur Linux$(NC)"
endif

# Development helpers
dev-setup: setup-deps
	@echo "$(YELLOW)Configuration environnement de développement...$(NC)"
	git config --local core.autocrlf false
	git config --local pull.rebase true
	@echo "$(GREEN)Environnement de développement configuré!$(NC)"

# Quick build and test cycle
quick-test: 
	$(MAKE) build-tests CMAKE_BUILD_TYPE=Debug
	$(MAKE) test

# Targets pour CI/CD
ci-build:
	$(MAKE) check-deps
	$(MAKE) build CMAKE_BUILD_TYPE=Release ENABLE_TESTS=ON
	$(MAKE) test
	$(MAKE) package

# Docker helpers (si Docker est disponible)
docker-build:
	@echo "$(YELLOW)Build Docker...$(NC)"
	docker build -t biomining-platform .

docker-run:
	@echo "$(YELLOW)Lancement container Docker...$(NC)"
	docker run -it --rm biomining-platform

# Status du projet
status:
	@echo "$(BLUE)=== STATUS PROJET BIO-MINING ===$(NC)"
	@echo "Platform: $(PLATFORM)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Install prefix: $(INSTALL_PREFIX)"
	@echo "Threads available: $(THREADS)"
	@echo ""
	@echo "$(YELLOW)Fichiers du projet:$(NC)"
	@find src include -name "*.cpp" -o -name "*.h" | wc -l | sed 's/^/  Code files: /'
	@find tests -name "*.cpp" 2>/dev/null | wc -l | sed 's/^/  Test files: /'
	@find examples -name "*.cpp" 2>/dev/null | wc -l | sed 's/^/  Example files: /'
	@echo ""
	@echo "$(YELLOW)Build status:$(NC)"
	@if [ -d "$(BUILD_DIR)" ]; then echo "  Build dir: EXISTS"; else echo "  Build dir: NOT FOUND"; fi
	@if [ -f "$(BUILD_DIR)/bin/biomining_gui" ]; then echo "  GUI binary: EXISTS"; else echo "  GUI binary: NOT FOUND"; fi
	@if [ -f "$(BUILD_DIR)/bin/biomining_cli" ]; then echo "  CLI binary: EXISTS"; else echo "  CLI binary: NOT FOUND"; fi