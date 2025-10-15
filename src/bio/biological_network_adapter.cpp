#include "bio/biological_network_adapter.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QDateTime>
#include <cmath>

namespace BioMining {
namespace Bio {

BiologicalNetworkAdapter::BiologicalNetworkAdapter(QObject *parent)
    : IBioComputeInterface(parent)
    , m_network(QSharedPointer<Network::BiologicalNetwork>::create())
    , m_initialized(false)
{
    // Connecter les signaux du réseau
    connect(m_network.data(), &Network::BiologicalNetwork::learningStateChanged,
            this, &BiologicalNetworkAdapter::onNetworkLearningStateChanged);
    
    qDebug() << "[BiologicalNetworkAdapter] Adapteur créé pour réseau simulé";
}

BiologicalNetworkAdapter::BiologicalNetworkAdapter(QSharedPointer<Network::BiologicalNetwork> network, QObject *parent)
    : IBioComputeInterface(parent)
    , m_network(network)
    , m_initialized(false)
{
    if (m_network) {
        connect(m_network.data(), &Network::BiologicalNetwork::learningStateChanged,
                this, &BiologicalNetworkAdapter::onNetworkLearningStateChanged);
    }
    qDebug() << "[BiologicalNetworkAdapter] Adapteur créé avec réseau existant";
}

BiologicalNetworkAdapter::~BiologicalNetworkAdapter()
{
    qDebug() << "[BiologicalNetworkAdapter] Adapteur détruit";
}

bool BiologicalNetworkAdapter::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    if (!m_network) {
        qWarning() << "[BiologicalNetworkAdapter] Réseau non disponible";
        return false;
    }
    
    // Initialiser le réseau avec config par défaut
    if (!m_network->initialize()) {
        qWarning() << "[BiologicalNetworkAdapter] Échec de l'initialisation du réseau";
        return false;
    }
    
    // Configurer le réseau pour mining
    Network::BiologicalNetwork::NetworkConfig config;
    config.inputSize = 60;          // 60 électrodes simulées
    config.outputSize = 32;         // 32 bits pour nonce
    config.neuronCount = 60;
    config.learningRate = 0.01;
    config.useReinforcementLearning = true;
    
    m_network->setNetworkConfig(config);
    
    m_initialized = true;
    qDebug() << "[BiologicalNetworkAdapter] Adapteur initialisé avec succès";
    return true;
}

bool BiologicalNetworkAdapter::isReady() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_network) {
        return false;
    }
    
    return m_initialized;
}

bool BiologicalNetworkAdapter::applyStimulus(const StimulusPattern &pattern)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isReady()) {
        qWarning() << "[BiologicalNetworkAdapter] Réseau non prêt pour stimulation";
        return false;
    }
    
    if (pattern.amplitudes.size() != 60) {
        qWarning() << "[BiologicalNetworkAdapter] Pattern invalide: doit avoir 60 valeurs";
        return false;
    }
    
    // Stocker le pattern de stimulation pour capture ultérieure
    m_lastStimulusSignals = pattern.amplitudes;
    
    // Propager le stimulus à travers le réseau
    m_network->forwardPropagation(m_lastStimulusSignals);
    
    // Simuler un délai biologique réaliste
    simulateBiologicalDelay(pattern.durationMs);
    
    emit stimulusApplied();
    qDebug() << "[BiologicalNetworkAdapter] Stimulation propagée dans le réseau";
    
    return true;
}

BioResponse BiologicalNetworkAdapter::captureResponse(int waitTimeMs)
{
    QMutexLocker locker(&m_mutex);
    
    BioResponse response;
    response.isValid = false;
    
    if (!isReady()) {
        qWarning() << "[BiologicalNetworkAdapter] Réseau non prêt pour capture";
        return response;
    }
    
    // Simuler attente de réponse biologique
    simulateBiologicalDelay(waitTimeMs);
    
    // Capturer la sortie du réseau (60 neurones d'entrée comme "électrodes")
    // En simulation, on utilise l'activité du réseau après propagation
    QVector<double> networkOutput = m_network->getOutputValues();
    
    // Si le réseau n'a pas encore de sortie, utiliser les signaux d'entrée
    if (networkOutput.isEmpty() && !m_lastStimulusSignals.isEmpty()) {
        // Simuler une réponse basée sur l'entrée avec du bruit
        networkOutput.resize(60);
        for (int i = 0; i < 60; ++i) {
            double noise = (QRandomGenerator::global()->bounded(200) - 100) / 1000.0;
            networkOutput[i] = m_lastStimulusSignals[i] * 0.8 + noise;
        }
    }
    
    // Convertir en BioResponse
    response = convertNetworkOutputToResponse(networkOutput);
    
    if (response.isValid) {
        emit responseReady(response);
    }
    
    return response;
}

BioResponse BiologicalNetworkAdapter::stimulateAndCapture(const StimulusPattern &pattern)
{
    BioResponse response;
    response.isValid = false;
    
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    // Appliquer la stimulation
    if (!applyStimulus(pattern)) {
        qWarning() << "[BiologicalNetworkAdapter] Échec de la stimulation";
        return response;
    }
    
    // Capturer la réponse
    response = captureResponse(50); // Temps de réponse plus court pour simulation
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    response.responseTime = endTime - startTime;
    
    return response;
}

void BiologicalNetworkAdapter::reinforcePattern(const StimulusPattern &pattern, uint32_t nonce, double reward)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isReady()) {
        return;
    }
    
    qDebug() << QString("[BiologicalNetworkAdapter] Renforcement: nonce=%1, reward=%2")
                .arg(nonce).arg(reward);
    
    // Créer des données d'apprentissage
    Network::BiologicalNetwork::LearningData learningData;
    learningData.inputSignals = pattern.amplitudes;
    learningData.targetNonce = nonce;
    learningData.wasSuccessful = (reward > 0.5);
    learningData.timestamp = QDateTime::currentDateTime();
    
    // Ajouter l'exemple d'apprentissage
    m_network->addLearningExample(learningData);
    
    // Optimiser depuis le feedback
    m_network->optimizeFromFeedback(learningData.wasSuccessful, learningData);
    
    emit patternReinforced(reward);
}

QString BiologicalNetworkAdapter::getDiagnosticInfo() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_network) {
        return "Network: Non disponible";
    }
    
    QString info = "=== BiologicalNetwork Adapter Diagnostic ===\n";
    info += QString("Mode: Simulated Network (No Physical Cells)\n");
    info += QString("Initialized: %1\n").arg(m_initialized ? "Yes" : "No");
    info += QString("Ready: %1\n").arg(isReady() ? "Yes" : "No");
    info += QString("\n--- Network Info ---\n");
    info += m_network->getNetworkDiagnostic();
    info += QString("\nNetwork Efficiency: %1\n").arg(m_network->getNetworkEfficiency());
    info += QString("Training Epochs: %1\n").arg(m_network->getTrainingEpochs());
    
    auto config = m_network->getNetworkConfig();
    info += QString("Neuron Count: %1\n").arg(config.neuronCount);
    info += QString("Learning Rate: %1\n").arg(config.learningRate);
    
    return info;
}

void BiologicalNetworkAdapter::reset()
{
    QMutexLocker locker(&m_mutex);
    
    qDebug() << "[BiologicalNetworkAdapter] Réinitialisation de l'adapteur";
    
    if (m_network) {
        m_network->stopLearning();
    }
    
    m_lastStimulusSignals.clear();
    m_initialized = false;
}

void BiologicalNetworkAdapter::setNetworkConfig(const Network::BiologicalNetwork::NetworkConfig &config)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_network) {
        m_network->setNetworkConfig(config);
    }
}

// Private slots
void BiologicalNetworkAdapter::onNetworkLearningStateChanged(Network::BiologicalNetwork::LearningState state)
{
    qDebug() << "[BiologicalNetworkAdapter] État d'apprentissage changé:" << static_cast<int>(state);
}

// Private methods
BioResponse BiologicalNetworkAdapter::convertNetworkOutputToResponse(const QVector<double> &output)
{
    BioResponse response;
    
    if (output.isEmpty()) {
        response.isValid = false;
        return response;
    }
    
    // Utiliser la sortie du réseau comme signaux de réponse
    // Si output < 60, on le complète avec les derniers signaux
    response.rsignals.resize(60);
    
    for (int i = 0; i < 60; ++i) {
        if (i < output.size()) {
            response.rsignals[i] = output[i];
        } else if (i < m_lastStimulusSignals.size()) {
            // Compléter avec stimulus atténué
            response.rsignals[i] = m_lastStimulusSignals[i] * 0.5;
        } else {
            response.rsignals[i] = 0.0;
        }
    }
    
    // Calculer la force de réponse
    response.responseStrength = calculateNetworkResponseStrength(response.rsignals);
    
    // Calculer la qualité du signal (simulation = haute qualité)
    response.signalQuality = calculateNetworkSignalQuality(response.rsignals);
    
    // Timestamp actuel
    response.responseTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    response.isValid = true;
    
    return response;
}

double BiologicalNetworkAdapter::calculateNetworkResponseStrength(const QVector<double> &output)
{
    if (output.isEmpty()) {
        return 0.0;
    }
    
    double totalAmplitude = 0.0;
    for (double value : output) {
        totalAmplitude += std::abs(value);
    }
    
    return totalAmplitude / output.size();
}

double BiologicalNetworkAdapter::calculateNetworkSignalQuality(const QVector<double> &output)
{
    if (output.isEmpty()) {
        return 0.0;
    }
    
    // Simulation = qualité élevée (0.9-1.0)
    // En pratique, varie selon la variance du signal
    double variance = 0.0;
    double mean = 0.0;
    
    for (double value : output) {
        mean += value;
    }
    mean /= output.size();
    
    for (double value : output) {
        variance += (value - mean) * (value - mean);
    }
    variance /= output.size();
    
    // Qualité inversement proportionnelle à la variance (plus stable = meilleur)
    double quality = 1.0 / (1.0 + variance);
    return qBound(0.7, quality, 1.0); // Entre 0.7 et 1.0
}

void BiologicalNetworkAdapter::simulateBiologicalDelay(int durationMs)
{
    // Simuler un délai biologique réaliste
    // En simulation, on peut réduire le temps pour accélérer
    int simulatedDelay = durationMs / 2; // Simulation 2x plus rapide
    QThread::msleep(qMax(10, simulatedDelay)); // Minimum 10ms
}

} // namespace Bio
} // namespace BioMining
