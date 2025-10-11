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
#include <algorithm>

namespace BioMining {
namespace Bio {

RealMEAInterface::RealMEAInterface(QObject *parent)
    : QObject(parent)
    , m_status(ConnectionStatus::Disconnected)
    , m_continuousMode(false)
    , m_calibrationInProgress(false)
    , m_currentCalibrationElectrode(0)
    , m_spikeDetectionEnabled(true)
    , m_spikeThreshold(-50.0)
    , m_spikeWindowSize(2)
    , m_lastTimestamp(0)
    , m_bitcoinLearningEnabled(false)
    , m_randomGenerator(std::random_device{}())
{
    // Initialize timers
    m_acquisitionTimer = std::make_unique<QTimer>(this);
    m_calibrationTimer = std::make_unique<QTimer>(this);
    
    // Connect timer signals
    connect(m_acquisitionTimer.get(), &QTimer::timeout, this, &RealMEAInterface::onAcquisitionTimer);
    connect(m_calibrationTimer.get(), &QTimer::timeout, this, &RealMEAInterface::onCalibrationTimer);
    
    // Initialize default configuration
    m_config = RealMEAConfig{};
    m_config.electrodeCount = 60;
    m_config.samplingRate = 25000.0;
    m_config.amplification = 1200.0;
    m_config.filterLowCut = 300.0;
    m_config.filterHighCut = 8000.0;
    m_config.bufferSize = 1024;
    
    // Initialize electrode data
    m_currentData.resize(m_config.electrodeCount);
    m_electrodeImpedances.resize(m_config.electrodeCount);
    m_activeElectrodes.resize(m_config.electrodeCount);
    m_calibrationFactors.resize(m_config.electrodeCount);
    
    // Initialize all electrodes as active
    std::fill(m_activeElectrodes.begin(), m_activeElectrodes.end(), true);
    std::fill(m_electrodeImpedances.begin(), m_electrodeImpedances.end(), 1000.0);
    std::fill(m_calibrationFactors.begin(), m_calibrationFactors.end(), 1.0);
    
    // Initialize electrode data
    for (int i = 0; i < m_config.electrodeCount; ++i) {
        m_currentData[i].electrodeId = i;
        m_currentData[i].voltage = 0.0;
        m_currentData[i].current = 0.0;
        m_currentData[i].impedance = 1000.0;
        m_currentData[i].isActive = true;
        m_currentData[i].signalQuality = 1.0;
        m_currentData[i].timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
    }
    
    qDebug() << "[RealMEA] Interface initialized with" << m_config.electrodeCount << "electrodes";
}

RealMEAInterface::~RealMEAInterface()
{
    disconnect();
    qDebug() << "[RealMEA] Interface destroyed";
}

bool RealMEAInterface::initialize(const RealMEAConfig &config)
{
    QMutexLocker locker(&m_configMutex);
    
    qDebug() << "[RealMEA] Initializing with device type:" << static_cast<int>(config.deviceType);
    
    m_config = config;
    
    // Resize arrays based on electrode count
    m_currentData.resize(m_config.electrodeCount);
    m_electrodeImpedances.resize(m_config.electrodeCount);
    m_activeElectrodes.resize(m_config.electrodeCount);
    m_calibrationFactors.resize(m_config.electrodeCount);
    
    // Initialize electrode data
    for (int i = 0; i < m_config.electrodeCount; ++i) {
        m_currentData[i].electrodeId = i;
        m_currentData[i].voltage = 0.0;
        m_currentData[i].current = 0.0;
        m_currentData[i].impedance = 1000.0;
        m_currentData[i].isActive = true;
        m_currentData[i].signalQuality = 1.0;
        m_currentData[i].timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
    }
    
    setStatus(ConnectionStatus::Connecting);
    
    // Attempt connection based on device type
    bool connected = false;
    switch (m_config.deviceType) {
        case MEADeviceType::Custom_Serial:
            connected = connectSerial();
            break;
        case MEADeviceType::Custom_Network:
            connected = connectNetwork();
            break;
        case MEADeviceType::Custom_SharedMemory:
            connected = connectSharedMemory();
            break;
        default:
            connected = connectCustomAPI();
            break;
    }
    
    if (connected) {
        setStatus(ConnectionStatus::Connected);
        qDebug() << "[RealMEA] Successfully connected to device";
        return true;
    } else {
        setStatus(ConnectionStatus::Error);
        setError("Failed to connect to MEA device");
        return false;
    }
}

void RealMEAInterface::disconnect()
{
    QMutexLocker locker(&m_configMutex);
    
    if (m_status == ConnectionStatus::Disconnected) {
        return;
    }
    
    qDebug() << "[RealMEA] Disconnecting from device";
    
    // Stop continuous acquisition
    stopContinuousAcquisition();
    
    // Close connections
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
        m_customProcess.reset();
    }
    
    setStatus(ConnectionStatus::Disconnected);
    emit deviceDisconnected();
}

QString RealMEAInterface::getDeviceInfo() const
{
    QMutexLocker locker(&m_configMutex);
    
    QString info = QString("MEA Device Type: %1\n")
                   .arg(static_cast<int>(m_config.deviceType));
    info += QString("Electrode Count: %1\n").arg(m_config.electrodeCount);
    info += QString("Sampling Rate: %1 Hz\n").arg(m_config.samplingRate);
    info += QString("Amplification: %1x\n").arg(m_config.amplification);
    info += QString("Status: %1\n").arg(static_cast<int>(m_status));
    
    return info;
}

QVector<int> RealMEAInterface::getActiveElectrodes() const
{
    QMutexLocker locker(&m_dataMutex);
    
    QVector<int> active;
    for (int i = 0; i < m_activeElectrodes.size(); ++i) {
        if (m_activeElectrodes[i]) {
            active.append(i);
        }
    }
    return active;
}

double RealMEAInterface::getSamplingRate() const
{
    QMutexLocker locker(&m_configMutex);
    return m_config.samplingRate;
}

QVector<ElectrodeData> RealMEAInterface::readElectrodeData()
{
    QMutexLocker locker(&m_dataMutex);
    
    // Simulate real-time data for demonstration
    for (int i = 0; i < m_currentData.size(); ++i) {
        if (m_activeElectrodes[i]) {
            // Generate realistic neural signal simulation
            double baseSignal = (QRandomGenerator::global()->bounded(200) - 100) / 1000.0; // ±100 μV
            double noise = (QRandomGenerator::global()->bounded(20) - 10) / 1000.0; // ±10 μV noise
            
            m_currentData[i].voltage = baseSignal + noise;
            m_currentData[i].current = m_currentData[i].voltage / m_electrodeImpedances[i];
            m_currentData[i].timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
            m_currentData[i].signalQuality = 0.8 + (QRandomGenerator::global()->bounded(20)) / 100.0; // 0.8-1.0
        }
    }
    
    emit electrodeDataReady(m_currentData);
    return m_currentData;
}

QVector<double> RealMEAInterface::readRawSignals()
{
    QVector<ElectrodeData> data = readElectrodeData();
    QVector<double> signals;
    
    for (const auto &electrode : data) {
        signals.append(electrode.voltage);
    }
    
    emit rawSignalsReady(signals);
    return signals;
}

bool RealMEAInterface::startContinuousAcquisition()
{
    QMutexLocker locker(&m_configMutex);
    
    if (m_status != ConnectionStatus::Connected) {
        setError("Device not connected");
        return false;
    }
    
    if (m_continuousMode) {
        return true; // Already running
    }
    
    qDebug() << "[RealMEA] Starting continuous acquisition";
    
    // Calculate timer interval based on sampling rate
    int intervalMs = static_cast<int>(1000.0 / m_config.samplingRate);
    m_acquisitionTimer->start(intervalMs);
    
    m_continuousMode = true;
    setStatus(ConnectionStatus::Streaming);
    
    return true;
}

void RealMEAInterface::stopContinuousAcquisition()
{
    QMutexLocker locker(&m_configMutex);
    
    if (!m_continuousMode) {
        return;
    }
    
    qDebug() << "[RealMEA] Stopping continuous acquisition";
    
    m_acquisitionTimer->stop();
    m_continuousMode = false;
    
    if (m_status == ConnectionStatus::Streaming) {
        setStatus(ConnectionStatus::Connected);
    }
}

bool RealMEAInterface::stimulateElectrode(int electrodeId, double amplitude, double duration)
{
    QMutexLocker locker(&m_configMutex);
    
    if (electrodeId < 0 || electrodeId >= m_config.electrodeCount) {
        setError("Invalid electrode ID");
        return false;
    }
    
    if (amplitude > m_config.stimMaxVoltage) {
        setError("Stimulation amplitude exceeds maximum");
        return false;
    }
    
    qDebug() << QString("[RealMEA] Stimulating electrode %1: %2V for %3ms")
                .arg(electrodeId)
                .arg(amplitude)
                .arg(duration);
    
    // Simulate stimulation command
    QByteArray command = formatStimulationCommand(electrodeId, amplitude, duration);
    bool success = sendCommand(command);
    
    if (success) {
        // Simulate stimulation completion after duration
        QTimer::singleShot(static_cast<int>(duration), [this, electrodeId]() {
            emit stimulationComplete(electrodeId);
        });
    }
    
    return success;
}

bool RealMEAInterface::stimulatePattern(const QVector<double> &pattern)
{
    QMutexLocker locker(&m_configMutex);
    
    if (pattern.size() != m_config.electrodeCount) {
        setError("Pattern size does not match electrode count");
        return false;
    }
    
    qDebug() << "[RealMEA] Stimulating pattern across all electrodes";
    
    bool allSuccess = true;
    for (int i = 0; i < pattern.size(); ++i) {
        if (pattern[i] != 0.0) {
            bool success = stimulateElectrode(i, pattern[i], 50.0); // 50ms default duration
            if (!success) {
                allSuccess = false;
            }
        }
    }
    
    return allSuccess;
}

void RealMEAInterface::setStimulationParameters(double maxVoltage, double maxCurrent)
{
    QMutexLocker locker(&m_configMutex);
    
    m_config.stimMaxVoltage = maxVoltage;
    m_config.stimMaxCurrent = maxCurrent;
    
    qDebug() << QString("[RealMEA] Stimulation parameters set: %1V max, %2μA max")
                .arg(maxVoltage)
                .arg(maxCurrent);
}

bool RealMEAInterface::performElectrodeImpedanceTest()
{
    QMutexLocker locker(&m_configMutex);
    
    qDebug() << "[RealMEA] Starting electrode impedance test";
    
    for (int i = 0; i < m_config.electrodeCount; ++i) {
        performImpedanceTest(i);
    }
    
    return true;
}

bool RealMEAInterface::calibrateElectrode(int electrodeId)
{
    QMutexLocker locker(&m_configMutex);
    
    if (electrodeId < 0 || electrodeId >= m_config.electrodeCount) {
        setError("Invalid electrode ID for calibration");
        return false;
    }
    
    qDebug() << QString("[RealMEA] Calibrating electrode %1").arg(electrodeId);
    
    // Simulate calibration process
    double calibrationFactor = 0.8 + (QRandomGenerator::global()->bounded(40)) / 100.0; // 0.8-1.2
    m_calibrationFactors[electrodeId] = calibrationFactor;
    
    emit calibrationProgress(electrodeId, 1.0);
    
    return true;
}

bool RealMEAInterface::performFullCalibration()
{
    qDebug() << "[RealMEA] Début de la calibration complète";
    
    QMutexLocker locker(&m_configMutex);
    
    m_calibrationInProgress = true;
    m_currentCalibrationElectrode = 0;
    m_calibrationFactors.clear();
    m_calibrationFactors.resize(m_config.electrodeCount);
    std::fill(m_calibrationFactors.begin(), m_calibrationFactors.end(), 1.0);

    setStatus(ConnectionStatus::Calibrating);

    // Lancer le timer de calibration
    m_calibrationTimer->start(1000); // 1 seconde par électrode

    emit calibrationProgress(0, 0.0);

    return true;
}

QVector<double> RealMEAInterface::getElectrodeImpedances() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_electrodeImpedances;
}

void RealMEAInterface::enableSpikeDetection(bool enable)
{
    QMutexLocker locker(&m_configMutex);
    
    m_spikeDetectionEnabled = enable;
    qDebug() << "[RealMEA] Spike detection" << (enable ? "enabled" : "disabled");
}

void RealMEAInterface::setSpikeThreshold(double threshold)
{
    QMutexLocker locker(&m_configMutex);
    
    m_spikeThreshold = threshold;
    qDebug() << "[RealMEA] Spike threshold set to" << threshold << "μV";
}

QVector<SpikeEvent> RealMEAInterface::getRecentSpikes(int lastNSpikes)
{
    QMutexLocker locker(&m_dataMutex);
    
    QVector<SpikeEvent> recent;
    int start = qMax(0, m_recentSpikes.size() - lastNSpikes);
    
    for (int i = start; i < m_recentSpikes.size(); ++i) {
        recent.append(m_recentSpikes[i]);
    }
    
    return recent;
}

double RealMEAInterface::getSignalQuality() const
{
    QMutexLocker locker(&m_dataMutex);
    
    if (m_currentData.isEmpty()) {
        return 0.0;
    }
    
    double totalQuality = 0.0;
    int activeCount = 0;
    
    for (const auto &electrode : m_currentData) {
        if (electrode.isActive) {
            totalQuality += electrode.signalQuality;
            activeCount++;
        }
    }
    
    return activeCount > 0 ? totalQuality / activeCount : 0.0;
}

QString RealMEAInterface::getLastError() const
{
    QMutexLocker locker(&m_configMutex);
    return m_lastError;
}

QJsonObject RealMEAInterface::getSystemStatus() const
{
    QMutexLocker locker(&m_configMutex);
    
    QJsonObject status;
    status["connectionStatus"] = static_cast<int>(m_status);
    status["electrodeCount"] = m_config.electrodeCount;
    status["samplingRate"] = m_config.samplingRate;
    status["continuousMode"] = m_continuousMode;
    status["calibrationInProgress"] = m_calibrationInProgress;
    status["spikeDetectionEnabled"] = m_spikeDetectionEnabled;
    status["signalQuality"] = getSignalQuality();
    status["lastError"] = m_lastError;
    
    return status;
}

void RealMEAInterface::setElectrodeActive(int electrodeId, bool active)
{
    QMutexLocker locker(&m_dataMutex);
    
    if (electrodeId >= 0 && electrodeId < m_activeElectrodes.size()) {
        m_activeElectrodes[electrodeId] = active;
        m_currentData[electrodeId].isActive = active;
        
        qDebug() << QString("[RealMEA] Electrode %1 %2")
                    .arg(electrodeId)
                    .arg(active ? "activated" : "deactivated");
    }
}

void RealMEAInterface::setAcquisitionFilter(double lowCut, double highCut)
{
    QMutexLocker locker(&m_configMutex);
    
    m_config.filterLowCut = lowCut;
    m_config.filterHighCut = highCut;
    
    qDebug() << QString("[RealMEA] Filter set: %1-%2 Hz")
                .arg(lowCut)
                .arg(highCut);
}

bool RealMEAInterface::saveRecording(const QString &filepath, const QString &format)
{
    QMutexLocker locker(&m_dataMutex);
    
    qDebug() << QString("[RealMEA] Saving recording to %1 in %2 format")
                .arg(filepath)
                .arg(format);
    
    // Simulate recording save
    QJsonObject recording;
    recording["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    recording["electrodeCount"] = m_config.electrodeCount;
    recording["samplingRate"] = m_config.samplingRate;
    recording["duration"] = "simulated";
    
    QJsonDocument doc(recording);
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        return true;
    }
    
    return false;
}

// === BITCOIN LEARNING METHODS ===

bool RealMEAInterface::initializeBitcoinLearning(const BitcoinLearningConfig &config)
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    qDebug() << "[RealMEA] Initializing Bitcoin learning system";
    
    m_bitcoinConfig = config;
    m_bitcoinLearningEnabled = true;
    
    // Initialize neural model
    m_neuralModel.modelId = QString("bitcoin_model_%1").arg(QDateTime::currentMSecsSinceEpoch());
    m_neuralModel.synapticWeights.resize(m_config.electrodeCount * m_config.electrodeCount);
    m_neuralModel.electrodeThresholds.resize(m_config.electrodeCount);
    m_neuralModel.learningRates.resize(m_config.electrodeCount);
    
    // Initialize weights randomly
    std::fill(m_neuralModel.synapticWeights.begin(), m_neuralModel.synapticWeights.end(), 0.0);
    std::fill(m_neuralModel.electrodeThresholds.begin(), m_neuralModel.electrodeThresholds.end(), 0.5);
    std::fill(m_neuralModel.learningRates.begin(), m_neuralModel.learningRates.end(), config.learningRate);
    
    m_neuralModel.globalLearningRate = config.learningRate;
    m_neuralModel.trainingStartTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    m_neuralModel.lastUpdateTime = m_neuralModel.trainingStartTime;
    m_neuralModel.trainingEpochs = 0;
    m_neuralModel.averageAccuracy = 0.0;
    
    // Initialize learning stats
    m_learningStats = BitcoinLearningStats{};
    
    qDebug() << "[RealMEA] Bitcoin learning system initialized";
    return true;
}

bool RealMEAInterface::trainBitcoinPattern(const BitcoinLearningPattern &pattern)
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    if (!m_bitcoinLearningEnabled) {
        setError("Bitcoin learning not initialized");
        return false;
    }
    
    qDebug() << "[RealMEA] Training Bitcoin pattern for block:" << pattern.blockHash.left(16) << "...";
    
    // Generate stimulation pattern from block hash
    QVector<double> stimulationPattern = generateStimulationPattern(pattern.blockHash);
    
    // Apply stimulation
    bool stimSuccess = stimulatePattern(stimulationPattern);
    if (!stimSuccess) {
        setError("Failed to apply stimulation pattern");
        return false;
    }
    
    // Simulate neural response
    QVector<SpikeEvent> neuralResponse;
    for (int i = 0; i < m_config.electrodeCount; ++i) {
        if (stimulationPattern[i] > 0.1) { // If electrode was stimulated
            SpikeEvent spike;
            spike.electrodeId = i;
            spike.amplitude = stimulationPattern[i] * 100.0; // Convert to μV
            spike.timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
            spike.waveform = QVector<double>(10, spike.amplitude); // Simple waveform
            neuralResponse.append(spike);
        }
    }
    
    // Extract predicted nonce
    quint32 predictedNonce = extractNonceFromNeuralResponse(neuralResponse);
    
    // Create response
    NeuralBitcoinResponse response;
    response.inputPattern = pattern;
    response.neuralResponse = neuralResponse;
    response.predictedNonce = predictedNonce;
    response.confidence = calculateResponseStrength(neuralResponse);
    response.rewardSignal = 0.0; // Will be set by reinforcement learning
    response.responseTime = QDateTime::currentMSecsSinceEpoch() * 1000 - pattern.timestamp;
    
    // Store pattern in model
    m_neuralModel.trainedPatterns.append(pattern);
    m_learningStats.totalPatternsLearned++;
    
    emit bitcoinPatternLearned(pattern);
    emit noncePredicationReady(response);
    
    return true;
}

NeuralBitcoinResponse RealMEAInterface::predictNonce(const QString &blockHash, double difficulty)
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    if (!m_bitcoinLearningEnabled) {
        NeuralBitcoinResponse empty;
        empty.confidence = 0.0;
        return empty;
    }
    
    qDebug() << "[RealMEA] Predicting nonce for block:" << blockHash.left(16) << "...";
    
    // Create input pattern
    BitcoinLearningPattern pattern;
    pattern.blockHash = blockHash;
    pattern.difficulty = difficulty;
    pattern.timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    // Generate stimulation pattern
    QVector<double> stimulationPattern = generateStimulationPattern(blockHash);
    
    // Apply stimulation
    stimulatePattern(stimulationPattern);
    
    // Simulate neural response
    QVector<SpikeEvent> neuralResponse;
    for (int i = 0; i < m_config.electrodeCount; ++i) {
        if (stimulationPattern[i] > 0.1) {
            SpikeEvent spike;
            spike.electrodeId = i;
            spike.amplitude = stimulationPattern[i] * 100.0;
            spike.timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
            spike.waveform = QVector<double>(10, spike.amplitude);
            neuralResponse.append(spike);
        }
    }
    
    // Extract predicted nonce
    quint32 predictedNonce = extractNonceFromNeuralResponse(neuralResponse);
    
    // Create response
    NeuralBitcoinResponse response;
    response.inputPattern = pattern;
    response.neuralResponse = neuralResponse;
    response.predictedNonce = predictedNonce;
    response.confidence = calculateResponseStrength(neuralResponse);
    response.rewardSignal = 0.0;
    response.responseTime = QDateTime::currentMSecsSinceEpoch() * 1000 - pattern.timestamp;
    
    m_learningStats.totalPredictions++;
    
    emit noncePredicationReady(response);
    
    return response;
}

bool RealMEAInterface::applyReinforcementLearning(const NeuralBitcoinResponse &response, bool wasSuccessful)
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    if (!m_bitcoinLearningEnabled) {
        return false;
    }
    
    qDebug() << "[RealMEA] Applying reinforcement learning, success:" << wasSuccessful;
    
    // Calculate reward signal
    double reward = wasSuccessful ? 1.0 : -0.5;
    
    // Update learning stats
    if (wasSuccessful) {
        m_learningStats.successfulPredictions++;
    }
    
    m_learningStats.currentAccuracy = static_cast<double>(m_learningStats.successfulPredictions) / 
                                     qMax(1, m_learningStats.totalPredictions);
    
    if (m_learningStats.currentAccuracy > m_learningStats.bestAccuracy) {
        m_learningStats.bestAccuracy = m_learningStats.currentAccuracy;
    }
    
    // Update synaptic weights using Hebbian learning
    updateSynapticWeights(response.neuralResponse, response.neuralResponse, reward);
    
    // Update model accuracy
    m_neuralModel.averageAccuracy = m_learningStats.currentAccuracy;
    m_neuralModel.lastUpdateTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    m_neuralModel.trainingEpochs++;
    
    emit reinforcementLearningApplied(reward, m_learningStats.currentAccuracy);
    emit learningStatsUpdated(m_learningStats);
    
    return true;
}

bool RealMEAInterface::saveNeuralModel(const QString &modelPath)
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    QString path = modelPath.isEmpty() ? 
                   QString("%1/%2.json").arg(m_bitcoinConfig.modelPersistencePath).arg(m_neuralModel.modelId) :
                   modelPath;
    
    qDebug() << "[RealMEA] Saving neural model to:" << path;
    
    QJsonObject model;
    model["modelId"] = m_neuralModel.modelId;
    model["trainingEpochs"] = m_neuralModel.trainingEpochs;
    model["averageAccuracy"] = m_neuralModel.averageAccuracy;
    model["globalLearningRate"] = m_neuralModel.globalLearningRate;
    model["trainingStartTime"] = m_neuralModel.trainingStartTime;
    model["lastUpdateTime"] = m_neuralModel.lastUpdateTime;
    
    // Save synaptic weights
    QJsonArray weights;
    for (double weight : m_neuralModel.synapticWeights) {
        weights.append(weight);
    }
    model["synapticWeights"] = weights;
    
    // Save electrode thresholds
    QJsonArray thresholds;
    for (double threshold : m_neuralModel.electrodeThresholds) {
        thresholds.append(threshold);
    }
    model["electrodeThresholds"] = thresholds;
    
    QJsonDocument doc(model);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        m_learningStats.lastModelFile = path;
        emit neuralModelSaved(path);
        return true;
    }
    
    return false;
}

bool RealMEAInterface::loadNeuralModel(const QString &modelPath)
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    qDebug() << "[RealMEA] Loading neural model from:" << modelPath;
    
    QFile file(modelPath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError("Failed to open model file");
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject model = doc.object();
    
    m_neuralModel.modelId = model["modelId"].toString();
    m_neuralModel.trainingEpochs = model["trainingEpochs"].toInt();
    m_neuralModel.averageAccuracy = model["averageAccuracy"].toDouble();
    m_neuralModel.globalLearningRate = model["globalLearningRate"].toDouble();
    m_neuralModel.trainingStartTime = model["trainingStartTime"].toVariant().toLongLong();
    m_neuralModel.lastUpdateTime = model["lastUpdateTime"].toVariant().toLongLong();
    
    // Load synaptic weights
    QJsonArray weights = model["synapticWeights"].toArray();
    m_neuralModel.synapticWeights.clear();
    for (const QJsonValue &value : weights) {
        m_neuralModel.synapticWeights.append(value.toDouble());
    }
    
    // Load electrode thresholds
    QJsonArray thresholds = model["electrodeThresholds"].toArray();
    m_neuralModel.electrodeThresholds.clear();
    for (const QJsonValue &value : thresholds) {
        m_neuralModel.electrodeThresholds.append(value.toDouble());
    }
    
    m_learningStats.lastModelFile = modelPath;
    
    qDebug() << "[RealMEA] Neural model loaded successfully";
    return true;
}

BitcoinLearningStats RealMEAInterface::getLearningStats() const
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    return m_learningStats;
}

void RealMEAInterface::resetNeuralModel()
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    qDebug() << "[RealMEA] Resetting neural model";
    
    m_neuralModel = BiologicalNeuralModel{};
    m_learningStats = BitcoinLearningStats{};
    
    // Reinitialize with current config
    if (m_bitcoinLearningEnabled) {
        initializeBitcoinLearning(m_bitcoinConfig);
    }
}

void RealMEAInterface::enableRealtimeLearning(bool enable)
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    m_bitcoinConfig.enableRealtimeLearning = enable;
    qDebug() << "[RealMEA] Realtime learning" << (enable ? "enabled" : "disabled");
}

bool RealMEAInterface::optimizeLearningParameters()
{
    QMutexLocker locker(&m_bitcoinLearningMutex);
    
    qDebug() << "[RealMEA] Optimizing learning parameters";
    
    // Simple optimization: adjust learning rate based on accuracy
    if (m_learningStats.currentAccuracy < 0.5) {
        m_bitcoinConfig.learningRate *= 1.1; // Increase learning rate
    } else if (m_learningStats.currentAccuracy > 0.8) {
        m_bitcoinConfig.learningRate *= 0.95; // Decrease learning rate
    }
    
    // Update global learning rate
    m_neuralModel.globalLearningRate = m_bitcoinConfig.learningRate;
    
    return true;
}

QVector<double> RealMEAInterface::generateStimulationPattern(const QString &blockHash)
{
    QVector<double> pattern(m_config.electrodeCount, 0.0);
    
    // Convert hash to stimulation pattern
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(blockHash.toUtf8());
    QByteArray hashBytes = hash.result();
    
    // Use hash bytes to determine stimulation amplitudes
    for (int i = 0; i < qMin(hashBytes.size(), m_config.electrodeCount); ++i) {
        double amplitude = static_cast<unsigned char>(hashBytes[i]) / 255.0;
        amplitude *= m_bitcoinConfig.stimulationAmplitude;
        pattern[i] = amplitude;
    }
    
    return pattern;
}

quint32 RealMEAInterface::extractNonceFromNeuralResponse(const QVector<SpikeEvent> &spikes)
{
    if (spikes.isEmpty()) {
        return 0;
    }
    
    // Simple nonce extraction: use spike amplitudes and timings
    quint32 nonce = 0;
    
    for (const auto &spike : spikes) {
        nonce ^= static_cast<quint32>(spike.amplitude * 1000);
        nonce ^= static_cast<quint32>(spike.timestamp & 0xFFFFFFFF);
    }
    
    return nonce;
}

void RealMEAInterface::updateSynapticWeights(const QVector<SpikeEvent> &preSpikes, 
                                            const QVector<SpikeEvent> &postSpikes, 
                                            double reward)
{
    // Simple Hebbian learning rule
    for (const auto &preSpike : preSpikes) {
        for (const auto &postSpike : postSpikes) {
            if (preSpike.electrodeId != postSpike.electrodeId) {
                int weightIndex = preSpike.electrodeId * m_config.electrodeCount + postSpike.electrodeId;
                if (weightIndex < m_neuralModel.synapticWeights.size()) {
                    double learningRate = m_neuralModel.learningRates[preSpike.electrodeId];
                    double weightChange = learningRate * preSpike.amplitude * postSpike.amplitude * reward;
                    m_neuralModel.synapticWeights[weightIndex] += weightChange;
                    
                    // Clamp weights
                    m_neuralModel.synapticWeights[weightIndex] = 
                        qBound(-1.0, m_neuralModel.synapticWeights[weightIndex], 1.0);
                }
            }
        }
    }
}

void RealMEAInterface::applySTDP(int preElectrode, int postElectrode, 
                                qint64 timeDifference, double learningRate)
{
    if (preElectrode < 0 || preElectrode >= m_config.electrodeCount ||
        postElectrode < 0 || postElectrode >= m_config.electrodeCount) {
        return;
    }
    
    int weightIndex = preElectrode * m_config.electrodeCount + postElectrode;
    if (weightIndex >= m_neuralModel.synapticWeights.size()) {
        return;
    }
    
    // STDP rule: LTP if pre before post, LTD if post before pre
    double weightChange = 0.0;
    if (timeDifference > 0) {
        // LTP: pre-spike before post-spike
        weightChange = learningRate * exp(-timeDifference / 20.0); // 20ms time constant
    } else {
        // LTD: post-spike before pre-spike
        weightChange = -learningRate * exp(timeDifference / 20.0);
    }
    
    m_neuralModel.synapticWeights[weightIndex] += weightChange;
    m_neuralModel.synapticWeights[weightIndex] = 
        qBound(-1.0, m_neuralModel.synapticWeights[weightIndex], 1.0);
}

bool RealMEAInterface::stimulateWithReinforcement(const QVector<double> &pattern, 
                                                  double reinforcementSignal)
{
    QMutexLocker locker(&m_configMutex);
    
    qDebug() << "[RealMEA] Stimulating with reinforcement signal:" << reinforcementSignal;
    
    // Apply stimulation pattern
    bool success = stimulatePattern(pattern);
    
    if (success) {
        // Apply reinforcement after stimulation
        QTimer::singleShot(static_cast<int>(m_bitcoinConfig.reinforcementDelay), [this, reinforcementSignal]() {
            // Apply reinforcement stimulation
            QVector<double> reinforcementPattern(m_config.electrodeCount, 0.0);
            double amplitude = reinforcementSignal > 0 ? 
                              m_bitcoinConfig.stimulationAmplitude : 
                              m_bitcoinConfig.punishmentAmplitude;
            
            // Apply to all electrodes
            std::fill(reinforcementPattern.begin(), reinforcementPattern.end(), amplitude);
            stimulatePattern(reinforcementPattern);
        });
    }
    
    return success;
}

// === PRIVATE SLOTS ===

void RealMEAInterface::onSerialDataReady()
{
    // Handle serial data
    if (m_serialPort && m_serialPort->bytesAvailable() > 0) {
        QByteArray data = m_serialPort->readAll();
        processRawData(data);
    }
}

void RealMEAInterface::onNetworkDataReady()
{
    // Handle network data
    if (m_tcpSocket && m_tcpSocket->bytesAvailable() > 0) {
        QByteArray data = m_tcpSocket->readAll();
        processRawData(data);
    }
}

void RealMEAInterface::onAcquisitionTimer()
{
    // Read electrode data
    readElectrodeData();
    
    // Detect spikes if enabled
    if (m_spikeDetectionEnabled) {
        detectSpikes(m_currentData);
    }
}

void RealMEAInterface::onCalibrationTimer()
{
    QMutexLocker locker(&m_configMutex);
    
    if (!m_calibrationInProgress) {
        return;
    }
    
    // Calibrate current electrode
    calibrateElectrode(m_currentCalibrationElectrode);
    
    // Move to next electrode
    m_currentCalibrationElectrode++;
    
    double progress = static_cast<double>(m_currentCalibrationElectrode) / m_config.electrodeCount;
    emit calibrationProgress(m_currentCalibrationElectrode, progress);
    
    if (m_currentCalibrationElectrode >= m_config.electrodeCount) {
        // Calibration complete
        m_calibrationTimer->stop();
        m_calibrationInProgress = false;
        setStatus(ConnectionStatus::Connected);
        emit calibrationComplete();
        
        qDebug() << "[RealMEA] Calibration complete";
    }
}

// === PRIVATE METHODS ===

bool RealMEAInterface::connectSerial()
{
    qDebug() << "[RealMEA] Attempting serial connection to:" << m_config.devicePath;
    
    m_serialPort = std::make_unique<QSerialPort>();
    m_serialPort->setPortName(m_config.devicePath);
    m_serialPort->setBaudRate(m_config.baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        connect(m_serialPort.get(), &QSerialPort::readyRead, this, &RealMEAInterface::onSerialDataReady);
        qDebug() << "[RealMEA] Serial connection established";
        return true;
    } else {
        setError("Failed to open serial port: " + m_serialPort->errorString());
        return false;
    }
}

bool RealMEAInterface::connectNetwork()
{
    qDebug() << "[RealMEA] Attempting network connection to:" << m_config.networkHost << ":" << m_config.networkPort;
    
    m_tcpSocket = std::make_unique<QTcpSocket>();
    
    connect(m_tcpSocket.get(), &QTcpSocket::readyRead, this, &RealMEAInterface::onNetworkDataReady);
    connect(m_tcpSocket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            [this](QAbstractSocket::SocketError error) {
                setError("Network error: " + m_tcpSocket->errorString());
            });
    
    m_tcpSocket->connectToHost(m_config.networkHost, m_config.networkPort);
    
    if (m_tcpSocket->waitForConnected(m_config.connectionTimeoutMs)) {
        qDebug() << "[RealMEA] Network connection established";
        return true;
    } else {
        setError("Failed to connect to network host");
        return false;
    }
}

bool RealMEAInterface::connectSharedMemory()
{
    qDebug() << "[RealMEA] Shared memory connection not implemented yet";
    setError("Shared memory connection not implemented");
    return false;
}

bool RealMEAInterface::connectCustomAPI()
{
    qDebug() << "[RealMEA] Custom API connection not implemented yet";
    setError("Custom API connection not implemented");
    return false;
}

bool RealMEAInterface::sendCommand(const QByteArray &command)
{
    if (m_serialPort && m_serialPort->isOpen()) {
        qint64 bytesWritten = m_serialPort->write(command);
        return bytesWritten == command.size();
    } else if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        qint64 bytesWritten = m_tcpSocket->write(command);
        return bytesWritten == command.size();
    }
    
    return false;
}

QByteArray RealMEAInterface::receiveResponse(int timeoutMs)
{
    if (m_serialPort && m_serialPort->isOpen()) {
        if (m_serialPort->waitForReadyRead(timeoutMs)) {
            return m_serialPort->readAll();
        }
    } else if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        if (m_tcpSocket->waitForReadyRead(timeoutMs)) {
            return m_tcpSocket->readAll();
        }
    }
    
    return QByteArray();
}

bool RealMEAInterface::parseElectrodeData(const QByteArray &data, QVector<ElectrodeData> &electrodes)
{
    // Simple parsing for demonstration
    // In real implementation, this would parse binary data from the MEA device
    
    if (data.size() < static_cast<int>(sizeof(ElectrodeData) * electrodes.size())) {
        return false;
    }
    
    // For now, just simulate data
    for (int i = 0; i < electrodes.size(); ++i) {
        electrodes[i].voltage = (QRandomGenerator::global()->bounded(200) - 100) / 1000.0;
        electrodes[i].current = electrodes[i].voltage / m_electrodeImpedances[i];
        electrodes[i].timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
    }
    
    return true;
}

void RealMEAInterface::processRawData(const QByteArray &rawData)
{
    QMutexLocker locker(&m_dataMutex);
    
    m_dataBuffer.append(rawData);
    
    // Process complete electrode data packets
    int packetSize = sizeof(ElectrodeData) * m_config.electrodeCount;
    while (m_dataBuffer.size() >= packetSize) {
        QByteArray packet = m_dataBuffer.left(packetSize);
        m_dataBuffer.remove(0, packetSize);
        
        QVector<ElectrodeData> electrodes(m_config.electrodeCount);
        if (parseElectrodeData(packet, electrodes)) {
            m_currentData = electrodes;
            emit electrodeDataReady(electrodes);
        }
    }
}

void RealMEAInterface::detectSpikes(const QVector<ElectrodeData> &data)
{
    QMutexLocker locker(&m_dataMutex);
    
    for (const auto &electrode : data) {
        if (!electrode.isActive) continue;
        
        // Simple spike detection: voltage below threshold
        if (electrode.voltage < m_spikeThreshold) {
            SpikeEvent spike;
            spike.electrodeId = electrode.electrodeId;
            spike.amplitude = electrode.voltage;
            spike.timestamp = electrode.timestamp;
            spike.waveform = QVector<double>(10, electrode.voltage); // Simple waveform
            
            m_recentSpikes.append(spike);
            
            // Keep only recent spikes
            if (m_recentSpikes.size() > MAX_SPIKE_BUFFER) {
                m_recentSpikes.removeFirst();
            }
            
            emit spikeDetected(spike);
        }
    }
}

bool RealMEAInterface::validateDataIntegrity(const QVector<ElectrodeData> &data)
{
    for (const auto &electrode : data) {
        if (electrode.signalQuality < MIN_SIGNAL_QUALITY) {
            return false;
        }
    }
    return true;
}

void RealMEAInterface::performImpedanceTest(int electrodeId)
{
    if (electrodeId < 0 || electrodeId >= m_config.electrodeCount) {
        qWarning() << "[RealMEA] ID électrode invalide pour test d'impédance:" << electrodeId;
        return;
    }

    double baseImpedance = 1000.0; // 1 kΩ de base
    double variation = (QRandomGenerator::global()->bounded(200) - 100) / 100.0; // ±100 Ω
    double impedance = baseImpedance + variation;

    bool isValid = (impedance < MAX_IMPEDANCE_MOHM * 1000000.0); // Convertir MΩ en Ω

    int oldSize = m_electrodeImpedances.size();
    if (oldSize < electrodeId + 1) {
        m_electrodeImpedances.resize(electrodeId + 1);
        std::fill(m_electrodeImpedances.begin() + oldSize, m_electrodeImpedances.end(), impedance);
    }
    m_electrodeImpedances[electrodeId] = impedance;

    qDebug() << QString("[RealMEA] Électrode %1: Impédance = %2 Ω (%3)")
                .arg(electrodeId)
                .arg(impedance, 0, 'f', 1)
                .arg(isValid ? "OK" : "HORS LIMITES");
}

void RealMEAInterface::updateCalibrationFactors()
{
    QMutexLocker locker(&m_configMutex);
    
    qDebug() << "[RealMEA] Updating calibration factors";
    
    // Simple calibration factor update
    for (int i = 0; i < m_calibrationFactors.size(); ++i) {
        double factor = 0.8 + (QRandomGenerator::global()->bounded(40)) / 100.0; // 0.8-1.2
        m_calibrationFactors[i] = factor;
    }
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
    qWarning() << "[RealMEA] Error:" << error;
    emit errorOccurred(error);
}

QByteArray RealMEAInterface::formatStimulationCommand(int electrodeId, double amplitude, double duration)
{
    // Simple command formatting for demonstration
    QByteArray command;
    command.append(static_cast<char>(0x01)); // Command type
    command.append(static_cast<char>(electrodeId));
    command.append(reinterpret_cast<const char*>(&amplitude), sizeof(amplitude));
    command.append(reinterpret_cast<const char*>(&duration), sizeof(duration));
    return command;
}

double RealMEAInterface::calculateSpikeConsistency(const QVector<SpikeEvent> &spikes)
{
    if (spikes.size() < 2) {
        return 0.0;
    }
    
    // Calculate consistency based on spike timing intervals
    QVector<qint64> intervals;
    for (int i = 1; i < spikes.size(); ++i) {
        intervals.append(spikes[i].timestamp - spikes[i-1].timestamp);
    }
    
    if (intervals.isEmpty()) {
        return 0.0;
    }
    
    // Calculate standard deviation of intervals
    double mean = 0.0;
    for (qint64 interval : intervals) {
        mean += interval;
    }
    mean /= intervals.size();
    
    double variance = 0.0;
    for (qint64 interval : intervals) {
        variance += (interval - mean) * (interval - mean);
    }
    variance /= intervals.size();
    
    double stdDev = sqrt(variance);
    
    // Consistency is inverse of coefficient of variation
    return mean > 0 ? mean / (mean + stdDev) : 0.0;
}

double RealMEAInterface::calculateResponseStrength(const QVector<SpikeEvent> &spikes)
{
    if (spikes.isEmpty()) {
        return 0.0;
    }
    
    double totalAmplitude = 0.0;
    for (const auto &spike : spikes) {
        totalAmplitude += abs(spike.amplitude);
    }
    
    return totalAmplitude / spikes.size();
}

bool RealMEAInterface::configureDevice()
{
    qDebug() << "[RealMEA] Configuring device";
    
    // Send configuration commands
    QByteArray configCmd = QByteArray::fromHex("01020304"); // Example command
    return sendCommand(configCmd);
}

bool RealMEAInterface::testCommunication()
{
    qDebug() << "[RealMEA] Testing communication";
    
    QByteArray testCmd = QByteArray::fromHex("FF");
    QByteArray response = receiveResponse(1000);
    
    return !response.isEmpty();
}

bool RealMEAInterface::parseBinaryElectrodeData(const QByteArray &data, QVector<ElectrodeData> &electrodes)
{
    // Parse binary data from MEA device
    // This is a placeholder implementation
    
    if (data.size() < static_cast<int>(sizeof(ElectrodeData) * electrodes.size())) {
        return false;
    }
    
    const char *ptr = data.constData();
    for (int i = 0; i < electrodes.size(); ++i) {
        electrodes[i].electrodeId = i;
        electrodes[i].voltage = *reinterpret_cast<const double*>(ptr);
        ptr += sizeof(double);
        electrodes[i].current = *reinterpret_cast<const double*>(ptr);
        ptr += sizeof(double);
        electrodes[i].impedance = *reinterpret_cast<const double*>(ptr);
        ptr += sizeof(double);
        electrodes[i].isActive = *reinterpret_cast<const bool*>(ptr);
        ptr += sizeof(bool);
        electrodes[i].signalQuality = *reinterpret_cast<const double*>(ptr);
        ptr += sizeof(double);
        electrodes[i].timestamp = *reinterpret_cast<const qint64*>(ptr);
        ptr += sizeof(qint64);
    }
    
    return true;
}

} // namespace Bio
} // namespace BioMining