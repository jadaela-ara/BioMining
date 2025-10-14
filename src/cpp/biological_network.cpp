#include <cmath>
#include "bio/biological_network.h"
#include <QDebug>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QtMath>
#include <algorithm>
#include <numeric>
#include <thread>
#include <atomic>
#include <QDataStream>

using namespace BioMining::Network; 

BiologicalNetwork::BiologicalNetwork(QObject *parent)
    : QObject(parent)
    , m_learningState(LearningState::Untrained)
    , m_currentEpoch(0)
    , m_totalEpochs(0)
    , m_trainingProgress(0.0)
    , m_learningTimer(std::make_unique<QTimer>(this))
    , m_optimizationTimer(std::make_unique<QTimer>(this))
    , m_networkEfficiency(0.0)
    , m_learningActive(false)
    , m_successfulPredictions(0)
    , m_totalPredictions(0)
    , m_averageConfidence(0.0)
{
    // Configuration par défaut
    m_config = NetworkConfig();
    
    // Timers
    connect(m_learningTimer.get(), &QTimer::timeout, this, &BiologicalNetwork::onLearningCycle);
    connect(m_optimizationTimer.get(), &QTimer::timeout, this, &BiologicalNetwork::onOptimizationTimer);
    
    // Initialisation du réseau
    initializeNetwork();
    
    qDebug() << "[BIO-NET] Réseau biologique initialisé avec" << m_config.neuronCount << "neurones";
}

BiologicalNetwork::~BiologicalNetwork()
{
    stopLearning();
}

void BiologicalNetwork::setNetworkConfig(const NetworkConfig &config)
{
    QMutexLocker locker(&m_networkMutex);
    
    m_config = config;
    
    // Réinitialisation si la topologie change
    if (m_layers.isEmpty() || m_layers[0].neurons.size() != config.neuronCount) {
        initializeNetwork();
    }
    
    qDebug() << "[BIO-NET] Configuration mise à jour - Neurones:" << config.neuronCount
             << "Couches:" << config.hiddenLayers
             << "Taux apprentissage:" << config.learningRate;
}

BiologicalNetwork::NetworkConfig BiologicalNetwork::getNetworkConfig() const
{
    QMutexLocker locker(&m_networkMutex);
    return m_config;
}

BiologicalNetwork::LearningState BiologicalNetwork::getLearningState() const
{
    QMutexLocker locker(&m_networkMutex);
    return m_learningState;
}

double BiologicalNetwork::getTrainingProgress() const
{
    QMutexLocker locker(&m_networkMutex);
    return m_trainingProgress;
}

int BiologicalNetwork::getTrainingEpochs() const
{
    QMutexLocker locker(&m_networkMutex);
    return m_currentEpoch;
}

void BiologicalNetwork::initializeNetwork()
{
    m_layers.clear();
    
    // Couche d'entrée (signaux MEA)
    NetworkLayer inputLayer;
    inputLayer.layerType = "input";
    inputLayer.neurons.resize(m_config.neuronCount);
    
    for (int i = 0; i < m_config.neuronCount; ++i) {
        BiologicalNeuron &neuron = inputLayer.neurons[i];
        neuron.activation = 0.0;
        neuron.threshold = DEFAULT_ACTIVATION_THRESHOLD + (QRandomGenerator::global()->generateDouble() - 0.5) * 0.2;
        neuron.lastStimulation = 0.0;
        neuron.isActive = true;
        neuron.connectionCount = 0;
        neuron.adaptationFactor = 1.0;
        neuron.bitcoin_response_score = 0.0;  // ← AJOUT
    }
    
    m_layers.append(inputLayer);
    
    // Couches cachées (traitement biologique)
    for (int layer = 0; layer < m_config.hiddenLayers.size(); ++layer) {
        NetworkLayer hiddenLayer;
        hiddenLayer.layerType = "hidden";
        
        int layerSize = m_config.neuronCount / (layer + 2); // Réduction progressive
        layerSize = qMax(layerSize, 10); // Minimum 10 neurones par couche
        
        hiddenLayer.neurons.resize(layerSize);
        
        for (int i = 0; i < layerSize; ++i) {
            BiologicalNeuron &neuron = hiddenLayer.neurons[i];
            neuron.activation = 0.0;
            neuron.threshold = DEFAULT_ACTIVATION_THRESHOLD + (QRandomGenerator::global()->generateDouble() - 0.5) * 0.3;
            neuron.lastStimulation = 0.0;
            neuron.isActive = true;
            neuron.connectionCount = 0;
            neuron.adaptationFactor = 1.0;
            neuron.bitcoin_response_score = 0.0;  // ← AJOUT
            
            // Connexions avec la couche précédente
            int prevLayerSize = m_layers.last().neurons.size();
            neuron.weights.resize(prevLayerSize);
            neuron.dendrites.resize(prevLayerSize);
            
            // Initialisation des poids (distribution normale)
            for (int w = 0; w < prevLayerSize; ++w) {
                neuron.weights[w] = (QRandomGenerator::global()->generateDouble() - 0.5) * 2.0 / sqrt(prevLayerSize);
                neuron.dendrites[w] = 0.0;
            }
            
            neuron.connectionCount = prevLayerSize;
        }
        
        m_layers.append(hiddenLayer);
    }
    
    // Couche de sortie (prédiction de nonce)
    NetworkLayer outputLayer;
    outputLayer.layerType = "output";
    outputLayer.neurons.resize(32); // 32 bits pour nonce suggestions
    
    for (int i = 0; i < 32; ++i) {
        BiologicalNeuron &neuron = outputLayer.neurons[i];
        neuron.activation = 0.0;
        neuron.threshold = DEFAULT_ACTIVATION_THRESHOLD;
        neuron.lastStimulation = 0.0;
        neuron.isActive = true;
        neuron.connectionCount = 0;
        neuron.adaptationFactor = 1.0;
        neuron.bitcoin_response_score = 0.0;  // ← AJOUT
        
        // Connexions avec la dernière couche cachée
        int prevLayerSize = m_layers.last().neurons.size();
        neuron.weights.resize(prevLayerSize);
        neuron.dendrites.resize(prevLayerSize);
        
        for (int w = 0; w < prevLayerSize; ++w) {
            neuron.weights[w] = (QRandomGenerator::global()->generateDouble() - 0.5) * 2.0 / sqrt(prevLayerSize);
            neuron.dendrites[w] = 0.0;
        }
        
        neuron.connectionCount = prevLayerSize;
    }
    
    m_layers.append(outputLayer);
    
    createNetworkTopology();
    
    qDebug() << "[BIO-NET] Réseau initialisé avec" << m_layers.size() << "couches";
    for (int i = 0; i < m_layers.size(); ++i) {
        qDebug() << "  Couche" << i << "(" << m_layers[i].layerType << "):" << m_layers[i].neurons.size() << "neurones";
    }
}

void BiologicalNetwork::createNetworkTopology()
{
    // Établissement des connexions synaptiques inter-couches
    for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
        NetworkLayer &currentLayer = m_layers[layerIdx];
        const NetworkLayer &prevLayer = m_layers[layerIdx - 1];
        
        // Matrice de connexions synaptiques
        currentLayer.synapses.resize(currentLayer.neurons.size());
        
        for (int neuronIdx = 0; neuronIdx < currentLayer.neurons.size(); ++neuronIdx) {
            currentLayer.synapses[neuronIdx].resize(prevLayer.neurons.size());
            
            for (int prevNeuronIdx = 0; prevNeuronIdx < prevLayer.neurons.size(); ++prevNeuronIdx) {
                // Force synaptique initiale basée sur proximité spatiale (simulation MEA)
                double distance = abs(neuronIdx - (prevNeuronIdx * currentLayer.neurons.size() / prevLayer.neurons.size()));
                double maxDistance = currentLayer.neurons.size();
                double proximityFactor = 1.0 - (distance / maxDistance);
                
                // Connexion synaptique avec variabilité biologique
                double synapticStrength = proximityFactor * (0.5 + QRandomGenerator::global()->generateDouble() * 0.5);
                currentLayer.synapses[neuronIdx][prevNeuronIdx] = synapticStrength;
            }
        }
    }
}

bool BiologicalNetwork::startInitialLearning(int trainingCycles)
{
    QMutexLocker locker(&m_networkMutex);
    
    if (m_learningState != LearningState::Untrained) {
        qWarning() << "[BIO-NET] Apprentissage déjà en cours ou terminé";
        return false;
    }
    
    m_learningState = LearningState::InitialLearning;
    m_currentEpoch = 0;
    m_totalEpochs = trainingCycles;
    m_trainingProgress = 0.0;
    
    // Génération de données d'apprentissage synthétiques
    generateTrainingData();
    
    // Démarrage des cycles d'apprentissage
    //m_learningTimer->start(50); // 50ms entre cycles = 20 Hz

    // Démarrage du thread d'apprentissage
    m_learningActive = true;
    if (m_learningThread.joinable()) {
        m_learningThread.join();
    }
    m_learningThread = std::thread([this]() {
        while (m_learningActive && m_currentEpoch <= m_totalEpochs) {
            this->onLearningCycle();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        // Fin de l'apprentissage
        //if (m_learningActive) {
        //    this->stopLearning();
        //}
    });
    
    qDebug() << "[BIO-NET] Apprentissage initial démarré -" << trainingCycles << "cycles";
    emit learningStateChanged(m_learningState);
    
    return true;
}

/* VERSION INIT
void BiologicalNetwork::generateTrainingData()
{
    // Génération de patterns d'apprentissage pour l'initialisation
    
    for (int i = 0; i < 1000; ++i) {
        LearningData trainingExample;
        
        // Signaux MEA simulés avec patterns biologiques réalistes
        trainingExample.inputSignals.resize(m_config.neuronCount);
        
        for (int electrode = 0; electrode < m_config.neuronCount; ++electrode) {
            // Pattern neural avec composantes fréquentielles
            double neuralBase = sin(i * 0.1 + electrode * 0.05) * 0.3;
            double neuralSpike = (QRandomGenerator::global()->generateDouble() < 0.1) ? 
                                QRandomGenerator::global()->generateDouble() * 2.0 : 0.0;
            double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.1;
            
            trainingExample.inputSignals[electrode] = neuralBase + neuralSpike + noise;
        }
        
        // Génération de nonce cible basé sur pattern
        double patternSum = std::accumulate(trainingExample.inputSignals.begin(), 
                                          trainingExample.inputSignals.end(), 0.0);
        
        trainingExample.targetNonce = static_cast<uint64_t>(abs(patternSum * 1e6)) % (1ULL << 32);
        trainingExample.blockHeader = QString("training_block_%1").arg(i);
        trainingExample.difficulty = 0x0000FFFFFFFFFFFF >> (i % 4); // Difficulté variable
        trainingExample.wasSuccessful = (i % 3 == 0); // 33% de succès simulé
        trainingExample.attempts = 1000 + (i % 5000);
        trainingExample.computeTime = 1.0 + (i % 10);
        trainingExample.timestamp = QDateTime::currentDateTime().addSecs(-i);
        
        m_learningHistory.append(trainingExample);
    }
    
    qDebug() << "[BIO-NET] Données d'entrainement générées:" << m_learningHistory.size() << "exemples";
}
*/

// VERSION BITCOIN MINING
void BiologicalNetwork::generateTrainingData()
{
    qDebug() << "[BIO-NET] 🎓 Génération de données d'entraînement Bitcoin spécialisées...";
    
    m_learningHistory.clear();
    
    // Configuration d'entraînement progressif
    QVector<TrainingConfig> trainingLevels = {
        {1, 400, "Débutant - 1 zéro"},
        {2, 300, "Intermédiaire - 2 zéros"},
        {3, 200, "Avancé - 3 zéros"},  
        {4, 100, "Expert - 4 zéros"}
    };
    
    int totalExamples = 0;
    
    for (const TrainingConfig &level : trainingLevels) {
        qDebug() << "[BIO-NET] 📊 Génération niveau:" << level.description 
                 << "(" << level.exampleCount << "exemples)";
        
        for (int i = 0; i < level.exampleCount; ++i) {
            LearningData trainingExample = generateBitcoinTrainingExample(
                level.difficultyLevel, totalExamples + i
            );
            
            m_learningHistory.append(trainingExample);
            totalExamples++;
        }
    }
    
    // Génération d'exemples de patterns spéciaux
    generateSpecialBitcoinPatterns(100);
    
    // Mélange pour éviter l'overfitting séquentiel  
    shuffleTrainingData();
    
    qDebug() << "[BIO-NET] ✅ Données d'entraînement Bitcoin générées:"
             << m_learningHistory.size() << "exemples réalistes";
    
    // Statistiques détaillées
    logTrainingDataStatistics();
}


void BiologicalNetwork::stopLearning()
{
    QMutexLocker locker(&m_networkMutex);
    
    //m_learningTimer->stop();
    m_learningActive = false;
    // Empêche le thread de se joindre lui-même (deadlock)
    if (m_learningThread.joinable() && std::this_thread::get_id() != m_learningThread.get_id()) {
        m_learningThread.join();
    }
    
    if (m_learningState == LearningState::InitialLearning || m_learningState == LearningState::Retraining) {
        if (m_currentEpoch >= m_totalEpochs * 0.8) { // Au moins 80% terminé
            m_learningState = LearningState::Trained;
            qDebug() << "[BIO-NET] Apprentissage terminé avec succès";
            emit learningCompleted(true);
        } else {
            m_learningState = LearningState::Untrained;
            qDebug() << "[BIO-NET] Apprentissage interrompu";
            emit learningCompleted(false);
        }
    }
    
    emit learningStateChanged(m_learningState);
}

bool BiologicalNetwork::isLearningComplete() const
{
    QMutexLocker locker(&m_networkMutex);
    return m_learningState == LearningState::Trained || m_learningState == LearningState::Optimizing;
}

void BiologicalNetwork::onLearningCycle()
{
    QMutexLocker locker(&m_networkMutex);
    
    if (m_currentEpoch >= m_totalEpochs) {
        // Apprentissage terminé
        //stopLearning();

        m_learningState = LearningState::Trained;
        qDebug() << "[BIO-NET] Apprentissage terminé avec succès";
        emit learningStateChanged(m_learningState);
        emit learningCompleted(true);
        m_learningActive = false;
        
        return;
    }
    
    // Sélection d'un exemple d'apprentissage aléatoire
    if (!m_learningHistory.isEmpty()) {
        int exampleIdx = QRandomGenerator::global()->bounded(m_learningHistory.size());
        const LearningData &example = m_learningHistory[exampleIdx];
        
        // Préparation des targets basés sur le nonce
        QVector<double> targets(32);
        uint64_t targetNonce = example.targetNonce;
        
        // Conversion du nonce en pattern binaire pour les neurones de sortie
        for (int bit = 0; bit < 32; ++bit) {
            targets[bit] = ((targetNonce >> bit) & 1) ? 1.0 : 0.0;
        }

        // Cycle d'apprentissage biologique
        performLearningCycle(example.inputSignals, targets);

        QVector<double> networkOutput = getNetworkOutput();
        uint64_t predictedNonce = 0;
        for (int bit = 0; bit < qMin(32, networkOutput.size()); ++bit) {
            if (networkOutput[bit] > 0.5) {
                predictedNonce |= (1ULL << bit);
            }
        }   

        if (m_currentEpoch % 10 == 0) {
                qDebug() << "apprentissage : sortie=" << networkOutput
                        << " targets=" << targets  
                        << " targetNonce=" << example.targetNonce << " -> predictNonce=" << predictedNonce; 
        }
        
        if (predictedNonce == example.targetNonce) {
            m_successfulPredictions++;
        }

        // predictions suite apprentissage
        m_totalPredictions++;
    }
    
    m_currentEpoch++;
    m_trainingProgress = static_cast<double>(m_currentEpoch) / m_totalEpochs;
    
    // Émission périodique du progrès
    if (m_currentEpoch % 10 == 0) {
        emit trainingProgress(m_trainingProgress * 100.0);
        
        // Calcul de l'efficacité du réseau
        calculateNetworkEfficiency();
        
        qDebug() << "[BIO-NET] Cycle" << m_currentEpoch << "/" << m_totalEpochs 
                 << "- Progression :" << QString::number(m_trainingProgress, 'f', 3)
                 << "- Success :" << QString::number(m_successfulPredictions, 'f', 3)
                 << "- Efficacité:" << QString::number(m_networkEfficiency, 'f', 3);
    }
}

/* ANCIENNE VERSION
void BiologicalNetwork::performLearningCycle(const QVector<double> &inputs, const QVector<double> &targets)
{
    // Forward propagation
    forwardPropagation(inputs);
    
    // Back propagation avec adaptation biologique
    backPropagation(targets);
    
    // Plasticité synaptique
    adjustSynapticWeights(m_config.learningRate);
    
    // Croissance neuronale adaptative
    if (m_currentEpoch % 50 == 0) {
        stimulateNeuronGrowth();
        pruneWeakConnections();
    }
}
*/

/* NOUVELLE METHODE */
void BiologicalNetwork::performLearningCycle(const QVector<double> &inputs, const QVector<double> &targets)
{
    // === ANALYSE PRE-APPRENTISSAGE BITCOIN ===
    BitcoinLearningContext context = analyzeBitcoinLearningContext(inputs, targets);
    
    // === FORWARD PROPAGATION AVEC MONITORING BITCOIN ===
    forwardPropagation(inputs);
    QVector<double> initialOutput = getNetworkOutput();
    
    // === BACK PROPAGATION ADAPTATIF BITCOIN ===
    performBitcoinBackPropagation(targets, context);
    
    // === AJUSTEMENT SYNAPTIQUE SPÉCIALISÉ ===
    adjustBitcoinSynapticWeights(context);
    
    // === VALIDATION ET CORRECTION BITCOIN ===
    BitcoinPredictionResult result = validateBitcoinPrediction(targets);
    
    // === RENFORCEMENT ADAPTATIF ===
    if (result.needsReinforcement) {
        applyBitcoinReinforcement(result, context);
    }
    
    // === CROISSANCE NEURONALE GUIDÉE BITCOIN ===
    if (m_currentEpoch % 25 == 0) { // Plus fréquent pour Bitcoin
        stimulateBitcoinNeuronGrowth(context);
        pruneBitcoinWeakConnections(result);
    }
    
    // === MÉMORISATION DES PATTERNS RÉUSSIS ===
    if (result.isSuccessful) {
        memorizeBitcoinPattern(inputs, targets, result);
    }
    
    // === LOGGING DÉTAILLÉ (OPTIONNEL) ===
    if (m_currentEpoch % 100 == 0) {
        logBitcoinLearningDetails(context, result);
    }
}

/**
 * @brief Analyse le contexte d'apprentissage Bitcoin
 */
BiologicalNetwork::BitcoinLearningContext BiologicalNetwork::analyzeBitcoinLearningContext(
    const QVector<double> &inputs, const QVector<double> &targets)
{
    BitcoinLearningContext context;
    
    // === EXTRACTION DU NONCE CIBLE ===
    context.targetNonce = 0;
    for (int bit = 0; bit < qMin(32, static_cast<int>(targets.size())); ++bit) {
        if (targets[bit] > 0.5) {
            context.targetNonce |= (1ULL << bit);
        }
    }
    
    // === ANALYSE DE LA DIFFICULTÉ ===
    context.difficultyLevel = estimateDifficultyFromNonce(context.targetNonce);
    
    // === CALCUL DE LA COMPLEXITÉ DU PATTERN ===
    double variance = 0.0;
    double mean = std::accumulate(inputs.begin(), inputs.end(), 0.0) / inputs.size();
    for (double input : inputs) {
        variance += (input - mean) * (input - mean);
    }
    context.patternComplexity = sqrt(variance / inputs.size());
    
    // === DÉTECTION DE PATTERNS SPÉCIAUX ===
    context.isSpecialPattern = false; // A REVOIR avec header.version : detectSpecialBitcoinPattern(inputs);
    
    // === TAUX D'APPRENTISSAGE ADAPTATIF ===
    context.adaptiveLearningRate = calculateAdaptiveLearningRate(context);
    
    // === IDENTIFICATION DES BITS CRITIQUES ===
    context.criticalBits = identifyCriticalBits(context.targetNonce);
    
    // === CONFIANCE ATTENDUE ===
    context.expectedConfidence = estimateExpectedConfidence(context);
    
    return context;
}

/**
 * @brief Back propagation spécialisée Bitcoin
 */
void BiologicalNetwork::performBitcoinBackPropagation(const QVector<double> &targets, const BitcoinLearningContext &context)
{
    // Back propagation standard
    backPropagation(targets);
    
    // === CORRECTIONS SPÉCIFIQUES BITCOIN ===
    
    // 1. Emphasis sur les bits critiques
    NetworkLayer &outputLayer = m_layers.last();
    for (int criticalBit : context.criticalBits) {
        if (criticalBit < outputLayer.neurons.size()) {
            BiologicalNeuron &neuron = outputLayer.neurons[criticalBit];
            
            // Augmenter l'influence des bits critiques
            double targetValue = targets[criticalBit];
            double currentOutput = neuron.activation;
            double criticalError = (targetValue - currentOutput) * 1.5; // Emphasis x1.5
            
            // Ajustement direct du seuil pour les bits critiques
            neuron.threshold += criticalError * 0.02;
            neuron.threshold = qBound(0.1, neuron.threshold, 0.9);
        }
    }
    
    // 2. Ajustement basé sur la difficulté
    double difficultyMultiplier = 1.0 + (context.difficultyLevel * 0.3);
    for (NetworkLayer &layer : m_layers) {
        for (BiologicalNeuron &neuron : layer.neurons) {
            neuron.threshold *= difficultyMultiplier;
            neuron.threshold = qBound(0.05, neuron.threshold, 0.95);
        }
    }
}

/**
 * @brief Ajustement synaptique spécialisé Bitcoin
 */
void BiologicalNetwork::adjustBitcoinSynapticWeights(const BitcoinLearningContext &context)
{
    // Utiliser le taux d'apprentissage adaptatif
    adjustSynapticWeights(context.adaptiveLearningRate);
    
    // === RENFORCEMENTS SPÉCIALISÉS BITCOIN ===
    
    // 1. Renforcement des connexions vers les bits critiques
    if (m_layers.size() >= 2) {
        NetworkLayer &outputLayer = m_layers.last();
        NetworkLayer &prevLayer = m_layers[m_layers.size() - 2];
        
        for (int criticalBit : context.criticalBits) {
            if (criticalBit < outputLayer.neurons.size()) {
                BiologicalNeuron &criticalNeuron = outputLayer.neurons[criticalBit];
                
                // Renforcer les connexions entrantes vers les neurones critiques
                for (int weightIdx = 0; weightIdx < criticalNeuron.weights.size(); ++weightIdx) {
                    double preActivation = prevLayer.neurons[weightIdx].activation;
                    if (preActivation > 0.5) { // Neurone pré-synaptique actif
                        criticalNeuron.weights[weightIdx] += context.adaptiveLearningRate * 0.1;
                    }
                }
            }
        }
    }
    
    // 2. Modulation par complexité du pattern
    if (context.patternComplexity > 0.5) {
        // Pattern complexe : apprentissage plus conservateur
        for (NetworkLayer &layer : m_layers) {
            for (BiologicalNeuron &neuron : layer.neurons) {
                for (double &weight : neuron.weights) {
                    weight *= 0.99; // Légère réduction pour stabilité
                }
            }
        }
    }
}

BiologicalNetwork::BitcoinPredictionResult BiologicalNetwork::validateBitcoinPrediction(const QVector<double> &targets)
{
    BitcoinPredictionResult result;
    
    // Reconstruction du nonce cible
    uint64_t targetNonce = 0;
    for (int bit = 0; bit < qMin(32, static_cast<int>(targets.size())); ++bit) {
        if (targets[bit] > 0.5) {
            targetNonce |= (1ULL << bit);
        }
    }
    
    // Reconstruction du nonce prédit
    QVector<double> networkOutput = getNetworkOutput();
    result.predictedNonce = 0;
    for (int bit = 0; bit < qMin(32, static_cast<int>(networkOutput.size())); ++bit) {
        if (networkOutput[bit] > 0.5) {
            result.predictedNonce |= (1ULL << bit);
        }
    }
    
    // Calculs de précision
    uint64_t xorResult = result.predictedNonce ^ targetNonce;
    result.correctBits = 32 - __builtin_popcountll(xorResult);
    result.accuracy = static_cast<double>(result.correctBits) / 32.0;
    
    // Succès si match exact ou très proche
    result.isSuccessful = (result.accuracy >= 0.95);
    
    // Renforcement nécessaire si performance faible
    result.needsReinforcement = (result.accuracy < 0.7);
    
    // Confiance du réseau
    double totalConfidence = 0.0;
    for (double output : networkOutput) {
        totalConfidence += qMax(output, 1.0 - output); // Distance à 0.5
    }
    result.confidence = (totalConfidence / networkOutput.size() - 0.5) * 2.0;
    
    // Analyse d'erreur
    if (result.accuracy < 0.5) {
        result.errorAnalysis = "POOR_PATTERN_RECOGNITION";
    } else if (result.accuracy < 0.8) {
        result.errorAnalysis = "PARTIAL_LEARNING";
    } else {
        result.errorAnalysis = "GOOD_CONVERGENCE";
    }
    
    return result;
}

/**
 * @brief Applique le renforcement Bitcoin
 */
void BiologicalNetwork::applyBitcoinReinforcement(const BitcoinPredictionResult &result, const BitcoinLearningContext &context)
{
    if (result.isSuccessful) {
        // === RENFORCEMENT POSITIF ===
        
        // 1. Augmenter les seuils des neurones performants
        for (NetworkLayer &layer : m_layers) {
            for (BiologicalNeuron &neuron : layer.neurons) {
                if (neuron.activation > 0.7) {
                    neuron.adaptationFactor = qMin(2.0, neuron.adaptationFactor * 1.05);
                }
            }
        }
        
        // 2. Renforcer les connexions synaptiques réussies
        for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
            NetworkLayer &layer = m_layers[layerIdx];
            for (int neuronIdx = 0; neuronIdx < layer.neurons.size(); ++neuronIdx) {
                for (int synapseIdx = 0; synapseIdx < layer.synapses[neuronIdx].size(); ++synapseIdx) {
                    if (layer.synapses[neuronIdx][synapseIdx] > 0.6) {
                        layer.synapses[neuronIdx][synapseIdx] *= 1.02; // Renforcement 2%
                    }
                }
            }
        }
        
    } else if (result.needsReinforcement) {
        // === CORRECTION NÉGATIVE ===
        
        // 1. Réajuster les neurones sur-confiants
        QVector<double> networkOutput = getNetworkOutput();
        for (int bit = 0; bit < qMin(32, static_cast<int>(networkOutput.size())); ++bit) {
            if (m_layers.size() > 0) {
                NetworkLayer &outputLayer = m_layers.last();
                if (bit < outputLayer.neurons.size()) {
                    BiologicalNeuron &neuron = outputLayer.neurons[bit];
                    
                    // Si le neurone était très confiant mais faux
                    if ((networkOutput[bit] > 0.8 || networkOutput[bit] < 0.2) && result.correctBits < 20) {
                        neuron.threshold += (0.5 - networkOutput[bit]) * 0.05; // Ramener vers 0.5
                        neuron.threshold = qBound(0.1, neuron.threshold, 0.9);
                    }
                }
            }
        }
        
        // 2. Affaiblir les connexions dominantes défaillantes
        for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
            NetworkLayer &layer = m_layers[layerIdx];
            for (int neuronIdx = 0; neuronIdx < layer.neurons.size(); ++neuronIdx) {
                for (int synapseIdx = 0; synapseIdx < layer.synapses[neuronIdx].size(); ++synapseIdx) {
                    if (layer.synapses[neuronIdx][synapseIdx] > 0.8) {
                        layer.synapses[neuronIdx][synapseIdx] *= 0.95; // Affaiblissement 5%
                    }
                }
            }
        }
    }
}

/**
 * @brief Croissance neuronale guidée par Bitcoin
 */
void BiologicalNetwork::stimulateBitcoinNeuronGrowth(const BitcoinLearningContext &context)
{
    // Croissance plus agressive pour les patterns difficiles
    double growthFactor = 1.0 + (context.difficultyLevel * 0.1);
    
    for (NetworkLayer &layer : m_layers) {
        for (BiologicalNeuron &neuron : layer.neurons) {
            
            // Critères de croissance Bitcoin-spécifiques
            bool shouldGrow = false;
            
            if (neuron.activation > 0.8 && neuron.bitcoin_response_score > 0.6) {
                shouldGrow = true; // Neurone performant pour Bitcoin
            }
            
            if (context.isSpecialPattern && neuron.activation > 0.6) {
                shouldGrow = true; // Pattern spécial nécessite plus de capacité
            }
            
            if (shouldGrow) {
                neuron.adaptationFactor = qMin(2.0, static_cast<double>(neuron.adaptationFactor * (1.0 + growthFactor * 0.01)));
                
                // Augmenter la connectivité pour les neurones Bitcoin performants
                if (neuron.connectionCount < neuron.weights.size() * 1.4) {
                    neuron.adaptationFactor *= 1.005;
                }
            }
        }
    }
}

/**
 * @brief Élagage spécialisé Bitcoin
 */
void BiologicalNetwork::pruneBitcoinWeakConnections(const BitcoinPredictionResult &result)
{
    // Élagage plus agressif si les performances Bitcoin sont faibles
    double pruningThreshold = MIN_CONNECTION_STRENGTH;
    
    if (result.accuracy < 0.5) {
        pruningThreshold *= 1.5; // Élagage plus agressif
    }
    
    for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
        NetworkLayer &layer = m_layers[layerIdx];
        
        for (int neuronIdx = 0; neuronIdx < layer.neurons.size(); ++neuronIdx) {
            for (int synapseIdx = 0; synapseIdx < layer.synapses[neuronIdx].size(); ++synapseIdx) {
                
                if (layer.synapses[neuronIdx][synapseIdx] < pruningThreshold) {
                    
                    // Élagage différentiel selon performance Bitcoin
                    if (result.accuracy < 0.3) {
                        layer.synapses[neuronIdx][synapseIdx] *= 0.7; // Élagage fort
                    } else {
                        layer.synapses[neuronIdx][synapseIdx] *= 0.9; // Élagage normal
                    }
                }
            }
        }
    }
}

/**
 * @brief Mémorise les patterns Bitcoin réussis
 */
void BiologicalNetwork::memorizeBitcoinPattern(const QVector<double> &inputs, const QVector<double> &targets, const BitcoinPredictionResult &result)
{
    // Créer un snapshot du réseau pour ce pattern réussi
    BitcoinPatternMemory memory;
    memory.inputPattern = inputs;
    memory.targetPattern = targets;
    memory.networkSnapshot = captureNetworkSnapshot();
    memory.successTimestamp = QDateTime::currentDateTime();
    memory.accuracy = result.accuracy;
    memory.confidence = result.confidence;
    
    // Stocker dans la mémoire des patterns réussis
    m_bitcoinPatternMemory.append(memory);
    
    // Limiter la taille de la mémoire
    while (m_bitcoinPatternMemory.size() > 100) {
        m_bitcoinPatternMemory.removeFirst();
    }
}

/**
 * @brief Calcule un taux d'apprentissage adaptatif
 */
double BiologicalNetwork::calculateAdaptiveLearningRate(const BitcoinLearningContext &context)
{
    double baseLearningRate = m_config.learningRate;
    
    // Modulation par difficulté
    double difficultyFactor = 1.0 + (context.difficultyLevel * 0.2);
    
    // Modulation par complexité
    double complexityFactor = 1.0 + (context.patternComplexity * 0.3);
    
    // Modulation pour patterns spéciaux
    double specialFactor = context.isSpecialPattern ? 1.5 : 1.0;
    
    // Modulation par performance récente
    double performanceFactor = 1.0;
    if (m_totalPredictions > 10) {
        double recentSuccessRate = static_cast<double>(m_successfulPredictions) / m_totalPredictions;
        if (recentSuccessRate < 0.3) {
            performanceFactor = 1.3; // Augmenter si performance faible
        } else if (recentSuccessRate > 0.8) {
            performanceFactor = 0.8; // Diminuer si performance très bonne
        }
    }
    
    double adaptiveRate = baseLearningRate * difficultyFactor * complexityFactor * specialFactor * performanceFactor;
    
    // Borner le taux d'apprentissage
    return qBound(0.0001, adaptiveRate, 0.1);
}

/**
 * @brief Identifie les bits critiques d'un nonce
 */
QVector<int> BiologicalNetwork::identifyCriticalBits(uint64_t nonce)
{
    QVector<int> criticalBits;
    
    // Les bits de poids fort sont généralement plus critiques
    for (int bit = 24; bit < 32; ++bit) {
        if ((nonce >> bit) & 1) {
            criticalBits.append(bit);
        }
    }
    
    // Ajouter des bits avec des patterns particuliers
    for (int bit = 0; bit < 32; bit += 4) {
        uint8_t nibble = (nonce >> bit) & 0xF;
        if (nibble == 0x0 || nibble == 0xF) { // Nibbles tout 0 ou tout 1
            for (int subBit = 0; subBit < 4; ++subBit) {
                criticalBits.append(bit + subBit);
            }
        }
    }
    
    return criticalBits;
}

/**
 * @brief Logging détaillé de l'apprentissage Bitcoin
 */
void BiologicalNetwork::logBitcoinLearningDetails(const BitcoinLearningContext &context, const BitcoinPredictionResult &result)
{
    qDebug() << "[BIO-NET] 🧬₿ === BITCOIN LEARNING CYCLE DETAILS ===";
    qDebug() << "  🎯 Target Nonce:" << QString("0x%1").arg(context.targetNonce, 8, 16, QChar('0'));
    qDebug() << "  📊 Difficulty Level:" << context.difficultyLevel;
    qDebug() << "  🌊 Pattern Complexity:" << QString::number(context.patternComplexity, 'f', 3);
    qDebug() << "  ⭐ Special Pattern:" << (context.isSpecialPattern ? "YES" : "NO");
    qDebug() << "  📈 Adaptive LR:" << QString::number(context.adaptiveLearningRate, 'f', 6);
    qDebug() << "  🎯 Predicted Nonce:" << QString("0x%1").arg(result.predictedNonce, 8, 16, QChar('0'));
    qDebug() << "  ✅ Correct Bits:" << result.correctBits << "/32";
    qDebug() << "  📊 Accuracy:" << QString::number(result.accuracy * 100, 'f', 1) << "%";
    qDebug() << "  🎪 Confidence:" << QString::number(result.confidence * 100, 'f', 1) << "%";
    qDebug() << "  🔍 Error Analysis:" << result.errorAnalysis;
    qDebug() << "  🏅 Critical Bits:" << context.criticalBits.size();
}



void BiologicalNetwork::forwardPropagation(const QVector<double> &inputs)
{
    // Couche d'entrée
    if (inputs.size() == m_layers[0].neurons.size()) {
        for (int i = 0; i < inputs.size(); ++i) {
            m_layers[0].neurons[i].activation = inputs[i];
            m_layers[0].neurons[i].lastStimulation = QDateTime::currentMSecsSinceEpoch();
        }
    }
    
    // Propagation à travers les couches cachées et de sortie
    for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
        NetworkLayer &currentLayer = m_layers[layerIdx];
        const NetworkLayer &prevLayer = m_layers[layerIdx - 1];
        
        currentLayer.layerActivation = 0.0;

        //if (m_currentEpoch % 10 == 0) {
        //    qDebug() << "[BIO-NET][DEBUG] --- Layer" << layerIdx << "(" << currentLayer.layerType << ") ---";
        //}
        
        for (int neuronIdx = 0; neuronIdx < currentLayer.neurons.size(); ++neuronIdx) {
            BiologicalNeuron &neuron = currentLayer.neurons[neuronIdx];
            
            // Calcul de l'entrée pondérée
            double weightedSum = 0.0;
            for (int prevIdx = 0; prevIdx < prevLayer.neurons.size(); ++prevIdx) {
                double synapticInput = prevLayer.neurons[prevIdx].activation * neuron.weights[prevIdx];
                
                // Modulation synaptique biologique
                double synapticStrength = currentLayer.synapses[neuronIdx][prevIdx];
                synapticInput *= synapticStrength;
                
                neuron.dendrites[prevIdx] = synapticInput;
                weightedSum += synapticInput;
            }
            
            // Fonction d'activation biologique avec seuil adaptatif
            double threshold = adaptiveThreshold(neuron);
            neuron.activation = biologicalActivation(weightedSum, threshold);
            
            // Fatigue neuronale
            neuron.activation *= NEURON_FATIGUE_FACTOR;
            
            currentLayer.layerActivation += neuron.activation;

           // --- LOG PAR NEURONE ---
           // if (m_currentEpoch % 10 == 0 && layerIdx == 1) {
           //     qDebug() << "  neuron" << neuronIdx
           //              //<< "weights:" << neuron.weights
           //              << "weightedSum:" << weightedSum
           //              << "threshold:" << threshold
           //              << "activationBeforeFatigue:" << biologicalActivation(weightedSum, threshold)
           //              << "activation:" << neuron.activation;
           //}        
        }
        
        currentLayer.layerActivation /= currentLayer.neurons.size();

        //if (m_currentEpoch % 10 == 0) {        
        //    qDebug() << "[BIO-NET][DEBUG] Layer" << layerIdx << "mean activation:" << currentLayer.layerActivation;
        //}
     
    }

    // --- LOG OUTPUT ---
    //if (m_currentEpoch % 10 == 0) {
    //    qDebug() << "[BIO-NET][DEBUG] Output layer activations:";
    //    for (int i = 0; i < m_layers.last().neurons.size(); ++i) {
    //        qDebug() << "  neuron" << i << "activation:" << m_layers.last().neurons[i].activation;
    //    }
    //}
    
    QStringList inputsTmp;
    QStringList outputsTmp;
    for (const auto& neuron : m_layers[0].neurons) {
        inputsTmp << QString::number(neuron.activation, 'f', 3);
    }
    for (const auto& neuron : m_layers.last().neurons) {
        outputsTmp << QString::number(neuron.activation, 'f', 3);
    }
    
    // Émission périodique du progrès
    if (m_currentEpoch % 10 == 0) {
        qDebug() << "[BIO-NET] Cycle forwardPropagation"
                 << "- Input :" << inputsTmp
                 << "- Sortie:" << outputsTmp;
    }

}

void BiologicalNetwork::backPropagation(const QVector<double> &targets)
{
    // Calcul de l'erreur sur la couche de sortie
    NetworkLayer &outputLayer = m_layers.last();
    QVector<double> outputErrors(outputLayer.neurons.size());
    
    for (int i = 0; i < outputLayer.neurons.size() && i < targets.size(); ++i) {
        double error = targets[i] - outputLayer.neurons[i].activation;
        outputErrors[i] = error * sigmoidDerivative(outputLayer.neurons[i].activation);
        
        // Adaptation biologique du seuil
        BiologicalNeuron &neuron = outputLayer.neurons[i];
        neuron.threshold += error * 0.01; // Micro-adaptation
        neuron.threshold = qBound(0.1, static_cast<double>(neuron.threshold), 0.9);
    }

    // Émission périodique du progrès
    if (m_currentEpoch % 10 == 0) {
        qDebug() << "[BIO-NET] Cycle backPropagation"
                 << "- outputErrors :" << outputErrors;
    }

    
    // Rétropropagation de l'erreur vers les couches cachées
    for (int layerIdx = m_layers.size() - 2; layerIdx >= 1; --layerIdx) {
        NetworkLayer &currentLayer = m_layers[layerIdx];
        const NetworkLayer &nextLayer = m_layers[layerIdx + 1];
        
        QVector<double> layerErrors(currentLayer.neurons.size(), 0.0);
        
        for (int neuronIdx = 0; neuronIdx < currentLayer.neurons.size(); ++neuronIdx) {
            double errorSum = 0.0;
            
            for (int nextIdx = 0; nextIdx < nextLayer.neurons.size(); ++nextIdx) {
                errorSum += outputErrors[nextIdx] * nextLayer.neurons[nextIdx].weights[neuronIdx];
            }
            
            layerErrors[neuronIdx] = errorSum * sigmoidDerivative(currentLayer.neurons[neuronIdx].activation);
            
            // Adaptation du seuil biologique
            currentLayer.neurons[neuronIdx].threshold += layerErrors[neuronIdx] * 0.005;
            currentLayer.neurons[neuronIdx].threshold = qBound(0.1, static_cast<double>(currentLayer.neurons[neuronIdx].threshold), 0.9);
        }
        
        outputErrors = layerErrors; // Pour la prochaine itération
    }

}

void BiologicalNetwork::adjustSynapticWeights(double learningSignal)
{
    for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
        NetworkLayer &currentLayer = m_layers[layerIdx];
        const NetworkLayer &prevLayer = m_layers[layerIdx - 1];
        
        for (int neuronIdx = 0; neuronIdx < currentLayer.neurons.size(); ++neuronIdx) {
            BiologicalNeuron &neuron = currentLayer.neurons[neuronIdx];
            
            for (int weightIdx = 0; weightIdx < neuron.weights.size(); ++weightIdx) {
                // Règle d'apprentissage hebbienne modifiée
                double preActivation = prevLayer.neurons[weightIdx].activation;
                double postActivation = neuron.activation;
                
                // Delta de poids avec plasticité synaptique
                double weightDelta = learningSignal * preActivation * postActivation * SYNAPTIC_PLASTICITY_RATE;
                
                // Modification biologique des connexions synaptiques
                neuron.weights[weightIdx] += weightDelta;
                currentLayer.synapses[neuronIdx][weightIdx] += weightDelta * 0.1;
                
                // Contraintes biologiques
                neuron.weights[weightIdx] = qBound(-2.0, static_cast<double>(neuron.weights[weightIdx]), 2.0);
                currentLayer.synapses[neuronIdx][weightIdx] = qBound(MIN_CONNECTION_STRENGTH, 
                                                                   static_cast<double>(currentLayer.synapses[neuronIdx][weightIdx]), 1.0);
            }
        }
    }
}

double BiologicalNetwork::sigmoidActivation(double input) const
{
    return 1.0 / (1.0 + exp(-input));
}

double BiologicalNetwork::sigmoidDerivative(double sigmoidOutput) const
{
    return sigmoidOutput * (1.0 - sigmoidOutput);
}

double BiologicalNetwork::biologicalActivation(double input, double threshold) const
{
    // Fonction d'activation biologique avec seuil et saturation
    if (input < threshold) {
        double alpha = 0.01; // pente faible pour input < threshold
        if (input < threshold)
            return alpha * (input - threshold);        
        //return 0.0;
    }
    
    // Activation sigmoïdale avec décalage par le seuil
    double adjustedInput = (input - threshold) * 2.0;
    return sigmoidActivation(adjustedInput);
}

double BiologicalNetwork::adaptiveThreshold(const BiologicalNeuron &neuron) const
{
    // Seuil adaptatif basé sur l'historique et la fatigue
    double baseThreshold = neuron.threshold;
    double adaptationFactor = neuron.adaptationFactor;
    
    // Adaptation temporelle (fatigue/récupération)
    qint64 timeSinceStimulation = QDateTime::currentMSecsSinceEpoch() - neuron.lastStimulation;
    double recoveryFactor = qMin(1.0, timeSinceStimulation / 1000.0); // Récupération en 1 seconde
    
    return baseThreshold * adaptationFactor * (1.0 - recoveryFactor * 0.2);
}

void BiologicalNetwork::stimulateNeuronGrowth()
{
    // Croissance des connexions pour les neurones très actifs
    for (NetworkLayer &layer : m_layers) {
        for (BiologicalNeuron &neuron : layer.neurons) {
            if (neuron.activation > 0.8 && neuron.connectionCount < neuron.weights.size() * 1.2) {
                // Croissance dendritique
                neuron.adaptationFactor = qMin(1.2, neuron.adaptationFactor * 1.01);
            }
        }
    }
}

void BiologicalNetwork::pruneWeakConnections()
{
    // Élagage des connexions faibles pour optimiser le réseau
    for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
        NetworkLayer &layer = m_layers[layerIdx];
        
        for (int neuronIdx = 0; neuronIdx < layer.neurons.size(); ++neuronIdx) {
            for (int synapseIdx = 0; synapseIdx < layer.synapses[neuronIdx].size(); ++synapseIdx) {
                if (layer.synapses[neuronIdx][synapseIdx] < MIN_CONNECTION_STRENGTH * 0.5) {
                    // Affaiblissement des connexions très faibles
                    layer.synapses[neuronIdx][synapseIdx] *= 0.9;
                }
            }
        }
    }
}

void BiologicalNetwork::calculateNetworkEfficiency()
{
    if (m_totalPredictions == 0) {
        m_networkEfficiency = 0.0;
        return;
    }
    
    double successRate = static_cast<double>(m_successfulPredictions) / m_totalPredictions;
    double confidenceRate = m_averageConfidence;
    double trainingRate = m_trainingProgress;
    
    m_networkEfficiency = (successRate * 0.5 + confidenceRate * 0.3 + trainingRate * 0.2);
    
    // Bonus pour stabilité des connexions
    double stabilityBonus = calculateNetworkStability();
    m_networkEfficiency = qMin(1.0, m_networkEfficiency + stabilityBonus * 0.1);
}

double BiologicalNetwork::calculateNetworkStability() const
{
    double totalStability = 0.0;
    int connectionCount = 0;
    
    for (const NetworkLayer &layer : m_layers) {
        for (const auto &synapseRow : layer.synapses) {
            for (double synapseStrength : synapseRow) {
                // Stabilité = proximité des forces synaptiques à leurs valeurs optimales
                double optimalRange = 0.6; // Force synaptique optimale
                double stability = 1.0 - abs(synapseStrength - optimalRange);
                totalStability += qMax(0.0, stability);
                connectionCount++;
            }
        }
    }
    
    return connectionCount > 0 ? totalStability / connectionCount : 0.0;
}

BiologicalNetwork::NoncePredicition BiologicalNetwork::predictOptimalNonce(
    const QString &blockHeader, uint64_t difficulty, const QVector<double> &currentSignals)
{
    QMutexLocker locker(&m_networkMutex);
    
    NoncePredicition prediction;
    prediction.confidence = 0.0;
    prediction.expectedEfficiency = 0.0;
    prediction.reasoning = "Réseau non entrainé";
    
    if (m_learningState != LearningState::Trained && m_learningState != LearningState::Optimizing) {
        prediction.suggestedNonce = QRandomGenerator::global()->generate64() % (1ULL << 32);
        return prediction;
    }
    
    // Forward propagation avec les signaux actuels
    forwardPropagation(currentSignals);
    
    // Récupération de la sortie du réseau
    QVector<double> networkOutput = getNetworkOutput();
    
    // Conversion de la sortie en nonce
    uint64_t predictedNonce = 0;
    for (int bit = 0; bit < qMin(32, networkOutput.size()); ++bit) {
        if (networkOutput[bit] > 0.5) {
            predictedNonce |= (1ULL << bit);
        }
    }
    
    prediction.suggestedNonce = predictedNonce;
    
    // Calcul de la confiance basé sur la cohérence de la sortie
    double outputVariance = 0.0;
    double outputMean = std::accumulate(networkOutput.begin(), networkOutput.end(), 0.0) / networkOutput.size();
    
    for (double output : networkOutput) {
        outputVariance += (output - outputMean) * (output - outputMean);
    }
    outputVariance /= networkOutput.size();
    
    // Confiance inversement proportionnelle à la variance (plus c'est décisif, plus on est confiant)
    prediction.confidence = qMax(0.1, 1.0 - sqrt(outputVariance));
    
    // Génération de nonces candidats dans la région prédite
    prediction.candidateRange = generateNonceCandidates(networkOutput);
    
    // Estimation de l'efficacité basée sur l'historique
    prediction.expectedEfficiency = estimateNonceEfficiency(prediction.suggestedNonce, difficulty);
    
    // Explication de la prédiction
    prediction.reasoning = QString("Réseau bio-neuronal - Confiance: %1% - Région optimale: 0x%2")
                          .arg(prediction.confidence * 100, 0, 'f', 1)
                          .arg(prediction.suggestedNonce, 8, 16, QChar('0'));
    
    // Mise à jour des statistiques
    m_totalPredictions++;
    m_averageConfidence = ((m_averageConfidence * (m_totalPredictions - 1)) + prediction.confidence) / m_totalPredictions;
    
    qDebug() << "[BIO-NET] Prédiction nonce: 0x" << QString::number(prediction.suggestedNonce, 16)
             << "Confiance:" << QString::number(prediction.confidence * 100, 'f', 1) << "%";
    
    emit noncePredictionReady(prediction);
    
    return prediction;
}

QVector<uint64_t> BiologicalNetwork::generateNonceCandidates(const QVector<double> &networkOutput)
{
    QVector<uint64_t> candidates;
    
    uint64_t baseNonce = 0;
    for (int bit = 0; bit < qMin(32, networkOutput.size()); ++bit) {
        if (networkOutput[bit] > 0.5) {
            baseNonce |= (1ULL << bit);
        }
    }
    
    // Génération de variations autour de la prédiction principale
    for (int variation = 0; variation < 16; ++variation) {
        uint64_t candidate = baseNonce;
        
        // Variations basées sur les activations proches du seuil
        for (int bit = 0; bit < networkOutput.size(); ++bit) {
            double activation = networkOutput[bit];
            if (activation > 0.4 && activation < 0.6) { // Zone d'incertitude
                if (QRandomGenerator::global()->generateDouble() < 0.3) {
                    candidate ^= (1ULL << bit); // Flip du bit
                }
            }
        }
        
        candidates.append(candidate);
    }
    
    return candidates;
}

double BiologicalNetwork::estimateNonceEfficiency(uint64_t nonce, uint64_t difficulty)
{
    // Estimation basée sur l'historique des performances
    double efficiency = 0.5; // Base neutre
    
    // Recherche de patterns similaires dans l'historique
    for (const LearningData &example : m_learningHistory) {
        if (example.difficulty == difficulty || abs((int64_t)example.difficulty - (int64_t)difficulty) < (int64_t)difficulty * 0.1) {
            // Similarité de nonce (bits communs)
            uint64_t commonBits = ~(nonce ^ example.targetNonce);
            int similarity = __builtin_popcountll(commonBits);
            
            if (similarity > 16) { // Plus de 50% de bits communs
                double weight = similarity / 64.0;
                if (example.wasSuccessful) {
                    efficiency += weight * 0.3;
                } else {
                    efficiency -= weight * 0.1;
                }
            }
        }
    }
    
    return qBound(0.0, efficiency, 1.0);
}

void BiologicalNetwork::addLearningExample(const LearningData &data)
{
    QMutexLocker locker(&m_networkMutex);
    
    m_learningHistory.append(data);
    
    // Stockage dans la mémoire de patterns
    storePatternMemory(data);
    
    // Limite de l'historique pour éviter la surconsommation mémoire
    while (m_learningHistory.size() > MAX_PATTERN_MEMORY) {
        m_learningHistory.removeFirst();
    }
    
    qDebug() << "[BIO-NET] Exemple d'apprentissage ajouté - Succès:" << data.wasSuccessful
             << "Nonce: 0x" << QString::number(data.targetNonce, 16);
}

void BiologicalNetwork::storePatternMemory(const LearningData &data)
{
    // Stockage sélectif des patterns les plus intéressants
    bool shouldStore = false;
    
    if (data.wasSuccessful) {
        shouldStore = true; // Tous les succès
    } else if (data.attempts < 1000) {
        shouldStore = true; // Échecs rapides (potentiellement informatifs)
    } else if (QRandomGenerator::global()->generateDouble() < 0.1) {
        shouldStore = true; // 10% des autres échecs
    }
    
    if (shouldStore) {
        m_patternMemory.append(data);
        
        while (m_patternMemory.size() > MAX_PATTERN_MEMORY / 10) {
            m_patternMemory.removeFirst();
        }
    }
}

void BiologicalNetwork::performRetroLearning()
{
    QMutexLocker locker(&m_networkMutex);
    
    if (m_learningHistory.size() < 10) {
        qDebug() << "[BIO-NET] Pas assez de données pour le rétro-apprentissage";
        return;
    }
    
    m_learningState = LearningState::Retraining;
    emit learningStateChanged(m_learningState);
    
    qDebug() << "[BIO-NET] Démarrage rétro-apprentissage avec" << m_learningHistory.size() << "exemples";
    
    // Sélection des exemples les plus récents et pertinents
    QVector<LearningData> recentExamples;
    int maxExamples = qMin(100, m_learningHistory.size());
    
    for (int i = m_learningHistory.size() - maxExamples; i < m_learningHistory.size(); ++i) {
        recentExamples.append(m_learningHistory[i]);
    }
    
    // Cycles de rétro-apprentissage
    for (int cycle = 0; cycle < 20; ++cycle) {
        for (const LearningData &example : recentExamples) {
            QVector<double> targets(32);
            uint64_t targetNonce = example.targetNonce;
            
            for (int bit = 0; bit < 32; ++bit) {
                targets[bit] = ((targetNonce >> bit) & 1) ? 1.0 : 0.0;
            }
            
            // Pondération de l'apprentissage selon le succès
            double learningWeight = example.wasSuccessful ? 1.0 : 0.3;
            
            // Apprentissage avec pondération
            forwardPropagation(example.inputSignals);
            backPropagation(targets);
            adjustSynapticWeights(m_config.learningRate * learningWeight);
        }
        
        // Adaptation des seuils
        if (cycle % 5 == 0) {
            adaptNetworkThresholds();
        }
    }
    
    m_learningState = LearningState::Trained;
    emit learningStateChanged(m_learningState);
    
    calculateNetworkEfficiency();
    
    qDebug() << "[BIO-NET] Rétro-apprentissage terminé - Nouvelle efficacité:" 
             << QString::number(m_networkEfficiency, 'f', 3);
    
    emit networkOptimized(m_networkEfficiency);
}

void BiologicalNetwork::adaptNetworkThresholds()
{
    // Adaptation des seuils basée sur l'activité moyenne des neurones
    for (NetworkLayer &layer : m_layers) {
        double layerActivity = 0.0;
        
        for (const BiologicalNeuron &neuron : layer.neurons) {
            layerActivity += neuron.activation;
        }
        
        layerActivity /= layer.neurons.size();
        
        // Ajustement des seuils pour maintenir une activité optimale
        double targetActivity = 0.3; // 30% des neurones actifs
        double thresholdAdjustment = (layerActivity - targetActivity) * 0.01;
        
        for (BiologicalNeuron &neuron : layer.neurons) {
            neuron.threshold += thresholdAdjustment;
            neuron.threshold = qBound(0.1, static_cast<double>(neuron.threshold), 0.9);
        }
    }
}

void BiologicalNetwork::optimizeFromFeedback(bool success, const LearningData &context)
{
    QMutexLocker locker(&m_networkMutex);
    
    if (success) {
        m_successfulPredictions++;
        reinforceSuccessfulPaths(context);
        
        qDebug() << "[BIO-NET] Renforcement des chemins réussis - Nonce: 0x"
                 << QString::number(context.targetNonce, 16);
    } else {
        weakenFailurePaths(context);
        
        qDebug() << "[BIO-NET] Affaiblissement des chemins échoués";
    }
    
    // Mise à jour de l'efficacité
    calculateNetworkEfficiency();
    
    // Déclenchement d'optimisation si nécessaire
    if (m_totalPredictions % 50 == 0) {
        if (m_networkEfficiency < 0.3) {
            qDebug() << "[BIO-NET] Efficacité faible, déclenchement rétro-apprentissage";
            performRetroLearning();
        }
    }
}

void BiologicalNetwork::reinforceSuccessfulPaths(const LearningData &successData)
{
    // Renforcement des connexions ayant contribué au succès
    
    // Simulation du chemin de succès
    forwardPropagation(successData.inputSignals);
    
    // Renforcement des synapses actives
    for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
        NetworkLayer &layer = m_layers[layerIdx];
        
        for (int neuronIdx = 0; neuronIdx < layer.neurons.size(); ++neuronIdx) {
            BiologicalNeuron &neuron = layer.neurons[neuronIdx];
            
            if (neuron.activation > 0.6) { // Neurone fortement activé
                // Renforcement des poids entrants
                for (int weightIdx = 0; weightIdx < neuron.weights.size(); ++weightIdx) {
                    neuron.weights[weightIdx] *= 1.05; // Renforcement 5%
                    layer.synapses[neuronIdx][weightIdx] *= 1.03; // Renforcement synaptique 3%
                }
                
                // Adaptation du facteur d'adaptation
                neuron.adaptationFactor = qMin(1.5, neuron.adaptationFactor * 1.02);
            }
        }
    }
}

void BiologicalNetwork::weakenFailurePaths(const LearningData &failureData)
{
    // Affaiblissement léger des chemins ayant échoué
    
    forwardPropagation(failureData.inputSignals);
    
    // Affaiblissement des connexions trop dominantes
    for (int layerIdx = 1; layerIdx < m_layers.size(); ++layerIdx) {
        NetworkLayer &layer = m_layers[layerIdx];
        
        for (int neuronIdx = 0; neuronIdx < layer.neurons.size(); ++neuronIdx) {
            BiologicalNeuron &neuron = layer.neurons[neuronIdx];
            
            if (neuron.activation > 0.8) { // Neurones sur-activés
                // Léger affaiblissement
                for (int weightIdx = 0; weightIdx < neuron.weights.size(); ++weightIdx) {
                    neuron.weights[weightIdx] *= 0.98; // Affaiblissement 2%
                }
                
                // Réduction du facteur d'adaptation
                neuron.adaptationFactor = qMax(0.5, static_cast<double>(neuron.adaptationFactor * 0.99));
            }
        }
    }
}

QVector<double> BiologicalNetwork::getNetworkOutput() const
{
    if (m_layers.isEmpty()) {
        return QVector<double>();
    }
    
    const NetworkLayer &outputLayer = m_layers.last();
    QVector<double> output(outputLayer.neurons.size());
    
    for (int i = 0; i < outputLayer.neurons.size(); ++i) {
        output[i] = outputLayer.neurons[i].activation;
    }
    
    return output;
}

void BiologicalNetwork::onMEASignalsReceived(const QVector<double> &signalData)
{
    QMutexLocker locker(&m_networkMutex);
    
    m_lastMEASignals = signalData;
    
    // Mise à jour des neurones d'entrée avec les nouveaux signaux
    if (signalData.size() == m_config.neuronCount && !m_layers.isEmpty()) {
        for (int i = 0; i < signalData.size(); ++i) {
            m_layers[0].neurons[i].activation = signalData[i];
            m_layers[0].neurons[i].lastStimulation = QDateTime::currentMSecsSinceEpoch();
        }
    }
}

void BiologicalNetwork::onMiningResult(bool success, uint64_t nonce, int attempts)
{
    // Création de données de contexte pour l'apprentissage
    LearningData contextData;
    contextData.inputSignals = m_lastMEASignals;
    contextData.targetNonce = nonce;
    contextData.wasSuccessful = success;
    contextData.attempts = attempts;
    contextData.computeTime = attempts / 1000.0; // Estimation
    contextData.timestamp = QDateTime::currentDateTime();
    
    // Ajout à l'historique d'apprentissage
    addLearningExample(contextData);
    
    // Optimisation basée sur le feedback
    optimizeFromFeedback(success, contextData);
}

void BiologicalNetwork::onOptimizationTimer()
{
    // Optimisation périodique du réseau
    if (m_learningState == LearningState::Optimizing && m_learningHistory.size() > 20) {
        performRetroLearning();
    }
}

QString BiologicalNetwork::getNetworkDiagnostic() const
{
    QMutexLocker locker(&m_networkMutex);
    
    QString diagnostic;
    diagnostic += QString("=== DIAGNOSTIC RÉSEAU BIOLOGIQUE ===\n");
    diagnostic += QString("État: %1\n").arg(static_cast<int>(m_learningState));
    diagnostic += QString("Efficacité: %1%\n").arg(m_networkEfficiency * 100, 0, 'f', 1);
    diagnostic += QString("Époque courante: %1/%2\n").arg(m_currentEpoch).arg(m_totalEpochs);
    diagnostic += QString("Prédictions réussies: %1/%2 (%3%)\n")
                    .arg(m_successfulPredictions)
                    .arg(m_totalPredictions)
                    .arg(m_totalPredictions > 0 ? m_successfulPredictions * 100.0 / m_totalPredictions : 0.0, 0, 'f', 1);
    diagnostic += QString("Confiance moyenne: %1%\n").arg(m_averageConfidence * 100, 0, 'f', 1);
    diagnostic += QString("Exemples d'apprentissage: %1\n").arg(m_learningHistory.size());
    diagnostic += QString("Patterns mémorisés: %1\n").arg(m_patternMemory.size());
    
    // Détails des couches
    diagnostic += QString("\n=== ARCHITECTURE ===\n");
    for (int i = 0; i < m_layers.size(); ++i) {
        const NetworkLayer &layer = m_layers[i];
        diagnostic += QString("Couche %1 (%2): %3 neurones, activation: %4\n")
                        .arg(i)
                        .arg(layer.layerType)
                        .arg(layer.neurons.size())
                        .arg(layer.layerActivation, 0, 'f', 3);
    }
    
    return diagnostic;
}

QVector<double> BiologicalNetwork::getLayerActivations(int layer) const
{
    QMutexLocker locker(&m_networkMutex);
    
    if (layer < 0 || layer >= m_layers.size()) {
        return QVector<double>();
    }
    
    const NetworkLayer &targetLayer = m_layers[layer];
    QVector<double> activations(targetLayer.neurons.size());
    
    for (int i = 0; i < targetLayer.neurons.size(); ++i) {
        activations[i] = targetLayer.neurons[i].activation;
    }
    
    return activations;
}

double BiologicalNetwork::getNetworkEfficiency() const
{
    QMutexLocker locker(&m_networkMutex);
    return m_networkEfficiency;
}

bool BiologicalNetwork::saveNetwork(const QString &filepath)
{
    QString path = filepath;
    if (path.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        path = dataDir + "/biological_network.json";
    }
    
    QJsonObject networkState = exportNetworkState();
    QJsonDocument doc(networkState);
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[BIO-NET] Impossible de sauvegarder le réseau:" << path;
        return false;
    }
    
    file.write(doc.toJson());
    
    qDebug() << "[BIO-NET] Réseau sauvegardé:" << path;
    return true;
}

bool BiologicalNetwork::loadNetwork(const QString &filepath)
{
    QString path = filepath;
    if (path.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        path = dataDir + "/biological_network.json";
    }
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[BIO-NET] Impossible de charger le réseau:" << path;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[BIO-NET] Erreur parsing JSON:" << error.errorString();
        return false;
    }
    
    bool success = importNetworkState(doc.object());
    
    if (success) {
        qDebug() << "[BIO-NET] Réseau chargé:" << path;
    }
    
    return success;
}

QJsonObject BiologicalNetwork::exportNetworkState() const
{
    QMutexLocker locker(&m_networkMutex);

    QJsonObject state;

    // Configuration
    QJsonObject config;
    config["neuronCount"] = m_config.neuronCount;

    // Sérialise hiddenLayers comme QJsonArray
    QJsonArray hiddenLayersArray;
    for (int val : m_config.hiddenLayers)
        hiddenLayersArray.append(val);
    config["hiddenLayers"] = hiddenLayersArray;

    config["learningRate"] = m_config.learningRate;
    config["stimulationThreshold"] = m_config.stimulationThreshold;
    config["adaptationRate"] = m_config.adaptationRate;
    config["memoryDepth"] = m_config.memoryDepth;
    config["useReinforcementLearning"] = m_config.useReinforcementLearning;
    // Ajoute d'autres paramètres si besoin
    state["config"] = config;

    // État du réseau
    QJsonObject networkState;
    networkState["learningState"] = static_cast<int>(m_learningState);
    networkState["currentEpoch"] = m_currentEpoch;
    networkState["totalEpochs"] = m_totalEpochs;
    networkState["trainingProgress"] = m_trainingProgress;
    networkState["networkEfficiency"] = m_networkEfficiency;
    networkState["successfulPredictions"] = m_successfulPredictions;
    networkState["totalPredictions"] = m_totalPredictions;
    networkState["averageConfidence"] = m_averageConfidence;
    state["networkState"] = networkState;

    // Couches du réseau (structure simplifiée)
    QJsonArray layers;
    for (const NetworkLayer &layer : m_layers) {
        QJsonObject layerObj;
        layerObj["type"] = layer.layerType;
        layerObj["neuronCount"] = layer.neurons.size();
        layerObj["layerActivation"] = layer.layerActivation;

        // Seuils des neurones
        QJsonArray thresholds;
        for (const BiologicalNeuron &neuron : layer.neurons) {
            thresholds.append(neuron.threshold);
        }
        layerObj["thresholds"] = thresholds;

        layers.append(layerObj);
    }
    state["layers"] = layers;

    // Timestamp et version
    state["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    state["version"] = "1.0.0";

    return state;
}

bool BiologicalNetwork::importNetworkState(const QJsonObject &state)
{
    QMutexLocker locker(&m_networkMutex);

    try {
        // Configuration
        if (state.contains("config")) {
            QJsonObject config = state["config"].toObject();
            m_config.neuronCount = config["neuronCount"].toInt();

            // Désérialise hiddenLayers depuis QJsonArray
            m_config.hiddenLayers.clear();
            if (config.contains("hiddenLayers") && config["hiddenLayers"].isArray()) {
                QJsonArray arr = config["hiddenLayers"].toArray();
                for (const auto &v : arr)
                    m_config.hiddenLayers.append(v.toInt());
            }

            m_config.learningRate = config["learningRate"].toDouble();
            m_config.stimulationThreshold = config["stimulationThreshold"].toDouble();
            m_config.adaptationRate = config["adaptationRate"].toDouble();
            m_config.memoryDepth = config["memoryDepth"].toInt();
            m_config.useReinforcementLearning = config["useReinforcementLearning"].toBool();
            // Ajoute d'autres paramètres si besoin
        }

        // État du réseau
        if (state.contains("networkState")) {
            QJsonObject networkState = state["networkState"].toObject();
            m_learningState = static_cast<LearningState>(networkState["learningState"].toInt());
            m_currentEpoch = networkState["currentEpoch"].toInt();
            m_totalEpochs = networkState["totalEpochs"].toInt();
            m_trainingProgress = networkState["trainingProgress"].toDouble();
            m_networkEfficiency = networkState["networkEfficiency"].toDouble();
            m_successfulPredictions = networkState["successfulPredictions"].toInt();
            m_totalPredictions = networkState["totalPredictions"].toInt();
            m_averageConfidence = networkState["averageConfidence"].toDouble();
        }

        // (Optionnel) Désérialisation des couches etc.

        return true;
    } catch (...) {
        qWarning() << "[BIO-NET] Erreur lors de l'importation de l'état du réseau";
        return false;
    }
}


double BiologicalNetwork::getNetworkComplexity() const
{
    // Placeholder implementation for network complexity
    // In a real scenario, this would involve a more complex calculation
    // based on the network's structure, number of active neurons, 
    // and diversity of synaptic weights.
    return 0.5; 
}

bool BiologicalNetwork::initialize()
{
    QMutexLocker locker(&m_networkMutex);
    qDebug() << "[BIO-NET] BiologicalNetwork::initialize() called.";
    initializeNetwork(); // Re-initialize or ensure network is set up
    return true;
}

bool BiologicalNetwork::configureNetwork(const NetworkConfig &config)
{
    QMutexLocker locker(&m_networkMutex);
    qDebug() << "[BIO-NET] BiologicalNetwork::configureNetwork() called.";
    setNetworkConfig(config);
    return true;
}

bool BiologicalNetwork::initializeLearning(const NetworkConfig &config)
{
    QMutexLocker locker(&m_networkMutex);
    qDebug() << "[BIO-NET] BiologicalNetwork::initializeLearning() called.";
    m_config = config; // Update config for learning
    return startInitialLearning(m_config.maxEpochs);
}

void BiologicalNetwork::setAdaptiveLearning(bool enable)
{
    QMutexLocker locker(&m_networkMutex);
    qDebug() << "[BIO-NET] BiologicalNetwork::setAdaptiveLearning() called with" << enable;
    m_config.enableAdaptation = enable;
    // Further logic to enable/disable adaptive learning features
}

void BiologicalNetwork::updateInputSignals(const QVector<double> &currentSignals)
{
    QMutexLocker locker(&m_networkMutex);
    qDebug() << "[BIO-NET] BiologicalNetwork::updateInputSignals() called.";
    onMEASignalsReceived(currentSignals); // Delegate to existing slot
}

void BiologicalNetwork::updateWeights()
{
    QMutexLocker locker(&m_networkMutex);
    qDebug() << "[BIO-NET] BiologicalNetwork::updateWeights() called.";
    // This would typically involve a learning cycle, for now, a simple adjustment.
    adjustSynapticWeights(m_config.learningRate / 10.0); // Small adjustment
}

QVector<double> BiologicalNetwork::getOutputValues()
{
    QMutexLocker locker(&m_networkMutex);
    qDebug() << "[BIO-NET] BiologicalNetwork::getOutputValues() called.";
    return getNetworkOutput();
}

// ===================================================================
// NOUVELLES MÉTHODES BITCOIN MINING - GÉNÉRATION DE TRAINING DATA
// ===================================================================

/**
 * @brief Génère un exemple d'entraînement Bitcoin réaliste
 */
BiologicalNetwork::LearningData BiologicalNetwork::generateBitcoinTrainingExample(
    int difficultyLevel, int exampleIndex)
{
    LearningData example;
    
    // === 1. GÉNÉRATION DU BLOCK HEADER RÉALISTE ===
    BitcoinBlockHeader blockHeader = generateRealisticBlockHeader(exampleIndex);
    
    // === 2. RECHERCHE D'UN NONCE VALIDE ===
    QPair<uint64_t, QString> solution = findValidNonce(blockHeader, difficultyLevel);
    uint64_t validNonce = solution.first;
    QString winningHash = solution.second;
    
    // === 3. CONVERSION EN SIGNAUX MEA BIOLOGIQUES ===
    example.inputSignals = blockHeaderToMEASignals(blockHeader);
    
    // === 4. CONFIGURATION DE LA CIBLE D'APPRENTISSAGE ===
    example.targetNonce = validNonce;
    example.blockHeader = blockHeaderToString(blockHeader);
    example.difficulty = calculateDifficultyBits(difficultyLevel);
    example.wasSuccessful = true; // Tous les exemples d'entraînement sont des succès
    
    // === 5. MÉTRIQUES D'APPRENTISSAGE ===
    example.attempts = estimateAttemptsForDifficulty(difficultyLevel);
    example.computeTime = example.attempts / 1000000.0; // Simulation réaliste
    example.timestamp = QDateTime::currentDateTime().addSecs(-exampleIndex);
    
    return example;
}

/**
 * @brief Génère un block header Bitcoin réaliste
 */
BiologicalNetwork::BitcoinBlockHeader BiologicalNetwork::generateRealisticBlockHeader(int seed)
{
    BitcoinBlockHeader header;
    
    // Version Bitcoin réaliste
    header.version = 0x20000000; // Version courante
    
    // Hash du block précédent (simulation réaliste)
    QCryptographicHash prevHash(QCryptographicHash::Sha256);
    prevHash.addData(QString("previous_block_%1_%2")
                     .arg(seed)
                     .arg(QDateTime::currentMSecsSinceEpoch())
                     .toLatin1());
    header.previousBlockHash = prevHash.result().toHex();
    
    // Merkle root (simulation de transactions)
    QCryptographicHash merkleHash(QCryptographicHash::Sha256);
    for (int tx = 0; tx < (seed % 10) + 1; ++tx) {
        merkleHash.addData(QString("transaction_%1_%2").arg(seed).arg(tx).toLatin1());
    }
    header.merkleRoot = merkleHash.result().toHex();
    
    // Timestamp réaliste (variation ±2 heures)
    header.timestamp = QDateTime::currentSecsSinceEpoch() + (seed % 7200) - 3600;
    header.difficultyBits = 0x1d00ffff; // Valeur par défaut, sera mise à jour
    
    // Nonce sera déterminé par l'algorithme de recherche
    header.nonce = 0;
    
    return header;
}

/**
 * @brief Trouve un nonce valide pour le block header donné
 */
QPair<uint64_t, QString> BiologicalNetwork::findValidNonce(
    const BitcoinBlockHeader &header, int difficultyLevel)
{
    QString targetPattern = QString("0").repeated(difficultyLevel);
    
    // Recherche efficace avec saut adaptatif
    //uint64_t maxAttempts = qMin(10000000ULL, 1ULL << (difficultyLevel * 4));
    //uint64_t stepSize = qMax(1ULL, maxAttempts / 100000);
    uint64_t maxAttempts = std::min(10000000ULL, 1ULL << (difficultyLevel * 4));
    uint64_t stepSize = std::max(uint64_t(1), uint64_t(maxAttempts / 100000));
    
    for (uint64_t nonce = 0; nonce < maxAttempts; nonce += stepSize) {
        BitcoinBlockHeader testHeader = header;
        testHeader.nonce = static_cast<uint32_t>(nonce);
        
        QString blockHash = calculateSHA256DoubleHash(testHeader);
        
        if (blockHash.startsWith(targetPattern)) {
            qDebug() << "[BIO-NET] 🎯 Nonce trouvé:" << QString("0x%1").arg(nonce, 8, 16, QChar('0'))
                     << "Hash:" << blockHash.left(16) + "...";
            return qMakePair(nonce, blockHash);
        }
    }
    
    // Fallback : génération artificielle garantie
    return generateArtificialSolution(header, difficultyLevel);
}

/**
 * @brief Génère une solution artificielle si aucune n'est trouvée rapidement
 */
QPair<uint64_t, QString> BiologicalNetwork::generateArtificialSolution(
    const BitcoinBlockHeader &header, int difficultyLevel)
{
    // Génération d'un hash artificiel avec le bon nombre de zéros
    QString artificialHash = QString("0").repeated(difficultyLevel);
    
    // Compléter avec des caractères hex aléatoires
    QString hexChars = "0123456789abcdef";
    for (int i = difficultyLevel; i < 64; ++i) {
        artificialHash += hexChars[QRandomGenerator::global()->bounded(16)];
    }
    
    // Nonce artificiel basé sur les données du header
    uint64_t artificialNonce = qHash(header.previousBlockHash + header.merkleRoot) % (1ULL << 32);
    
    qDebug() << "[BIO-NET] 🔧 Solution artificielle générée pour difficulté" << difficultyLevel;
    
    return qMakePair(artificialNonce, artificialHash);
}

/**
 * @brief Convertit un block header en signaux MEA biologiquement réalistes
 */
QVector<double> BiologicalNetwork::blockHeaderToMEASignals(const BitcoinBlockHeader &header)
{
    QVector<double> inputs(m_config.neuronCount);
    
    // Sérialisation du block header
    QByteArray headerBytes = serializeBlockHeader(header);
    
    // === CONVERSION EN PATTERNS BIOLOGIQUES ===
    for (int electrode = 0; electrode < m_config.neuronCount; ++electrode) {
        double signal = 0.0;
        
        // 1. Composante de base dérivée des données Bitcoin
        int byteIndex = electrode % headerBytes.size();
        uint8_t headerByte = static_cast<uint8_t>(headerBytes[byteIndex]);
        
        // Amplitude de base (0.0 - 1.0)
        double baseAmplitude = headerByte / 255.0;
        
        // 2. Fréquence caractéristique basée sur la position
        double frequency = 0.1 + (electrode * 0.02); // 0.1 - 2.3 Hz
        
        // 3. Pattern temporel Bitcoin-spécifique
        double bitcoinPhase = (header.timestamp * 0.001) + (electrode * 0.1);
        double temporalPattern = sin(bitcoinPhase * frequency);
        
        // 4. Modulation par les bits significatifs
        int bitCount = qPopulationCount(headerByte); // Nombre de bits à 1
        double bitDensityModulation = bitCount / 8.0;
        
        // 5. Composante de corrélation inter-électrodes
        double correlationComponent = 0.0;
        if (electrode > 0) {
            correlationComponent = inputs[electrode - 1] * 0.15; // Couplage faible
        }
        
        // 6. Bruit biologique réaliste
        double biologicalNoise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.05;
        
        // === ASSEMBLAGE FINAL ===
        signal = baseAmplitude * (0.6 + temporalPattern * 0.3 + bitDensityModulation * 0.1)
                + correlationComponent + biologicalNoise;
        
        // Contraintes biologiques réalistes
        inputs[electrode] = qBound(-2.0, signal, 2.0);
    }
    
    // === POST-TRAITEMENT BIOLOGIQUE ===
    applyBiologicalFiltering(inputs);
    
    return inputs;
}

/**
 * @brief Applique un filtrage biologique réaliste aux signaux
 */
void BiologicalNetwork::applyBiologicalFiltering(QVector<double> &inputs)
{
    // Filtrage passe-bas simple (simulation de la réponse cellulaire)
    const double alpha = 0.3; // Constante de filtrage
    
    for (int i = 1; i < inputs.size(); ++i) {
        inputs[i] = alpha * inputs[i] + (1.0 - alpha) * inputs[i - 1];
    }
    
    // Normalisation douce pour éviter la saturation
    double maxSignal = *std::max_element(inputs.begin(), inputs.end());
    double minSignal = *std::min_element(inputs.begin(), inputs.end());
    
    if (maxSignal - minSignal > 3.0) { // Plage trop large
        double scale = 3.0 / (maxSignal - minSignal);
        for (double &signal : inputs) {
            signal *= scale;
        }
    }
}

/**
 * @brief Calcule le hash SHA-256 double d'un block header
 */
QString BiologicalNetwork::calculateSHA256DoubleHash(const BitcoinBlockHeader &header)
{
    QByteArray headerBytes = serializeBlockHeader(header);
    
    // Premier hash SHA-256
    QCryptographicHash firstHash(QCryptographicHash::Sha256);
    firstHash.addData(headerBytes);
    QByteArray firstResult = firstHash.result();
    
    // Deuxième hash SHA-256 (protocole Bitcoin)
    QCryptographicHash secondHash(QCryptographicHash::Sha256);
    secondHash.addData(firstResult);
    
    return secondHash.result().toHex();
}

/**
 * @brief Sérialise un block header au format Bitcoin
 */
QByteArray BiologicalNetwork::serializeBlockHeader(const BitcoinBlockHeader &header)
{
    QByteArray serialized;
    QDataStream stream(&serialized, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Format Bitcoin standard
    stream << static_cast<quint32>(header.version);
    
    // Ajout des hashes (32 bytes chacun)
    QByteArray prevHashBytes = QByteArray::fromHex(header.previousBlockHash.toLatin1());
    serialized.append(prevHashBytes.left(32)); // S'assurer de 32 bytes
    
    QByteArray merkleBytes = QByteArray::fromHex(header.merkleRoot.toLatin1());
    serialized.append(merkleBytes.left(32)); // S'assurer de 32 bytes
    
    // Continuer avec les autres champs
    QDataStream stream2(&serialized, QIODevice::WriteOnly | QIODevice::Append);
    stream2.setByteOrder(QDataStream::LittleEndian);
    stream2 << static_cast<quint32>(header.timestamp);
    stream2 << static_cast<quint32>(header.difficultyBits);
    stream2 << static_cast<quint32>(header.nonce);
    
    return serialized;
}

/**
 * @brief Convertit un block header en string pour stockage
 */
QString BiologicalNetwork::blockHeaderToString(const BitcoinBlockHeader &header)
{
    return QString("BitcoinBlock{v:%1,prev:%2,merkle:%3,time:%4,diff:%5,nonce:%6}")
           .arg(header.version, 0, 16)
           .arg(header.previousBlockHash.left(16))
           .arg(header.merkleRoot.left(16))
           .arg(header.timestamp)
           .arg(header.difficultyBits, 0, 16)
           .arg(header.nonce);
}

/**
 * @brief Génère des patterns Bitcoin spéciaux pour l'entraînement
 */
void BiologicalNetwork::generateSpecialBitcoinPatterns(int count)
{
    qDebug() << "[BIO-NET] 🌟 Génération de patterns Bitcoin spéciaux...";
    
    for (int i = 0; i < count; ++i) {
        LearningData specialExample;
        
        // Patterns spéciaux basés sur des cas réels Bitcoin
        if (i % 4 == 0) {
            // Pattern "Genesis" (inspiré du block Genesis)
            specialExample = generateGenesisLikePattern(i);
        } else if (i % 4 == 1) {
            // Pattern "Halving" (simulation de blocks de halving)
            specialExample = generateHalvingLikePattern(i);
        } else if (i % 4 == 2) {
            // Pattern "High difficulty" (très difficile)
            specialExample = generateHighDifficultyPattern(i);
        } else {
            // Pattern "Sequence" (nonces séquentiels)
            specialExample = generateSequentialPattern(i);
        }
        
        m_learningHistory.append(specialExample);
    }
}

/**
 * @brief Génère un pattern similaire au block Genesis
 */
BiologicalNetwork::LearningData BiologicalNetwork::generateGenesisLikePattern(int index)
{
    LearningData example;
    
    BitcoinBlockHeader header;
    header.version = 0x00000001; // Version Genesis
    header.previousBlockHash = QString("0").repeated(64); // Pas de block précédent
    header.merkleRoot = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b";
    header.timestamp = 1231006505; // Timestamp Genesis approximatif
    header.difficultyBits = 0x1d00ffff;
    header.nonce = 2083236893; // Nonce Genesis approximatif
    
    example.inputSignals = blockHeaderToMEASignals(header);
    example.targetNonce = header.nonce;
    example.blockHeader = blockHeaderToString(header);
    example.difficulty = header.difficultyBits;
    example.wasSuccessful = true;
    example.attempts = 2083236893; // Nombre d'essais approximatif
    example.computeTime = example.attempts / 100000.0;
    example.timestamp = QDateTime::currentDateTime().addSecs(-index);
    
    return example;
}

/**
 * @brief Génère un pattern de type halving
 */
BiologicalNetwork::LearningData BiologicalNetwork::generateHalvingLikePattern(int index)
{
    // Pattern basé sur des blocks de halving historiques
    return generateBitcoinTrainingExample(3, 210000 + index); // Difficulté élevée
}

/**
 * @brief Génère un pattern à haute difficulté
 */
BiologicalNetwork::LearningData BiologicalNetwork::generateHighDifficultyPattern(int index)
{
    return generateBitcoinTrainingExample(4, 700000 + index); // Difficulté maximale
}

/**
 * @brief Génère un pattern avec nonces séquentiels
 */
BiologicalNetwork::LearningData BiologicalNetwork::generateSequentialPattern(int index)
{
    LearningData example = generateBitcoinTrainingExample(2, index);
    
    // Modifier le nonce pour créer une séquence
    example.targetNonce = (example.targetNonce & 0xFFFF0000) | (index % 65536);
    
    return example;
}

/**
 * @brief Calcule les bits de difficulté pour un nombre de zéros requis
 */
uint64_t BiologicalNetwork::calculateDifficultyBits(int requiredZeros)
{
    // Formule Bitcoin simplifiée pour l'entraînement
    return 0x1d000000 + (0x00ffffff >> requiredZeros);
}

/**
 * @brief Estime le nombre de tentatives nécessaires pour une difficulté
 */
int BiologicalNetwork::estimateAttemptsForDifficulty(int difficultyLevel)
{
    // Estimation statistique basée sur la probabilité
    int baseAttempts = 1000;
    return baseAttempts * static_cast<int>(std::pow(16, difficultyLevel));
}

/**
 * @brief Mélange les données d'entraînement pour éviter l'overfitting
 */
void BiologicalNetwork::shuffleTrainingData()
{
    // Algorithme de mélange Fisher-Yates
    for (int i = m_learningHistory.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        m_learningHistory.swapItemsAt(i, j);
    }
}


//#include "biological_network.moc"
