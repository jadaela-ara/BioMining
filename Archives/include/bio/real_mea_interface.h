#ifndef REAL_MEA_INTERFACE_H
#define REAL_MEA_INTERFACE_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QSerialPort>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QProcess>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QFile>
#include <QDateTime>
#include <memory>
#include <random>

namespace BioMining {
namespace Bio {

/**
 * @brief Types de dispositifs MEA supportés
 */
enum class MEADeviceType {
    MultiChannelSystems_MCS,     // Multi Channel Systems (MCS)
    AlphaOmega_AlphaMap,        // Alpha Omega Alpha Map
    Blackrock_CerePlex,         // Blackrock CerePlex
    Plexon_OmniPlex,           // Plexon OmniPlex
    Intan_RHD,                 // Intan RHD series
    OpenEphys_ONIX,            // Open Ephys ONIX
    NeuroNexus_SmartBox,       // NeuroNexus SmartBox
    Custom_Serial,             // Interface série personnalisée
    Custom_Network,            // Interface réseau personnalisée
    Custom_SharedMemory        // Mémoire partagée personnalisée
};

/**
 * @brief Protocoles de communication supportés
 */
enum class CommunicationProtocol {
    SerialPort,         // Port série RS232/USB
    TCP,               // TCP Socket
    UDP,               // UDP Socket
    SharedMemory,      // Mémoire partagée
    NamedPipe,         // Named pipe
    HDF5_File,         // Fichier HDF5 en temps réel
    Custom_API         // API propriétaire
};

/**
 * @brief Configuration avancée pour MEA réel
 */
struct RealMEAConfig {
    // Configuration de base
    MEADeviceType deviceType = MEADeviceType::Custom_Serial;
    CommunicationProtocol protocol = CommunicationProtocol::SerialPort;
    
    // Paramètres de connexion
    QString devicePath = "/dev/ttyUSB0";     // Pour série
    QString networkHost = "localhost";       // Pour réseau
    int networkPort = 8080;                 // Pour réseau
    int baudRate = 115200;                  // Pour série
    
    // Configuration électrodes
    int electrodeCount = 60;
    QVector<int> activeElectrodes;          // Électrodes actives
    QVector<double> electrodeImpedances;    // Impédances des électrodes
    
    // Acquisition
    double samplingRate = 25000.0;          // Hz
    double amplification = 1200.0;
    double filterLowCut = 300.0;           // Hz
    double filterHighCut = 8000.0;         // Hz
    int bufferSize = 1024;
    
    // Stimulation
    double stimMaxVoltage = 5.0;           // V
    double stimMaxCurrent = 100.0;         // μA
    bool bidirectionalStim = true;
    
    // Détection de pics
    bool spikeDetection = true;
    double spikeThreshold = -50.0;         // μV
    int spikeWindowMs = 2;                 // ms
    
    // Timeouts et retry
    int connectionTimeoutMs = 5000;
    int readTimeoutMs = 1000;
    int maxRetries = 3;
    
    // Calibration
    QString calibrationFile = "";
    bool autoCalibration = true;
};

/**
 * @brief Données d'une électrode en temps réel
 */
struct ElectrodeData {
    int electrodeId;
    double voltage;              // V
    double current;              // A  
    double impedance;           // Ω
    bool isActive;
    double signalQuality;       // 0.0-1.0
    qint64 timestamp;          // μs depuis epoch
};

/**
 * @brief Événement de spike détecté
 */
struct SpikeEvent {
    int electrodeId;
    double amplitude;           // μV
    qint64 timestamp;          // μs
    QVector<double> waveform;   // Forme d'onde complète
};

/**
 * @brief Pattern de stimulation pour apprentissage Bitcoin
 */
struct BitcoinLearningPattern {
    QString blockHash;                    // Hash du bloc Bitcoin
    quint32 targetNonce;                 // Nonce cible
    QVector<double> stimulationPattern;  // Pattern de stimulation (60 électrodes)
    double difficulty;                   // Difficulté du bloc
    qint64 timestamp;                    // Timestamp d'apprentissage
    double successRate;                  // Taux de succès (0.0-1.0)
};

/**
 * @brief Réponse neuronale à un pattern Bitcoin
 */
struct NeuralBitcoinResponse {
    BitcoinLearningPattern inputPattern;
    QVector<SpikeEvent> neuralResponse;  // Réponse des neurones
    quint32 predictedNonce;              // Nonce prédit par les neurones
    double confidence;                   // Confiance de la prédiction (0.0-1.0)
    double rewardSignal;                 // Signal de récompense/punition
    qint64 responseTime;                 // Temps de réponse en μs
};

/**
 * @brief Modèle d'apprentissage neuronal persistant
 */
struct BiologicalNeuralModel {
    QString modelId;                           // ID unique du modèle
    QVector<BitcoinLearningPattern> trainedPatterns;  // Patterns appris
    QVector<double> synapticWeights;           // Poids synaptiques (matrice 60x60)
    QVector<double> electrodeThresholds;       // Seuils par électrode
    QVector<double> learningRates;             // Taux d'apprentissage par électrode
    double globalLearningRate;                 // Taux d'apprentissage global
    qint64 trainingStartTime;                  // Début d'entraînement
    qint64 lastUpdateTime;                     // Dernière mise à jour
    int trainingEpochs;                        // Nombre d'époques d'entraînement
    double averageAccuracy;                    // Précision moyenne
};

/**
 * @brief Configuration d'apprentissage Bitcoin
 */
struct BitcoinLearningConfig {
    double learningRate = 0.01;               // Taux d'apprentissage Hebbien
    double decayRate = 0.95;                  // Taux de décroissance
    int maxTrainingEpochs = 1000;             // Epochs maximum
    double targetAccuracy = 0.85;             // Précision cible
    double stimulationAmplitude = 3.0;        // Amplitude de stimulation (V)
    double stimulationDuration = 50.0;        // Durée de stimulation (ms)
    double reinforcementDelay = 100.0;        // Délai de renforcement (ms)
    double punishmentAmplitude = -1.5;        // Amplitude de punition (V)
    QString modelPersistencePath = "./neural_models/"; // Chemin de sauvegarde
    bool enableRealtimeLearning = true;       // Apprentissage en temps réel
    bool enableBackpropagation = true;        // Rétropropagation biologique
};

/**
 * @brief Statistiques d'apprentissage Bitcoin
 */
struct BitcoinLearningStats {
    int totalPatternsLearned = 0;
    int successfulPredictions = 0;
    int totalPredictions = 0;
    double currentAccuracy = 0.0;
    double bestAccuracy = 0.0;
    qint64 totalTrainingTime = 0;        // En microsecondes
    QVector<double> accuracyHistory;     // Historique de précision
    QVector<qint64> responseTimeHistory; // Historique des temps de réponse
    QString lastModelFile;               // Dernier fichier de modèle sauvé
};

/**
 * @brief Interface réelle pour MEA hardware
 */
class RealMEAInterface : public QObject
{
    Q_OBJECT

public:
    enum class ConnectionStatus {
        Disconnected,
        Connecting,
        Connected,
        Error,
        Calibrating,
        Streaming
    };

    explicit RealMEAInterface(QObject *parent = nullptr);
    ~RealMEAInterface();

    // Configuration et connexion
    bool initialize(const RealMEAConfig &config);
    void disconnect();
    ConnectionStatus getStatus() const { return m_status; }
    
    // Informations du dispositif
    QString getDeviceInfo() const;
    QVector<int> getActiveElectrodes() const;
    double getSamplingRate() const;
    
    // Acquisition de données
    QVector<ElectrodeData> readElectrodeData();
    QVector<double> readRawSignals();                    // Compatibilité
    bool startContinuousAcquisition();
    void stopContinuousAcquisition();
    
    // Stimulation
    bool stimulateElectrode(int electrodeId, double amplitude, double duration);
    bool stimulatePattern(const QVector<double> &pattern);
    void setStimulationParameters(double maxVoltage, double maxCurrent);
    
    // Calibration et impédance
    bool performElectrodeImpedanceTest();
    bool calibrateElectrode(int electrodeId);
    bool performFullCalibration();
    QVector<double> getElectrodeImpedances() const;
    
    // Détection de spikes
    void enableSpikeDetection(bool enable);
    void setSpikeThreshold(double threshold);
    QVector<SpikeEvent> getRecentSpikes(int lastNSpikes = 100);
    
    // Diagnostic et monitoring
    double getSignalQuality() const;
    QString getLastError() const;
    QJsonObject getSystemStatus() const;
    
    // Contrôle avancé
    void setElectrodeActive(int electrodeId, bool active);
    void setAcquisitionFilter(double lowCut, double highCut);
    bool saveRecording(const QString &filepath, const QString &format = "HDF5");
    
    // === APPRENTISSAGE BITCOIN NEURONAL ===
    
    /**
     * @brief Initialise le système d'apprentissage Bitcoin
     */
    bool initializeBitcoinLearning(const BitcoinLearningConfig &config);
    
    /**
     * @brief Entraîne les neurones avec un pattern Bitcoin
     */
    bool trainBitcoinPattern(const BitcoinLearningPattern &pattern);
    
    /**
     * @brief Prédit un nonce basé sur un hash de bloc
     */
    NeuralBitcoinResponse predictNonce(const QString &blockHash, double difficulty);
    
    /**
     * @brief Applique le rétro-apprentissage basé sur le succès/échec
     */
    bool applyReinforcementLearning(const NeuralBitcoinResponse &response, bool wasSuccessful);
    
    /**
     * @brief Sauvegarde le modèle neuronal
     */
    bool saveNeuralModel(const QString &modelPath = "");
    
    /**
     * @brief Charge un modèle neuronal existant
     */
    bool loadNeuralModel(const QString &modelPath);
    
    /**
     * @brief Obtient les statistiques d'apprentissage actuelles
     */
    BitcoinLearningStats getLearningStats() const;
    
    /**
     * @brief Réinitialise le modèle neuronal
     */
    void resetNeuralModel();
    
    /**
     * @brief Active/désactive l'apprentissage en temps réel
     */
    void enableRealtimeLearning(bool enable);
    
    /**
     * @brief Optimise les paramètres d'apprentissage
     */
    bool optimizeLearningParameters();
    
    /**
     * @brief Génère un pattern de stimulation à partir d'un hash
     */
    QVector<double> generateStimulationPattern(const QString &blockHash);
    
    /**
     * @brief Analyse la réponse neuronale pour extraire un nonce
     */
    quint32 extractNonceFromNeuralResponse(const QVector<SpikeEvent> &spikes);
    
    /**
     * @brief Met à jour les poids synaptiques (apprentissage Hebbien)
     */
    void updateSynapticWeights(const QVector<SpikeEvent> &preSpikes, 
                               const QVector<SpikeEvent> &postSpikes, 
                               double reward);
    
    /**
     * @brief Applique la plasticité synaptique dépendante du temps
     */
    void applySTDP(int preElectrode, int postElectrode, 
                   qint64 timeDifference, double learningRate);
                   
    /**
     * @brief Stimule les neurones avec renforcement/punition
     */
    bool stimulateWithReinforcement(const QVector<double> &pattern, 
                                    double reinforcementSignal);

signals:
    // Signaux de données
    void electrodeDataReady(const QVector<ElectrodeData> &data);
    void rawSignalsReady(const QVector<double> &signalData);
    void spikeDetected(const SpikeEvent &spike);
    
    // Signaux de contrôle
    void stimulationComplete(int electrodeId);
    void calibrationProgress(int electrode, double progress);
    void calibrationComplete();
    
    // Signaux de statut
    void statusChanged(ConnectionStatus status);
    void errorOccurred(const QString &error);
    void deviceDisconnected();
    
    // Signaux d'apprentissage Bitcoin
    void bitcoinPatternLearned(const BitcoinLearningPattern &pattern);
    void noncePredicationReady(const NeuralBitcoinResponse &response);
    void learningStatsUpdated(const BitcoinLearningStats &stats);
    void neuralModelSaved(const QString &modelPath);
    void reinforcementLearningApplied(double reward, double newAccuracy);

private slots:
    void onSerialDataReady();
    void onNetworkDataReady();
    void onAcquisitionTimer();
    void onCalibrationTimer();

private:
    // Méthodes de connexion spécifiques
    bool connectSerial();
    bool connectNetwork();
    bool connectSharedMemory();
    bool connectCustomAPI();
    
    // Protocoles de communication
    bool sendCommand(const QByteArray &command);
    QByteArray receiveResponse(int timeoutMs = 1000);
    bool parseElectrodeData(const QByteArray &data, QVector<ElectrodeData> &electrodes);
    
    // Traitement des données
    void processRawData(const QByteArray &rawData);
    void detectSpikes(const QVector<ElectrodeData> &data);
    bool validateDataIntegrity(const QVector<ElectrodeData> &data);
    
    // Calibration
    void performImpedanceTest(int electrodeId);
    void updateCalibrationFactors();
    
    // Utilitaires
    void setStatus(ConnectionStatus status);
    void setError(const QString &error);
    QByteArray formatStimulationCommand(int electrodeId, double amplitude, double duration);
    
    // Méthodes utilitaires pour apprentissage Bitcoin
    double calculateSpikeConsistency(const QVector<SpikeEvent> &spikes);
    double calculateResponseStrength(const QVector<SpikeEvent> &spikes);
    
    // Méthodes de configuration et test
    bool configureDevice();
    bool testCommunication();
    bool parseBinaryElectrodeData(const QByteArray &data, QVector<ElectrodeData> &electrodes);
    
    // Configuration
    RealMEAConfig m_config;
    ConnectionStatus m_status;
    QString m_lastError;
    
    // Connexions hardware
    std::unique_ptr<QSerialPort> m_serialPort;
    std::unique_ptr<QTcpSocket> m_tcpSocket;
    std::unique_ptr<QUdpSocket> m_udpSocket;
    std::unique_ptr<QProcess> m_customProcess;
    
    // Données en temps réel
    QVector<ElectrodeData> m_currentData;
    QVector<double> m_electrodeImpedances;
    QVector<SpikeEvent> m_recentSpikes;
    QVector<bool> m_activeElectrodes;
    
    // Timers et acquisition
    std::unique_ptr<QTimer> m_acquisitionTimer;
    std::unique_ptr<QTimer> m_calibrationTimer;
    bool m_continuousMode;
    
    // Calibration
    bool m_calibrationInProgress;
    int m_currentCalibrationElectrode;
    QVector<double> m_calibrationFactors;
    
    // Détection de spikes
    bool m_spikeDetectionEnabled;
    double m_spikeThreshold;
    int m_spikeWindowSize;
    
    // Buffer de données
    QByteArray m_dataBuffer;
    qint64 m_lastTimestamp;
    
    // Thread safety
    mutable QMutex m_dataMutex;
    mutable QMutex m_configMutex;
    mutable QMutex m_bitcoinLearningMutex;
    
    // Apprentissage Bitcoin
    BitcoinLearningConfig m_bitcoinConfig;
    BiologicalNeuralModel m_neuralModel;
    BitcoinLearningStats m_learningStats;
    bool m_bitcoinLearningEnabled;
    std::mt19937 m_randomGenerator;
    
    // Constantes
    static constexpr int DEFAULT_BUFFER_SIZE = 4096;
    static constexpr int MAX_SPIKE_BUFFER = 1000;
    static constexpr double MIN_SIGNAL_QUALITY = 0.1;
    static constexpr double MAX_IMPEDANCE_MOHM = 10.0;
};

/**
 * @brief Factory pour créer différents types d'interfaces MEA
 */
class MEAInterfaceFactory
{
public:
    static std::unique_ptr<RealMEAInterface> createInterface(MEADeviceType deviceType);
    static QStringList getSupportedDevices();
    static RealMEAConfig getDefaultConfig(MEADeviceType deviceType);
    static bool isDeviceAvailable(MEADeviceType deviceType, const QString &devicePath = "");
};

} // namespace Bio
} // namespace BioMining

Q_DECLARE_METATYPE(BioMining::Bio::ElectrodeData)
Q_DECLARE_METATYPE(BioMining::Bio::SpikeEvent)
Q_DECLARE_METATYPE(BioMining::Bio::BitcoinLearningPattern)
Q_DECLARE_METATYPE(BioMining::Bio::NeuralBitcoinResponse)
Q_DECLARE_METATYPE(BioMining::Bio::BiologicalNeuralModel)
Q_DECLARE_METATYPE(BioMining::Bio::BitcoinLearningConfig)
Q_DECLARE_METATYPE(BioMining::Bio::BitcoinLearningStats)
Q_DECLARE_METATYPE(BioMining::Bio::RealMEAInterface::ConnectionStatus)

#endif // REAL_MEA_INTERFACE_H