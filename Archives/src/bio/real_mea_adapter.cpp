#include "bio/real_mea_adapter.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <cmath>

namespace BioMining {
namespace Bio {

RealMEAAdapter::RealMEAAdapter(QObject *parent)
    : IBioComputeInterface(parent)
    , m_meaInterface(QSharedPointer<RealMEAInterface>::create())
    , m_initialized(false)
{
    // Connecter les signaux du MEA
    connect(m_meaInterface.data(), &RealMEAInterface::statusChanged,
            this, &RealMEAAdapter::onMEAStatusChanged);
    connect(m_meaInterface.data(), &RealMEAInterface::errorOccurred,
            this, &RealMEAAdapter::onMEAError);
    
    qDebug() << "[RealMEAAdapter] Adapteur créé pour MEA réel";
}

RealMEAAdapter::RealMEAAdapter(QSharedPointer<RealMEAInterface> meaInterface, QObject *parent)
    : IBioComputeInterface(parent)
    , m_meaInterface(meaInterface)
    , m_initialized(false)
{
    if (m_meaInterface) {
        connect(m_meaInterface.data(), &RealMEAInterface::statusChanged,
                this, &RealMEAAdapter::onMEAStatusChanged);
        connect(m_meaInterface.data(), &RealMEAInterface::errorOccurred,
                this, &RealMEAAdapter::onMEAError);
    }
    qDebug() << "[RealMEAAdapter] Adapteur créé avec MEA existant";
}

RealMEAAdapter::~RealMEAAdapter()
{
    qDebug() << "[RealMEAAdapter] Adapteur détruit";
}

bool RealMEAAdapter::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    if (!m_meaInterface) {
        qWarning() << "[RealMEAAdapter] Interface MEA non disponible";
        return false;
    }
    
    // Initialiser avec config par défaut si pas encore fait
    if (m_meaInterface->getStatus() == RealMEAInterface::ConnectionStatus::Disconnected) {
        RealMEAConfig config;
        config.electrodeCount = 60;
        config.samplingRate = 25000.0;
        config.deviceType = MEADeviceType::Custom_Serial;
        
        if (!m_meaInterface->initialize(config)) {
            qWarning() << "[RealMEAAdapter] Échec de l'initialisation MEA";
            return false;
        }
    }
    
    m_initialized = true;
    qDebug() << "[RealMEAAdapter] Adapteur initialisé avec succès";
    return true;
}

bool RealMEAAdapter::isReady() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_meaInterface) {
        return false;
    }
    
    auto status = m_meaInterface->getStatus();
    return m_initialized && 
           (status == RealMEAInterface::ConnectionStatus::Connected ||
            status == RealMEAInterface::ConnectionStatus::Streaming);
}

bool RealMEAAdapter::applyStimulus(const StimulusPattern &pattern)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isReady()) {
        qWarning() << "[RealMEAAdapter] MEA non prêt pour stimulation";
        return false;
    }
    
    if (pattern.amplitudes.size() != 60) {
        qWarning() << "[RealMEAAdapter] Pattern invalide: doit avoir 60 valeurs";
        return false;
    }
    
    // Convertir le pattern en format MEA
    QVector<double> meaPattern = pattern.amplitudes;
    
    // Appliquer la stimulation via RealMEAInterface
    bool success = m_meaInterface->stimulatePattern(meaPattern);
    
    if (success) {
        emit stimulusApplied();
        qDebug() << "[RealMEAAdapter] Stimulation appliquée avec succès";
    }
    
    return success;
}

BioMining::Bio::IBioComputeInterface::BioResponse RealMEAAdapter::captureResponse(int waitTimeMs)
{
    QMutexLocker locker(&m_mutex);
    
    BioMining::Bio::IBioComputeInterface::BioResponse response;
    response.isValid = false;
    
    if (!isReady()) {
        qWarning() << "[RealMEAAdapter] MEA non prêt pour capture";
        return response;
    }
    
    // Attendre que les cellules répondent
    QThread::msleep(waitTimeMs);
    
    // Capturer les données des électrodes
    QVector<ElectrodeData> electrodeData = m_meaInterface->readElectrodeData();
    
    // Convertir en BioResponse
    response = convertElectrodeDataToResponse(electrodeData);
    
    if (response.isValid) {
        emit responseReady(response);
    }
    
    return response;
}

BioMining::Bio::IBioComputeInterface::BioResponse RealMEAAdapter::stimulateAndCapture(const StimulusPattern &pattern)
{
    BioMining::Bio::IBioComputeInterface::BioResponse response;
    response.isValid = false;
    
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    // Appliquer la stimulation
    if (!applyStimulus(pattern)) {
        qWarning() << "[RealMEAAdapter] Échec de la stimulation";
        return response;
    }
    
    // Capturer la réponse
    response = captureResponse(pattern.durationMs + 100);
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    response.responseTime = endTime - startTime;
    
    return response;
}

void RealMEAAdapter::reinforcePattern(const StimulusPattern &pattern, uint32_t nonce, double reward)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isReady()) {
        return;
    }
    
    qDebug() << QString("[RealMEAAdapter] Renforcement: nonce=%1, reward=%2")
                .arg(nonce).arg(reward);
    
    // Créer un pattern de renforcement
    QVector<double> reinforcementPattern = pattern.amplitudes;
    
    // Appliquer stimulation de renforcement via RealMEAInterface
    bool success = m_meaInterface->stimulateWithReinforcement(reinforcementPattern, reward);
    
    if (success) {
        emit patternReinforced(reward);
    }
}

QString RealMEAAdapter::getDiagnosticInfo() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_meaInterface) {
        return "MEA Interface: Non disponible";
    }
    
    QString info = "=== RealMEA Adapter Diagnostic ===\n";
    info += QString("Mode: Real MEA (Biological Cells)\n");
    info += QString("Initialized: %1\n").arg(m_initialized ? "Yes" : "No");
    info += QString("Ready: %1\n").arg(isReady() ? "Yes" : "No");
    info += QString("\n--- MEA Device Info ---\n");
    info += m_meaInterface->getDeviceInfo();
    info += QString("\nSignal Quality: %1\n").arg(m_meaInterface->getSignalQuality());
    
    QJsonObject systemStatus = m_meaInterface->getSystemStatus();
    info += QString("Connection Status: %1\n").arg(systemStatus["connectionStatus"].toInt());
    info += QString("Electrode Count: %1\n").arg(systemStatus["electrodeCount"].toInt());
    info += QString("Sampling Rate: %1 Hz\n").arg(systemStatus["samplingRate"].toDouble());
    
    return info;
}

void RealMEAAdapter::reset()
{
    QMutexLocker locker(&m_mutex);
    
    qDebug() << "[RealMEAAdapter] Réinitialisation de l'adapteur";
    
    if (m_meaInterface) {
        m_meaInterface->stopContinuousAcquisition();
    }
    
    m_initialized = false;
}

void RealMEAAdapter::setMEAConfig(const RealMEAConfig &config)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_meaInterface) {
        m_meaInterface->initialize(config);
    }
}

bool RealMEAAdapter::initializeBitcoinLearning(const BitcoinLearningConfig &config)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_meaInterface) {
        return false;
    }
    
    return m_meaInterface->initializeBitcoinLearning(config);
}

// Private slots
void RealMEAAdapter::onMEAStatusChanged(RealMEAInterface::ConnectionStatus status)
{
    qDebug() << "[RealMEAAdapter] Statut MEA changé:" << static_cast<int>(status);
}

void RealMEAAdapter::onMEAError(const QString &error)
{
    qWarning() << "[RealMEAAdapter] Erreur MEA:" << error;
    emit errorOccurred(error);
}

// Private methods
BioMining::Bio::IBioComputeInterface::BioResponse RealMEAAdapter::convertElectrodeDataToResponse(const QVector<ElectrodeData> &electrodeData)
{
    BioMining::Bio::IBioComputeInterface::BioResponse response;
    
    if (electrodeData.isEmpty() || electrodeData.size() != 60) {
        response.isValid = false;
        return response;
    }
    
    // Extraire les signaux de voltage
    response.rsignals.resize(60);
    for (int i = 0; i < 60; ++i) {
        response.rsignals[i] = electrodeData[i].voltage;
    }
    
    // Calculer la force de réponse
    response.responseStrength = calculateResponseStrength(electrodeData);
    
    // Calculer la qualité du signal
    response.signalQuality = calculateSignalQuality(electrodeData);
    
    // Timestamp
    response.responseTime = electrodeData[0].timestamp;
    
    response.isValid = true;
    
    return response;
}

double RealMEAAdapter::calculateResponseStrength(const QVector<ElectrodeData> &data)
{
    if (data.isEmpty()) {
        return 0.0;
    }
    
    double totalAmplitude = 0.0;
    int activeCount = 0;
    
    for (const auto &electrode : data) {
        if (electrode.isActive) {
            totalAmplitude += std::abs(electrode.voltage);
            activeCount++;
        }
    }
    
    return activeCount > 0 ? totalAmplitude / activeCount : 0.0;
}

double RealMEAAdapter::calculateSignalQuality(const QVector<ElectrodeData> &data)
{
    if (data.isEmpty()) {
        return 0.0;
    }
    
    double totalQuality = 0.0;
    int activeCount = 0;
    
    for (const auto &electrode : data) {
        if (electrode.isActive) {
            totalQuality += electrode.signalQuality;
            activeCount++;
        }
    }
    
    return activeCount > 0 ? totalQuality / activeCount : 0.0;
}

} // namespace Bio
} // namespace BioMining
