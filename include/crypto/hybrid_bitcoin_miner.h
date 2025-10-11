#ifndef HYBRID_BITCOIN_MINER_H
#define HYBRID_BITCOIN_MINER_H

#include "bio/biological_network.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QDateTime>
#include <QHash>
#include <QVector>
#include <QAtomicInt>
#include <QAtomicPointer>
#include <QWaitCondition>
#include <QThreadPool>
#include <QRunnable>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <cstdint>
#include <memory>
#include <atomic>
#include <vector>
#include <array>
#include <chrono>
#include <functional>
#include <algorithm>
#include <cmath>

#include "bitcoin_miner.h"
#include "../bio/biological_network.h"
#include "../bio/mea_interface.h"
#include "../bio/real_mea_interface.h"

// Forward declarations to avoid circular dependencies
namespace BioMining {
namespace HCrypto {
    
struct MiningConfig {
    uint64_t difficulty = 1;
    uint32_t threads = 1; 
    bool useGPU = false;
    double targetEfficiency = 0.5;
    double targetDifficulty = 1.0;
};

}
namespace HBio {
    enum class NetworkLearningState {
        Untrained,
        InitialLearning, 
        Trained,
        Retraining,
        Optimizing
    };
}
}

namespace BioMining {
namespace HCrypto {

/**
 * @brief Structure représentant les paramètres d'apprentissage du réseau biologique
 */
struct BiologicalLearningParams {
    double initialLearningRate = 0.01;
    double retroLearningRate = 0.005;
    double decayRate = 0.995;
    double momentumFactor = 0.9;
    double adaptationThreshold = 0.1;
    int maxIterations = 10000;
    int retroIterations = 5000;
    bool enablePlasticity = true;
    bool enableAdaptation = true;
    
    BiologicalLearningParams() = default;
    BiologicalLearningParams(const BiologicalLearningParams& other) = default;
    BiologicalLearningParams& operator=(const BiologicalLearningParams& other) = default;
};

/**
 * @brief Structure pour les métriques de performance du mining hybride
 */
struct HybridMiningMetrics {
    std::atomic<uint64_t> totalHashes{0};
    std::atomic<uint64_t> biologicalPredictions{0};
    std::atomic<uint64_t> successfulPredictions{0};
    std::atomic<uint64_t> traditionalHashes{0};
    std::atomic<double> biologicalAccuracy{0.0};
    std::atomic<double> hybridHashRate{0.0};
    std::atomic<double> energyEfficiency{0.0};
    std::atomic<double> adaptationScore{0.0};
    
    QDateTime startTime;
    QMutex metricsMutex;
    
    HybridMiningMetrics() {
        startTime = QDateTime::currentDateTime();
    }

    // Constructeurs pour gérer les atomics
    HybridMiningMetrics(const HybridMiningMetrics& other) : 
        totalHashes(other.totalHashes.load()),
        biologicalPredictions(other.biologicalPredictions.load()),
        successfulPredictions(other.successfulPredictions.load()),
        traditionalHashes(other.traditionalHashes.load()),
        biologicalAccuracy(other.biologicalAccuracy.load()),
        hybridHashRate(other.hybridHashRate.load()),
        energyEfficiency(other.energyEfficiency.load()),
        adaptationScore(other.adaptationScore.load()),
        startTime(other.startTime) {}
    
    HybridMiningMetrics& operator=(const HybridMiningMetrics& other) {
        if (this != &other) {
            totalHashes = other.totalHashes.load();
            biologicalPredictions = other.biologicalPredictions.load();
            successfulPredictions = other.successfulPredictions.load();
            traditionalHashes = other.traditionalHashes.load();
            biologicalAccuracy = other.biologicalAccuracy.load();
            hybridHashRate = other.hybridHashRate.load();
            energyEfficiency = other.energyEfficiency.load();
            adaptationScore = other.adaptationScore.load();
            startTime = other.startTime;
        }
        return *this;
    }

    
    void reset() {
        QMutexLocker locker(&metricsMutex);
        totalHashes = 0;
        biologicalPredictions = 0;
        successfulPredictions = 0;
        traditionalHashes = 0;
        biologicalAccuracy = 0.0;
        hybridHashRate = 0.0;
        energyEfficiency = 0.0;
        adaptationScore = 0.0;
        startTime = QDateTime::currentDateTime();
    }
};

/**
 * @brief Structure pour les données de training du réseau biologique
 */
struct BiologicalTrainingData {
    std::vector<double> inputPattern;
    std::vector<double> targetOutput;
    uint32_t expectedNonce;
    double difficulty;
    QDateTime timestamp;
    double validationScore;
    bool isValidated;
    
    BiologicalTrainingData() : expectedNonce(0), difficulty(0.0), validationScore(0.0), isValidated(false) {}
};

/**
 * @brief États de l'apprentissage hybride
 */
enum class HybridLearningState {
    Uninitialized,
    InitialLearning,
    ActiveMining,
    RetroLearning,
    Optimizing,
    Adapting,
    Error
};

/**
 * @brief Structure pour les prédictions de nonce biologiques
 */
struct BiologicalNoncePrediction {
    uint32_t predictedNonce;
    double confidence;
    double biologicalEntropy;
    std::vector<double> networkOutput;
    QDateTime predictionTime;
    bool isValidated;
    
    BiologicalNoncePrediction() : predictedNonce(0), confidence(0.0), biologicalEntropy(0.0), isValidated(false) {}
};

/**
 * @brief Énumération des méthodes de mining disponibles
 */
enum class MiningMethod {
    TraditionalSHA256,      // Mining SHA-256 classique
    BiologicalNetwork,      // Réseau biologique
    RealMEANeurons,        // Neurones biologiques via MEA
    HybridFusion           // Fusion intelligente des trois
};

/**
 * @brief Structure de prédiction unifiée pour les trois systèmes
 */
struct TripleSystemPrediction {
    // Prédictions individuelles
    uint32_t sha256Nonce = 0;
    uint32_t networkNonce = 0;
    uint32_t meaNonce = 0;
    
    // Confidences individuelles
    double sha256Confidence = 0.0;
    double networkConfidence = 0.0;
    double meaConfidence = 0.0;
    
    // Prédiction fusionnée
    uint32_t fusedNonce = 0;
    double fusedConfidence = 0.0;
    MiningMethod selectedMethod = MiningMethod::TraditionalSHA256;
    
    // Métriques de performance
    qint64 predictionTime = 0;
    qint64 sha256Time = 0;
    qint64 networkTime = 0;
    qint64 meaTime = 0;
    
    // Validation
    bool isValidated = false;
    bool wasSuccessful = false;
    MiningMethod actualSuccessMethod = MiningMethod::TraditionalSHA256;
};

/**
 * @brief Configuration pour optimisation triple
 */
struct TripleOptimizationConfig {
    // Poids dynamiques des systèmes (somme = 1.0)
    double sha256Weight = 0.4;
    double networkWeight = 0.3;
    double meaWeight = 0.3;
    
    // Seuils de confiance minimum
    double minSha256Confidence = 0.1;
    double minNetworkConfidence = 0.5;
    double minMeaConfidence = 0.4;
    
    // Paramètres d'adaptation
    double adaptationRate = 0.02;
    double convergenceThreshold = 0.05;
    bool enableDynamicWeighting = true;
    
    // Critères de sélection
    bool preferHighConfidence = true;
    bool enableCrossLearning = true;
    bool enablePerformanceTracking = true;
};

/**
 * @brief Classe principale pour le mining Bitcoin hybride
 * 
 * Cette classe combine la méthode SHA-256 traditionnelle avec un réseau biologique
 * connecté au MEA pour une recherche optimisée de nonce.
 */
class HybridBitcoinMiner : public QObject
{
    Q_OBJECT

public:
    explicit HybridBitcoinMiner(QObject* parent = nullptr);
    virtual ~HybridBitcoinMiner();

    // Configuration et initialisation
    bool initialize();
    bool configureBiologicalNetwork(const BioMining::HCrypto::BiologicalLearningParams& params);
    bool connectToMEA(std::shared_ptr<BioMining::Bio::MEAInterface> meaInterface);
    bool connectToRealMEA(std::shared_ptr<BioMining::Bio::RealMEAInterface> realMeaInterface);
    void setMiningParameters(const BioMining::HCrypto::MiningConfig& config);
    
    // === SYSTÈME TRIPLE HYBRIDE ===
    
    /**
     * @brief Initialise le système d'optimisation triple
     */
    bool initializeTripleSystem(const TripleOptimizationConfig& config);
    
    /**
     * @brief Configuration des trois systèmes en parallèle
     */
    bool configureTripleSystemLearning();
    
    /**
     * @brief Prédiction parallèle avec les trois systèmes
     */
    TripleSystemPrediction predictNonceTriple(const QString& blockHeader);
    
    /**
     * @brief Fusion intelligente des prédictions
     */
    uint32_t fuseTriplePredictions(const TripleSystemPrediction& predictions);
    
    /**
     * @brief Sélection dynamique de la meilleure méthode
     */
    MiningMethod selectOptimalMethod(const TripleSystemPrediction& predictions);
    
    /**
     * @brief Validation croisée des trois systèmes
     */
    bool validateTriplePrediction(const TripleSystemPrediction& prediction, bool wasSuccessful);
    
    /**
     * @brief Apprentissage croisé entre systèmes
     */
    bool performCrossSystemLearning();
    
    /**
     * @brief Optimisation dynamique des poids
     */
    void optimizeSystemWeights();
    
    /**
     * @brief Mining avec optimisation triple
     */
    bool performTripleOptimizedMining(const QString& blockHeader, uint32_t& nonce, QString& blockHash);
    
    // Contrôle du mining
    bool startHybridMining();
    void stopHybridMining();
    void pauseMining();
    void resumeMining();
    bool isMining() const;
    
    // Apprentissage biologique
    bool initializeBiologicalLearning();
    bool performInitialLearning(const std::vector<BioMining::HCrypto::BiologicalTrainingData>& trainingData);
    bool performRetroLearning();
    void enableAdaptiveLearning(bool enable);
    
    // Gestion des prédictions
    BiologicalNoncePrediction predictNonce(const QString& blockHeader);
    void validatePrediction(const BiologicalNoncePrediction& prediction, bool wasSuccessful);
    double getBiologicalAccuracy() const;
    
    // Métriques et monitoring
    HybridMiningMetrics getMetrics() const;
    BioMining::HCrypto::HybridLearningState getLearningState() const;
    double getNetworkAdaptationScore() const;
    QString getStatusReport() const;
    
    // Configuration avancée
    void setLearningParameters(const BioMining::HCrypto::BiologicalLearningParams& params);
    BioMining::HCrypto::BiologicalLearningParams getLearningParameters() const;
    void setBiologicalWeight(double weight); // 0.0 = SHA-256 pur, 1.0 = biologique pur
    double getBiologicalWeight() const;

public slots:
    void onMEADataReceived(const std::vector<double>& electrodeData);
    void onNetworkStateChanged(BioMining::HBio::NetworkLearningState state);
    void onMiningDifficultyChanged(double newDifficulty);
    void updateLearningMetrics();

signals:
    void hybridMiningStarted();
    void hybridMiningStopped();
    void blockFound(const QString& blockHash, uint32_t nonce, double biologicalContribution);
    void biologicalPredictionMade(const BiologicalNoncePrediction& prediction);
    void learningStateChanged(BioMining::HCrypto::HybridLearningState newState);
    void metricsUpdated(const HybridMiningMetrics& metrics);
    void adaptationScoreChanged(double score);
    void errorOccurred(const QString& error);
    
    // Nouveaux signaux pour système triple
    void triplePredictionReady(const TripleSystemPrediction& prediction);
    void systemWeightsUpdated(double sha256Weight, double networkWeight, double meaWeight);
    void optimalMethodSelected(MiningMethod method, double confidence);
    void crossLearningCompleted(bool success, double improvement);
    void tripleSystemOptimized(double overallImprovement);

private slots:
    void performHybridMiningCycle();
    void processNetworkLearning();
    void updateAdaptationParameters();
    void validateBiologicalPerformance();

public:
    // Méthodes de mining hybride
    bool performHybridHash(const QString& blockHeader, uint32_t& nonce, QString& blockHash);
private:
    // Méthodes de mining hybride
    uint32_t generateBiologicalNonce(const std::vector<double>& meaSignals);
    bool validateNonceWithSHA256(const QString& blockHeader, uint32_t nonce, const QString& targetHash);
    
    // Méthodes d'apprentissage
    bool trainNetworkWithPattern(const std::vector<double>& input, const std::vector<double>& target);
    void performForwardPropagation(const std::vector<double>& input);
    void performBackwardPropagation(const std::vector<double>& target);
    void updateSynapticWeights();
    
    // Méthodes d'adaptation
    void adaptLearningRate();
    void adjustBiologicalParameters();
    bool detectPatternConvergence();
    void reinforceSuccessfulPatterns();
    
    // Utilitaires
    std::vector<double> preprocessMEASignals(const std::vector<double>& rawSignals);
    std::vector<double> extractNetworkFeatures();
    double calculateBiologicalEntropy(const std::vector<double>& signalData);
    double evaluatePredictionConfidence(const std::vector<double>& networkOutput);
    
    // Threading et synchronisation
    void setupMiningThreads();
    void cleanupMiningThreads();
    
    // Méthodes utilitaires supplémentaires (manquantes)
    void generateInitialTrainingData(std::vector<BioMining::HCrypto::BiologicalTrainingData>& trainingData);
    void prepareRetroLearningData(std::vector<BioMining::HCrypto::BiologicalTrainingData>& retroData);
    std::vector<double> convertNonceToOutput(uint32_t nonce);
    double calculatePredictionError(const std::vector<double>& predicted, const std::vector<double>& target);
    void updateBiologicalAccuracy();
    void updateHashRate();
    void updateEnergyEfficiency();
    double calculateRecentAccuracy();
    double calculateNetworkComplexity();
    double calculateAdaptationRate();
    double calculateBiologicalContribution();
    QString generateTestBlockHeader();
    QString calculateSHA256Hash(const QString& data, uint32_t nonce);
    
    // === MÉTHODES SYSTÈME TRIPLE ===
    
    // Prédictions individuelles par système
    uint32_t predictWithSHA256(const QString& blockHeader, double& confidence, qint64& processTime);
    uint32_t predictWithBiologicalNetwork(const QString& blockHeader, double& confidence, qint64& processTime);
    uint32_t predictWithRealMEA(const QString& blockHeader, double& confidence, qint64& processTime);
    
    // Algorithmes de fusion
    uint32_t weightedAverageFusion(const TripleSystemPrediction& predictions);
    uint32_t confidenceBasedFusion(const TripleSystemPrediction& predictions);
    uint32_t adaptiveFusion(const TripleSystemPrediction& predictions);
    
    // Apprentissage croisé
    bool trainNetworkFromMEASuccess(const TripleSystemPrediction& successfulPrediction);
    bool trainMEAFromNetworkSuccess(const TripleSystemPrediction& successfulPrediction);
    bool shareSuccessfulPatterns();
    
    // Optimisation dynamique  
    void updateSystemPerformanceMetrics();
    double calculateSystemEfficiency(MiningMethod method);
    void adaptWeightsBasedOnPerformance();
    bool detectPerformancePattern();
    
    // Validation et apprentissage
    void validateAndLearnFromResult(const TripleSystemPrediction& prediction, bool wasSuccessful);
    void updateSuccessCounters(MiningMethod method, bool wasSuccessful);
    double calculateTripleSystemAccuracy();
    
    // Utilitaires système triple
    QString blockHeaderToMEAPattern(const QString& blockHeader);
    QVector<double> blockHeaderToNetworkInput(const QString& blockHeader);
    bool isTripleSystemReady() const;
    
private:
    // Composants principaux - SYSTÈME TRIPLE
    std::unique_ptr<BioMining::Crypto::BitcoinMiner> m_traditionalMiner;
    std::unique_ptr<BioMining::Network::BiologicalNetwork> m_biologicalNetwork;
    std::shared_ptr<BioMining::Bio::MEAInterface> m_meaInterface;
    std::shared_ptr<BioMining::Bio::RealMEAInterface> m_realMeaInterface;
    
    // Configuration
    BioMining::HCrypto::BiologicalLearningParams m_learningParams;
    BioMining::HCrypto::MiningConfig m_miningConfig;
    double m_biologicalWeight;
    
    // Configuration système triple
    TripleOptimizationConfig m_tripleConfig;
    std::atomic<bool> m_tripleSystemEnabled;
    
    // Historique des prédictions triples
    std::vector<TripleSystemPrediction> m_tripleHistory;
    
    // Métriques de performance par système
    std::atomic<uint64_t> m_sha256Successes;
    std::atomic<uint64_t> m_networkSuccesses; 
    std::atomic<uint64_t> m_meaSuccesses;
    std::atomic<uint64_t> m_fusionSuccesses;
    
    // État du système
    std::atomic<bool> m_isInitialized;
    std::atomic<bool> m_isMining;
    std::atomic<bool> m_isPaused;
    std::atomic<BioMining::HCrypto::HybridLearningState> m_learningState;
    
    // Données d'apprentissage
    std::vector<BioMining::HCrypto::BiologicalTrainingData> m_trainingHistory;
    std::vector<BiologicalNoncePrediction> m_predictionHistory;
    QHash<QString, double> m_patternMemory;
    
    // Métriques et performance
    HybridMiningMetrics m_metrics;
    std::atomic<double> m_adaptationScore;
    
    // Threading
    QTimer* m_miningTimer;
    QTimer* m_learningTimer;
    QTimer* m_metricsTimer;
    QThreadPool* m_threadPool;
    
    // Synchronisation
    mutable QMutex m_stateMutex;
    mutable QMutex m_dataMutex;
    mutable QMutex m_learningMutex;
    QWaitCondition m_miningCondition;
    
    // Constantes
    static constexpr int MAX_TRAINING_HISTORY = 10000;
    static constexpr int MAX_PREDICTION_HISTORY = 5000;
    static constexpr double MIN_BIOLOGICAL_WEIGHT = 0.0;
    static constexpr double MAX_BIOLOGICAL_WEIGHT = 1.0;
    static constexpr double DEFAULT_BIOLOGICAL_WEIGHT = 0.5;
    static constexpr int MINING_CYCLE_MS = 10;
    static constexpr int LEARNING_CYCLE_MS = 100;
    static constexpr int METRICS_UPDATE_MS = 1000;
};

/**
 * @brief Classe pour les tâches de mining hybride en parallèle
 */
class HybridMiningTask : public QRunnable
{
public:
    explicit HybridMiningTask(HybridBitcoinMiner* miner, const QString& blockHeader, uint32_t startNonce, uint32_t endNonce);
    void run() override;

private:
    HybridBitcoinMiner* m_miner;
    QString m_blockHeader;
    uint32_t m_startNonce;
    uint32_t m_endNonce;
};

/**
 * @brief Classe pour l'apprentissage biologique asynchrone
 */
class BiologicalLearningTask : public QRunnable
{
public:
    explicit BiologicalLearningTask(HybridBitcoinMiner* miner, const std::vector<BioMining::HCrypto::BiologicalTrainingData>& trainingData);
    void run() override;

private:
    HybridBitcoinMiner* m_miner;
    std::vector<BioMining::HCrypto::BiologicalTrainingData> m_trainingData;
};

} // namespace HCrypto
} // namespace BioMining

Q_DECLARE_METATYPE(BioMining::HCrypto::HybridLearningState)
Q_DECLARE_METATYPE(BioMining::HCrypto::HybridMiningMetrics)
Q_DECLARE_METATYPE(BioMining::HCrypto::BiologicalNoncePrediction)
Q_DECLARE_METATYPE(BioMining::HCrypto::MiningMethod)
Q_DECLARE_METATYPE(BioMining::HCrypto::TripleSystemPrediction)
Q_DECLARE_METATYPE(BioMining::HCrypto::TripleOptimizationConfig)

#endif // HYBRID_BITCOIN_MINER_H
