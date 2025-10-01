# 🔧 API Reference - BioMining Platform

## 📋 Table des Matières

- [Interface MEA Réelle](#interface-mea-réelle)
- [Mining Hybride](#mining-hybride)
- [Configuration](#configuration)
- [Signaux et Slots Qt](#signaux-et-slots-qt)
- [Structures de Données](#structures-de-données)
- [Exemples d'Utilisation](#exemples-dutilisation)

---

## 🧬 Interface MEA Réelle

### `RealMEAInterface`

Interface principale pour la communication avec les dispositifs MEA hardware.

#### Constructeur et Initialisation

```cpp
class RealMEAInterface : public QObject
{
    Q_OBJECT

public:
    explicit RealMEAInterface(QObject *parent = nullptr);
    ~RealMEAInterface();

    // Configuration et connexion
    bool initialize(const RealMEAConfig &config);
    void disconnect();
    ConnectionStatus getStatus() const;
};
```

##### `initialize(const RealMEAConfig &config)`
**Description**: Initialise et connecte l'interface avec un dispositif MEA hardware.

**Paramètres**:
- `config`: Configuration complète du dispositif MEA

**Retour**: `bool` - `true` si la connexion est établie avec succès

**Exemple**:
```cpp
RealMEAConfig config;
config.deviceType = MEADeviceType::MultiChannelSystems_MCS;
config.protocol = CommunicationProtocol::TCP;
config.networkHost = "localhost";
config.networkPort = 3333;

RealMEAInterface meaInterface;
if (meaInterface.initialize(config)) {
    qDebug() << "MEA connecté avec succès";
} else {
    qDebug() << "Erreur:" << meaInterface.getLastError();
}
```

#### Acquisition de Données

##### `readElectrodeData()`
```cpp
QVector<ElectrodeData> readElectrodeData();
```
**Description**: Lit les données actuelles de toutes les électrodes actives.

**Retour**: `QVector<ElectrodeData>` - Données de chaque électrode avec timestamp

**Exemple**:
```cpp
QVector<ElectrodeData> data = meaInterface.readElectrodeData();
for (const auto& electrode : data) {
    qDebug() << QString("Électrode %1: %2 μV (Qualité: %3%)")
                .arg(electrode.electrodeId)
                .arg(electrode.voltage)
                .arg(electrode.signalQuality * 100);
}
```

##### `readRawSignals()`
```cpp
QVector<double> readRawSignals();
```
**Description**: Lit les signaux bruts de toutes les électrodes (compatibilité).

**Retour**: `QVector<double>` - Valeurs de tension en μV

##### `startContinuousAcquisition()` / `stopContinuousAcquisition()`
```cpp
bool startContinuousAcquisition();
void stopContinuousAcquisition();
```
**Description**: Démarre/arrête l'acquisition en continu avec émission de signaux Qt.

**Exemple**:
```cpp
// Connexion au signal de données
connect(&meaInterface, &RealMEAInterface::electrodeDataReady,
        this, &MyClass::onNewElectrodeData);

// Démarrage de l'acquisition
if (meaInterface.startContinuousAcquisition()) {
    qDebug() << "Acquisition démarrée";
}
```

#### Stimulation des Électrodes

##### `stimulateElectrode(int electrodeId, double amplitude, double duration)`
```cpp
bool stimulateElectrode(int electrodeId, double amplitude, double duration);
```
**Description**: Stimule une électrode spécifique avec les paramètres donnés.

**Paramètres**:
- `electrodeId`: ID de l'électrode (0-59)
- `amplitude`: Amplitude en Volts (max 5.0V)
- `duration`: Durée en millisecondes (0.1-10ms)

**Retour**: `bool` - `true` si la stimulation est acceptée

**Sécurité**: Vérifications automatiques des limites hardware

**Exemple**:
```cpp
// Stimulation sécurisée
if (meaInterface.stimulateElectrode(15, 2.5, 5.0)) {
    qDebug() << "Stimulation électrode 15: 2.5V pendant 5ms";
} else {
    qDebug() << "Stimulation refusée - vérifier les limites";
}
```

##### `stimulatePattern(const QVector<double> &pattern)`
```cpp
bool stimulatePattern(const QVector<double> &pattern);
```
**Description**: Applique un pattern de stimulation complexe.

**Paramètres**:
- `pattern`: Séquence de valeurs de tension (biphasique recommandé)

**Exemple**:
```cpp
// Pattern biphasique pour charge balancing
QVector<double> biphasic = {0, 2.5, 0, -2.5, 0};
meaInterface.stimulatePattern(biphasic);
```

##### `setStimulationParameters(double maxVoltage, double maxCurrent)`
```cpp
void setStimulationParameters(double maxVoltage, double maxCurrent);
```
**Description**: Définit les limites de sécurité pour la stimulation.

#### Calibration et Tests

##### `performElectrodeImpedanceTest()`
```cpp
bool performElectrodeImpedanceTest();
```
**Description**: Lance un test d'impédance complet sur toutes les électrodes.

**Processus**:
1. Test séquentiel de chaque électrode
2. Mesure d'impédance avec signal de test
3. Validation des seuils acceptables
4. Mise à jour des facteurs de calibration

##### `performFullCalibration()`
```cpp
bool performFullCalibration();
```
**Description**: Effectue une calibration complète du système MEA.

**Étapes**:
1. Tests d'impédance sur toutes les électrodes
2. Calcul des facteurs de correction
3. Ajustement des gains d'amplification
4. Validation de la qualité du signal

##### `getElectrodeImpedances()`
```cpp
QVector<double> getElectrodeImpedances() const;
```
**Description**: Retourne les valeurs d'impédance mesurées pour chaque électrode.

**Retour**: `QVector<double>` - Impédances en Ohms

#### Détection de Spikes

##### `enableSpikeDetection(bool enable)`
```cpp
void enableSpikeDetection(bool enable);
```
**Description**: Active/désactive la détection automatique de spikes.

##### `setSpikeThreshold(double threshold)`
```cpp
void setSpikeThreshold(double threshold);
```
**Description**: Définit le seuil de détection des spikes.

**Paramètres**:
- `threshold`: Seuil en μV (valeur négative, ex: -50.0)

##### `getRecentSpikes(int lastNSpikes = 100)`
```cpp
QVector<SpikeEvent> getRecentSpikes(int lastNSpikes = 100);
```
**Description**: Récupère les derniers spikes détectés.

**Exemple**:
```cpp
// Configuration détection de spikes
meaInterface.enableSpikeDetection(true);
meaInterface.setSpikeThreshold(-50.0);

// Connexion au signal
connect(&meaInterface, &RealMEAInterface::spikeDetected,
        [](const SpikeEvent& spike) {
    qDebug() << QString("Spike - Électrode:%1, Amplitude:%2μV")
                .arg(spike.electrodeId)
                .arg(spike.amplitude);
});
```

#### Diagnostic et Monitoring

##### `getDeviceInfo()`
```cpp
QString getDeviceInfo() const;
```
**Description**: Retourne les informations du dispositif MEA connecté.

##### `getSignalQuality()`
```cpp
double getSignalQuality() const;
```
**Description**: Évalue la qualité globale du signal (0.0-1.0).

##### `getLastError()`
```cpp
QString getLastError() const;
```
**Description**: Retourne le dernier message d'erreur.

##### `getSystemStatus()`
```cpp
QJsonObject getSystemStatus() const;
```
**Description**: Retourne un rapport complet du système en format JSON.

**Exemple**:
```cpp
QJsonObject status = meaInterface.getSystemStatus();
qDebug() << "Status:" << status["connection_status"].toString();
qDebug() << "Électrodes actives:" << status["active_electrodes"].toInt();
qDebug() << "Qualité signal:" << status["signal_quality"].toDouble();
```

---

## ⛏️ Mining Hybride

### `HybridBitcoinMiner`

Moteur de mining Bitcoin intégrant l'entropie biologique des signaux MEA.

#### Configuration et Démarrage

```cpp
class HybridBitcoinMiner : public QObject
{
    Q_OBJECT

public:
    explicit HybridBitcoinMiner(QObject *parent = nullptr);
    
    void setMEAInterface(RealMEAInterface *meaInterface);
    void setBiologicalEntropy(bool enabled);
    void startMining(const BitcoinMiningConfig &config);
    void stopMining();
};
```

##### `setMEAInterface(RealMEAInterface *meaInterface)`
**Description**: Connecte le miner à une interface MEA pour l'entropie biologique.

##### `setBiologicalEntropy(bool enabled)`
**Description**: Active/désactive l'utilisation de l'entropie biologique.

**Exemple**:
```cpp
HybridBitcoinMiner miner;
miner.setMEAInterface(&meaInterface);
miner.setBiologicalEntropy(true);

BitcoinMiningConfig config;
config.difficulty = 0x1d00ffff;
config.biologicalWeight = 0.3;  // 30% entropie bio
config.useHardwareAcceleration = true;

miner.startMining(config);
```

#### Monitoring des Performances

##### Signaux Qt
```cpp
signals:
    void blockFound(const Block &block);
    void hashRateUpdated(double hashesPerSecond);
    void biologicalEntropyUpdated(double entropyLevel);
    void miningStarted();
    void miningStopped();
```

**Exemple d'utilisation**:
```cpp
connect(&miner, &HybridBitcoinMiner::blockFound, 
        [](const Block& block) {
    qDebug() << "🎉 Bloc trouvé!";
    qDebug() << "Hash:" << block.hash;
    qDebug() << "Entropie bio:" << block.biologicalEntropy << "%";
    qDebug() << "Nonce:" << block.nonce;
});

connect(&miner, &HybridBitcoinMiner::hashRateUpdated,
        [](double hashRate) {
    qDebug() << QString("Hash Rate: %1 H/s").arg(hashRate, 0, 'f', 2);
});
```

---

## ⚙️ Configuration

### `RealMEAConfig`

Structure de configuration complète pour les dispositifs MEA.

```cpp
struct RealMEAConfig {
    // Configuration de base
    MEADeviceType deviceType = MEADeviceType::Custom_Serial;
    CommunicationProtocol protocol = CommunicationProtocol::SerialPort;
    
    // Paramètres de connexion
    QString devicePath = "/dev/ttyUSB0";
    QString networkHost = "localhost";
    int networkPort = 8080;
    int baudRate = 115200;
    
    // Configuration électrodes
    int electrodeCount = 60;
    QVector<int> activeElectrodes;
    QVector<double> electrodeImpedances;
    
    // Acquisition
    double samplingRate = 25000.0;
    double amplification = 1200.0;
    double filterLowCut = 300.0;
    double filterHighCut = 8000.0;
    int bufferSize = 1024;
    
    // Stimulation
    double stimMaxVoltage = 5.0;
    double stimMaxCurrent = 100.0;
    bool bidirectionalStim = true;
    
    // Détection de pics
    bool spikeDetection = true;
    double spikeThreshold = -50.0;
    int spikeWindowMs = 2;
    
    // Timeouts et retry
    int connectionTimeoutMs = 5000;
    int readTimeoutMs = 1000;
    int maxRetries = 3;
};
```

#### Enums Disponibles

##### `MEADeviceType`
```cpp
enum class MEADeviceType {
    MultiChannelSystems_MCS,     // Multi Channel Systems
    AlphaOmega_AlphaMap,        // Alpha Omega
    Blackrock_CerePlex,         // Blackrock
    Plexon_OmniPlex,           // Plexon
    Intan_RHD,                 // Intan RHD series
    OpenEphys_ONIX,            // Open Ephys
    NeuroNexus_SmartBox,       // NeuroNexus
    Custom_Serial,             // Interface série personnalisée
    Custom_Network,            // Interface réseau personnalisée
    Custom_SharedMemory        // Mémoire partagée
};
```

##### `CommunicationProtocol`
```cpp
enum class CommunicationProtocol {
    SerialPort,         // Port série RS232/USB
    TCP,               // TCP Socket
    UDP,               // UDP Socket
    SharedMemory,      // Mémoire partagée
    NamedPipe,         // Named pipe
    HDF5_File,         // Fichier HDF5 temps réel
    Custom_API         // API propriétaire
};
```

---

## 📊 Structures de Données

### `ElectrodeData`

Données en temps réel d'une électrode.

```cpp
struct ElectrodeData {
    int electrodeId;            // ID de l'électrode (0-59)
    double voltage;             // Tension en Volts
    double current;             // Courant en Ampères
    double impedance;           // Impédance en Ohms
    bool isActive;              // État actif/inactif
    double signalQuality;       // Qualité 0.0-1.0
    qint64 timestamp;          // Timestamp μs depuis epoch
};
```

### `SpikeEvent`

Événement de spike détecté.

```cpp
struct SpikeEvent {
    int electrodeId;            // ID de l'électrode source
    double amplitude;           // Amplitude du spike en μV
    qint64 timestamp;          // Timestamp μs précis
    QVector<double> waveform;   // Forme d'onde complète (optionnel)
};
```

### `BitcoinMiningConfig`

Configuration du mining Bitcoin.

```cpp
struct BitcoinMiningConfig {
    uint32_t difficulty;              // Difficulté de mining
    bool useHardwareAcceleration;     // GPU/ASIC si disponible
    double biologicalWeight;          // Poids entropie bio (0.0-1.0)
    int threadCount;                  // Nombre de threads CPU
    QString poolAddress;              // Adresse pool (optionnel)
    bool enableProfitability;         // Mode profitabilité
};
```

---

## 🔄 Signaux et Slots Qt

### Signaux `RealMEAInterface`

#### Données
```cpp
signals:
    // Données fraîches disponibles
    void electrodeDataReady(const QVector<ElectrodeData> &data);
    void rawSignalsReady(const QVector<double> &signalData);
    void spikeDetected(const SpikeEvent &spike);
```

#### Contrôle
```cpp
signals:
    // Opérations terminées
    void stimulationComplete(int electrodeId);
    void calibrationProgress(int electrode, double progress);
    void calibrationComplete();
```

#### Statut
```cpp
signals:
    // Changements d'état
    void statusChanged(ConnectionStatus status);
    void errorOccurred(const QString &error);
    void deviceDisconnected();
```

### Exemple d'Utilisation Complète

```cpp
class MEAController : public QObject
{
    Q_OBJECT

public slots:
    void onElectrodeData(const QVector<ElectrodeData> &data) {
        // Traitement des données temps réel
        for (const auto& electrode : data) {
            if (electrode.signalQuality > 0.8) {
                // Signal de bonne qualité
                processHighQualitySignal(electrode);
            }
        }
    }
    
    void onSpikeDetected(const SpikeEvent &spike) {
        // Réaction aux spikes
        qDebug() << QString("Spike détecté - E%1: %2μV")
                    .arg(spike.electrodeId)
                    .arg(spike.amplitude);
        
        // Possibilité de stimulation en réponse
        if (spike.amplitude < -80.0) {  // Spike fort
            meaInterface->stimulateElectrode(spike.electrodeId + 5, 1.0, 2.0);
        }
    }
    
    void onCalibrationProgress(int electrode, double progress) {
        emit calibrationProgressChanged(electrode, progress);
        
        if (progress >= 1.0) {
            qDebug() << QString("Électrode %1 calibrée").arg(electrode);
        }
    }

private:
    RealMEAInterface *meaInterface;
    HybridBitcoinMiner *miner;
    
    void setupConnections() {
        // Connexions MEA
        connect(meaInterface, &RealMEAInterface::electrodeDataReady,
                this, &MEAController::onElectrodeData);
        connect(meaInterface, &RealMEAInterface::spikeDetected,
                this, &MEAController::onSpikeDetected);
        connect(meaInterface, &RealMEAInterface::calibrationProgress,
                this, &MEAController::onCalibrationProgress);
        
        // Connexions Mining
        connect(miner, &HybridBitcoinMiner::blockFound,
                this, &MEAController::onBlockFound);
    }
};
```

---

## 🚀 Factory Pattern

### `MEAInterfaceFactory`

Factory pour créer différents types d'interfaces MEA.

```cpp
class MEAInterfaceFactory
{
public:
    static std::unique_ptr<RealMEAInterface> createInterface(MEADeviceType deviceType);
    static QStringList getSupportedDevices();
    static RealMEAConfig getDefaultConfig(MEADeviceType deviceType);
    static bool isDeviceAvailable(MEADeviceType deviceType, const QString &devicePath = "");
};
```

#### Utilisation
```cpp
// Détection automatique
QStringList devices = MEAInterfaceFactory::getSupportedDevices();
for (const QString& device : devices) {
    qDebug() << "Dispositif disponible:" << device;
}

// Création automatique avec config par défaut
auto meaInterface = MEAInterfaceFactory::createInterface(
    MEADeviceType::MultiChannelSystems_MCS);

// Configuration par défaut
RealMEAConfig defaultConfig = MEAInterfaceFactory::getDefaultConfig(
    MEADeviceType::Intan_RHD);
```

---

## 🔍 Debugging et Logging

### Macros de Debug
```cpp
// Activation du debug MEA
#define BIOMINING_DEBUG_MEA 1

// Dans le code
#ifdef BIOMINING_DEBUG_MEA
    qDebug() << "[MEA]" << QString("Électrode %1: %2μV")
                .arg(electrode.electrodeId)
                .arg(electrode.voltage);
#endif
```

### Logging Structuré
```cpp
// Configuration du logging
QLoggingCategory meaCategory("biomining.mea");
QLoggingCategory miningCategory("biomining.mining");

// Utilisation
qCInfo(meaCategory) << "MEA connecté:" << deviceInfo;
qCWarning(meaCategory) << "Qualité signal faible:" << quality;
qCCritical(meaCategory) << "Perte de connexion MEA";
```

---

Cette API Reference couvre les fonctionnalités principales de la plateforme BioMining. Pour des exemples plus avancés et des cas d'usage spécifiques, consultez la section [Exemples Avancés](ADVANCED_EXAMPLES.md).