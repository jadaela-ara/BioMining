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
#include <memory>

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
Q_DECLARE_METATYPE(BioMining::Bio::RealMEAInterface::ConnectionStatus)

#endif // REAL_MEA_INTERFACE_H