#include "bio/real_mea_interface.h"
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QElapsedTimer>
#include <QThread>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <cmath>

namespace BioMining {
namespace Bio {

RealMEAInterface::RealMEAInterface(QObject *parent)
    : QObject(parent)
    , m_status(ConnectionStatus::Disconnected)
    , m_serialPort(nullptr)
    , m_tcpSocket(nullptr)
    , m_udpSocket(nullptr)
    , m_customProcess(nullptr)
    , m_acquisitionTimer(std::make_unique<QTimer>(this))
    , m_calibrationTimer(std::make_unique<QTimer>(this))
    , m_continuousMode(false)
    , m_calibrationInProgress(false)
    , m_currentCalibrationElectrode(0)
    , m_spikeDetectionEnabled(false)
    , m_spikeThreshold(-50.0)
    , m_spikeWindowSize(2)
    , m_lastTimestamp(0)
{
    connect(m_acquisitionTimer.get(), &QTimer::timeout,
            this, &RealMEAInterface::onAcquisitionTimer);
    connect(m_calibrationTimer.get(), &QTimer::timeout,
            this, &RealMEAInterface::onCalibrationTimer);
    
    qDebug() << "[RealMEA] Interface initialisée";
}

RealMEAInterface::~RealMEAInterface()
{
    if (m_status == ConnectionStatus::Connected || m_status == ConnectionStatus::Streaming) {
        disconnect();
    }
}

bool RealMEAInterface::initialize(const RealMEAConfig &config)
{
    QMutexLocker locker(&m_configMutex);
    
    m_config = config;
    setStatus(ConnectionStatus::Connecting);
    
    try {
        qDebug() << "[RealMEA] Initialisation avec dispositif:" 
                 << static_cast<int>(config.deviceType);
        
        // Initialiser les structures de données
        m_currentData.resize(config.electrodeCount);
        m_electrodeImpedances.resize(config.electrodeCount);
        m_activeElectrodes.resize(config.electrodeCount);
        m_calibrationFactors.resize(config.electrodeCount);
        
        // Initialiser les électrodes actives
        if (config.activeElectrodes.isEmpty()) {
            // Toutes les électrodes actives par défaut
            for (int i = 0; i < config.electrodeCount; ++i) {
                m_activeElectrodes[i] = true;
            }
        } else {
            // Utiliser la configuration spécifiée
            m_activeElectrodes.fill(false);
            for (int electrodeId : config.activeElectrodes) {
                if (electrodeId >= 0 && electrodeId < config.electrodeCount) {
                    m_activeElectrodes[electrodeId] = true;
                }
            }
        }
        
        // Initialiser les facteurs de calibration
        m_calibrationFactors.fill(1.0);
        
        // Connexion selon le protocole
        bool connected = false;
        switch (config.protocol) {
            case CommunicationProtocol::SerialPort:
                connected = connectSerial();
                break;
            case CommunicationProtocol::TCP:
            case CommunicationProtocol::UDP:
                connected = connectNetwork();
                break;
            case CommunicationProtocol::SharedMemory:
                connected = connectSharedMemory();
                break;
            case CommunicationProtocol::Custom_API:
                connected = connectCustomAPI();
                break;
            default:
                setError("Protocole de communication non supporté");
                return false;
        }
        
        if (!connected) {
            setStatus(ConnectionStatus::Error);
            return false;
        }
        
        // Configuration du dispositif
        if (!configureDevice()) {
            setError("Échec de configuration du dispositif");
            setStatus(ConnectionStatus::Error);
            return false;
        }
        
        // Calibration automatique si demandée
        if (config.autoCalibration) {
            setStatus(ConnectionStatus::Calibrating);
            if (!performFullCalibration()) {
                qWarning() << "[RealMEA] Calibration automatique échouée, continuant avec les valeurs par défaut";
            }
        }
        
        setStatus(ConnectionStatus::Connected);
        qDebug() << "[RealMEA] Connexion établie avec" << config.electrodeCount << "électrodes";
        
        return true;
        
    } catch (const std::exception &e) {
        setError(QString("Erreur d'initialisation: %1").arg(e.what()));
        setStatus(ConnectionStatus::Error);
        return false;
    }
}

void RealMEAInterface::disconnect()
{
    QMutexLocker locker(&m_configMutex);
    
    stopContinuousAcquisition();
    
    // Fermer les connexions
    if (m_serialPort) {
        m_serialPort->close();
        m_serialPort.reset();
    }
    
    if (m_tcpSocket) {
        m_tcpSocket->close();
        m_tcpSocket.reset();
    }
    
    if (m_udpSocket) {
        m_udpSocket->close();
        m_udpSocket.reset();
    }
    
    if (m_customProcess) {
        m_customProcess->terminate();
        if (!m_customProcess->waitForFinished(3000)) {
            m_customProcess->kill();
        }
        m_customProcess.reset();
    }
    
    setStatus(ConnectionStatus::Disconnected);
    emit deviceDisconnected();
    qDebug() << "[RealMEA] Déconnecté";
}

bool RealMEAInterface::connectSerial()
{
    try {
        m_serialPort = std::make_unique<QSerialPort>(this);
        
        m_serialPort->setPortName(m_config.devicePath);
        m_serialPort->setBaudRate(m_config.baudRate);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        
        if (!m_serialPort->open(QIODevice::ReadWrite)) {
            setError(QString("Impossible d'ouvrir le port série %1: %2")
                    .arg(m_config.devicePath)
                    .arg(m_serialPort->errorString()));
            return false;
        }
        
        connect(m_serialPort.get(), &QSerialPort::readyRead,
                this, &RealMEAInterface::onSerialDataReady);
        
        // Test de communication
        if (!testCommunication()) {
            setError("Test de communication échoué");
            return false;
        }
        
        qDebug() << "[RealMEA] Connexion série établie sur" << m_config.devicePath;
        return true;
        
    } catch (const std::exception &e) {
        setError(QString("Erreur de connexion série: %1").arg(e.what()));
        return false;
    }
}

bool RealMEAInterface::connectNetwork()
{
    try {
        if (m_config.protocol == CommunicationProtocol::TCP) {
            m_tcpSocket = std::make_unique<QTcpSocket>(this);
            
            connect(m_tcpSocket.get(), &QTcpSocket::readyRead,
                    this, &RealMEAInterface::onNetworkDataReady);
            connect(m_tcpSocket.get(), &QTcpSocket::disconnected,
                    this, &RealMEAInterface::deviceDisconnected);
            
            m_tcpSocket->connectToHost(m_config.networkHost, m_config.networkPort);
            
            if (!m_tcpSocket->waitForConnected(m_config.connectionTimeoutMs)) {
                setError(QString("Impossible de se connecter à %1:%2 - %3")
                        .arg(m_config.networkHost)
                        .arg(m_config.networkPort)
                        .arg(m_tcpSocket->errorString()));
                return false;
            }
            
            qDebug() << "[RealMEA] Connexion TCP établie à" 
                     << m_config.networkHost << ":" << m_config.networkPort;
            
        } else if (m_config.protocol == CommunicationProtocol::UDP) {
            m_udpSocket = std::make_unique<QUdpSocket>(this);
            
            connect(m_udpSocket.get(), &QUdpSocket::readyRead,
                    this, &RealMEAInterface::onNetworkDataReady);
            
            if (!m_udpSocket->bind(QHostAddress::Any, m_config.networkPort)) {
                setError(QString("Impossible de lier le port UDP %1").arg(m_config.networkPort));
                return false;
            }
            
            qDebug() << "[RealMEA] Socket UDP lié au port" << m_config.networkPort;
        }
        
        return true;
        
    } catch (const std::exception &e) {
        setError(QString("Erreur de connexion réseau: %1").arg(e.what()));
        return false;
    }
}

bool RealMEAInterface::connectSharedMemory()
{
    // Implémentation pour mémoire partagée
    // Cette méthode dépend du système d'exploitation et de l'API spécifique
    qDebug() << "[RealMEA] Connexion mémoire partagée non implémentée dans cette version";
    setError("Mémoire partagée non supportée dans cette version");
    return false;
}

bool RealMEAInterface::connectCustomAPI()
{
    // Implémentation pour API personnalisée
    // Cette méthode dépend de l'API spécifique du fabricant
    
    try {
        // Exemple d'utilisation d'un processus externe pour communiquer avec l'API
        m_customProcess = std::make_unique<QProcess>(this);
        
        QString program;
        QStringList arguments;
        
        // Configuration selon le type de dispositif
        switch (m_config.deviceType) {
            case MEADeviceType::MultiChannelSystems_MCS:
                program = "mcs_interface";
                arguments << "--device" << m_config.devicePath
                         << "--electrodes" << QString::number(m_config.electrodeCount)
                         << "--sampling" << QString::number(m_config.samplingRate);
                break;
                
            case MEADeviceType::Blackrock_CerePlex:
                program = "cereplex_interface";
                arguments << "--config" << m_config.devicePath;
                break;
                
            case MEADeviceType::Plexon_OmniPlex:
                program = "omniplex_interface";
                arguments << "--host" << m_config.networkHost
                         << "--port" << QString::number(m_config.networkPort);
                break;
                
            case MEADeviceType::OpenEphys_ONIX:
                program = "openephys_interface";
                arguments << "--probe" << m_config.devicePath;
                break;
                
            default:
                program = "generic_mea_interface";
                arguments << "--device" << m_config.devicePath;
                break;
        }
        
        connect(m_customProcess.get(), 
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
                    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                        setError("Processus MEA terminé de manière inattendue");
                        setStatus(ConnectionStatus::Error);
                    }
                });
        
        m_customProcess->start(program, arguments);
        
        if (!m_customProcess->waitForStarted(m_config.connectionTimeoutMs)) {
            setError(QString("Impossible de démarrer %1: %2")
                    .arg(program)
                    .arg(m_customProcess->errorString()));
            return false;
        }
        
        // Attendre la réponse d'initialisation
        if (!m_customProcess->waitForReadyRead(m_config.connectionTimeoutMs)) {
            setError("Timeout lors de l'initialisation de l'API personnalisée");
            return false;
        }
        
        QByteArray response = m_customProcess->readAll();
        if (!response.contains("READY")) {
            setError(QString("Réponse d'initialisation inattendue: %1").arg(QString::fromUtf8(response)));
            return false;
        }
        
        qDebug() << "[RealMEA] API personnalisée initialisée:" << program;
        return true;
        
    } catch (const std::exception &e) {
        setError(QString("Erreur d'API personnalisée: %1").arg(e.what()));
        return false;
    }
}

bool RealMEAInterface::configureDevice()
{
    // Configuration des paramètres du dispositif
    QByteArray configCommand;
    
    // Construire la commande de configuration selon le protocole
    switch (m_config.deviceType) {
        case MEADeviceType::Custom_Serial: {
            // Format de commande pour dispositif série personnalisé
            QJsonObject configObj;
            configObj["electrodes"] = m_config.electrodeCount;
            configObj["sampling_rate"] = m_config.samplingRate;
            configObj["amplification"] = m_config.amplification;
            configObj["filter_low"] = m_config.filterLowCut;
            configObj["filter_high"] = m_config.filterHighCut;
            configObj["buffer_size"] = m_config.bufferSize;
            
            QJsonDocument doc(configObj);
            configCommand = "CONFIG " + doc.toJson(QJsonDocument::Compact) + "\r\n";
            break;
        }
        
        case MEADeviceType::MultiChannelSystems_MCS:
            // Configuration spécifique MCS
            configCommand = QString("SET ELECTRODES %1\r\nSET SAMPLING %2\r\nSET GAIN %3\r\n")
                          .arg(m_config.electrodeCount)
                          .arg(m_config.samplingRate)
                          .arg(m_config.amplification)
                          .toUtf8();
            break;
            
        default:
            // Configuration générique
            configCommand = QString("INIT %1 %2 %3\r\n")
                          .arg(m_config.electrodeCount)
                          .arg(m_config.samplingRate)
                          .arg(m_config.amplification)
                          .toUtf8();
            break;
    }
    
    // Envoyer la commande de configuration
    if (!sendCommand(configCommand)) {
        return false;
    }
    
    // Attendre la confirmation
    QByteArray response = receiveResponse(m_config.readTimeoutMs);
    if (response.isEmpty() || !response.contains("OK")) {
        setError(QString("Configuration échouée: %1").arg(QString::fromUtf8(response)));
        return false;
    }
    
    qDebug() << "[RealMEA] Dispositif configuré";
    return true;
}

bool RealMEAInterface::testCommunication()
{
    // Test simple de communication
    QByteArray testCommand = "PING\r\n";
    
    if (!sendCommand(testCommand)) {
        return false;
    }
    
    QByteArray response = receiveResponse(m_config.readTimeoutMs);
    if (response.isEmpty() || !response.contains("PONG")) {
        setError(QString("Test de communication échoué: %1").arg(QString::fromUtf8(response)));
        return false;
    }
    
    qDebug() << "[RealMEA] Test de communication réussi";
    return true;
}

QVector<ElectrodeData> RealMEAInterface::readElectrodeData()
{
    QMutexLocker locker(&m_dataMutex);
    
    if (m_status != ConnectionStatus::Connected && m_status != ConnectionStatus::Streaming) {
        setError("MEA non connecté");
        return QVector<ElectrodeData>();
    }
    
    // Demander les données actuelles
    if (!sendCommand("READ\r\n")) {
        return QVector<ElectrodeData>();
    }
    
    QByteArray response = receiveResponse(m_config.readTimeoutMs);
    if (response.isEmpty()) {
        setError("Timeout lors de la lecture des données");
        return QVector<ElectrodeData>();
    }
    
    QVector<ElectrodeData> data;
    if (!parseElectrodeData(response, data)) {
        setError("Erreur de parsing des données d'électrodes");
        return QVector<ElectrodeData>();
    }
    
    // Mettre à jour les données courantes
    m_currentData = data;
    
    // Détecter les spikes si activé
    if (m_spikeDetectionEnabled) {
        detectSpikes(data);
    }
    
    return data;
}

QVector<double> RealMEAInterface::readRawSignals()
{
    QVector<ElectrodeData> data = readElectrodeData();
    QVector<double> signalData;
    
    signalData.reserve(data.size());
    for (const ElectrodeData &electrode : data) {
        signalData.append(electrode.voltage);
    }
    
    return signalData;
}

bool RealMEAInterface::startContinuousAcquisition()
{
    if (m_status != ConnectionStatus::Connected) {
        setError("MEA non connecté");
        return false;
    }
    
    // Démarrer l'acquisition continue sur le dispositif
    if (!sendCommand("START\r\n")) {
        return false;
    }
    
    QByteArray response = receiveResponse(m_config.readTimeoutMs);
    if (response.isEmpty() || !response.contains("STARTED")) {
        setError("Impossible de démarrer l'acquisition continue");
        return false;
    }
    
    // Démarrer le timer local
    int intervalMs = static_cast<int>(1000.0 / m_config.samplingRate * m_config.bufferSize);
    m_acquisitionTimer->start(intervalMs);
    m_continuousMode = true;
    
    setStatus(ConnectionStatus::Streaming);
    qDebug() << "[RealMEA] Acquisition continue démarrée";
    
    return true;
}

void RealMEAInterface::stopContinuousAcquisition()
{
    if (m_continuousMode) {
        m_acquisitionTimer->stop();
        m_continuousMode = false;
        
        // Arrêter l'acquisition sur le dispositif
        sendCommand("STOP\r\n");
        
        if (m_status == ConnectionStatus::Streaming) {
            setStatus(ConnectionStatus::Connected);
        }
        
        qDebug() << "[RealMEA] Acquisition continue arrêtée";
    }
}

bool RealMEAInterface::stimulateElectrode(int electrodeId, double amplitude, double duration)
{
    if (m_status != ConnectionStatus::Connected && m_status != ConnectionStatus::Streaming) {
        setError("MEA non connecté");
        return false;
    }
    
    if (electrodeId < 0 || electrodeId >= m_config.electrodeCount) {
        setError(QString("ID d'électrode invalide: %1").arg(electrodeId));
        return false;
    }
    
    if (!m_activeElectrodes[electrodeId]) {
        setError(QString("Électrode %1 non active").arg(electrodeId));
        return false;
    }
    
    // Vérifier les limites de sécurité
    if (std::abs(amplitude) > m_config.stimMaxVoltage) {
        setError(QString("Amplitude de stimulation trop élevée: %1V (max: %2V)")
                .arg(amplitude).arg(m_config.stimMaxVoltage));
        return false;
    }
    
    QByteArray stimCommand = formatStimulationCommand(electrodeId, amplitude, duration);
    
    if (!sendCommand(stimCommand)) {
        return false;
    }
    
    QByteArray response = receiveResponse(m_config.readTimeoutMs);
    if (response.isEmpty() || !response.contains("STIM_OK")) {
        setError(QString("Stimulation échouée: %1").arg(QString::fromUtf8(response)));
        return false;
    }
    
    emit stimulationComplete(electrodeId);
    qDebug() << "[RealMEA] Stimulation électrode" << electrodeId 
             << "amplitude:" << amplitude << "V, durée:" << duration << "ms";
    
    return true;
}

bool RealMEAInterface::stimulatePattern(const QVector<double> &pattern)
{
    if (pattern.size() != m_config.electrodeCount) {
        setError(QString("Taille du pattern invalide: %1 (attendu: %2)")
                .arg(pattern.size()).arg(m_config.electrodeCount));
        return false;
    }
    
    // Stimuler chaque électrode selon le pattern
    bool allSuccess = true;
    for (int i = 0; i < pattern.size(); ++i) {
        if (m_activeElectrodes[i] && std::abs(pattern[i]) > 0.001) {
            if (!stimulateElectrode(i, pattern[i], 100.0)) { // 100ms par défaut
                allSuccess = false;
            }
        }
    }
    
    return allSuccess;
}

// Méthodes privées pour la gestion des données et signaux

void RealMEAInterface::onSerialDataReady()
{
    if (!m_serialPort) return;
    
    QByteArray data = m_serialPort->readAll();
    m_dataBuffer.append(data);
    
    processRawData(m_dataBuffer);
}

void RealMEAInterface::onNetworkDataReady()
{
    QByteArray data;
    
    if (m_tcpSocket && m_tcpSocket->bytesAvailable() > 0) {
        data = m_tcpSocket->readAll();
    } else if (m_udpSocket && m_udpSocket->hasPendingDatagrams()) {
        data.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(data.data(), data.size());
    }
    
    if (!data.isEmpty()) {
        m_dataBuffer.append(data);
        processRawData(m_dataBuffer);
    }
}

void RealMEAInterface::onAcquisitionTimer()
{
    QVector<ElectrodeData> data = readElectrodeData();
    if (!data.isEmpty()) {
        emit electrodeDataReady(data);
        
        // Émettre aussi le signal de compatibilité
        QVector<double> rawSignals;
        rawSignals.reserve(data.size());
        for (const ElectrodeData &electrode : data) {
            rawSignals.append(electrode.voltage);
        }
        emit rawSignalsReady(rawSignals);
    }
}

void RealMEAInterface::onCalibrationTimer()
{
    if (m_calibrationInProgress && m_currentCalibrationElectrode < m_config.electrodeCount) {
        performImpedanceTest(m_currentCalibrationElectrode);
        
        double progress = static_cast<double>(m_currentCalibrationElectrode) / m_config.electrodeCount;
        emit calibrationProgress(m_currentCalibrationElectrode, progress);
        
        ++m_currentCalibrationElectrode;
        
        if (m_currentCalibrationElectrode >= m_config.electrodeCount) {
            m_calibrationTimer->stop();
            m_calibrationInProgress = false;
            updateCalibrationFactors();
            emit calibrationComplete();
            qDebug() << "[RealMEA] Calibration complète terminée";
        }
    }
}

bool RealMEAInterface::sendCommand(const QByteArray &command)
{
    try {
        if (m_serialPort && m_serialPort->isOpen()) {
            qint64 written = m_serialPort->write(command);
            m_serialPort->flush();
            return written == command.size();
        } else if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
            qint64 written = m_tcpSocket->write(command);
            m_tcpSocket->flush();
            return written == command.size();
        } else if (m_customProcess && m_customProcess->state() == QProcess::Running) {
            qint64 written = m_customProcess->write(command);
            return written == command.size();
        }
        
        setError("Aucune connexion active pour envoyer la commande");
        return false;
        
    } catch (const std::exception &e) {
        setError(QString("Erreur d'envoi de commande: %1").arg(e.what()));
        return false;
    }
}

QByteArray RealMEAInterface::receiveResponse(int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    
    QByteArray response;
    
    while (timer.elapsed() < timeoutMs) {
        if (m_serialPort && m_serialPort->isOpen()) {
            if (m_serialPort->waitForReadyRead(100)) {
                response.append(m_serialPort->readAll());
                if (response.contains('\n')) break;
            }
        } else if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
            if (m_tcpSocket->waitForReadyRead(100)) {
                response.append(m_tcpSocket->readAll());
                if (response.contains('\n')) break;
            }
        } else if (m_customProcess && m_customProcess->state() == QProcess::Running) {
            if (m_customProcess->waitForReadyRead(100)) {
                response.append(m_customProcess->readAll());
                if (response.contains('\n')) break;
            }
        }
        
        QCoreApplication::processEvents();
    }
    
    return response.trimmed();
}

void RealMEAInterface::setStatus(ConnectionStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void RealMEAInterface::setError(const QString &error)
{
    m_lastError = error;
    qWarning() << "[RealMEA] Erreur:" << error;
    emit errorOccurred(error);
}

// Implémentation des autres méthodes (parseElectrodeData, detectSpikes, etc.)
// Ces méthodes seraient spécifiques au format de données de chaque dispositif MEA

bool RealMEAInterface::parseElectrodeData(const QByteArray &data, QVector<ElectrodeData> &electrodes)
{
    // Exemple d'implémentation pour format JSON
    // Le format réel dépendrait du dispositif MEA spécifique
    
    try {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            // Essayer un format binaire ou texte simple
            return parseBinaryElectrodeData(data, electrodes);
        }
        
        QJsonObject obj = doc.object();
        QJsonArray electrodeArray = obj["electrodes"].toArray();
        
        electrodes.clear();
        electrodes.reserve(electrodeArray.size());
        
        for (int i = 0; i < electrodeArray.size(); ++i) {
            QJsonObject electrodeObj = electrodeArray[i].toObject();
            
            ElectrodeData electrode;
            electrode.electrodeId = electrodeObj["id"].toInt();
            electrode.voltage = electrodeObj["voltage"].toDouble();
            electrode.current = electrodeObj["current"].toDouble();
            electrode.impedance = electrodeObj["impedance"].toDouble();
            electrode.isActive = electrodeObj["active"].toBool();
            electrode.signalQuality = electrodeObj["quality"].toDouble();
            electrode.timestamp = QDateTime::currentMSecsSinceEpoch() * 1000; // Convert to μs
            
            electrodes.append(electrode);
        }
        
        return true;
        
    } catch (const std::exception &e) {
        setError(QString("Erreur de parsing JSON: %1").arg(e.what()));
        return false;
    }
}

bool RealMEAInterface::parseBinaryElectrodeData(const QByteArray &data, QVector<ElectrodeData> &electrodes)
{
    // Format binaire hypothétique : 
    // Header (4 bytes): nombre d'électrodes
    // Pour chaque électrode (20 bytes):
    //   - ID (4 bytes int)
    //   - Voltage (8 bytes double) 
    //   - Timestamp (8 bytes qint64)
    
    if (data.size() < 4) {
        return false;
    }
    
    const char* ptr = data.constData();
    int numElectrodes = *reinterpret_cast<const int*>(ptr);
    ptr += 4;
    
    if (numElectrodes <= 0 || numElectrodes > 1000 ||
        data.size() != 4 + numElectrodes * 20) {
        return false;
    }
    
    electrodes.clear();
    electrodes.reserve(numElectrodes);
    
    for (int i = 0; i < numElectrodes; ++i) {
        ElectrodeData electrode;
        electrode.electrodeId = *reinterpret_cast<const int*>(ptr);
        ptr += 4;
        electrode.voltage = *reinterpret_cast<const double*>(ptr);
        ptr += 8;
        electrode.timestamp = *reinterpret_cast<const qint64*>(ptr);
        ptr += 8;
        
        // Valeurs par défaut pour les autres champs
        electrode.current = 0.0;
        electrode.impedance = 1.0e6; // 1 MΩ par défaut
        electrode.isActive = true;
        electrode.signalQuality = 0.8;
        
        electrodes.append(electrode);
    }
    
    return true;
}

void RealMEAInterface::detectSpikes(const QVector<ElectrodeData> &data)
{
    for (const ElectrodeData &electrode : data) {
        // Détection de spike simple par seuil
        if (electrode.voltage < m_spikeThreshold) {
            SpikeEvent spike;
            spike.electrodeId = electrode.electrodeId;
            spike.amplitude = electrode.voltage;
            spike.timestamp = electrode.timestamp;
            
            // Pour une détection plus sophistiquée, on capturerait
            // la forme d'onde complète autour du pic
            spike.waveform.resize(m_spikeWindowSize);
            // Remplir avec des données réelles ou interpolées
            
            m_recentSpikes.append(spike);
            
            // Limiter le buffer de spikes
            if (m_recentSpikes.size() > MAX_SPIKE_BUFFER) {
                m_recentSpikes.removeFirst();
            }
            
            emit spikeDetected(spike);
        }
    }
}

QByteArray RealMEAInterface::formatStimulationCommand(int electrodeId, double amplitude, double duration)
{
    // Format dépendant du dispositif
    switch (m_config.deviceType) {
        case MEADeviceType::Custom_Serial:
            return QString("STIM %1 %2 %3\r\n")
                  .arg(electrodeId)
                  .arg(amplitude)
                  .arg(duration)
                  .toUtf8();
            
        case MEADeviceType::MultiChannelSystems_MCS:
            return QString("STIMULATE ELECTRODE %1 VOLTAGE %2 DURATION %3\r\n")
                  .arg(electrodeId)
                  .arg(amplitude)
                  .arg(duration)
                  .toUtf8();
            
        default:
            // Format JSON générique
            QJsonObject obj;
            obj["command"] = "stimulate";
            obj["electrode"] = electrodeId;
            obj["amplitude"] = amplitude;
            obj["duration"] = duration;
            
            QJsonDocument doc(obj);
            return doc.toJson(QJsonDocument::Compact) + "\r\n";
    }
}

// Factory pour créer différents types d'interfaces
std::unique_ptr<RealMEAInterface> MEAInterfaceFactory::createInterface(MEADeviceType deviceType)
{
    auto interface = std::make_unique<RealMEAInterface>();
    
    RealMEAConfig config = getDefaultConfig(deviceType);
    config.deviceType = deviceType;
    
    return interface;
}

QStringList MEAInterfaceFactory::getSupportedDevices()
{
    return {
        "Multi Channel Systems MCS",
        "Alpha Omega AlphaMap", 
        "Blackrock CerePlex",
        "Plexon OmniPlex",
        "Intan RHD Series",
        "Open Ephys ONIX",
        "NeuroNexus SmartBox",
        "Custom Serial Interface",
        "Custom Network Interface",
        "Custom Shared Memory"
    };
}

RealMEAConfig MEAInterfaceFactory::getDefaultConfig(MEADeviceType deviceType)
{
    RealMEAConfig config;
    
    switch (deviceType) {
        case MEADeviceType::MultiChannelSystems_MCS:
            config.protocol = CommunicationProtocol::TCP;
            config.networkHost = "localhost";
            config.networkPort = 3333;
            config.electrodeCount = 60;
            config.samplingRate = 25000.0;
            break;
            
        case MEADeviceType::Blackrock_CerePlex:
            config.protocol = CommunicationProtocol::Custom_API;
            config.devicePath = "/dev/cereplex0";
            config.electrodeCount = 96;
            config.samplingRate = 30000.0;
            break;
            
        case MEADeviceType::OpenEphys_ONIX:
            config.protocol = CommunicationProtocol::Custom_API;
            config.devicePath = "/dev/onix0";
            config.electrodeCount = 64;
            config.samplingRate = 30000.0;
            break;
            
        case MEADeviceType::Custom_Serial:
            config.protocol = CommunicationProtocol::SerialPort;
            config.devicePath = "/dev/ttyUSB0";
            config.baudRate = 115200;
            config.electrodeCount = 60;
            config.samplingRate = 25000.0;
            break;
            
        default:
            // Configuration par défaut
            config.protocol = CommunicationProtocol::SerialPort;
            config.devicePath = "/dev/ttyUSB0";
            config.baudRate = 115200;
            config.electrodeCount = 60;
            config.samplingRate = 25000.0;
            break;
    }
    
    return config;
}

bool MEAInterfaceFactory::isDeviceAvailable(MEADeviceType deviceType, const QString &devicePath)
{
    switch (deviceType) {
        case MEADeviceType::Custom_Serial: {
            QSerialPort testPort;
            testPort.setPortName(devicePath.isEmpty() ? "/dev/ttyUSB0" : devicePath);
            bool available = testPort.open(QIODevice::ReadWrite);
            if (available) {
                testPort.close();
            }
            return available;
        }
        
        default:
            // Pour les autres types, on pourrait vérifier l'existence de fichiers,
            // la présence de pilotes, etc.
            return QFile::exists(devicePath);
    }
}

bool RealMEAInterface::performFullCalibration()
{
    qDebug() << "[RealMEA] Début de la calibration complète";
    
    QMutexLocker locker(&m_configMutex);
    
    m_calibrationInProgress = true;
    m_currentCalibrationElectrode = 0;
    m_calibrationFactors.clear();
    m_calibrationFactors.resize(m_config.electrodeCount, 1.0);
    
    setStatus(ConnectionStatus::Calibrating);
    
    // Lancer le timer de calibration
    m_calibrationTimer->start(1000); // 1 seconde par électrode
    
    emit calibrationProgress(0, 0.0);
    
    return true;
}

void RealMEAInterface::processRawData(const QByteArray &rawData)
{
    if (rawData.isEmpty()) {
        return;
    }
    
    QVector<ElectrodeData> electrodeData;
    
    // Parser les données selon le protocole configuré
    if (parseElectrodeData(rawData, electrodeData)) {
        QMutexLocker locker(&m_dataMutex);
        
        // Mettre à jour les données courantes
        m_currentData = electrodeData;
        m_lastTimestamp = QDateTime::currentMSecsSinceEpoch() * 1000; // en microsecondes
        
        // Détecter les spikes si activé
        if (m_spikeDetectionEnabled) {
            detectSpikes(electrodeData);
        }
        
        // Émettre les signaux de données
        emit electrodeDataReady(electrodeData);
        
        // Convertir en signaux bruts pour compatibilité
        QVector<double> rawSignals;
        rawSignals.reserve(electrodeData.size());
        for (const ElectrodeData &electrode : electrodeData) {
            rawSignals.append(electrode.voltage);
        }
        emit rawSignalsReady(rawSignals);
    }
}

void RealMEAInterface::performImpedanceTest(int electrodeId)
{
    if (electrodeId < 0 || electrodeId >= m_config.electrodeCount) {
        qWarning() << "[RealMEA] ID électrode invalide pour test d'impédance:" << electrodeId;
        return;
    }
    
    // Simuler un test d'impédance basique
    // Dans une vraie implémentation, on enverrait des commandes spécifiques au hardware
    
    double baseImpedance = 1000.0; // 1 kΩ de base
    double variation = (QRandomGenerator::global()->bounded(200) - 100) / 100.0; // ±100 Ω
    double impedance = baseImpedance + variation;
    
    // Vérifier si l'impédance est dans les limites acceptables
    bool isValid = (impedance < MAX_IMPEDANCE_MOHM * 1000000.0); // Convertir MΩ en Ω
    
    if (electrodeId < m_electrodeImpedances.size()) {
        m_electrodeImpedances[electrodeId] = impedance;
    } else {
        m_electrodeImpedances.resize(electrodeId + 1, impedance);
    }
    
    qDebug() << QString("[RealMEA] Électrode %1: Impédance = %2 Ω (%3)")
                .arg(electrodeId)
                .arg(impedance, 0, 'f', 1)
                .arg(isValid ? "OK" : "HORS LIMITES");
}

void RealMEAInterface::updateCalibrationFactors()
{
    // Calculer les facteurs de calibration basés sur les tests d'impédance
    if (m_electrodeImpedances.isEmpty()) {
        qWarning() << "[RealMEA] Aucune donnée d'impédance pour calculer les facteurs de calibration";
        return;
    }
    
    // Facteur de référence basé sur l'impédance moyenne
    double meanImpedance = 0.0;
    int validElectrodes = 0;
    
    for (double impedance : m_electrodeImpedances) {
        if (impedance > 0 && impedance < MAX_IMPEDANCE_MOHM * 1000000.0) {
            meanImpedance += impedance;
            validElectrodes++;
        }
    }
    
    if (validElectrodes == 0) {
        qWarning() << "[RealMEA] Aucune électrode valide pour la calibration";
        return;
    }
    
    meanImpedance /= validElectrodes;
    
    // Calculer les facteurs de correction individuels
    for (int i = 0; i < m_electrodeImpedances.size() && i < m_calibrationFactors.size(); ++i) {
        if (m_electrodeImpedances[i] > 0) {
            m_calibrationFactors[i] = meanImpedance / m_electrodeImpedances[i];
            // Limiter les facteurs à une plage raisonnable
            m_calibrationFactors[i] = qBound(0.5, m_calibrationFactors[i], 2.0);
        }
    }
    
    qDebug() << QString("[RealMEA] Facteurs de calibration mis à jour (impédance moyenne: %1 Ω)")
                .arg(meanImpedance, 0, 'f', 1);
}

} // namespace Bio
} // namespace BioMining