# 🧬 Guide Hardware MEA - BioMining Platform

## 📖 Introduction

Ce guide détaille la configuration et l'utilisation des dispositifs Multi-Electrode Array (MEA) hardware avec la plateforme BioMining.

## 📋 Table des Matières

1. [Dispositifs MEA Supportés](#dispositifs-mea-supportés)
2. [Configuration Hardware](#configuration-hardware)  
3. [Protocoles de Communication](#protocoles-de-communication)
4. [Calibration et Tests](#calibration-et-tests)
5. [Optimisation Performance](#optimisation-performance)
6. [Troubleshooting Hardware](#troubleshooting-hardware)
7. [Spécifications Techniques](#spécifications-techniques)

---

## 🏭 Dispositifs MEA Supportés

### Fabricants Professionnels

#### Multi Channel Systems (MCS)
**Status**: ✅ Support Complet | **Priorité**: Référence

| Modèle | Électrodes | Protocole | Fréq. Max | Notes |
|--------|------------|-----------|-----------|-------|
| **MEA2100** | 60 | TCP/IP, Serial | 50 kHz | Système de référence |
| **MEA1060** | 60 | TCP/IP | 25 kHz | Version réseau uniquement |
| **USB-MEA** | 60 | USB-Serial | 20 kHz | Compact, portable |
| **MEA-IT** | 252 | TCP/IP | 25 kHz | Haute densité |

**Configuration Exemple MCS MEA2100**:
```json
{
  "device_type": "MultiChannelSystems_MCS",
  "model": "MEA2100",
  "connection": {
    "protocol": "TCP",
    "host": "192.168.1.100",
    "port": 3333,
    "timeout_ms": 5000
  },
  "acquisition": {
    "sampling_rate": 25000,
    "electrode_count": 60,
    "amplification": 1200,
    "filter_low_cut": 300,
    "filter_high_cut": 8000
  },
  "stimulation": {
    "max_voltage": 5.0,
    "max_current_ua": 100,
    "biphasic": true
  }
}
```

#### Blackrock Microsystems  
**Status**: ✅ Support Complet | **Priorité**: Production

| Modèle | Électrodes | Protocole | Fréq. Max | Notes |
|--------|------------|-----------|-----------|-------|
| **CerePlex Direct** | 96-128 | USB 3.0 | 30 kHz | Haute performance |
| **CerePlex W** | 96 | WiFi, USB | 30 kHz | Sans fil |
| **CerePlex E** | 96 | Ethernet | 30 kHz | Réseau dédié |

**Configuration Exemple Blackrock**:
```json
{
  "device_type": "Blackrock_CerePlex",
  "model": "CerePlex_Direct",
  "connection": {
    "protocol": "Serial",
    "device_path": "/dev/ttyUSB0",
    "baud_rate": 921600,
    "flow_control": "hardware"
  },
  "acquisition": {
    "sampling_rate": 30000,
    "electrode_count": 96,
    "amplification": 5000,
    "reference_electrode": "global"
  },
  "advanced": {
    "digital_filtering": true,
    "spike_sorting": false,
    "real_time_processing": true
  }
}
```

#### Plexon Inc.
**Status**: ✅ Support Complet | **Priorité**: Recherche

| Modèle | Électrodes | Protocole | Fréq. Max | Notes |
|--------|------------|-----------|-----------|-------|
| **OmniPlex D** | 256+ | Ethernet | 40 kHz | Système modulaire |
| **DigiAmp** | 128 | USB, Ethernet | 40 kHz | Amplificateur numérique |

**Configuration Exemple Plexon**:
```json
{
  "device_type": "Plexon_OmniPlex", 
  "model": "OmniPlex_D",
  "connection": {
    "protocol": "UDP",
    "multicast_group": "224.0.0.1",
    "port": 6000,
    "interface": "eth0"
  },
  "acquisition": {
    "sampling_rate": 40000,
    "electrode_count": 128,
    "channels": "auto_detect"
  }
}
```

#### Intan Technologies
**Status**: ✅ Support Complet | **Priorité**: Open Source

| Modèle | Électrodes | Protocole | Fréq. Max | Notes |
|--------|------------|-----------|-----------|-------|
| **RHD2000** | 32-256 | USB | 30 kHz | Open source, modulaire |
| **RHX Controller** | 512+ | USB 3.0 | 30 kHz | Nouvelle génération |

**Configuration Exemple Intan**:
```json
{
  "device_type": "Intan_RHD",
  "model": "RHD2132",
  "connection": {
    "protocol": "Serial", 
    "device_path": "/dev/ttyACM0",
    "baud_rate": 230400
  },
  "acquisition": {
    "sampling_rate": 30000,
    "electrode_count": 32,
    "amplification": 200,
    "dsp_enabled": true,
    "impedance_testing": true
  }
}
```

#### Open Ephys  
**Status**: 🔶 Beta | **Priorité**: Communautaire

| Modèle | Électrodes | Protocole | Fréq. Max | Notes |
|--------|------------|-----------|-----------|-------|
| **ONIX** | Variable | Custom API | 30 kHz | Plateforme ouverte |
| **Acquisition Board** | 64-256 | USB, PCIe | 30 kHz | DIY, personnalisable |

### Dispositifs en Développement

#### Alpha Omega (🔶 Beta)
- **AlphaMap**: Support partiel, en test
- **NeuroOmega**: Intégration prévue v1.2

#### NeuroNexus (🔶 Beta)  
- **SmartBox**: Support basique, amélioration continue
- **Wireless Systems**: Évaluation en cours

---

## 🔧 Configuration Hardware

### Setup Automatique (Recommandé)

#### Script de Configuration Global
```bash
# Configuration automatique complète
sudo ./setup_mea_device.sh --auto-setup

# Configuration par fabricant
sudo ./setup_mea_device.sh --device mcs --auto-detect
sudo ./setup_mea_device.sh --device blackrock --auto-detect
sudo ./setup_mea_device.sh --device plexon --network-scan
sudo ./setup_mea_device.sh --device intan --usb-scan
```

#### Détection Interactive
```bash
# Mode interactif guidé
./bin/mea_device_manager --interactive

# Output exemple:
🔍 MEA Device Detection Wizard
══════════════════════════════════

Step 1: Scanning connection methods...
✅ USB ports: /dev/ttyUSB0, /dev/ttyACM0
✅ Network interfaces: eth0 (192.168.1.0/24)
✅ Available drivers: MCS, Blackrock, Intan

Step 2: Testing detected devices...
🔍 Testing /dev/ttyUSB0...
   ✅ MCS MEA2100 detected (Firmware: v2.3.1)
   📊 60 electrodes, 25kHz capable

🔍 Testing network 192.168.1.100:3333...
   ✅ MCS Network Interface detected
   📊 Same device via network protocol

Select device [1-2]: 1

Step 3: Configuration...
✅ Device configured successfully
✅ Calibration data loaded
✅ Ready for BioMining integration

Launch BioMining with this device? [y/N]: y
```

### Configuration Manuelle par Protocole

#### Configuration Série/USB

##### Multi Channel Systems via Série
```bash
# Permissions utilisateur
sudo usermod -a -G dialout $USER
sudo chmod 666 /dev/ttyUSB0

# Test communication basique
stty -F /dev/ttyUSB0 115200 cs8 -cstopb -parenb
echo "ID?" > /dev/ttyUSB0
cat /dev/ttyUSB0

# Configuration BioMining
./biomining_gui --device-config \
    --type mcs \
    --protocol serial \
    --device /dev/ttyUSB0 \
    --baud 115200 \
    --electrodes 60
```

##### Blackrock via USB High-Speed  
```bash
# Driver spécialisé (si nécessaire)
sudo modprobe ftdi_sio
echo "0403 6014" | sudo tee /sys/bus/usb-serial/drivers/ftdi_sio/new_id

# Configuration haute vitesse
./biomining_gui --device-config \
    --type blackrock \
    --protocol serial \
    --device /dev/ttyUSB0 \
    --baud 921600 \
    --flow-control hardware \
    --electrodes 96
```

#### Configuration Réseau TCP/IP

##### MCS via Ethernet
```bash
# Configuration réseau statique (si nécessaire)  
sudo ip addr add 192.168.1.10/24 dev eth0
sudo ip route add 192.168.1.100 dev eth0

# Test connectivité
ping -c 3 192.168.1.100
telnet 192.168.1.100 3333

# Configuration BioMining
./biomining_gui --device-config \
    --type mcs \
    --protocol tcp \
    --host 192.168.1.100 \
    --port 3333 \
    --timeout 5000
```

##### Plexon via UDP Multicast
```bash
# Configuration multicast
sudo ip maddr add 224.0.0.1 dev eth0
sudo iptables -A INPUT -d 224.0.0.1 -j ACCEPT

# Test réception multicast
socat - UDP4-RECV:6000,bind=224.0.0.1,ip-add-membership=224.0.0.1:eth0

# Configuration BioMining
./biomining_gui --device-config \
    --type plexon \
    --protocol udp \
    --multicast-group 224.0.0.1 \
    --port 6000 \
    --interface eth0
```

---

## 📡 Protocoles de Communication

### Protocole Série (RS232/USB)

#### Caractéristiques
- **Débit**: 9600 - 921600 baud
- **Mode**: 8N1 (8 bits, no parity, 1 stop bit)
- **Flow Control**: None, Hardware (RTS/CTS), Software (XON/XOFF)
- **Latence**: 5-20ms typique
- **Fiabilité**: Haute, détection erreurs

#### Format de Trame Exemple (MCS)
```
Header (4 bytes) | Command (2 bytes) | Length (2 bytes) | Data (N bytes) | CRC (2 bytes)
0xAA 0x55 0x12 0x34 | CMD_READ_DATA | 0x00 0x3C | [60 electrode data] | CRC16
```

#### Implémentation Low-Level
```cpp
// Configuration port série
QSerialPort serial;
serial.setPortName("/dev/ttyUSB0");
serial.setBaudRate(115200);
serial.setDataBits(QSerialPort::Data8);
serial.setParity(QSerialPort::NoParity);  
serial.setStopBits(QSerialPort::OneStop);
serial.setFlowControl(QSerialPort::HardwareControl);

// Communication synchrone
QByteArray command = formatMEACommand(CMD_READ_ELECTRODES);
serial.write(command);
if (serial.waitForBytesWritten(1000)) {
    if (serial.waitForReadyRead(1000)) {
        QByteArray response = serial.readAll();
        parseElectrodeResponse(response);
    }
}
```

### Protocole TCP/IP

#### Caractéristiques  
- **Port standard**: 3333 (MCS), 8080 (custom)
- **Mode**: Connection persistante
- **Débit**: Limité par réseau (1-100 MB/s)
- **Latence**: 1-5ms sur LAN
- **Fiabilité**: Très haute, TCP garantit livraison

#### Format de Paquet Réseau
```
TCP Header | MEA Header | Timestamp (8 bytes) | Electrode Count (4 bytes) | Electrode Data Array
[Standard TCP] | [Device specific] | [μs precision] | [Number of electrodes] | [Voltage, Impedance per electrode]
```

#### Implémentation TCP Client
```cpp
// Client TCP pour MEA
QTcpSocket tcpSocket;
tcpSocket.connectToHost("192.168.1.100", 3333);

if (tcpSocket.waitForConnected(5000)) {
    // Handshake protocol  
    QByteArray handshake = createHandshakePacket();
    tcpSocket.write(handshake);
    
    // Continuous data reception
    connect(&tcpSocket, &QTcpSocket::readyRead, [&]() {
        QByteArray data = tcpSocket.readAll();
        processNetworkData(data);
    });
}
```

### Protocole UDP (Streaming haute performance)

#### Caractéristiques
- **Mode**: Datagrammes sans connexion  
- **Débit**: Très élevé (jusqu'à ligne speed)
- **Latence**: <1ms sur LAN rapide
- **Fiabilité**: Moindre, packets peuvent être perdus
- **Usage**: Streaming temps réel, multicast

#### Réception UDP Multicast
```cpp
// Socket UDP multicast
QUdpSocket udpSocket;
udpSocket.bind(QHostAddress::AnyIPv4, 6000);
udpSocket.joinMulticastGroup(QHostAddress("224.0.0.1"));

// Réception asynchrone
connect(&udpSocket, &QUdpSocket::readyRead, [&]() {
    while (udpSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket.pendingDatagramSize());
        
        QHostAddress sender;
        quint16 senderPort;
        
        udpSocket.readDatagram(datagram.data(), datagram.size(),
                              &sender, &senderPort);
        
        processUDPElectrodeData(datagram, sender);
    }
});
```

---

## 🎯 Calibration et Tests

### Processus de Calibration Complète

#### 1. Test de Connectivité
```cpp
bool RealMEAInterface::performConnectivityTest() {
    qInfo() << "[Calibration] Étape 1: Test de connectivité";
    
    // Test communication basique
    if (!sendPingCommand()) {
        setError("Échec ping device");
        return false;
    }
    
    // Vérification firmware
    QString firmware = queryFirmwareVersion(); 
    if (firmware.isEmpty()) {
        setError("Impossible de lire version firmware");
        return false;
    }
    
    qInfo() << "[Calibration] Device ping OK, firmware:" << firmware;
    return true;
}
```

#### 2. Test d'Impédance des Électrodes
```cpp
bool RealMEAInterface::performImpedanceTest() {
    qInfo() << "[Calibration] Étape 2: Test d'impédance électrodes";
    
    QVector<double> impedances;
    int failedElectrodes = 0;
    
    for (int i = 0; i < config_.electrodeCount; ++i) {
        // Signal de test 1kHz, 10μA  
        double impedance = measureElectrodeImpedance(i, 1000.0, 10e-6);
        impedances.append(impedance);
        
        // Validation limites
        if (impedance > MAX_IMPEDANCE_MOHM * 1e6) {  // > 10MΩ
            qWarning() << QString("[Calibration] Électrode %1: impédance élevée %2 MΩ")
                          .arg(i).arg(impedance / 1e6, 0, 'f', 2);
            failedElectrodes++;
        }
        
        // Progress callback
        emit calibrationProgress(i, double(i) / config_.electrodeCount);
    }
    
    electrodeImpedances_ = impedances;
    
    double successRate = double(config_.electrodeCount - failedElectrodes) / config_.electrodeCount;
    qInfo() << QString("[Calibration] Test impédance: %1% électrodes OK (%2/%3)")
               .arg(successRate * 100, 0, 'f', 1)
               .arg(config_.electrodeCount - failedElectrodes)
               .arg(config_.electrodeCount);
    
    return successRate >= MIN_ELECTRODE_SUCCESS_RATE; // 80% minimum
}

double RealMEAInterface::measureElectrodeImpedance(int electrodeId, double frequency, double current) {
    // Générer signal test sinusoïdal
    QByteArray testSignal = generateTestWaveform(frequency, current, 100); // 100ms
    
    // Appliquer via stimulation
    if (!sendStimulationCommand(electrodeId, testSignal)) {
        return std::numeric_limits<double>::infinity(); // Électrode déconnectée
    }
    
    // Mesurer réponse voltage
    QThread::msleep(50); // Stabilisation
    ElectrodeData response = readSingleElectrode(electrodeId);
    
    // Calculer impédance (Loi d'Ohm: Z = V/I)
    double impedance = std::abs(response.voltage) / current;
    
    qDebug() << QString("[Impedance] E%1: %2V / %3A = %4Ω")
                .arg(electrodeId)
                .arg(response.voltage, 0, 'e', 2) 
                .arg(current, 0, 'e', 2)
                .arg(impedance, 0, 'f', 0);
    
    return impedance;
}
```

#### 3. Calibration des Gains
```cpp
bool RealMEAInterface::performGainCalibration() {
    qInfo() << "[Calibration] Étape 3: Calibration des gains";
    
    // Signal de référence connu
    const double REFERENCE_VOLTAGE = 100e-6; // 100μV
    const double TARGET_ADC_VALUE = 2048;    // Mid-range 12-bit ADC
    
    QVector<double> gainFactors;
    
    for (int i = 0; i < config_.electrodeCount; ++i) {
        if (electrodeImpedances_[i] > MAX_IMPEDANCE_MOHM * 1e6) {
            gainFactors.append(1.0); // Électrode défaillante, gain neutre
            continue;
        }
        
        // Injecter signal de référence
        if (!injectReferenceSignal(i, REFERENCE_VOLTAGE)) {
            gainFactors.append(1.0);
            continue;
        }
        
        // Lire réponse ADC
        QThread::msleep(10); // Temps de stabilisation 
        ElectrodeData response = readSingleElectrode(i);
        
        // Calculer facteur de gain pour atteindre la cible
        double measuredADC = response.voltage * ADC_SCALE_FACTOR;
        double gainFactor = TARGET_ADC_VALUE / measuredADC;
        
        // Limiter gain à plage raisonnable
        gainFactor = qBound(0.1, gainFactor, 10.0);
        gainFactors.append(gainFactor);
        
        qDebug() << QString("[Gain] E%1: mesure=%2, cible=%3, gain=%4")
                    .arg(i).arg(measuredADC, 0, 'f', 1)
                    .arg(TARGET_ADC_VALUE).arg(gainFactor, 0, 'f', 3);
    }
    
    // Appliquer gains au hardware
    return applyGainFactorsToDevice(gainFactors);
}
```

#### 4. Test de Qualité Signal
```cpp
bool RealMEAInterface::performSignalQualityTest() {
    qInfo() << "[Calibration] Étape 4: Test qualité signal";
    
    // Acquisition test de 10 secondes
    const int TEST_DURATION_MS = 10000;
    const int SAMPLES_PER_SEC = config_.samplingRate;
    const int TOTAL_SAMPLES = (TEST_DURATION_MS * SAMPLES_PER_SEC) / 1000;
    
    QVector<QVector<double>> electrodeSignals(config_.electrodeCount);
    
    // Acquisition continue
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    int sampleCount = 0;
    
    while (sampleCount < TOTAL_SAMPLES) {
        QVector<ElectrodeData> data = readElectrodeData();
        
        for (int i = 0; i < data.size() && i < config_.electrodeCount; ++i) {
            electrodeSignals[i].append(data[i].voltage);
        }
        
        sampleCount += data.size();
        
        // Éviter surcharge CPU
        if (sampleCount % 1000 == 0) {
            QCoreApplication::processEvents();
        }
    }
    
    // Analyser qualité de chaque électrode
    QVector<double> qualityScores;
    double avgQuality = 0.0;
    
    for (int i = 0; i < config_.electrodeCount; ++i) {
        double quality = calculateSignalQuality(electrodeSignals[i]);
        qualityScores.append(quality);
        avgQuality += quality;
        
        qDebug() << QString("[Quality] E%1: %2%").arg(i).arg(quality * 100, 0, 'f', 1);
    }
    
    avgQuality /= config_.electrodeCount;
    
    qInfo() << QString("[Calibration] Qualité moyenne: %1%").arg(avgQuality * 100, 0, 'f', 1);
    
    return avgQuality >= MIN_SIGNAL_QUALITY; // 70% minimum
}

double RealMEAInterface::calculateSignalQuality(const QVector<double>& signal) {
    if (signal.isEmpty()) return 0.0;
    
    // Calcul SNR (Signal-to-Noise Ratio)
    double mean = std::accumulate(signal.begin(), signal.end(), 0.0) / signal.size();
    
    double variance = 0.0;
    for (double sample : signal) {
        variance += (sample - mean) * (sample - mean);
    }
    variance /= signal.size();
    
    double stdDev = std::sqrt(variance);
    double snr = std::abs(mean) / stdDev;
    
    // Normaliser SNR à score 0-1
    double quality = std::min(snr / 10.0, 1.0); // SNR > 10 = qualité parfaite
    
    return quality;
}
```

### Tests de Validation Fonctionnelle

#### Test de Stimulation Sécurisée
```cpp
bool RealMEAInterface::performStimulationTest() {
    qInfo() << "[Test] Validation stimulation sécurisée";
    
    // Sélectionner électrode test (première active)
    int testElectrode = -1;
    for (int i = 0; i < config_.electrodeCount; ++i) {
        if (electrodeImpedances_[i] < MAX_IMPEDANCE_MOHM * 1e6) {
            testElectrode = i;
            break;
        }
    }
    
    if (testElectrode == -1) {
        qWarning() << "[Test] Aucune électrode valide pour test stimulation";
        return false;
    }
    
    // Test stimulation minimale
    const double TEST_VOLTAGE = 0.5; // 0.5V seulement
    const double TEST_DURATION = 1.0; // 1ms
    
    // Mesure baseline avant stimulation
    ElectrodeData baseline = readSingleElectrode(testElectrode);
    
    // Stimulation test
    bool stimResult = stimulateElectrode(testElectrode, TEST_VOLTAGE, TEST_DURATION);
    
    if (!stimResult) {
        qWarning() << "[Test] Échec stimulation test";
        return false;
    }
    
    // Attendre et mesurer réponse
    QThread::msleep(50);
    ElectrodeData response = readSingleElectrode(testElectrode);
    
    // Vérifier changement de signal (preuve que stimulation a eu effet)
    double signalChange = std::abs(response.voltage - baseline.voltage);
    bool responseDetected = signalChange > MIN_STIMULATION_RESPONSE; // 5μV minimum
    
    qInfo() << QString("[Test] Stimulation E%1: baseline=%2μV, response=%3μV, change=%4μV")
               .arg(testElectrode)
               .arg(baseline.voltage * 1e6, 0, 'f', 1)
               .arg(response.voltage * 1e6, 0, 'f', 1) 
               .arg(signalChange * 1e6, 0, 'f', 1);
    
    return responseDetected;
}
```

---

## ⚡ Optimisation Performance

### Optimisations Hardware

#### Configuration Buffer Optimal
```cpp
void RealMEAInterface::optimizeBufferConfiguration() {
    // Calculer taille buffer optimale
    int samplesPerSecond = config_.samplingRate * config_.electrodeCount;
    int bytesPerSample = sizeof(ElectrodeData);
    int bytesPerSecond = samplesPerSecond * bytesPerSample;
    
    // Buffer pour 100ms de données (compromis latence/performance)
    int optimalBufferSize = (bytesPerSecond / 10) + 1024; // +1KB safety margin
    
    // Arrondir à multiple de 1024 (page size)
    optimalBufferSize = ((optimalBufferSize / 1024) + 1) * 1024;
    
    qInfo() << QString("[Optimization] Buffer size: %1 KB (%2 samples)")
               .arg(optimalBufferSize / 1024)
               .arg(optimalBufferSize / bytesPerSample);
    
    // Appliquer au hardware si supporté
    if (deviceSupportsDynamicBuffer()) {
        setHardwareBufferSize(optimalBufferSize);
    }
    
    // Configurer buffers application
    dataBuffer_.reserve(optimalBufferSize * 2); // Double buffering
    
    // Thread de lecture avec priorité temps réel
    acquisitionThread_ = new QThread;
    acquisitionWorker_ = new AcquisitionWorker(this);
    acquisitionWorker_->moveToThread(acquisitionThread_);
    
    // Priorité élevée pour acquisition
    acquisitionThread_->setPriority(QThread::TimeCriticalPriority);
    acquisitionThread_->start();
}
```

#### Optimisation Réseau
```cpp
void RealMEAInterface::optimizeNetworkConfiguration() {
    if (config_.protocol != CommunicationProtocol::TCP && 
        config_.protocol != CommunicationProtocol::UDP) {
        return;
    }
    
    // TCP optimizations
    if (tcpSocket_) {
        // Disable Nagle algorithm for low latency
        tcpSocket_->setSocketOption(QAbstractSocket::LowDelayOption, 1);
        
        // Increase socket buffer sizes
        tcpSocket_->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 1024); // 1MB
        tcpSocket_->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 64 * 1024);     // 64KB
        
        // Keep alive for persistent connections
        tcpSocket_->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
        
        qInfo() << "[Optimization] TCP socket optimized for low latency";
    }
    
    // UDP optimizations  
    if (udpSocket_) {
        // Large receive buffer for high throughput
        udpSocket_->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 2048 * 1024); // 2MB
        
        // Bind to specific interface for multicast
        if (!config_.networkInterface.isEmpty()) {
            udpSocket_->bind(QHostAddress::AnyIPv4, config_.networkPort, 
                           QUdpSocket::ReuseAddressHint);
        }
        
        qInfo() << "[Optimization] UDP socket optimized for high throughput";
    }
}
```

### Optimisations Système

#### Configuration CPU et Priorités
```bash
#!/bin/bash
# optimize_system_for_mea.sh

echo "🔧 Optimisation système pour acquisition MEA..."

# CPU Governor performance
echo "performance" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Disable CPU power saving
sudo cpupower frequency-set -g performance

# Real-time scheduling for BioMining process
sudo sysctl -w kernel.sched_rt_runtime_us=-1
sudo sysctl -w kernel.sched_rt_period_us=1000000

# Increase network buffer limits
sudo sysctl -w net.core.rmem_max=16777216
sudo sysctl -w net.core.wmem_max=16777216
sudo sysctl -w net.core.netdev_max_backlog=5000

# Reduce swapping
sudo sysctl -w vm.swappiness=1

# IRQ affinity (bind network IRQ to specific CPU)
# Find network IRQ
NET_IRQ=$(grep eth0 /proc/interrupts | cut -d: -f1 | tr -d ' ')
if [ ! -z "$NET_IRQ" ]; then
    echo "2" | sudo tee /proc/irq/$NET_IRQ/smp_affinity  # CPU 1
    echo "🔧 Network IRQ $NET_IRQ bound to CPU 1"
fi

echo "✅ Optimisation système terminée"
```

#### Monitoring Performance Temps Réel
```cpp
class MEAPerformanceMonitor {
public:
    struct PerformanceMetrics {
        double acquisitionLatency;     // ms
        double processingLatency;      // ms  
        double networkLatency;         // ms
        double cpuUsage;              // %
        double memoryUsage;           // MB
        int packetsLost;              // count
        double dataRate;              // MB/s
    };
    
    void startMonitoring() {
        monitorTimer_ = new QTimer;
        connect(monitorTimer_, &QTimer::timeout, 
                this, &MEAPerformanceMonitor::updateMetrics);
        monitorTimer_->start(1000); // Update every second
    }
    
private slots:
    void updateMetrics() {
        PerformanceMetrics metrics;
        
        // Mesurer latence acquisition
        auto acquisitionStart = std::chrono::high_resolution_clock::now();
        // ... acquisition call ...
        auto acquisitionEnd = std::chrono::high_resolution_clock::now();
        
        metrics.acquisitionLatency = std::chrono::duration<double, std::milli>(
            acquisitionEnd - acquisitionStart).count();
        
        // CPU usage via /proc/stat
        metrics.cpuUsage = getCurrentCPUUsage();
        
        // Memory usage via /proc/self/status  
        metrics.memoryUsage = getCurrentMemoryUsage();
        
        // Network statistics
        metrics.dataRate = calculateDataRate();
        metrics.packetsLost = getPacketLossCount();
        
        // Alertes performance
        if (metrics.acquisitionLatency > 10.0) { // > 10ms
            emit performanceAlert("Acquisition latency high: " + 
                                QString::number(metrics.acquisitionLatency) + "ms");
        }
        
        if (metrics.cpuUsage > 90.0) {
            emit performanceAlert("CPU usage critical: " + 
                                QString::number(metrics.cpuUsage) + "%");
        }
        
        emit metricsUpdated(metrics);
    }
};
```

---

## 🐛 Troubleshooting Hardware

### Diagnostics Automatiques

#### Outil de Diagnostic Complet
```bash
#!/bin/bash
# mea_diagnostics.sh - Diagnostic hardware MEA complet

echo "🔍 BioMining MEA Hardware Diagnostics"
echo "===================================="

# 1. Test connectivité physique
echo "📍 1. Tests de connectivité..."

# USB devices
echo "USB Devices:"
lsusb | grep -E "(MCS|Blackrock|Plexon|Intan)" || echo "  Aucun dispositif MEA USB détecté"

# Serial ports
echo "Serial Ports:"
for port in /dev/ttyUSB* /dev/ttyACM*; do
    if [ -e "$port" ]; then
        echo "  $port - $(udevadm info --name=$port --query=property | grep ID_VENDOR_ID | cut -d'=' -f2)"
    fi
done

# Network connectivity
echo "Network MEA Devices:"
nmap -p 3333,8080,6000 192.168.1.0/24 2>/dev/null | grep -A2 "open" || echo "  Aucun dispositif réseau détecté"

# 2. Test permissions
echo "📍 2. Tests de permissions..."
for port in /dev/ttyUSB* /dev/ttyACM*; do
    if [ -e "$port" ]; then
        if [ -r "$port" ] && [ -w "$port" ]; then
            echo "  ✅ $port - Permissions OK"
        else
            echo "  ❌ $port - Permissions insuffisantes"
            echo "     Fix: sudo chmod 666 $port"
        fi
    fi
done

# 3. Test communication basique
echo "📍 3. Tests de communication..."
./bin/mea_communication_test --auto-detect --timeout 5

# 4. Test performance système
echo "📍 4. Tests performance système..."

# CPU frequency
current_freq=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq)
max_freq=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq)
freq_percent=$((current_freq * 100 / max_freq))

if [ $freq_percent -lt 90 ]; then
    echo "  ⚠️ CPU frequency: $freq_percent% of max"
    echo "     Recommendation: Set performance governor"
else
    echo "  ✅ CPU frequency: $freq_percent% of max"
fi

# Memory available
mem_available=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
mem_available_mb=$((mem_available / 1024))

if [ $mem_available_mb -lt 1024 ]; then
    echo "  ⚠️ Memory available: ${mem_available_mb}MB"
    echo "     Recommendation: Close unnecessary applications"
else
    echo "  ✅ Memory available: ${mem_available_mb}MB"
fi

# 5. Génération rapport
echo "📍 5. Génération rapport de diagnostic..."
./bin/system_diagnostics --comprehensive --output mea_diagnostic_report.json

echo ""
echo "✅ Diagnostic terminé - Rapport: mea_diagnostic_report.json"
echo "📧 Support: Envoyez ce rapport à support@biomining.io"
```

### Problèmes Courants et Solutions

#### Connexion Impossible

**Symptômes**: Timeout, "Device not found", "Permission denied"

**Solutions Étape par Étape**:

```bash
# 1. Vérification hardware physique
echo "🔌 Vérification connexion physique..."
lsusb -v | grep -A10 -B5 "MEA\|MCS\|Blackrock\|Plexon"

# 2. Reset USB si nécessaire
echo "🔄 Reset bus USB..."
sudo systemctl restart usb

# 3. Permissions utilisateur
echo "👤 Configuration permissions..."
sudo usermod -a -G dialout,tty $USER
newgrp dialout  # Apply immediately

# 4. Test communication manuel
echo "📡 Test communication manuel..."
stty -F /dev/ttyUSB0 115200 raw -echo
echo "ID?" > /dev/ttyUSB0 &
timeout 3s cat /dev/ttyUSB0

# 5. Configuration udev rules
echo "⚙️ Configuration règles udev..."
sudo tee /etc/udev/rules.d/99-mea-devices.rules << EOF
# MCS Devices
SUBSYSTEM=="usb", ATTR{idVendor}=="04b4", ATTR{idProduct}=="8613", MODE="0666", GROUP="dialout"

# Blackrock Devices  
SUBSYSTEM=="usb", ATTR{idVendor}=="0403", ATTR{idProduct}=="6014", MODE="0666", GROUP="dialout"

# Intan Devices
SUBSYSTEM=="usb", ATTR{idVendor}=="04d8", ATTR{idProduct}=="00df", MODE="0666", GROUP="dialout"
EOF

sudo udevadm control --reload-rules
sudo udevadm trigger
```

#### Qualité Signal Dégradée

**Symptômes**: Signal quality < 80%, spikes manqués, bruit élevé

**Diagnostic Approfondi**:
```cpp
void diagnoseSignalQuality() {
    qInfo() << "[Diagnostic] Analyse qualité signal approfondie...";
    
    // 1. Analyse spectrale du bruit
    QVector<ElectrodeData> samples = acquireTestSamples(5000); // 5s
    
    for (int electrode = 0; electrode < 60; ++electrode) {
        QVector<double> signal = extractElectrodeSignal(samples, electrode);
        
        // FFT pour analyse fréquentielle
        auto fftResult = performFFT(signal);
        
        // Identifier sources de bruit
        double powerLine50Hz = fftResult.getPowerAt(50.0);  // Secteur EU
        double powerLine60Hz = fftResult.getPowerAt(60.0);  // Secteur US
        double digitalNoise = fftResult.getHighFrequencyPower(10000, 12500);
        
        if (powerLine50Hz > NOISE_THRESHOLD) {
            qWarning() << QString("E%1: Bruit secteur 50Hz détecté (%2 dB)")
                          .arg(electrode).arg(20*log10(powerLine50Hz));
        }
        
        if (digitalNoise > DIGITAL_NOISE_THRESHOLD) {
            qWarning() << QString("E%1: Bruit numérique haute fréquence (%2 dB)")
                          .arg(electrode).arg(20*log10(digitalNoise));
        }
    }
    
    // 2. Test impédance électrodes
    performDetailedImpedanceAnalysis();
    
    // 3. Vérification connexions hardware
    checkHardwareConnections();
    
    // 4. Recommandations automatiques
    generateQualityRecommendations();
}

void generateQualityRecommendations() {
    qInfo() << "[Diagnostic] Génération recommandations...";
    
    QStringList recommendations;
    
    // Analyse environnementale
    if (detectPowerLineNoise()) {
        recommendations << "🔌 Utiliser alimentation sur batterie";
        recommendations << "🔌 Vérifier mise à la terre électrique";
        recommendations << "🔌 Éloigner des sources électromagnétiques";
    }
    
    // Analyse hardware
    if (highImpedanceElectrodes() > 5) {
        recommendations << "🧪 Nettoyer électrodes avec solution saline";
        recommendations << "🧪 Vérifier connexions MEA chip";
        recommendations << "🧪 Remplacer électrodes défaillantes si possible";
    }
    
    // Analyse configuration
    if (getCurrentAmplification() < 1000) {
        recommendations << "⚡ Augmenter amplification à 1200x ou plus";
    }
    
    if (getSamplingRate() < 25000) {
        recommendations << "📊 Augmenter fréquence échantillonnage à 25kHz";
    }
    
    // Afficher recommandations
    for (const QString& rec : recommendations) {
        qInfo() << "[Recommendation]" << rec;
    }
}
```

#### Latence Élevée

**Diagnostic Performance**:
```cpp
class LatencyProfiler {
public:
    struct LatencyBreakdown {
        double hardwareLatency;    // Device to host
        double networkLatency;     // Network transmission  
        double processingLatency;  // Host processing
        double totalLatency;       // End-to-end
    };
    
    LatencyBreakdown measureLatency() {
        LatencyBreakdown breakdown;
        
        // 1. Hardware latency
        auto hwStart = std::chrono::high_resolution_clock::now();
        sendHardwarePing();
        waitForHardwarePong();
        auto hwEnd = std::chrono::high_resolution_clock::now();
        
        breakdown.hardwareLatency = std::chrono::duration<double, std::milli>(
            hwEnd - hwStart).count();
        
        // 2. Network latency (if applicable)
        if (isNetworkConnection()) {
            breakdown.networkLatency = measureNetworkRTT();
        }
        
        // 3. Processing latency
        auto procStart = std::chrono::high_resolution_clock::now();
        auto testData = generateTestElectrodeData();
        processElectrodeData(testData);
        auto procEnd = std::chrono::high_resolution_clock::now();
        
        breakdown.processingLatency = std::chrono::duration<double, std::milli>(
            procEnd - procStart).count();
        
        breakdown.totalLatency = breakdown.hardwareLatency + 
                                breakdown.networkLatency + 
                                breakdown.processingLatency;
        
        return breakdown;
    }
    
    void printLatencyReport(const LatencyBreakdown& breakdown) {
        qInfo() << "📊 LATENCY BREAKDOWN REPORT";
        qInfo() << "═══════════════════════════";
        qInfo() << QString("Hardware:    %1 ms").arg(breakdown.hardwareLatency, 0, 'f', 2);
        qInfo() << QString("Network:     %1 ms").arg(breakdown.networkLatency, 0, 'f', 2);  
        qInfo() << QString("Processing:  %1 ms").arg(breakdown.processingLatency, 0, 'f', 2);
        qInfo() << QString("TOTAL:       %1 ms").arg(breakdown.totalLatency, 0, 'f', 2);
        qInfo() << "";
        
        // Recommandations basées sur résultats
        if (breakdown.hardwareLatency > 5.0) {
            qWarning() << "⚠️ Hardware latency élevée - vérifier connexion";
        }
        if (breakdown.networkLatency > 2.0) {
            qWarning() << "⚠️ Network latency élevée - optimiser réseau";
        }
        if (breakdown.processingLatency > 1.0) {
            qWarning() << "⚠️ Processing latency élevée - optimiser CPU";
        }
    }
};
```

---

## 📊 Spécifications Techniques

### Limites Hardware par Fabricant

#### Multi Channel Systems
| Paramètre | MEA2100 | MEA1060 | USB-MEA |
|-----------|---------|---------|---------|
| **Électrodes** | 60 | 60 | 60 |
| **Résolution ADC** | 16-bit | 16-bit | 12-bit |
| **Fréquence Max** | 50 kHz | 25 kHz | 20 kHz |
| **Bande passante** | 1 Hz - 10 kHz | 1 Hz - 8 kHz | 10 Hz - 5 kHz |
| **Gain** | 1-10,000x | 1-5,000x | 55-1,100x |
| **Bruit (RMS)** | < 2.4 μV | < 3.0 μV | < 4.0 μV |
| **Stimulation Max** | ±5V, 100μA | ±3V, 50μA | ±2V, 25μA |

#### Blackrock Microsystems  
| Paramètre | CerePlex Direct | CerePlex W | CerePlex E |
|-----------|----------------|------------|------------|
| **Électrodes** | 96-128 | 96 | 96 |
| **Résolution ADC** | 16-bit | 16-bit | 16-bit |
| **Fréquence Max** | 30 kHz | 30 kHz | 30 kHz |
| **Bande passante** | 0.3 Hz - 7.5 kHz | 0.3 Hz - 7.5 kHz | 0.3 Hz - 7.5 kHz |
| **Gain** | 5,000x fixe | 5,000x fixe | 1,000-10,000x |
| **Bruit (RMS)** | < 2.5 μV | < 3.0 μV | < 2.2 μV |

### Formats de Données

#### Structure ElectrodeData Détaillée
```cpp
struct ElectrodeData {
    // Identification
    int32_t electrodeId;          // 0-255
    uint64_t timestamp;           // μs depuis epoch Unix
    
    // Signaux principaux
    float voltage;                // V (IEEE 754 float32)
    float current;                // A (si mesure directe disponible)  
    float impedance;              // Ω (mise à jour périodique)
    
    // Métadonnées qualité
    uint8_t signalQuality;        // 0-255 (0xFF = 100%)
    uint8_t isActive;             // 0=inactive, 1=active
    uint8_t isStimulating;        // 0=normal, 1=en cours stimulation
    uint8_t reserved;             // Padding/future use
    
    // Données étendues (optionnel)
    int16_t rawADC;              // Valeur ADC brute
    uint16_t flags;              // Bit flags (saturation, etc.)
    
    // Checksum pour intégrité
    uint32_t crc32;              // CRC32 des données ci-dessus
};

static_assert(sizeof(ElectrodeData) == 32, "ElectrodeData must be 32 bytes");
```

#### Format de Paquet Réseau
```cpp
struct MEANetworkPacket {
    // Header (16 bytes)
    uint32_t magic;              // 0xMEA1 (validation)
    uint16_t version;            // Version protocole (0x0100 = v1.0)
    uint16_t packetType;         // DATA=1, COMMAND=2, RESPONSE=3
    uint32_t sequenceNumber;     // Numéro séquence (détection perte)
    uint32_t payloadLength;      // Taille payload en bytes
    
    // Payload (variable)
    union {
        ElectrodeData electrodes[MAX_ELECTRODES_PER_PACKET];
        struct {
            uint16_t command;
            uint16_t paramCount;
            uint32_t parameters[MAX_COMMAND_PARAMS];
        } command;
        
        struct {
            uint16_t responseCode;
            uint16_t dataLength;
            uint8_t data[MAX_RESPONSE_DATA];
        } response;
    } payload;
    
    // Footer (4 bytes)
    uint32_t crc32;              // CRC32 de tout le paquet
};
```

### Calculs de Performance

#### Débit de Données Théorique
```cpp
class MEADataRateCalculator {
public:
    struct DataRateSpec {
        int electrodeCount;
        double samplingRate;        // Hz
        int bytesPerSample;
        double theoreticalRate;     // MB/s
        double practicalRate;       // MB/s (avec overhead)
        double networkOverhead;     // %
    };
    
    static DataRateSpec calculateDataRate(const RealMEAConfig& config) {
        DataRateSpec spec;
        
        spec.electrodeCount = config.electrodeCount;
        spec.samplingRate = config.samplingRate;
        spec.bytesPerSample = sizeof(ElectrodeData);
        
        // Débit théorique
        double samplesPerSec = config.electrodeCount * config.samplingRate;
        double bytesPerSec = samplesPerSec * spec.bytesPerSample;
        spec.theoreticalRate = bytesPerSec / (1024.0 * 1024.0); // MB/s
        
        // Overhead protocolaire
        if (config.protocol == CommunicationProtocol::TCP) {
            spec.networkOverhead = 0.15; // 15% overhead TCP
        } else if (config.protocol == CommunicationProtocol::UDP) {
            spec.networkOverhead = 0.08; // 8% overhead UDP
        } else {
            spec.networkOverhead = 0.05; // 5% overhead série
        }
        
        spec.practicalRate = spec.theoreticalRate * (1.0 + spec.networkOverhead);
        
        return spec;
    }
    
    static void printDataRateReport(const DataRateSpec& spec) {
        qInfo() << "📊 MEA DATA RATE ANALYSIS";
        qInfo() << "═════════════════════════";
        qInfo() << QString("Electrodes:      %1").arg(spec.electrodeCount);
        qInfo() << QString("Sampling Rate:   %1 Hz").arg(spec.samplingRate, 0, 'f', 0);
        qInfo() << QString("Bytes/Sample:    %1").arg(spec.bytesPerSample);
        qInfo() << QString("Theoretical:     %1 MB/s").arg(spec.theoreticalRate, 0, 'f', 2);
        qInfo() << QString("Practical:       %1 MB/s").arg(spec.practicalRate, 0, 'f', 2);
        qInfo() << QString("Network Overhead: %1%").arg(spec.networkOverhead * 100, 0, 'f', 1);
        
        // Recommandations réseau
        if (spec.practicalRate > 10.0) {
            qWarning() << "⚠️ Débit élevé - utiliser Gigabit Ethernet";
        } else if (spec.practicalRate > 1.0) {
            qInfo() << "💡 Fast Ethernet (100Mbps) suffisant";
        } else {
            qInfo() << "💡 Débit modéré - tous protocoles OK";
        }
    }
};

// Exemple d'utilisation:
// MEA2100: 60 électrodes × 25kHz × 32 bytes = 48 MB/s théorique
// Avec overhead TCP 15%: ~55 MB/s pratique
// Nécessite Gigabit Ethernet pour confort
```

---

**🧬⛏️ BioMining Platform - Hardware MEA Guide Complet**

*Ce guide couvre tous les aspects hardware des dispositifs MEA. Pour le développement software, consultez le [Developer Guide](DEVELOPER_GUIDE.md).*