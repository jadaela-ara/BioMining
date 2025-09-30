#!/bin/bash

# 🔌 Script de Configuration pour Dispositifs MEA Réels
# Configure l'environnement pour l'utilisation d'interfaces MEA hardware

set -e

echo "🔌 Configuration des Dispositifs MEA - BioMining Platform"
echo "======================================================="
echo ""

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Fonctions utilitaires
print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Fonction pour détecter le système d'exploitation
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Configuration des permissions Linux
setup_linux_permissions() {
    print_info "Configuration des permissions Linux..."
    
    # Ajouter l'utilisateur au groupe dialout pour accès série
    if groups "$USER" | grep -q "dialout"; then
        print_success "Utilisateur déjà dans le groupe dialout"
    else
        print_info "Ajout de l'utilisateur au groupe dialout..."
        sudo usermod -a -G dialout "$USER"
        print_warning "Déconnectez-vous et reconnectez-vous pour que les changements prennent effet"
    fi
    
    # Vérifier les règles udev pour les dispositifs MEA
    UDEV_RULES_FILE="/etc/udev/rules.d/99-mea-devices.rules"
    
    if [ ! -f "$UDEV_RULES_FILE" ]; then
        print_info "Création des règles udev pour dispositifs MEA..."
        
        sudo tee "$UDEV_RULES_FILE" > /dev/null << 'EOF'
# Règles udev pour dispositifs MEA - BioMining Platform
# Multi Channel Systems
SUBSYSTEM=="usb", ATTR{idVendor}=="04b4", ATTR{idProduct}=="8613", MODE="0666", GROUP="dialout"

# Intan Technologies
SUBSYSTEM=="usb", ATTR{idVendor}=="04d8", ATTR{idProduct}=="f2c4", MODE="0666", GROUP="dialout"

# Arduino/CH340 (pour MEA personnalisés)
SUBSYSTEM=="usb", ATTR{idVendor}=="1a86", ATTR{idProduct}=="7523", MODE="0666", GROUP="dialout"

# FTDI (pour divers adaptateurs série)
SUBSYSTEM=="usb", ATTR{idVendor}=="0403", ATTR{idProduct}=="6001", MODE="0666", GROUP="dialout"
SUBSYSTEM=="usb", ATTR{idVendor}=="0403", ATTR{idProduct}=="6014", MODE="0666", GROUP="dialout"

# Règles génériques pour ports série
SUBSYSTEM=="tty", ATTRS{idVendor}=="*", MODE="0666", GROUP="dialout"
EOF
        
        # Recharger les règles udev
        sudo udevadm control --reload-rules
        sudo udevadm trigger
        
        print_success "Règles udev créées et rechargées"
    else
        print_success "Règles udev déjà configurées"
    fi
}

# Détection des dispositifs série
detect_serial_devices() {
    print_info "Détection des dispositifs série disponibles..."
    
    local os=$(detect_os)
    local devices=()
    
    case $os in
        "linux")
            # Rechercher les ports série Linux
            for port in /dev/ttyUSB* /dev/ttyACM* /dev/ttyS*; do
                if [ -e "$port" ]; then
                    devices+=("$port")
                fi
            done
            ;;
        "macos")
            # Rechercher les ports série macOS
            for port in /dev/cu.* /dev/tty.*; do
                if [[ -e "$port" && "$port" != *"Bluetooth"* ]]; then
                    devices+=("$port")
                fi
            done
            ;;
        "windows")
            # Pour Windows (dans Git Bash/MSYS2)
            for port in /dev/ttyS*; do
                if [ -e "$port" ]; then
                    devices+=("$port")
                fi
            done
            ;;
    esac
    
    if [ ${#devices[@]} -eq 0 ]; then
        print_warning "Aucun dispositif série détecté"
        print_info "Connectez votre dispositif MEA et relancez ce script"
    else
        print_success "${#devices[@]} dispositif(s) série détecté(s):"
        for device in "${devices[@]}"; do
            # Obtenir des informations sur le dispositif si possible
            if command -v lsusb >/dev/null && [[ "$device" == *"USB"* ]]; then
                local usb_info=$(lsusb 2>/dev/null | grep -i "serial\|ftdi\|arduino\|ch340" | head -1)
                echo "    📱 $device ${usb_info:+- $usb_info}"
            else
                echo "    📱 $device"
            fi
        done
    fi
    
    echo ""
}

# Test de communication série
test_serial_communication() {
    print_info "Test de communication série..."
    
    local test_ports=("/dev/ttyUSB0" "/dev/ttyACM0" "/dev/cu.usbserial" "COM1")
    local found_working=false
    
    for port in "${test_ports[@]}"; do
        if [ -e "$port" ]; then
            print_info "Test de $port..."
            
            # Test simple d'ouverture du port
            if command -v python3 >/dev/null; then
                # Test avec Python si disponible
                if python3 -c "
import serial
import sys
try:
    ser = serial.Serial('$port', 9600, timeout=1)
    ser.close()
    print('✅ Port $port accessible')
    sys.exit(0)
except Exception as e:
    print('❌ Port $port: ', str(e))
    sys.exit(1)
" 2>/dev/null; then
                    found_working=true
                fi
            else
                # Test basique avec stty si disponible
                if command -v stty >/dev/null && stty -F "$port" 9600 2>/dev/null; then
                    print_success "Port $port accessible"
                    found_working=true
                fi
            fi
        fi
    done
    
    if [ "$found_working" = true ]; then
        print_success "Au moins un port série fonctionne correctement"
    else
        print_warning "Aucun port série testé avec succès"
        print_info "Cela peut être normal si aucun dispositif n'est connecté"
    fi
    
    echo ""
}

# Installation des dépendances Python pour les tests
install_python_dependencies() {
    print_info "Vérification des dépendances Python pour les tests..."
    
    if command -v python3 >/dev/null; then
        # Vérifier si pyserial est installé
        if python3 -c "import serial" 2>/dev/null; then
            print_success "PySerial déjà installé"
        else
            print_info "Installation de PySerial..."
            if pip3 install pyserial 2>/dev/null || pip install pyserial 2>/dev/null; then
                print_success "PySerial installé avec succès"
            else
                print_warning "Impossible d'installer PySerial automatiquement"
                print_info "Installez manuellement: pip3 install pyserial"
            fi
        fi
    else
        print_warning "Python3 non trouvé - certains tests seront limités"
    fi
    
    echo ""
}

# Génération d'un fichier de configuration de test
generate_test_config() {
    print_info "Génération du fichier de configuration de test..."
    
    local config_file="mea_test_config.json"
    
    cat > "$config_file" << 'EOF'
{
  "test_configuration": {
    "device_type": "Custom_Serial",
    "protocol": "SerialPort",
    "device_path": "/dev/ttyUSB0",
    "baud_rate": 115200,
    "electrode_count": 16,
    "sampling_rate": 10000.0,
    "test_mode": true,
    "debug_logging": true
  },
  
  "alternative_devices": [
    {
      "device_path": "/dev/ttyACM0",
      "description": "Arduino-compatible device"
    },
    {
      "device_path": "/dev/cu.usbserial",
      "description": "macOS USB Serial"
    },
    {
      "device_path": "COM1",
      "description": "Windows COM port"
    }
  ],
  
  "test_commands": {
    "ping": "PING\\n",
    "get_info": "INFO\\n",
    "start_acquisition": "START\\n",
    "stop_acquisition": "STOP\\n",
    "read_data": "READ\\n"
  }
}
EOF
    
    print_success "Configuration de test créée: $config_file"
    echo ""
}

# Affichage des informations de démarrage rapide
show_quick_start() {
    print_info "🚀 Guide de démarrage rapide:"
    echo ""
    echo "1. 📋 Compiler le projet avec support MEA réel:"
    echo "   mkdir -p build && cd build"
    echo "   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON"
    echo "   make example_real_mea"
    echo ""
    echo "2. 🔌 Connecter votre dispositif MEA:"
    echo "   - Vérifiez que le dispositif est sous tension"
    echo "   - Connectez via USB ou port série"
    echo "   - Notez le chemin du dispositif (ex: /dev/ttyUSB0)"
    echo ""
    echo "3. ⚙️  Configurer l'interface dans le code:"
    echo "   - Éditez real_mea_example.cpp"
    echo "   - Ajustez config.devicePath au bon port"
    echo "   - Ajustez config.baudRate si nécessaire"
    echo ""
    echo "4. 🧪 Tester l'interface:"
    echo "   ./bin/example_real_mea"
    echo ""
    echo "5. 📖 Consultez la documentation:"
    echo "   - config/mea_devices.json : Configuration des dispositifs"
    echo "   - include/bio/real_mea_interface.h : API complète"
    echo ""
}

# Fonction principale
main() {
    local os=$(detect_os)
    
    print_info "Système détecté: $os"
    echo ""
    
    # Configuration spécifique au système
    case $os in
        "linux")
            setup_linux_permissions
            ;;
        "macos")
            print_info "Configuration macOS..."
            print_info "Assurez-vous d'avoir installé les pilotes USB appropriés"
            ;;
        "windows")
            print_info "Configuration Windows..."
            print_info "Assurez-vous d'avoir installé les pilotes série appropriés"
            ;;
        *)
            print_warning "Système non reconnu - configuration manuelle requise"
            ;;
    esac
    
    # Détection et tests
    detect_serial_devices
    install_python_dependencies
    test_serial_communication
    generate_test_config
    
    # Guide de démarrage
    show_quick_start
    
    print_success "Configuration terminée!"
    print_info "Redémarrez votre session si des permissions ont été modifiées"
}

# Point d'entrée
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi