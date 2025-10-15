#include "crypto/hybrid_bitcoin_miner.h"
#include <QDebug>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QElapsedTimer>
#include <QThread>
#include <cmath>
#include <algorithm>
#include <random>

namespace BioMining {
namespace HCrypto {

HybridBitcoinMiner::HybridBitcoinMiner(QObject* parent)
    : QObject(parent)
    , m_traditionalMiner(std::make_unique<BioMining::Crypto::BitcoinMiner>())
    , m_biologicalNetwork(nullptr)
    , m_meaInterface(nullptr)
    , m_realMeaInterface(nullptr)
    , m_biologicalWeight(DEFAULT_BIOLOGICAL_WEIGHT)
    , m_tripleSystemEnabled(false)
    , m_sha256Successes(0)
    , m_networkSuccesses(0)
    , m_meaSuccesses(0)
    , m_fusionSuccesses(0)
    , m_isInitialized(false)
    , m_isMining(false)
    , m_isPaused(false)
    , m_learningState(HybridLearningState::Uninitialized)
    , m_adaptationScore(0.0)
    , m_miningTimer(new QTimer(this))
    , m_learningTimer(new QTimer(this))
    , m_metricsTimer(new QTimer(this))
    , m_threadPool(QThreadPool::globalInstance())
    // === BIO-ENTROPY INITIALIZATION ===
    , m_bioComputeMode(BioComputeMode::SimulatedNetwork) // Par défaut simulation
    , m_entropyGenerator(QSharedPointer<BioMining::Crypto::BioEntropyGenerator>::create())
{
    qDebug() << "Initializing HybridBitcoinMiner...";
    
    // Configuration des timers
    m_miningTimer->setInterval(MINING_CYCLE_MS);
    m_learningTimer->setInterval(LEARNING_CYCLE_MS);
    m_metricsTimer->setInterval(METRICS_UPDATE_MS);
    
    // Connexions des signaux
    connect(m_miningTimer, &QTimer::timeout, this, &HybridBitcoinMiner::performHybridMiningCycle);
    connect(m_learningTimer, &QTimer::timeout, this, &HybridBitcoinMiner::processNetworkLearning);
    connect(m_metricsTimer, &QTimer::timeout, this, &HybridBitcoinMiner::updateLearningMetrics);
    
    // Initialisation des paramètres par défaut
    m_learningParams = BiologicalLearningParams();
    m_learningState = HybridLearningState::Uninitialized;
    
    // === CRÉER LES ADAPTEURS BIO-ENTROPY ===
    m_realMeaAdapter = QSharedPointer<BioMining::Bio::RealMEAAdapter>::create();
    m_networkAdapter = QSharedPointer<BioMining::Bio::BiologicalNetworkAdapter>::create();
    
    // Par défaut, utiliser le mode simulé
    m_bioCompute = m_networkAdapter;
    
    qDebug() << "HybridBitcoinMiner initialized successfully";
    qDebug() << "Bio-Entropy mode:" << (m_bioComputeMode == BioComputeMode::RealMEA ? "RealMEA" : "SimulatedNetwork");
}

HybridBitcoinMiner::~HybridBitcoinMiner()
{
    qDebug() << "Destroying HybridBitcoinMiner...";
    
    if (m_isMining) {
        stopHybridMining();
    }
    
    cleanupMiningThreads();
    qDebug() << "HybridBitcoinMiner destroyed";
}

bool HybridBitcoinMiner::initialize()
{
    QMutexLocker locker(&m_stateMutex);
    
    qDebug() << "Initializing hybrid mining system...";
    
    try {
        // Initialisation du mineur traditionnel
        if (!m_traditionalMiner->initialize()) {
            qCritical() << "Failed to initialize traditional Bitcoin miner";
            return false;
        }
        
        // Création du réseau biologique
        m_biologicalNetwork = std::make_unique<BioMining::Network::BiologicalNetwork>();
        if (!m_biologicalNetwork->initialize()) {
            qCritical() << "Failed to initialize biological network";
            return false;
        }
        
        // Configuration des threads
        setupMiningThreads();
        
        // Initialisation des métriques
        m_metrics.reset();
        m_adaptationScore = 0.0;
        
        m_isInitialized = true;
        m_learningState = HybridLearningState::InitialLearning;
        
        qDebug() << "Hybrid mining system initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during initialization:" << e.what();
        m_learningState = HybridLearningState::Error;
        return false;
    }
}

bool HybridBitcoinMiner::configureBiologicalNetwork(const BiologicalLearningParams& params)
{
    QMutexLocker locker(&m_learningMutex);
    
    qDebug() << "Configuring biological network parameters...";
    
    if (!m_biologicalNetwork) {
        qCritical() << "Biological network not initialized";
        return false;
    }
    
    m_learningParams = params;
    
    // Configuration du réseau biologique avec les paramètres MEA
    BioMining::Network::BiologicalNetwork::NetworkConfig networkConfig;
    networkConfig.inputSize = 60; // 60 électrodes MEA
    networkConfig.outputSize = 32; // Pour prédiction de nonce (32 bits)
    networkConfig.hiddenLayers = QVector<int>{128, 64, 32}; // Architecture profonde
    networkConfig.learningRate = params.initialLearningRate;
    networkConfig.enablePlasticity = params.enablePlasticity;
    networkConfig.enableAdaptation = params.enableAdaptation;
    
    if (!m_biologicalNetwork->configureNetwork(networkConfig)) {
        qCritical() << "Failed to configure biological network";
        return false;
    }
    
    qDebug() << "Biological network configured successfully";
    return true;
}

bool HybridBitcoinMiner::connectToMEA(std::shared_ptr<BioMining::Bio::MEAInterface> meaInterface)
{
    QMutexLocker locker(&m_stateMutex);
    
    qDebug() << "Connecting to MEA interface...";
    
    if (!meaInterface) {
        qCritical() << "Invalid MEA interface";
        return false;
    }
    
    m_meaInterface = meaInterface;
    
    // Connexion des signaux MEA
    connect(m_meaInterface.get(), 
                   static_cast<void (BioMining::Bio::MEAInterface::*)(const QVector<double>&)>(&BioMining::Bio::MEAInterface::dataReady),
                   this, 
                   [this](const QVector<double>& data) {
                       std::vector<double> stdData(data.begin(), data.end());
                       this->onMEADataReceived(stdData);
                   });
    connect(m_meaInterface.get(), &BioMining::Bio::MEAInterface::errorOccurred,
            this, &HybridBitcoinMiner::errorOccurred);
    
    // Vérification de la connectivité
    if (!m_meaInterface->isConnected()) {
        qWarning() << "MEA interface not connected, attempting connection...";
        if (!m_meaInterface->connectToDevice()) {
            qCritical() << "Failed to connect to MEA device";
            return false;
        }
    }
    
    qDebug() << "Successfully connected to MEA interface";
    return true;
}

void HybridBitcoinMiner::setMiningParameters(const MiningConfig& config)
{
    QMutexLocker locker(&m_stateMutex);
    
    m_miningConfig = config;
    if (m_traditionalMiner) {
        BioMining::Crypto::BitcoinMiner::MiningConfig traditionalConfig;
        traditionalConfig.difficulty = config.difficulty;
        traditionalConfig.threads = config.threads;
        traditionalConfig.useGPU = config.useGPU;
        m_traditionalMiner->setMiningParameters(traditionalConfig);
    }
    
    qDebug() << "Mining parameters updated";
}

bool HybridBitcoinMiner::startHybridMining()
{
    QMutexLocker locker(&m_stateMutex);
    
    qDebug() << "Starting hybrid Bitcoin mining...";
    
    if (!m_isInitialized) {
        qCritical() << "Hybrid miner not initialized";
        return false;
    }
    
    if (m_isMining) {
        qWarning() << "Mining already in progress";
        return true;
    }
    
    if (!m_meaInterface || !m_meaInterface->isConnected()) {
        qCritical() << "MEA interface not available";
        return false;
    }
    
    try {
        // Démarrage de l'acquisition MEA
        if (!m_meaInterface->startAcquisition()) {
            qCritical() << "Failed to start MEA acquisition";
            return false;
        }
        
        // Réinitialisation des métriques
        m_metrics.reset();
        m_adaptationScore = 0.0;
        
        // Démarrage des timers
        m_miningTimer->start();
        m_learningTimer->start();
        m_metricsTimer->start();
        
        m_isMining = true;
        m_isPaused = false;
        m_learningState = HybridLearningState::ActiveMining;
        
        emit hybridMiningStarted();
        qDebug() << "Hybrid mining started successfully";
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during mining startup:" << e.what();
        m_learningState = HybridLearningState::Error;
        return false;
    }
}

void HybridBitcoinMiner::stopHybridMining()
{
    QMutexLocker locker(&m_stateMutex);
    
    qDebug() << "Stopping hybrid Bitcoin mining...";
    
    if (!m_isMining) {
        qWarning() << "Mining not in progress";
        return;
    }
    
    // Arrêt des timers
    m_miningTimer->stop();
    m_learningTimer->stop();
    m_metricsTimer->stop();
    
    // Arrêt de l'acquisition MEA
    if (m_meaInterface && m_meaInterface->isConnected()) {
        m_meaInterface->stopAcquisition();
    }
    
    // Attente de la fin des tâches en cours
    m_threadPool->waitForDone(5000);
    
    m_isMining = false;
    m_isPaused = false;
    m_learningState = HybridLearningState::Uninitialized;
    
    emit hybridMiningStopped();
    qDebug() << "Hybrid mining stopped";
}

void HybridBitcoinMiner::pauseMining()
{
    QMutexLocker locker(&m_stateMutex);
    
    if (m_isMining && !m_isPaused) {
        m_isPaused = true;
        qDebug() << "Hybrid mining paused";
    }
}

void HybridBitcoinMiner::resumeMining()
{
    QMutexLocker locker(&m_stateMutex);
    
    if (m_isMining && m_isPaused) {
        m_isPaused = false;
        qDebug() << "Hybrid mining resumed";
    }
}

bool HybridBitcoinMiner::isMining() const
{
    return m_isMining && !m_isPaused;
}

bool HybridBitcoinMiner::initializeBiologicalLearning()
{
    QMutexLocker locker(&m_learningMutex);
    
    qDebug() << "Initializing biological learning system...";
    
    if (!m_biologicalNetwork) {
        qCritical() << "Biological network not available";
        return false;
    }
    
    if (!m_meaInterface || !m_meaInterface->isConnected()) {
        qCritical() << "MEA interface not available for learning initialization";
        return false;
    }
    
    try {
        // Initialisation du réseau avec des paramètres d'apprentissage
        BioMining::Network::BiologicalNetwork::NetworkConfig learningConfig;
        learningConfig.learningRate = m_learningParams.initialLearningRate;
        learningConfig.momentum = m_learningParams.momentumFactor;
        learningConfig.decayRate = m_learningParams.decayRate;
        learningConfig.adaptiveThreshold = m_learningParams.adaptationThreshold;
        learningConfig.maxEpochs = m_learningParams.maxIterations;
        
        if (!m_biologicalNetwork->initializeLearning(learningConfig)) {
            qCritical() << "Failed to initialize biological learning";
            return false;
        }
        
        // Préparation des données d'apprentissage initial
        std::vector<BiologicalTrainingData> initialData;
        generateInitialTrainingData(initialData);
        
        if (!initialData.empty()) {
            qDebug() << "Generated" << initialData.size() << "initial training samples";
        }
        
        m_learningState = HybridLearningState::InitialLearning;
        qDebug() << "Biological learning system initialized successfully";
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during learning initialization:" << e.what();
        m_learningState = HybridLearningState::Error;
        return false;
    }
}

bool HybridBitcoinMiner::performInitialLearning(const std::vector<BiologicalTrainingData>& trainingData)
{
    QMutexLocker locker(&m_learningMutex);
    
    qDebug() << "Performing initial biological learning with" << trainingData.size() << "samples...";
    
    if (!m_biologicalNetwork) {
        qCritical() << "Biological network not available";
        return false;
    }
    
    if (trainingData.empty()) {
        qWarning() << "No training data provided";
        return false;
    }
    
    m_learningState = HybridLearningState::InitialLearning;
    
    try {
        double totalError = 0.0;
        int successfulTraining = 0;
        
        // Apprentissage initial avec les données fournies
        for (int epoch = 0; epoch < m_learningParams.maxIterations; ++epoch) {
            double epochError = 0.0;
            
            // Mélange des données à chaque époque
            std::vector<BiologicalTrainingData> shuffledData = trainingData;
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(shuffledData.begin(), shuffledData.end(), g);
            
            // Entraînement sur chaque échantillon
            for (const auto& sample : shuffledData) {
                if (trainNetworkWithPattern(sample.inputPattern, 
                                          convertNonceToOutput(sample.expectedNonce))) {
                    successfulTraining++;
                } else {
                    epochError += 1.0;
                }
                
                // Mise à jour des poids synaptiques
                updateSynapticWeights();
            }
            
            epochError /= shuffledData.size();
            totalError += epochError;
            
            // Vérification de la convergence
            if (epochError < m_learningParams.adaptationThreshold) {
                qDebug() << "Initial learning converged at epoch" << epoch 
                        << "with error" << epochError;
                break;
            }
            
            // Adaptation du taux d'apprentissage
            if (epoch % 100 == 0) {
                adaptLearningRate();
                qDebug() << "Epoch" << epoch << "- Error:" << epochError 
                        << "- Success rate:" << (double)successfulTraining / (epoch + 1);
            }
        }
        
        double successRate = (double)successfulTraining / (trainingData.size() * m_learningParams.maxIterations);
        qDebug() << "Initial learning completed - Success rate:" << successRate 
                << "Average error:" << totalError / m_learningParams.maxIterations;
        
        // Mise à jour de l'état si l'apprentissage est réussi
        if (successRate > 0.7) { // 70% de réussite minimum
            m_learningState = HybridLearningState::ActiveMining;
            m_adaptationScore = successRate;
            return true;
        } else {
            qWarning() << "Initial learning failed - insufficient success rate:" << successRate;
            m_learningState = HybridLearningState::Error;
            return false;
        }
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during initial learning:" << e.what();
        m_learningState = HybridLearningState::Error;
        return false;
    }
}

bool HybridBitcoinMiner::performRetroLearning()
{
    QMutexLocker locker(&m_learningMutex);
    
    qDebug() << "Performing retro-learning with historical data...";
    
    if (!m_biologicalNetwork) {
        qCritical() << "Biological network not available";
        return false;
    }
    
    if (m_predictionHistory.empty()) {
        qWarning() << "No prediction history available for retro-learning";
        return false;
    }
    
    m_learningState = HybridLearningState::RetroLearning;
    
    try {
        // Préparation des données de rétro-apprentissage
        std::vector<BiologicalTrainingData> retroData;
        prepareRetroLearningData(retroData);
        
        if (retroData.empty()) {
            qWarning() << "No suitable data for retro-learning";
            return false;
        }
        
        // Configuration pour le rétro-apprentissage
        double originalLearningRate = m_learningParams.initialLearningRate;
        m_learningParams.initialLearningRate = m_learningParams.retroLearningRate;
        
        qDebug() << "Starting retro-learning with" << retroData.size() 
                << "historical samples, learning rate:" << m_learningParams.retroLearningRate;
        
        double totalImprovement = 0.0;
        int successfulRetroTraining = 0;
        
        // Rétro-apprentissage avec données historiques
        for (int epoch = 0; epoch < m_learningParams.retroIterations; ++epoch) {
            double epochImprovement = 0.0;
            
            // Analyse des erreurs passées
            for (const auto& sample : retroData) {
                // Calcul de l'amélioration potentielle
                std::vector<double> currentOutput;
                performForwardPropagation(sample.inputPattern);
                currentOutput = extractNetworkFeatures();
                
                double currentError = calculatePredictionError(currentOutput, sample.targetOutput);
                
                // Rétro-propagation avec correction
                if (trainNetworkWithPattern(sample.inputPattern, sample.targetOutput)) {
                    // Mesure de l'amélioration
                    performForwardPropagation(sample.inputPattern);
                    std::vector<double> newOutput = extractNetworkFeatures();
                    double newError = calculatePredictionError(newOutput, sample.targetOutput);
                    
                    if (newError < currentError) {
                        epochImprovement += (currentError - newError);
                        successfulRetroTraining++;
                    }
                }
                
                updateSynapticWeights();
            }
            
            totalImprovement += epochImprovement;
            
            // Vérification de la convergence du rétro-apprentissage
            if (epochImprovement < m_learningParams.adaptationThreshold * 0.1) {
                qDebug() << "Retro-learning converged at epoch" << epoch;
                break;
            }
            
            if (epoch % 50 == 0) {
                qDebug() << "Retro-epoch" << epoch << "- Improvement:" << epochImprovement;
            }
        }
        
        // Restauration du taux d'apprentissage original
        m_learningParams.initialLearningRate = originalLearningRate;
        
        double avgImprovement = totalImprovement / m_learningParams.retroIterations;
        double retroSuccessRate = (double)successfulRetroTraining / (retroData.size() * m_learningParams.retroIterations);
        
        qDebug() << "Retro-learning completed - Success rate:" << retroSuccessRate 
                << "Average improvement:" << avgImprovement;
        
        // Mise à jour du score d'adaptation
        m_adaptationScore = std::min(1.0, m_adaptationScore + avgImprovement);
        
        // Retour à l'état de mining actif
        m_learningState = HybridLearningState::ActiveMining;
        
        return retroSuccessRate > 0.5; // 50% de réussite minimum pour le rétro-apprentissage
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during retro-learning:" << e.what();
        m_learningState = HybridLearningState::Error;
        return false;
    }
}

void HybridBitcoinMiner::enableAdaptiveLearning(bool enable)
{
    QMutexLocker locker(&m_learningMutex);
    
    m_learningParams.enableAdaptation = enable;
    
    if (m_biologicalNetwork) {
        m_biologicalNetwork->setAdaptiveLearning(enable);
    }
    
    qDebug() << "Adaptive learning" << (enable ? "enabled" : "disabled");
}

BiologicalNoncePrediction HybridBitcoinMiner::predictNonce(const QString& blockHeader)
{
    QMutexLocker locker(&m_dataMutex);
    
    BiologicalNoncePrediction prediction;
    
    if (!m_biologicalNetwork || !m_meaInterface) {
        qWarning() << "Network or MEA interface not available for prediction";
        return prediction;
    }
    
    try {
        // Acquisition des signaux MEA actuels
        QVector<double> qMeaSignals = m_meaInterface->getCurrentElectrodeData();
        std::vector<double> meaSignals(qMeaSignals.begin(), qMeaSignals.end());
        
        if (meaSignals.size() != 60) {
            qWarning() << "Invalid MEA signal size:" << meaSignals.size();
            return prediction;
        }
        
        // Prétraitement des signaux
        std::vector<double> processedSignals = preprocessMEASignals(meaSignals);
        
        // Propagation avant dans le réseau biologique
        performForwardPropagation(processedSignals);
        
        // Extraction de la prédiction
        std::vector<double> networkOutput = extractNetworkFeatures();
        
        // Génération du nonce prédit
        uint32_t predictedNonce = generateBiologicalNonce(processedSignals);
        
        // Calcul de la confiance et de l'entropie
        double confidence = evaluatePredictionConfidence(networkOutput);
        double entropy = calculateBiologicalEntropy(processedSignals);
        
        // Remplissage de la structure de prédiction
        prediction.predictedNonce = predictedNonce;
        prediction.confidence = confidence;
        prediction.biologicalEntropy = entropy;
        prediction.networkOutput = networkOutput;
        prediction.predictionTime = QDateTime::currentDateTime();
        prediction.isValidated = false;
        
        // Mise à jour des métriques
        m_metrics.biologicalPredictions++;
        
        // Ajout à l'historique
        if (m_predictionHistory.size() >= MAX_PREDICTION_HISTORY) {
            m_predictionHistory.erase(m_predictionHistory.begin());
        }
        m_predictionHistory.push_back(prediction);
        
        emit biologicalPredictionMade(prediction);
        
        qDebug() << "Nonce predicted:" << predictedNonce 
                << "Confidence:" << confidence 
                << "Entropy:" << entropy;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during nonce prediction:" << e.what();
    }
    
    return prediction;
}

void HybridBitcoinMiner::validatePrediction(const BiologicalNoncePrediction& prediction, bool wasSuccessful)
{
    QMutexLocker locker(&m_dataMutex);
    
    // Recherche de la prédiction dans l'historique
    auto it = std::find_if(m_predictionHistory.begin(), m_predictionHistory.end(),
                          [&prediction](const BiologicalNoncePrediction& p) {
                              return p.predictedNonce == prediction.predictedNonce &&
                                     p.predictionTime == prediction.predictionTime;
                          });
    
    if (it != m_predictionHistory.end()) {
        it->isValidated = true;
        
        if (wasSuccessful) {
            m_metrics.successfulPredictions++;
            
            // Renforcement des patterns réussis
            reinforceSuccessfulPatterns();
            
            qDebug() << "Prediction validated as successful - Nonce:" << prediction.predictedNonce;
        } else {
            qDebug() << "Prediction validated as unsuccessful - Nonce:" << prediction.predictedNonce;
        }
        
        // Mise à jour de l'accuracy biologique
        updateBiologicalAccuracy();
        
        // Ajout aux données d'entraînement pour amélioration future
        BiologicalTrainingData trainingData;
        trainingData.inputPattern = it->networkOutput; // Utilisation de la sortie réseau comme pattern
        trainingData.targetOutput = convertNonceToOutput(it->predictedNonce);
        trainingData.expectedNonce = it->predictedNonce;
        trainingData.timestamp = it->predictionTime;
        trainingData.validationScore = wasSuccessful ? 1.0 : 0.0;
        trainingData.isValidated = true;
        
        // Ajout à l'historique d'entraînement
        if (m_trainingHistory.size() >= MAX_TRAINING_HISTORY) {
            m_trainingHistory.erase(m_trainingHistory.begin());
        }
        m_trainingHistory.push_back(trainingData);
    }
}

double HybridBitcoinMiner::getBiologicalAccuracy() const
{
    return m_metrics.biologicalAccuracy.load();
}

HybridMiningMetrics HybridBitcoinMiner::getMetrics() const
{
    return m_metrics;
}

HybridLearningState HybridBitcoinMiner::getLearningState() const
{
    return m_learningState.load();
}

double HybridBitcoinMiner::getNetworkAdaptationScore() const
{
    return m_adaptationScore.load();
}

QString HybridBitcoinMiner::getStatusReport() const
{
    QMutexLocker locker(&m_stateMutex);
    
    QString report;
    report += QString("=== Hybrid Bitcoin Miner Status ===\n");
    report += QString("Mining State: %1\n").arg(m_isMining ? "Active" : "Inactive");
    report += QString("Learning State: %1\n").arg(static_cast<int>(m_learningState.load()));
    report += QString("Biological Weight: %1\n").arg(m_biologicalWeight);
    report += QString("Adaptation Score: %1\n").arg(m_adaptationScore.load());
    report += QString("Total Hashes: %1\n").arg(m_metrics.totalHashes.load());
    report += QString("Biological Predictions: %1\n").arg(m_metrics.biologicalPredictions.load());
    report += QString("Successful Predictions: %1\n").arg(m_metrics.successfulPredictions.load());
    report += QString("Biological Accuracy: %1%%\n").arg(m_metrics.biologicalAccuracy.load() * 100);
    report += QString("Hybrid Hash Rate: %1 H/s\n").arg(m_metrics.hybridHashRate.load());
    report += QString("Energy Efficiency: %1\n").arg(m_metrics.energyEfficiency.load());
    
    if (m_meaInterface) {
        report += QString("MEA Connected: %1\n").arg(m_meaInterface->isConnected() ? "Yes" : "No");
    }
    
    return report;
}

void HybridBitcoinMiner::setLearningParameters(const BiologicalLearningParams& params)
{
    QMutexLocker locker(&m_learningMutex);
    m_learningParams = params;
    qDebug() << "Learning parameters updated";
}

BiologicalLearningParams HybridBitcoinMiner::getLearningParameters() const
{
    QMutexLocker locker(&m_learningMutex);
    return m_learningParams;
}

void HybridBitcoinMiner::setBiologicalWeight(double weight)
{
    weight = qBound(MIN_BIOLOGICAL_WEIGHT, weight, MAX_BIOLOGICAL_WEIGHT);
    m_biologicalWeight = weight;
    qDebug() << "Biological weight set to:" << weight;
}

double HybridBitcoinMiner::getBiologicalWeight() const
{
    return m_biologicalWeight;
}

void HybridBitcoinMiner::onMEADataReceived(const std::vector<double>& electrodeData)
{
    if (!m_isMining || m_isPaused) {
        return;
    }
    
    // Traitement asynchrone des données MEA
    if (electrodeData.size() == 60) {
        // Mise à jour du réseau biologique avec les nouvelles données
        if (m_biologicalNetwork && m_learningState == HybridLearningState::ActiveMining) {
            QVector<double> qElectrodeData(electrodeData.begin(), electrodeData.end());
            m_biologicalNetwork->updateInputSignals(qElectrodeData);
        }
    } else {
        qWarning() << "Invalid electrode data size:" << electrodeData.size();
    }
}

void HybridBitcoinMiner::onNetworkStateChanged(BioMining::HBio::NetworkLearningState state)
{
    // Synchronisation des états d'apprentissage
    switch (state) {
        case BioMining::HBio::NetworkLearningState::Untrained:
            m_learningState = HybridLearningState::InitialLearning;
            break;
        case BioMining::HBio::NetworkLearningState::InitialLearning:
            m_learningState = HybridLearningState::InitialLearning;
            break;
        case BioMining::HBio::NetworkLearningState::Trained:
            m_learningState = HybridLearningState::ActiveMining;
            break;
        case BioMining::HBio::NetworkLearningState::Retraining:
            m_learningState = HybridLearningState::RetroLearning;
            break;
        case BioMining::HBio::NetworkLearningState::Optimizing:
            m_learningState = HybridLearningState::Optimizing;
            break;
        default:
            break;
    }
    
    emit learningStateChanged(m_learningState.load());
}

void HybridBitcoinMiner::onMiningDifficultyChanged(double newDifficulty)
{
    // Adaptation des paramètres en fonction de la difficulté
    if (newDifficulty > m_miningConfig.targetDifficulty * 1.5) {
        // Augmentation du poids biologique pour les difficultés élevées
        setBiologicalWeight(std::min(1.0, m_biologicalWeight + 0.1));
    } else if (newDifficulty < m_miningConfig.targetDifficulty * 0.5) {
        // Réduction du poids biologique pour les difficultés faibles
        setBiologicalWeight(std::max(0.0, m_biologicalWeight - 0.1));
    }
    
    qDebug() << "Mining difficulty changed to:" << newDifficulty 
            << "Biological weight adjusted to:" << m_biologicalWeight;
}

void HybridBitcoinMiner::updateLearningMetrics()
{
    if (!m_isMining) {
        return;
    }
    
    // Calcul des métriques de performance
    updateBiologicalAccuracy();
    updateHashRate();
    updateEnergyEfficiency();
    updateAdaptationParameters();
    
    emit metricsUpdated(m_metrics);
}

void HybridBitcoinMiner::performHybridMiningCycle()
{
    if (!m_isMining || m_isPaused) {
        return;
    }
    
    try {
        // Génération du header de bloc pour test
        QString testBlockHeader = generateTestBlockHeader();
        
        uint32_t nonce = 0;
        QString blockHash;
        
        // Utilisation du système triple si disponible
        bool success = false;
        if (isTripleSystemReady()) {
            success = performTripleOptimizedMining(testBlockHeader, nonce, blockHash);
        } else {
            // Fallback sur mining hybride classique
            success = performHybridHash(testBlockHeader, nonce, blockHash);
        }
        
        if (success) {
            // Bloc trouvé !
            double biologicalContribution = calculateBiologicalContribution();
            emit blockFound(blockHash, nonce, biologicalContribution);
            
            qDebug() << "[MiningCycle] Block found! Hash:" << blockHash.left(16)
                    << "Nonce:" << nonce 
                    << "Biological contribution:" << biologicalContribution;
        }
        
        // Mise à jour des métriques
        m_metrics.totalHashes++;
        
        // Optimisation périodique du système triple
        if (m_tripleSystemEnabled && m_metrics.totalHashes % 100 == 0) {
            QTimer::singleShot(0, [this]() {
                performCrossSystemLearning();
                optimizeSystemWeights();
            });
        }
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during mining cycle:" << e.what();
    }
}

void HybridBitcoinMiner::processNetworkLearning()
{
    if (!m_biologicalNetwork || m_learningState != HybridLearningState::ActiveMining) {
        return;
    }
    
    try {
        // Apprentissage continu pendant le mining
        if (!m_trainingHistory.empty()) {
            // Sélection d'un échantillon récent pour apprentissage
            int sampleIndex = QRandomGenerator::global()->bounded(
                std::min((int)m_trainingHistory.size(), 100));
            
            const auto& sample = m_trainingHistory[m_trainingHistory.size() - 1 - sampleIndex];
            
            if (sample.isValidated) {
                trainNetworkWithPattern(sample.inputPattern, sample.targetOutput);
                updateSynapticWeights();
            }
        }
        
        // Vérification de la convergence du pattern
        if (detectPatternConvergence()) {
            qDebug() << "Pattern convergence detected, triggering retro-learning";
            
            // Déclenchement du rétro-apprentissage si nécessaire
            if (m_predictionHistory.size() > 100) {
                QFuture<bool> future = QtConcurrent::run([this]() {
                    return performRetroLearning();
                });
            }
        }
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during network learning:" << e.what();
    }
}

void HybridBitcoinMiner::updateAdaptationParameters()
{
    if (!m_biologicalNetwork) {
        return;
    }
    
    // Calcul du score d'adaptation basé sur les performances récentes
    double recentAccuracy = calculateRecentAccuracy();
    double networkComplexity = calculateNetworkComplexity();
    double adaptationRate = calculateAdaptationRate();
    
    m_adaptationScore = (recentAccuracy * 0.5) + (networkComplexity * 0.3) + (adaptationRate * 0.2);
    
    emit adaptationScoreChanged(m_adaptationScore.load());
    
    // Ajustement automatique des paramètres
    if (m_adaptationScore > 0.8) {
        // Performance excellente, réduction du taux d'apprentissage
        m_learningParams.initialLearningRate *= 0.995;
    } else if (m_adaptationScore < 0.3) {
        // Performance faible, augmentation du taux d'apprentissage
        m_learningParams.initialLearningRate *= 1.005;
    }
    
    // Limitation des valeurs extrêmes
    m_learningParams.initialLearningRate = qBound(0.0001, m_learningParams.initialLearningRate, 0.1);
}

void HybridBitcoinMiner::validateBiologicalPerformance()
{
    // Validation périodique des performances biologiques
    if (m_predictionHistory.size() < 10) {
        return;
    }
    
    // Calcul des métriques de validation
    int validatedPredictions = 0;
    int successfulPredictions = 0;
    
    for (const auto& prediction : m_predictionHistory) {
        if (prediction.isValidated) {
            validatedPredictions++;
            // Validation basée sur la confiance et l'entropie
            if (prediction.confidence > 0.7 && prediction.biologicalEntropy > 0.5) {
                successfulPredictions++;
            }
        }
    }
    
    if (validatedPredictions > 0) {
        double performanceRatio = (double)successfulPredictions / validatedPredictions;
        
        qDebug() << "Biological performance validation - Success rate:" 
                << performanceRatio << "(" << successfulPredictions 
                << "/" << validatedPredictions << ")";
        
        // Ajustement du poids biologique basé sur les performances
        if (performanceRatio > 0.8) {
            setBiologicalWeight(std::min(1.0, m_biologicalWeight + 0.05));
        } else if (performanceRatio < 0.3) {
            setBiologicalWeight(std::max(0.0, m_biologicalWeight - 0.05));
        }
    }
}

bool HybridBitcoinMiner::performHybridHash(const QString& blockHeader, uint32_t& nonce, QString& blockHash)
{
    // Combinaison du mining traditionnel et biologique
    bool useBiological = (QRandomGenerator::global()->generateDouble() < m_biologicalWeight);
    
    if (useBiological && m_meaInterface && m_biologicalNetwork) {
        // Utilisation du réseau biologique pour la prédiction de nonce
        BiologicalNoncePrediction prediction = predictNonce(blockHeader);
        
        if (prediction.confidence > 0.5) {
            nonce = prediction.predictedNonce;
            
            // Validation avec SHA-256
            QString targetHash = "000000"; // Difficulté simplifiée pour les tests
            if (validateNonceWithSHA256(blockHeader, nonce, targetHash)) {
                blockHash = calculateSHA256Hash(blockHeader, nonce);
                validatePrediction(prediction, true);
                return true;
            } else {
                validatePrediction(prediction, false);
                // Fallback sur méthode traditionnelle
                return m_traditionalMiner->mineBlock(blockHeader, nonce, blockHash);
            }
        }
    }
    
    // Mining traditionnel
    m_metrics.traditionalHashes++;
    return m_traditionalMiner->mineBlock(blockHeader, nonce, blockHash);
}

uint32_t HybridBitcoinMiner::generateBiologicalNonce(const std::vector<double>& meaSignals)
{
    if (meaSignals.size() != 60) {
        return QRandomGenerator::global()->generate();
    }
    
    // Conversion des signaux MEA en nonce
    uint32_t nonce = 0;
    
    // Utilisation de l'entropie biologique pour générer le nonce
    for (size_t i = 0; i < std::min(meaSignals.size(), size_t(32)); ++i) {
        // Normalisation et conversion en bits
        double normalizedSignal = std::abs(meaSignals[i]);
        normalizedSignal = std::fmod(normalizedSignal, 1.0);
        
        if (normalizedSignal > 0.5) {
            nonce |= (1u << (i % 32));
        }
    }
    
    // Ajout d'une composante basée sur l'état du réseau
    if (m_biologicalNetwork) {
        std::vector<double> networkState = extractNetworkFeatures();
        if (!networkState.empty()) {
            uint32_t networkContrib = static_cast<uint32_t>(
                networkState[0] * networkState.size() * 1000000);
            nonce ^= networkContrib;
        }
    }
    
    return nonce;
}

bool HybridBitcoinMiner::validateNonceWithSHA256(const QString& blockHeader, uint32_t nonce, const QString& targetHash)
{
    QString fullHeader = blockHeader + QString::number(nonce);
    QString hash = calculateSHA256Hash(fullHeader, 0);
    
    return hash.startsWith(targetHash);
}

bool HybridBitcoinMiner::trainNetworkWithPattern(const std::vector<double>& input, const std::vector<double>& target)
{
    if (!m_biologicalNetwork || input.empty() || target.empty()) {
        return false;
    }
    
    try {
        // Forward propagation
        performForwardPropagation(input);
        
        // Backward propagation avec target
        performBackwardPropagation(target);
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during network training:" << e.what();
        return false;
    }
}

void HybridBitcoinMiner::performForwardPropagation(const std::vector<double>& input)
{
    if (m_biologicalNetwork) {
        QVector<double> qInput(input.begin(), input.end());
        m_biologicalNetwork->forwardPropagation(qInput);
    }
}

void HybridBitcoinMiner::performBackwardPropagation(const std::vector<double>& target)
{
    if (m_biologicalNetwork) {
        QVector<double> qTarget(target.begin(), target.end());
        m_biologicalNetwork->backPropagation(qTarget);
    }
}

void HybridBitcoinMiner::updateSynapticWeights()
{
    if (m_biologicalNetwork) {
        m_biologicalNetwork->updateWeights();
    }
}

void HybridBitcoinMiner::adaptLearningRate()
{
    // Adaptation du taux d'apprentissage basé sur les performances
    double currentAccuracy = getBiologicalAccuracy();
    
    if (currentAccuracy > 0.8) {
        // Haute précision, réduction du taux d'apprentissage
        m_learningParams.initialLearningRate *= m_learningParams.decayRate;
    } else if (currentAccuracy < 0.3) {
        // Faible précision, augmentation du taux d'apprentissage
        m_learningParams.initialLearningRate /= m_learningParams.decayRate;
    }
    
    // Limitation des valeurs
    m_learningParams.initialLearningRate = qBound(0.0001, 
                                                 m_learningParams.initialLearningRate, 
                                                 0.1);
}

void HybridBitcoinMiner::adjustBiologicalParameters()
{
    // Ajustement des paramètres biologiques basé sur les métriques
    double efficiency = m_metrics.energyEfficiency.load();
    double accuracy = m_metrics.biologicalAccuracy.load();
    
    // Ajustement du momentum
    if (accuracy > 0.7) {
        m_learningParams.momentumFactor = std::min(0.99, m_learningParams.momentumFactor + 0.01);
    } else {
        m_learningParams.momentumFactor = std::max(0.5, m_learningParams.momentumFactor - 0.01);
    }
    
    // Ajustement du seuil d'adaptation
    if (efficiency > 0.8) {
        m_learningParams.adaptationThreshold *= 0.95;
    } else {
        m_learningParams.adaptationThreshold *= 1.05;
    }
    
    m_learningParams.adaptationThreshold = qBound(0.01, 
                                                 m_learningParams.adaptationThreshold, 
                                                 0.5);
}

bool HybridBitcoinMiner::detectPatternConvergence()
{
    if (m_predictionHistory.size() < 50) {
        return false;
    }
    
    // Analyse des 50 dernières prédictions
    std::vector<double> recentConfidences;
    for (size_t i = m_predictionHistory.size() - 50; i < m_predictionHistory.size(); ++i) {
        recentConfidences.push_back(m_predictionHistory[i].confidence);
    }
    
    // Calcul de la variance des confidences
    double mean = std::accumulate(recentConfidences.begin(), recentConfidences.end(), 0.0) / recentConfidences.size();
    double variance = 0.0;
    
    for (double conf : recentConfidences) {
        variance += (conf - mean) * (conf - mean);
    }
    variance /= recentConfidences.size();
    
    // Convergence détectée si variance faible et moyenne élevée
    return (variance < 0.05 && mean > 0.7);
}

void HybridBitcoinMiner::reinforceSuccessfulPatterns()
{
    // Renforcement des patterns ayant conduit au succès
    for (auto& data : m_trainingHistory) {
        if (data.isValidated && data.validationScore > 0.8) {
            // Re-entraînement avec un taux d'apprentissage plus élevé
            double originalRate = m_learningParams.initialLearningRate;
            m_learningParams.initialLearningRate *= 2.0;
            
            trainNetworkWithPattern(data.inputPattern, data.targetOutput);
            
            m_learningParams.initialLearningRate = originalRate;
        }
    }
}

std::vector<double> HybridBitcoinMiner::preprocessMEASignals(const std::vector<double>& rawSignals)
{
    if (rawSignals.size() != 60) {
        return rawSignals;
    }
    
    std::vector<double> processed = rawSignals;
    
    // Normalisation des signaux
    double minVal = *std::min_element(processed.begin(), processed.end());
    double maxVal = *std::max_element(processed.begin(), processed.end());
    
    if (maxVal != minVal) {
        for (double& signal : processed) {
            signal = (signal - minVal) / (maxVal - minVal);
        }
    }
    
    // Filtrage passe-bas simple
    for (size_t i = 1; i < processed.size() - 1; ++i) {
        processed[i] = 0.25 * processed[i-1] + 0.5 * processed[i] + 0.25 * processed[i+1];
    }
    
    return processed;
}

std::vector<double> HybridBitcoinMiner::extractNetworkFeatures()
{
    if (!m_biologicalNetwork) {
        return {};
    }
    
    QVector<double> qOutput = m_biologicalNetwork->getOutputValues();
    return std::vector<double>(qOutput.begin(), qOutput.end());
}

double HybridBitcoinMiner::calculateBiologicalEntropy(const std::vector<double>& signalData)
{
    if (signalData.empty()) {
        return 0.0;
    }
    
    // Calcul de l'entropie de Shannon
    std::map<int, int> histogram;
    for (double signal : signalData) {
        int bin = static_cast<int>(signal * 100) % 100;
        histogram[bin]++;
    }
    
    double entropy = 0.0;
    double totalSamples = signalData.size();
    
    for (const auto& pair : histogram) {
        double probability = pair.second / totalSamples;
        if (probability > 0) {
            entropy -= probability * std::log2(probability);
        }
    }
    
    return entropy / std::log2(100); // Normalisation
}

double HybridBitcoinMiner::evaluatePredictionConfidence(const std::vector<double>& networkOutput)
{
    if (networkOutput.empty()) {
        return 0.0;
    }
    
    // Calcul de la confiance basée sur la distribution de sortie
    double maxOutput = *std::max_element(networkOutput.begin(), networkOutput.end());
    double minOutput = *std::min_element(networkOutput.begin(), networkOutput.end());
    
    // Confiance proportionnelle à la variance de sortie et à l'étendue
    double sum = std::accumulate(networkOutput.begin(), networkOutput.end(), 0.0);
    double mean = sum / networkOutput.size();
    
    // Utiliser l'étendue pour calculer la confiance
    double range = maxOutput - minOutput;
    
    double variance = 0.0;
    for (double output : networkOutput) {
        variance += (output - mean) * (output - mean);
    }
    variance /= networkOutput.size();
    
    // Confiance élevée si variance importante et étendue significative (décision claire)
    double confidence = std::min(1.0, (variance + range * 0.1) * 10.0);
    return confidence;
}

void HybridBitcoinMiner::setupMiningThreads()
{
    // Configuration du pool de threads pour mining parallèle
    int optimalThreadCount = QThread::idealThreadCount();
    m_threadPool->setMaxThreadCount(optimalThreadCount);
    
    qDebug() << "Mining threads configured - Count:" << optimalThreadCount;
}

void HybridBitcoinMiner::cleanupMiningThreads()
{
    // Nettoyage des threads de mining
    if (m_threadPool) {
        m_threadPool->waitForDone(5000);
    }
}

// Méthodes utilitaires supplémentaires

void HybridBitcoinMiner::generateInitialTrainingData(std::vector<BioMining::HCrypto::BiologicalTrainingData>& trainingData)
{
    // Génération de données d'entraînement synthétiques pour l'apprentissage initial
    trainingData.clear();
    
    for (int i = 0; i < 1000; ++i) {
        BiologicalTrainingData sample;
        
        // Génération de pattern d'entrée aléatoire (60 électrodes)
        sample.inputPattern.resize(60);
        for (int j = 0; j < 60; ++j) {
            sample.inputPattern[j] = QRandomGenerator::global()->generateDouble();
        }
        
        // Génération de nonce cible basé sur le pattern
        sample.expectedNonce = generateBiologicalNonce(sample.inputPattern);
        sample.targetOutput = convertNonceToOutput(sample.expectedNonce);
        sample.difficulty = 1.0;
        sample.timestamp = QDateTime::currentDateTime();
        sample.validationScore = 1.0;
        sample.isValidated = false;
        
        trainingData.push_back(sample);
    }
    
    qDebug() << "Generated" << trainingData.size() << "initial training samples";
}

void HybridBitcoinMiner::prepareRetroLearningData(std::vector<BiologicalTrainingData>& retroData)
{
    retroData.clear();
    
    // Conversion des prédictions validées en données d'apprentissage
    for (const auto& prediction : m_predictionHistory) {
        if (prediction.isValidated) {
            BiologicalTrainingData sample;
            sample.inputPattern = prediction.networkOutput;
            sample.expectedNonce = prediction.predictedNonce;
            sample.targetOutput = convertNonceToOutput(prediction.predictedNonce);
            sample.timestamp = prediction.predictionTime;
            sample.validationScore = prediction.confidence;
            sample.isValidated = true;
            
            retroData.push_back(sample);
        }
    }
    
    qDebug() << "Prepared" << retroData.size() << "retro-learning samples";
}

std::vector<double> HybridBitcoinMiner::convertNonceToOutput(uint32_t nonce)
{
    // Conversion du nonce en vecteur de sortie pour le réseau
    std::vector<double> output(32);
    
    for (int i = 0; i < 32; ++i) {
        output[i] = ((nonce >> i) & 1) ? 1.0 : 0.0;
    }
    
    return output;
}

double HybridBitcoinMiner::calculatePredictionError(const std::vector<double>& predicted, const std::vector<double>& target)
{
    if (predicted.size() != target.size()) {
        return 1.0; // Erreur maximale
    }
    
    double mse = 0.0;
    for (size_t i = 0; i < predicted.size(); ++i) {
        double diff = predicted[i] - target[i];
        mse += diff * diff;
    }
    
    return mse / predicted.size();
}

void HybridBitcoinMiner::updateBiologicalAccuracy()
{
    if (m_metrics.biologicalPredictions.load() == 0) {
        m_metrics.biologicalAccuracy = 0.0;
        return;
    }
    
    double accuracy = (double)m_metrics.successfulPredictions.load() / 
                     m_metrics.biologicalPredictions.load();
    m_metrics.biologicalAccuracy = accuracy;
}

void HybridBitcoinMiner::updateHashRate()
{
    auto currentTime = QDateTime::currentDateTime();
    auto elapsedTime = m_metrics.startTime.msecsTo(currentTime) / 1000.0; // secondes
    
    if (elapsedTime > 0) {
        double hashRate = m_metrics.totalHashes.load() / elapsedTime;
        m_metrics.hybridHashRate = hashRate;
    }
}

void HybridBitcoinMiner::updateEnergyEfficiency()
{
    // Calcul de l'efficacité énergétique (hashes par unité d'énergie biologique)
    double biologicalRatio = (double)m_metrics.biologicalPredictions.load() / 
                           std::max((uint64_t)1ULL, (uint64_t)m_metrics.totalHashes.load());
    double accuracy = m_metrics.biologicalAccuracy.load();
    
    m_metrics.energyEfficiency = biologicalRatio * accuracy;
}

double HybridBitcoinMiner::calculateRecentAccuracy()
{
    if (m_predictionHistory.size() < 10) {
        return 0.0;
    }
    
    // Calcul de l'accuracy sur les 20 dernières prédictions
    int recentCount = std::min(20, (int)m_predictionHistory.size());
    int successCount = 0;
    
    for (int i = m_predictionHistory.size() - recentCount; i < (int)m_predictionHistory.size(); ++i) {
        if (m_predictionHistory[i].isValidated && m_predictionHistory[i].confidence > 0.7) {
            successCount++;
        }
    }
    
    return (double)successCount / recentCount;
}

double HybridBitcoinMiner::calculateNetworkComplexity()
{
    if (!m_biologicalNetwork) {
        return 0.0;
    }
    
    // Complexité basée sur le nombre de connexions actives et la diversité des poids
    return m_biologicalNetwork->getNetworkComplexity();
}

double HybridBitcoinMiner::calculateAdaptationRate()
{
    // Taux d'adaptation basé sur l'évolution des performances
    if (m_trainingHistory.size() < 2) {
        return 0.0;
    }
    
    double recentPerf = calculateRecentAccuracy();
    double historicalPerf = 0.0;
    
    // Performance historique (première moitié)
    int halfSize = m_trainingHistory.size() / 2;
    int validCount = 0;
    
    for (int i = 0; i < halfSize; ++i) {
        if (m_trainingHistory[i].isValidated) {
            historicalPerf += m_trainingHistory[i].validationScore;
            validCount++;
        }
    }
    
    if (validCount > 0) {
        historicalPerf /= validCount;
        return std::abs(recentPerf - historicalPerf);
    }
    
    return 0.0;
}

double HybridBitcoinMiner::calculateBiologicalContribution()
{
    // Contribution du réseau biologique au mining
    return m_biologicalWeight * m_metrics.biologicalAccuracy.load();
}

QString HybridBitcoinMiner::generateTestBlockHeader()
{
    // Génération d'un header de bloc pour les tests
    return QString("test_block_header_%1").arg(QDateTime::currentMSecsSinceEpoch());
}

QString HybridBitcoinMiner::calculateSHA256Hash(const QString& data, uint32_t nonce)
{
    QString fullData = data + QString::number(nonce);
    QByteArray hash = QCryptographicHash::hash(fullData.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

// === IMPLÉMENTATION SYSTÈME TRIPLE ===

bool HybridBitcoinMiner::connectToRealMEA(std::shared_ptr<BioMining::Bio::RealMEAInterface> realMeaInterface)
{
    QMutexLocker locker(&m_stateMutex);
    
    qDebug() << "[TripleSystem] Connexion à RealMEAInterface...";
    
    if (!realMeaInterface) {
        qCritical() << "Interface RealMEA invalide";
        return false;
    }
    
    m_realMeaInterface = realMeaInterface;
    
    // Connexion des signaux RealMEA
    connect(m_realMeaInterface.get(), &BioMining::Bio::RealMEAInterface::bitcoinPatternLearned,
            this, [this](const BioMining::Bio::BitcoinLearningPattern& pattern) {
                qDebug() << "[TripleSystem] MEA pattern learned:" << pattern.blockHash.left(16);
            });
    
    connect(m_realMeaInterface.get(), &BioMining::Bio::RealMEAInterface::noncePredicationReady,
            this, [this](const BioMining::Bio::NeuralBitcoinResponse& response) {
                qDebug() << "[TripleSystem] MEA nonce prediction:" << response.predictedNonce;
            });
    
    connect(m_realMeaInterface.get(), &BioMining::Bio::RealMEAInterface::learningStatsUpdated,
            this, [this](const BioMining::Bio::BitcoinLearningStats& stats) {
                qDebug() << "[TripleSystem] MEA learning stats - Accuracy:" << stats.currentAccuracy;
            });
    
    qDebug() << "[TripleSystem] RealMEAInterface connecté avec succès";
    return true;
}

bool HybridBitcoinMiner::initializeTripleSystem(const TripleOptimizationConfig& config)
{
    QMutexLocker locker(&m_stateMutex);
    
    qDebug() << "[TripleSystem] Initialisation du système d'optimisation triple...";
    
    if (!m_traditionalMiner || !m_biologicalNetwork || !m_realMeaInterface) {
        qCritical() << "[TripleSystem] Tous les composants doivent être initialisés";
        return false;
    }
    
    m_tripleConfig = config;
    
    // Vérification des poids (doivent totaliser 1.0)
    double totalWeight = config.sha256Weight + config.networkWeight + config.meaWeight;
    if (std::abs(totalWeight - 1.0) > 0.01) {
        qWarning() << "[TripleSystem] Normalisation des poids - Total:" << totalWeight;
        m_tripleConfig.sha256Weight /= totalWeight;
        m_tripleConfig.networkWeight /= totalWeight;
        m_tripleConfig.meaWeight /= totalWeight;
    }
    
    // Initialisation des métriques triple
    m_sha256Successes = 0;
    m_networkSuccesses = 0;
    m_meaSuccesses = 0;
    m_fusionSuccesses = 0;
    
    // Configuration de l'apprentissage Bitcoin sur RealMEA
    BioMining::Bio::BitcoinLearningConfig meaLearningConfig;
    meaLearningConfig.learningRate = m_learningParams.initialLearningRate;
    meaLearningConfig.maxTrainingEpochs = 500;
    meaLearningConfig.targetAccuracy = 0.75;
    meaLearningConfig.enableRealtimeLearning = true;
    meaLearningConfig.enableBackpropagation = true;
    
    if (!m_realMeaInterface->initializeBitcoinLearning(meaLearningConfig)) {
        qCritical() << "[TripleSystem] Échec d'initialisation apprentissage Bitcoin MEA";
        return false;
    }
    
    m_tripleSystemEnabled = true;
    
    qDebug() << QString("[TripleSystem] Système triple initialisé - Poids: SHA256=%1, Network=%2, MEA=%3")
                .arg(m_tripleConfig.sha256Weight, 0, 'f', 3)
                .arg(m_tripleConfig.networkWeight, 0, 'f', 3)
                .arg(m_tripleConfig.meaWeight, 0, 'f', 3);
    
    return true;
}

bool HybridBitcoinMiner::configureTripleSystemLearning()
{
    QMutexLocker locker(&m_learningMutex);
    
    qDebug() << "[TripleSystem] Configuration apprentissage triple...";
    
    if (!m_tripleSystemEnabled) {
        qCritical() << "[TripleSystem] Système triple non initialisé";
        return false;
    }
    
    // Configuration réseau biologique pour collaboration
    BioMining::Network::BiologicalNetwork::NetworkConfig networkConfig;
    networkConfig.inputSize = 60;  // Compatible MEA
    networkConfig.outputSize = 32; // Nonce 32-bit
    networkConfig.hiddenLayers = QVector<int>{128, 96, 64, 32}; // Architecture profonde
    networkConfig.learningRate = m_learningParams.initialLearningRate;
    networkConfig.enablePlasticity = true;
    networkConfig.enableAdaptation = true;
    
    if (!m_biologicalNetwork->configureNetwork(networkConfig)) {
        qCritical() << "[TripleSystem] Échec configuration réseau biologique";
        return false;
    }
    
    qDebug() << "[TripleSystem] Configuration apprentissage triple terminée";
    return true;
}

TripleSystemPrediction HybridBitcoinMiner::predictNonceTriple(const QString& blockHeader)
{
    TripleSystemPrediction prediction;
    
    if (!m_tripleSystemEnabled) {
        qWarning() << "[TripleSystem] Système triple non activé";
        return prediction;
    }
    
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    qDebug() << QString("[TripleSystem] Prédiction triple pour: %1").arg(blockHeader.left(16));
    
    // Prédictions parallèles des trois systèmes
    QFuture<uint32_t> sha256Future, networkFuture, meaFuture;
    
    // 1. SHA-256 traditionnel (asynchrone)
    sha256Future = QtConcurrent::run([this, blockHeader, &prediction]() {
        return predictWithSHA256(blockHeader, prediction.sha256Confidence, prediction.sha256Time);
    });
    
    // 2. Réseau biologique (asynchrone)  
    networkFuture = QtConcurrent::run([this, blockHeader, &prediction]() {
        return predictWithBiologicalNetwork(blockHeader, prediction.networkConfidence, prediction.networkTime);
    });
    
    // 3. MEA réel avec neurones biologiques (asynchrone)
    meaFuture = QtConcurrent::run([this, blockHeader, &prediction]() {
        return predictWithRealMEA(blockHeader, prediction.meaConfidence, prediction.meaTime);
    });
    
    // Attente des résultats avec timeout
    const int TIMEOUT_MS = 5000;
    
    try {
        // Attente des résultats avec gestion de timeout manuelle
        QElapsedTimer timer;
        timer.start();
        
        // Attendre que toutes les futures se terminent ou timeout
        while (!sha256Future.isFinished() || !networkFuture.isFinished() || !meaFuture.isFinished()) {
            if (timer.elapsed() > TIMEOUT_MS) {
                qWarning() << "[HybridMiner] Timeout lors de l'attente des prédictions";
                // Utiliser des valeurs par défaut en cas de timeout
                prediction.sha256Nonce = 0;
                prediction.networkNonce = 0;
                prediction.meaNonce = 0;
                break;
            }
            QThread::msleep(10); // Attendre 10ms avant de vérifier à nouveau
        }
        
        // Si pas de timeout, récupérer les résultats
        if (timer.elapsed() <= TIMEOUT_MS) {
            prediction.sha256Nonce = sha256Future.result();
            prediction.networkNonce = networkFuture.result(); 
            prediction.meaNonce = meaFuture.result();
        }
        
        // Fusion intelligente des prédictions
        prediction.fusedNonce = fuseTriplePredictions(prediction);
        prediction.selectedMethod = selectOptimalMethod(prediction);
        
        // Calcul de la confiance fusionnée
        prediction.fusedConfidence = (
            prediction.sha256Confidence * m_tripleConfig.sha256Weight +
            prediction.networkConfidence * m_tripleConfig.networkWeight + 
            prediction.meaConfidence * m_tripleConfig.meaWeight
        );
        
        prediction.predictionTime = QDateTime::currentMSecsSinceEpoch() * 1000 - startTime;
        
        // Ajout à l'historique
        if (m_tripleHistory.size() >= 1000) {
            m_tripleHistory.erase(m_tripleHistory.begin());
        }
        m_tripleHistory.push_back(prediction);
        
        emit triplePredictionReady(prediction);
        
        qDebug() << QString("[TripleSystem] Prédictions - SHA256: %1 (conf: %2), Network: %3 (conf: %4), MEA: %5 (conf: %6)")
                    .arg(prediction.sha256Nonce).arg(prediction.sha256Confidence, 0, 'f', 3)
                    .arg(prediction.networkNonce).arg(prediction.networkConfidence, 0, 'f', 3)
                    .arg(prediction.meaNonce).arg(prediction.meaConfidence, 0, 'f', 3);
        
        qDebug() << QString("[TripleSystem] Fusion: %1 (conf: %2), Méthode sélectionnée: %3")
                    .arg(prediction.fusedNonce).arg(prediction.fusedConfidence, 0, 'f', 3)
                    .arg(static_cast<int>(prediction.selectedMethod));
        
    } catch (const std::exception& e) {
        qCritical() << "[TripleSystem] Exception pendant prédiction:" << e.what();
        prediction.fusedNonce = prediction.sha256Nonce; // Fallback
        prediction.selectedMethod = MiningMethod::TraditionalSHA256;
        prediction.fusedConfidence = prediction.sha256Confidence;
    }
    
    return prediction;
}

uint32_t HybridBitcoinMiner::predictWithSHA256(const QString& blockHeader, double& confidence, qint64& processTime)
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    // Algorithme SHA-256 déterministe basé sur le header
    QByteArray headerBytes = blockHeader.toUtf8();
    QByteArray hash = QCryptographicHash::hash(headerBytes, QCryptographicHash::Sha256);
    
    // Conversion du hash en nonce  
    uint32_t nonce = 0;
    for (int i = 0; i < std::min(4, hash.size()); ++i) {
        nonce |= (static_cast<uint8_t>(hash[i]) << (i * 8));
    }
    
    // Confiance basée sur la difficulté et l'entropie du hash
    double entropy = 0.0;
    for (int i = 0; i < hash.size(); ++i) {
        uint8_t byte = static_cast<uint8_t>(hash[i]);
        if (byte > 0) {
            entropy -= (byte / 255.0) * std::log2(byte / 255.0);
        }
    }
    
    confidence = qMin(1.0, entropy / 8.0); // Normalisation
    processTime = QDateTime::currentMSecsSinceEpoch() * 1000 - startTime;
    
    return nonce;
}

uint32_t HybridBitcoinMiner::predictWithBiologicalNetwork(const QString& blockHeader, double& confidence, qint64& processTime)
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    if (!m_biologicalNetwork) {
        confidence = 0.0;
        processTime = 0;
        return 0;
    }
    
    try {
        // Conversion du header en input réseau
        QVector<double> networkInput = blockHeaderToNetworkInput(blockHeader);
        
        // Propagation avant
        m_biologicalNetwork->forwardPropagation(networkInput);
        
        // Extraction de la sortie
        QVector<double> output = m_biologicalNetwork->getOutputValues();
        
        if (output.size() >= 32) {
            // Conversion sortie binaire vers nonce
            uint32_t nonce = 0;
            for (int i = 0; i < 32; ++i) {
                if (output[i] > 0.5) {
                    nonce |= (1u << i);
                }
            }
            
            // Confiance basée sur la netteté de la sortie
            double sharpness = 0.0;
            for (double val : output) {
                double deviation = std::abs(val - 0.5);
                sharpness += deviation;
            }
            confidence = qMin(1.0, sharpness / (output.size() * 0.5));
            
            processTime = QDateTime::currentMSecsSinceEpoch() * 1000 - startTime;
            return nonce;
        }
        
    } catch (const std::exception& e) {
        qCritical() << "[TripleSystem] Erreur réseau biologique:" << e.what();
    }
    
    confidence = 0.0;
    processTime = QDateTime::currentMSecsSinceEpoch() * 1000 - startTime;
    return 0;
}

uint32_t HybridBitcoinMiner::predictWithRealMEA(const QString& blockHeader, double& confidence, qint64& processTime)
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    if (!m_realMeaInterface) {
        confidence = 0.0;
        processTime = 0;
        return 0;
    }
    
    try {
        // Utilisation de l'interface MEA pour prédiction Bitcoin
        BioMining::Bio::NeuralBitcoinResponse meaResponse = m_realMeaInterface->predictNonce(blockHeader, 1000000.0);
        
        confidence = meaResponse.confidence;
        processTime = meaResponse.responseTime;
        
        qDebug() << QString("[TripleSystem] MEA prédiction: nonce=%1, confiance=%2, temps=%3μs")
                    .arg(meaResponse.predictedNonce)
                    .arg(confidence, 0, 'f', 3)
                    .arg(processTime);
        
        return meaResponse.predictedNonce;
        
    } catch (const std::exception& e) {
        qCritical() << "[TripleSystem] Erreur MEA réel:" << e.what();
    }
    
    confidence = 0.0;
    processTime = QDateTime::currentMSecsSinceEpoch() * 1000 - startTime;
    return 0;
}

uint32_t HybridBitcoinMiner::fuseTriplePredictions(const TripleSystemPrediction& predictions)
{
    // Fusion adaptive basée sur les confidences et les poids
    if (m_tripleConfig.enableDynamicWeighting) {
        return adaptiveFusion(predictions);
    } else {
        return confidenceBasedFusion(predictions);
    }
}

uint32_t HybridBitcoinMiner::weightedAverageFusion(const TripleSystemPrediction& predictions)
{
    // Fusion par moyenne pondérée
    double weightedSum = 
        predictions.sha256Nonce * m_tripleConfig.sha256Weight +
        predictions.networkNonce * m_tripleConfig.networkWeight +
        predictions.meaNonce * m_tripleConfig.meaWeight;
    
    return static_cast<uint32_t>(weightedSum);
}

uint32_t HybridBitcoinMiner::confidenceBasedFusion(const TripleSystemPrediction& predictions)
{
    // Fusion basée sur les confidences normalisées
    double totalConfidence = predictions.sha256Confidence + predictions.networkConfidence + predictions.meaConfidence;
    
    if (totalConfidence < 0.001) {
        // Fallback sur moyenne simple
        return (predictions.sha256Nonce + predictions.networkNonce + predictions.meaNonce) / 3;
    }
    
    double sha256Weight = predictions.sha256Confidence / totalConfidence;
    double networkWeight = predictions.networkConfidence / totalConfidence;  
    double meaWeight = predictions.meaConfidence / totalConfidence;
    
    double fusedNonce = 
        predictions.sha256Nonce * sha256Weight +
        predictions.networkNonce * networkWeight +
        predictions.meaNonce * meaWeight;
    
    return static_cast<uint32_t>(fusedNonce);
}

uint32_t HybridBitcoinMiner::adaptiveFusion(const TripleSystemPrediction& predictions)
{
    // Fusion adaptive basée sur l'historique de performance
    double sha256Perf = calculateSystemEfficiency(MiningMethod::TraditionalSHA256);
    double networkPerf = calculateSystemEfficiency(MiningMethod::BiologicalNetwork);
    double meaPerf = calculateSystemEfficiency(MiningMethod::RealMEANeurons);
    
    double totalPerf = sha256Perf + networkPerf + meaPerf;
    
    if (totalPerf < 0.001) {
        return confidenceBasedFusion(predictions);
    }
    
    // Poids adaptatifs basés sur les performances
    double adaptiveSha256Weight = (sha256Perf / totalPerf) * predictions.sha256Confidence;
    double adaptiveNetworkWeight = (networkPerf / totalPerf) * predictions.networkConfidence;
    double adaptiveMeaWeight = (meaPerf / totalPerf) * predictions.meaConfidence;
    
    double totalAdaptiveWeight = adaptiveSha256Weight + adaptiveNetworkWeight + adaptiveMeaWeight;
    
    if (totalAdaptiveWeight < 0.001) {
        return confidenceBasedFusion(predictions);
    }
    
    double fusedNonce = 
        predictions.sha256Nonce * (adaptiveSha256Weight / totalAdaptiveWeight) +
        predictions.networkNonce * (adaptiveNetworkWeight / totalAdaptiveWeight) +
        predictions.meaNonce * (adaptiveMeaWeight / totalAdaptiveWeight);
    
    return static_cast<uint32_t>(fusedNonce);
}

MiningMethod HybridBitcoinMiner::selectOptimalMethod(const TripleSystemPrediction& predictions)
{
    // Sélection basée sur la confiance et la performance historique
    
    if (m_tripleConfig.preferHighConfidence) {
        // Sélection par confiance maximale
        double maxConfidence = std::max({predictions.sha256Confidence, 
                                        predictions.networkConfidence, 
                                        predictions.meaConfidence});
        
        if (predictions.sha256Confidence == maxConfidence && 
            predictions.sha256Confidence >= m_tripleConfig.minSha256Confidence) {
            return MiningMethod::TraditionalSHA256;
        } else if (predictions.networkConfidence == maxConfidence &&
                  predictions.networkConfidence >= m_tripleConfig.minNetworkConfidence) {
            return MiningMethod::BiologicalNetwork;
        } else if (predictions.meaConfidence == maxConfidence &&
                  predictions.meaConfidence >= m_tripleConfig.minMeaConfidence) {
            return MiningMethod::RealMEANeurons;
        }
    }
    
    // Sélection par efficacité combinée (confiance × performance historique)
    double sha256Score = predictions.sha256Confidence * calculateSystemEfficiency(MiningMethod::TraditionalSHA256);
    double networkScore = predictions.networkConfidence * calculateSystemEfficiency(MiningMethod::BiologicalNetwork);
    double meaScore = predictions.meaConfidence * calculateSystemEfficiency(MiningMethod::RealMEANeurons);
    
    double maxScore = std::max({sha256Score, networkScore, meaScore});
    
    if (sha256Score == maxScore) {
        return MiningMethod::TraditionalSHA256;
    } else if (networkScore == maxScore) {
        return MiningMethod::BiologicalNetwork;  
    } else if (meaScore == maxScore) {
        return MiningMethod::RealMEANeurons;
    }
    
    // Fallback sur fusion si aucune méthode claire
    return MiningMethod::HybridFusion;
}

bool HybridBitcoinMiner::performTripleOptimizedMining(const QString& blockHeader, uint32_t& nonce, QString& blockHash)
{
    if (!m_tripleSystemEnabled) {
        qWarning() << "[TripleSystem] Système triple non activé, utilisation méthode hybride classique";
        return performHybridHash(blockHeader, nonce, blockHash);
    }
    
    // Prédiction triple
    TripleSystemPrediction prediction = predictNonceTriple(blockHeader);
    
    // Test de la méthode sélectionnée
    MiningMethod selectedMethod = prediction.selectedMethod;
    uint32_t testNonce = 0;
    
    switch (selectedMethod) {
        case MiningMethod::TraditionalSHA256:
            testNonce = prediction.sha256Nonce;
            break;
        case MiningMethod::BiologicalNetwork:
            testNonce = prediction.networkNonce;
            break;
        case MiningMethod::RealMEANeurons:
            testNonce = prediction.meaNonce;
            break;
        case MiningMethod::HybridFusion:
        default:
            testNonce = prediction.fusedNonce;
            break;
    }
    
    // Validation du nonce avec SHA-256
    QString targetHash = "0000"; // Difficulté simplifiée
    if (validateNonceWithSHA256(blockHeader, testNonce, targetHash)) {
        // Succès !
        nonce = testNonce;
        blockHash = calculateSHA256Hash(blockHeader, nonce);
        
        // Mise à jour des métriques de succès
        updateSuccessCounters(selectedMethod, true);
        validateAndLearnFromResult(prediction, true);
        
        qDebug() << QString("[TripleSystem] SUCCÈS ! Méthode: %1, Nonce: %2, Hash: %3")
                    .arg(static_cast<int>(selectedMethod))
                    .arg(nonce)
                    .arg(blockHash.left(16));
        
        return true;
    } else {
        // Échec, essai avec méthodes de fallback
        updateSuccessCounters(selectedMethod, false);
        
        // Test des autres prédictions
        QVector<QPair<uint32_t, MiningMethod>> fallbacks = {
            {prediction.fusedNonce, MiningMethod::HybridFusion},
            {prediction.sha256Nonce, MiningMethod::TraditionalSHA256},
            {prediction.networkNonce, MiningMethod::BiologicalNetwork},
            {prediction.meaNonce, MiningMethod::RealMEANeurons}
        };
        
        for (const auto& fallback : fallbacks) {
            if (fallback.second != selectedMethod && 
                validateNonceWithSHA256(blockHeader, fallback.first, targetHash)) {
                
                nonce = fallback.first;
                blockHash = calculateSHA256Hash(blockHeader, nonce);
                
                updateSuccessCounters(fallback.second, true);
                
                // Marquer la prédiction originale comme échec mais noter la méthode de succès
                TripleSystemPrediction correctedPrediction = prediction;
                correctedPrediction.actualSuccessMethod = fallback.second;
                validateAndLearnFromResult(correctedPrediction, false);
                
                qDebug() << QString("[TripleSystem] SUCCÈS Fallback! Méthode: %1, Nonce: %2")
                            .arg(static_cast<int>(fallback.second))
                            .arg(nonce);
                
                return true;
            }
        }
        
        validateAndLearnFromResult(prediction, false);
        
        // Fallback final sur mining traditionnel
        return m_traditionalMiner->mineBlock(blockHeader, nonce, blockHash);
    }
}

bool HybridBitcoinMiner::validateTriplePrediction(const TripleSystemPrediction& prediction, bool wasSuccessful)
{
    QMutexLocker locker(&m_dataMutex);
    
    // Recherche dans l'historique et mise à jour
    auto it = std::find_if(m_tripleHistory.begin(), m_tripleHistory.end(),
                          [&prediction](TripleSystemPrediction& p) {
                              return p.predictionTime == prediction.predictionTime &&
                                     p.fusedNonce == prediction.fusedNonce;
                          });
    
    if (it != m_tripleHistory.end()) {
        it->isValidated = true;
        it->wasSuccessful = wasSuccessful;
        
        // Apprentissage depuis le résultat
        validateAndLearnFromResult(*it, wasSuccessful);
        
        qDebug() << QString("[TripleSystem] Validation - Succès: %1, Méthode: %2")
                    .arg(wasSuccessful ? "OUI" : "NON")
                    .arg(static_cast<int>(prediction.selectedMethod));
        
        return true;
    }
    
    return false;
}

bool HybridBitcoinMiner::performCrossSystemLearning()
{
    QMutexLocker locker(&m_learningMutex);
    
    if (!m_tripleConfig.enableCrossLearning) {
        return false;
    }
    
    qDebug() << "[TripleSystem] Apprentissage croisé entre systèmes...";
    
    bool improvement = false;
    
    // Partage des patterns réussis entre systèmes
    if (shareSuccessfulPatterns()) {
        improvement = true;
    }
    
    // Formation du réseau à partir des succès MEA
    for (const auto& prediction : m_tripleHistory) {
        if (prediction.isValidated && prediction.wasSuccessful) {
            if (prediction.actualSuccessMethod == MiningMethod::RealMEANeurons ||
                prediction.selectedMethod == MiningMethod::RealMEANeurons) {
                
                if (trainNetworkFromMEASuccess(prediction)) {
                    improvement = true;
                }
            } else if (prediction.actualSuccessMethod == MiningMethod::BiologicalNetwork ||
                      prediction.selectedMethod == MiningMethod::BiologicalNetwork) {
                
                if (trainMEAFromNetworkSuccess(prediction)) {
                    improvement = true;
                }
            }
        }
    }
    
    if (improvement) {
        emit crossLearningCompleted(true, 0.05); // 5% d'amélioration estimée
    }
    
    qDebug() << QString("[TripleSystem] Apprentissage croisé terminé - Amélioration: %1")
                .arg(improvement ? "OUI" : "NON");
    
    return improvement;
}

void HybridBitcoinMiner::optimizeSystemWeights()
{
    if (!m_tripleConfig.enableDynamicWeighting) {
        return;
    }
    
    // Calcul des efficacités actuelles
    double sha256Efficiency = calculateSystemEfficiency(MiningMethod::TraditionalSHA256);
    double networkEfficiency = calculateSystemEfficiency(MiningMethod::BiologicalNetwork);
    double meaEfficiency = calculateSystemEfficiency(MiningMethod::RealMEANeurons);
    
    double totalEfficiency = sha256Efficiency + networkEfficiency + meaEfficiency;
    
    if (totalEfficiency < 0.001) {
        return; // Pas assez de données
    }
    
    // Nouveaux poids basés sur l'efficacité
    double newSha256Weight = sha256Efficiency / totalEfficiency;
    double newNetworkWeight = networkEfficiency / totalEfficiency;
    double newMeaWeight = meaEfficiency / totalEfficiency;
    
    // Application avec facteur d'adaptation
    double adaptationFactor = m_tripleConfig.adaptationRate;
    
    m_tripleConfig.sha256Weight = m_tripleConfig.sha256Weight * (1.0 - adaptationFactor) + 
                                 newSha256Weight * adaptationFactor;
    m_tripleConfig.networkWeight = m_tripleConfig.networkWeight * (1.0 - adaptationFactor) + 
                                  newNetworkWeight * adaptationFactor;
    m_tripleConfig.meaWeight = m_tripleConfig.meaWeight * (1.0 - adaptationFactor) + 
                              newMeaWeight * adaptationFactor;
    
    // Normalisation pour garantir somme = 1.0
    double weightSum = m_tripleConfig.sha256Weight + m_tripleConfig.networkWeight + m_tripleConfig.meaWeight;
    m_tripleConfig.sha256Weight /= weightSum;
    m_tripleConfig.networkWeight /= weightSum;
    m_tripleConfig.meaWeight /= weightSum;
    
    emit systemWeightsUpdated(m_tripleConfig.sha256Weight, 
                             m_tripleConfig.networkWeight, 
                             m_tripleConfig.meaWeight);
    
    qDebug() << QString("[TripleSystem] Poids optimisés - SHA256: %1, Network: %2, MEA: %3")
                .arg(m_tripleConfig.sha256Weight, 0, 'f', 3)
                .arg(m_tripleConfig.networkWeight, 0, 'f', 3)
                .arg(m_tripleConfig.meaWeight, 0, 'f', 3);
}

// Méthodes utilitaires système triple

QVector<double> HybridBitcoinMiner::blockHeaderToNetworkInput(const QString& blockHeader)
{
    QVector<double> input(60, 0.0); // 60 entrées pour compatibilité MEA
    
    QByteArray headerBytes = blockHeader.toUtf8();
    
    // Conversion du header en vecteur d'entrée normalisé
    for (int i = 0; i < std::min(headerBytes.size(), input.size()); ++i) {
        input[i] = (static_cast<uint8_t>(headerBytes[i]) / 255.0) * 2.0 - 1.0; // [-1, 1]
    }
    
    return input;
}

double HybridBitcoinMiner::calculateSystemEfficiency(MiningMethod method)
{
    if (m_tripleHistory.size() < 10) {
        return 0.5; // Valeur par défaut
    }
    
    uint64_t totalAttempts = 0;
    uint64_t successfulAttempts = 0;
    
    // Calcul basé sur l'historique récent (100 dernières prédictions)
    int startIndex = std::max(0, static_cast<int>(m_tripleHistory.size()) - 100);
    
    for (int i = startIndex; i < static_cast<int>(m_tripleHistory.size()); ++i) {
        const auto& prediction = m_tripleHistory[i];
        
        if (!prediction.isValidated) continue;
        
        bool wasSelectedMethod = (prediction.selectedMethod == method);
        bool wasSuccessfulMethod = (prediction.actualSuccessMethod == method);
        
        if (wasSelectedMethod) {
            totalAttempts++;
            if (prediction.wasSuccessful || wasSuccessfulMethod) {
                successfulAttempts++;
            }
        }
    }
    
    if (totalAttempts == 0) {
        return 0.5; // Valeur neutre
    }
    
    return static_cast<double>(successfulAttempts) / totalAttempts;
}

void HybridBitcoinMiner::updateSuccessCounters(MiningMethod method, bool wasSuccessful)
{
    if (wasSuccessful) {
        switch (method) {
            case MiningMethod::TraditionalSHA256:
                m_sha256Successes++;
                break;
            case MiningMethod::BiologicalNetwork:
                m_networkSuccesses++;
                break;
            case MiningMethod::RealMEANeurons:
                m_meaSuccesses++;
                break;
            case MiningMethod::HybridFusion:
                m_fusionSuccesses++;
                break;
        }
    }
}

void HybridBitcoinMiner::validateAndLearnFromResult(const TripleSystemPrediction& prediction, bool wasSuccessful)
{
    // Apprentissage spécifique du système MEA
    if (m_realMeaInterface && (prediction.selectedMethod == MiningMethod::RealMEANeurons ||
                               prediction.actualSuccessMethod == MiningMethod::RealMEANeurons)) {
        
        BioMining::Bio::NeuralBitcoinResponse meaResponse;
        meaResponse.predictedNonce = prediction.meaNonce;
        meaResponse.confidence = prediction.meaConfidence;
        meaResponse.responseTime = prediction.meaTime;
        
        m_realMeaInterface->applyReinforcementLearning(meaResponse, wasSuccessful);
    }
    
    // Apprentissage du réseau biologique
    if (m_biologicalNetwork && (prediction.selectedMethod == MiningMethod::BiologicalNetwork ||
                                prediction.actualSuccessMethod == MiningMethod::BiologicalNetwork)) {
        
        // Le réseau biologique apprend de ses succès/échecs
        // (Implémentation déjà existante dans les méthodes du réseau)
    }
    
    // Mise à jour des métriques globales
    updateSystemPerformanceMetrics();
}

bool HybridBitcoinMiner::shareSuccessfulPatterns()
{
    // Partage des patterns entre les systèmes biologiques
    
    bool patternsShared = false;
    
    for (const auto& prediction : m_tripleHistory) {
        if (prediction.isValidated && prediction.wasSuccessful) {
            
            // Si MEA a réussi, enseigner au réseau
            if (prediction.actualSuccessMethod == MiningMethod::RealMEANeurons && m_biologicalNetwork) {
                
                QVector<double> successPattern = blockHeaderToNetworkInput(QString::number(prediction.meaNonce));
                QVector<double> targetOutput(32);
                
                // Conversion du nonce réussi en target
                for (int i = 0; i < 32; ++i) {
                    targetOutput[i] = ((prediction.meaNonce >> i) & 1) ? 1.0 : 0.0;
                }
                
                m_biologicalNetwork->forwardPropagation(successPattern);
                m_biologicalNetwork->backPropagation(targetOutput);
                m_biologicalNetwork->updateWeights();
                
                patternsShared = true;
            }
            
            // Si réseau a réussi, enseigner au MEA
            if (prediction.actualSuccessMethod == MiningMethod::BiologicalNetwork && m_realMeaInterface) {
                
                BioMining::Bio::BitcoinLearningPattern meaPattern;
                meaPattern.blockHash = QString::number(prediction.networkNonce);
                meaPattern.targetNonce = prediction.networkNonce;
                meaPattern.difficulty = 1000000.0;
                meaPattern.timestamp = prediction.predictionTime;
                
                m_realMeaInterface->trainBitcoinPattern(meaPattern);
                
                patternsShared = true;
            }
        }
    }
    
    return patternsShared;
}

bool HybridBitcoinMiner::trainNetworkFromMEASuccess(const TripleSystemPrediction& successfulPrediction)
{
    if (!m_biologicalNetwork) {
        return false;
    }
    
    // Entraîner le réseau biologique avec les patterns réussis du MEA
    QVector<double> input = blockHeaderToNetworkInput(QString::number(successfulPrediction.meaNonce));
    QVector<double> target(32);
    
    for (int i = 0; i < 32; ++i) {
        target[i] = ((successfulPrediction.meaNonce >> i) & 1) ? 1.0 : 0.0;
    }
    
    try {
        m_biologicalNetwork->forwardPropagation(input);
        m_biologicalNetwork->backPropagation(target);
        m_biologicalNetwork->updateWeights();
        
        qDebug() << "[TripleSystem] Réseau entraîné depuis succès MEA";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "[TripleSystem] Erreur entraînement croisé MEA->Network:" << e.what();
        return false;
    }
}

bool HybridBitcoinMiner::trainMEAFromNetworkSuccess(const TripleSystemPrediction& successfulPrediction)
{
    if (!m_realMeaInterface) {
        return false;
    }
    
    // Entraîner le MEA avec les patterns réussis du réseau biologique
    try {
        BioMining::Bio::BitcoinLearningPattern pattern;
        pattern.blockHash = QString("network_success_%1").arg(successfulPrediction.networkNonce);
        pattern.targetNonce = successfulPrediction.networkNonce;
        pattern.difficulty = 1000000.0;
        pattern.timestamp = successfulPrediction.predictionTime;
        pattern.successRate = 1.0; // Succès avéré
        
        m_realMeaInterface->trainBitcoinPattern(pattern);
        
        qDebug() << "[TripleSystem] MEA entraîné depuis succès Network";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "[TripleSystem] Erreur entraînement croisé Network->MEA:" << e.what();
        return false;
    }
}

void HybridBitcoinMiner::updateSystemPerformanceMetrics()
{
    // Mise à jour des métriques de performance pour optimisation
    
    if (m_tripleHistory.size() < 10) {
        return;
    }
    
    // Calcul des métriques sur les 50 dernières prédictions
    int recentCount = std::min(50, static_cast<int>(m_tripleHistory.size()));
    int startIndex = m_tripleHistory.size() - recentCount;
    
    int sha256Success = 0, networkSuccess = 0, meaSuccess = 0, fusionSuccess = 0;
    int sha256Total = 0, networkTotal = 0, meaTotal = 0, fusionTotal = 0;
    
    for (int i = startIndex; i < static_cast<int>(m_tripleHistory.size()); ++i) {
        const auto& prediction = m_tripleHistory[i];
        
        if (!prediction.isValidated) continue;
        
        // Comptage par méthode sélectionnée
        switch (prediction.selectedMethod) {
            case MiningMethod::TraditionalSHA256:
                sha256Total++;
                if (prediction.wasSuccessful) sha256Success++;
                break;
            case MiningMethod::BiologicalNetwork:
                networkTotal++;
                if (prediction.wasSuccessful) networkSuccess++;
                break;
            case MiningMethod::RealMEANeurons:
                meaTotal++;
                if (prediction.wasSuccessful) meaSuccess++;
                break;
            case MiningMethod::HybridFusion:
                fusionTotal++;
                if (prediction.wasSuccessful) fusionSuccess++;
                break;
        }
    }
    
    // Calcul des taux de réussite
    double sha256Rate = (sha256Total > 0) ? (double)sha256Success / sha256Total : 0.0;
    double networkRate = (networkTotal > 0) ? (double)networkSuccess / networkTotal : 0.0;
    double meaRate = (meaTotal > 0) ? (double)meaSuccess / meaTotal : 0.0;
    double fusionRate = (fusionTotal > 0) ? (double)fusionSuccess / fusionTotal : 0.0;
    
    // Logs pour monitoring
    qDebug() << QString("[TripleSystem] Taux de réussite récents - SHA256: %1%, Network: %2%, MEA: %3%, Fusion: %4%")
                .arg(sha256Rate * 100, 0, 'f', 1)
                .arg(networkRate * 100, 0, 'f', 1)
                .arg(meaRate * 100, 0, 'f', 1)
                .arg(fusionRate * 100, 0, 'f', 1);
    
    // Déclencher optimisation si nécessaire
    if (recentCount >= 20) {
        optimizeSystemWeights();
    }
}

bool HybridBitcoinMiner::isTripleSystemReady() const
{
    return m_tripleSystemEnabled.load() && 
           m_traditionalMiner && 
           m_biologicalNetwork && 
           m_realMeaInterface;
}

// Classes de tâches parallèles

HybridMiningTask::HybridMiningTask(HybridBitcoinMiner* miner, const QString& blockHeader, uint32_t startNonce, uint32_t endNonce)
    : m_miner(miner)
    , m_blockHeader(blockHeader)
    , m_startNonce(startNonce)
    , m_endNonce(endNonce)
{
}

void HybridMiningTask::run()
{
    if (!m_miner) {
        return;
    }
    
    for (uint32_t nonce = m_startNonce; nonce <= m_endNonce; ++nonce) {
        QString blockHash;
        uint32_t resultNonce = nonce;
        
        if (m_miner->performHybridHash(m_blockHeader, resultNonce, blockHash)) {
            // Bloc trouvé !
            break;
        }
        
        // Vérification périodique de l'arrêt
        if (nonce % 1000 == 0 && !m_miner->isMining()) {
            break;
        }
    }
}

BiologicalLearningTask::BiologicalLearningTask(HybridBitcoinMiner* miner, const std::vector<BiologicalTrainingData>& trainingData)
    : m_miner(miner)
    , m_trainingData(trainingData)
{
}

void BiologicalLearningTask::run()
{
    if (!m_miner) {
        return;
    }
    
    // Exécution de l'apprentissage biologique en arrière-plan
    m_miner->performInitialLearning(m_trainingData);
}

// ============================================================================
// === BIO-ENTROPY MINING IMPLEMENTATION (Approach 3) ===
// ============================================================================

void HybridBitcoinMiner::setBioComputeMode(BioComputeMode mode)
{
    QMutexLocker locker(&m_stateMutex);
    
    if (m_bioComputeMode == mode) {
        return;
    }
    
    qDebug() << "[HybridBitcoinMiner] Changing bio-compute mode to:" << 
                (mode == BioComputeMode::RealMEA ? "RealMEA" : "SimulatedNetwork");
    
    m_bioComputeMode = mode;
    
    // Basculer l'interface
    if (mode == BioComputeMode::RealMEA) {
        m_bioCompute = m_realMeaAdapter;
        qDebug() << "[HybridBitcoinMiner] Using real biological cells (MEA)";
    } else {
        m_bioCompute = m_networkAdapter;
        qDebug() << "[HybridBitcoinMiner] Using simulated biological network";
    }
    
    // Réinitialiser le système
    initializeBioEntropy();
}

bool HybridBitcoinMiner::initializeBioEntropy()
{
    QMutexLocker locker(&m_stateMutex);
    
    if (!m_bioCompute) {
        qWarning() << "[HybridBitcoinMiner] Bio-compute interface not available";
        return false;
    }
    
    // Initialiser l'interface sélectionnée
    if (!m_bioCompute->initialize()) {
        qWarning() << "[HybridBitcoinMiner] Failed to initialize bio-compute";
        return false;
    }
    
    qDebug() << "[HybridBitcoinMiner] Bio-entropy system initialized successfully";
    qDebug() << "[HybridBitcoinMiner] Diagnostic:\n" << m_bioCompute->getDiagnosticInfo();
    
    return true;
}

uint32_t HybridBitcoinMiner::mineWithBioEntropy(const QString& blockHeader, uint64_t difficulty)
{
    qDebug() << "[HybridBitcoinMiner] Mining with bio-entropy, mode:" << 
                (m_bioComputeMode == BioComputeMode::RealMEA ? "RealMEA" : "SimulatedNetwork");
    
    if (!m_bioCompute || !m_bioCompute->isReady()) {
        qWarning() << "[HybridBitcoinMiner] Bio-compute system not ready";
        return 0;
    }
    
    // ÉTAPE 1: Extraire features du header
    auto features = m_entropyGenerator->extractHeaderFeatures(blockHeader, difficulty);
    
    // ÉTAPE 2: Générer pattern de stimulation
    auto stimulusPattern = m_entropyGenerator->featuresToStimulus(features, 3.0);
    
    // Convertir vers IBioComputeInterface::StimulusPattern
    BioMining::Bio::IBioComputeInterface::StimulusPattern bioStimulus;
    bioStimulus.amplitudes = stimulusPattern.amplitudes;
    bioStimulus.frequencies = stimulusPattern.frequencies;
    bioStimulus.durationMs = stimulusPattern.durationMs;
    
    // ÉTAPE 3: Stimuler le système biologique (MEA réel OU réseau simulé)
    auto bioResponse = m_bioCompute->stimulateAndCapture(bioStimulus);
    
    if (!bioResponse.isValid) {
        qWarning() << "[HybridBitcoinMiner] Invalid biological response";
        return 0;
    }
    
    qDebug() << QString("[HybridBitcoinMiner] Bio response received: strength=%1, quality=%2")
                .arg(bioResponse.responseStrength)
                .arg(bioResponse.signalQuality);
    
    // ÉTAPE 4: Générer seed d'entropie
    auto entropySeed = m_entropyGenerator->generateEntropySeed(bioResponse.rsignals, features);
    
    qDebug() << QString("[HybridBitcoinMiner] Entropy seed: 0x%1, confidence=%2")
                .arg(entropySeed.primarySeed, 16, 16, QChar('0'))
                .arg(entropySeed.confidence);
    
    // ÉTAPE 5: Générer points de départ intelligents
    int pointCount = 1000; // 1000 points de départ
    uint32_t windowSize = 4194304; // 4M nonces par point
    auto startingPoints = m_entropyGenerator->generateStartingPoints(entropySeed, pointCount, windowSize);
    
    qDebug() << QString("[HybridBitcoinMiner] %1 points generated (strategy: %2, coverage=%3%)")
                .arg(startingPoints.nonceStarts.size())
                .arg(startingPoints.strategy)
                .arg(startingPoints.expectedCoverage * 100, 0, 'f', 2);
    
    // ÉTAPE 6: Recherche parallèle GPU/CPU
    uint32_t foundNonce = 0;
    parallelSearchCandidates(blockHeader, difficulty, startingPoints.nonceStarts, windowSize, foundNonce);
    
    // ÉTAPE 7: Renforcement si succès
    if (foundNonce != 0) {
        double reward = 1.0;
        m_bioCompute->reinforcePattern(bioStimulus, foundNonce, reward);
        m_entropyGenerator->reinforceSuccessfulPattern(features, bioResponse.rsignals, foundNonce);
        
        qDebug() << "[HybridBitcoinMiner] Pattern reinforced successfully";
        
        // Mettre à jour les métriques
        m_metrics.successfulPredictions++;
        m_metrics.biologicalPredictions++;
    }
    
    return foundNonce;
}

bool HybridBitcoinMiner::validateNonceCandidate(const QString& blockHeader, uint64_t difficulty, uint32_t nonce)
{
    // Construire header complet avec nonce
    QString fullHeader = blockHeader + QString::number(nonce);
    
    // Double SHA-256
    QByteArray data = fullHeader.toUtf8();
    QByteArray hash1 = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    QByteArray hash2 = QCryptographicHash::hash(hash1, QCryptographicHash::Sha256);
    
    // Vérifier leading zeros selon difficulty
    QString hashHex = hash2.toHex();
    
    int requiredZeros = static_cast<int>(std::log2(difficulty));
    int actualZeros = 0;
    
    for (QChar c : hashHex) {
        if (c == '0') {
            actualZeros++;
        } else {
            break;
        }
    }
    
    return actualZeros >= requiredZeros;
}

void HybridBitcoinMiner::parallelSearchCandidates(const QString& blockHeader, 
                                                   uint64_t difficulty,
                                                   const QVector<uint32_t>& candidates,
                                                   uint32_t windowSize,
                                                   uint32_t& foundNonce)
{
    foundNonce = 0;
    std::atomic<bool> found(false);
    
    // Recherche parallèle avec OpenMP (ou QtConcurrent)
    #pragma omp parallel for
    for (int i = 0; i < candidates.size(); ++i) {
        if (found.load()) {
            continue; // Déjà trouvé, skip
        }
        
        uint32_t start = candidates[i];
        uint32_t end = start + windowSize;
        
        for (uint32_t nonce = start; nonce < end; ++nonce) {
            if (found.load()) {
                break;
            }
            
            if (validateNonceCandidate(blockHeader, difficulty, nonce)) {
                bool expected = false;
                if (found.compare_exchange_strong(expected, true)) {
                    foundNonce = nonce;
                    qDebug() << QString("[HybridBitcoinMiner] ✅ NONCE FOUND: 0x%1")
                                .arg(nonce, 8, 16, QChar('0'));
                }
                break;
            }
            
            // Mettre à jour métriques tous les 10000 hashs
            if (nonce % 10000 == 0) {
                m_metrics.totalHashes += 10000;
            }
        }
    }
}

QJsonObject HybridBitcoinMiner::getBioEntropyStats() const
{
    if (m_entropyGenerator) {
        return m_entropyGenerator->getEntropyStats();
    }
    return QJsonObject();
}

} // namespace HCrypto
} // namespace BioMining
