#ifndef BIOLOGICAL_NETWORK_H
#define BIOLOGICAL_NETWORK_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <functional>

/**
 * @brief Réseau biologique intelligent pour l'optimisation de recherche de nonce
 * 
 * Ce réseau utilise des cellules biologiques connectées au MEA pour créer un système
 * d'apprentissage adaptatif capable d'optimiser la recherche de nonce Bitcoin.
 * 
 * Fonctionnalités:
 * - Apprentissage initial par stimulation contrôlée
 * - Rétro-apprentissage basé sur les succès/échecs de mining
 * - Prédiction de zones prometteuses dans l'espace de nonce
 * - Adaptation dynamique aux patterns de difficultés Bitcoin
 */
class BiologicalNetwork : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief État de formation/apprentissage du réseau
     */
    enum class LearningState {
        Untrained,      // Non entrainé
        InitialLearning, // Apprentissage initial en cours
        Trained,        // Entrainé et opérationnel
        Retraining,     // Rétro-apprentissage en cours
        Optimizing      // Optimisation continue
    };

    /**
     * @brief Configuration du réseau biologique
     */
    struct NetworkConfig {
        int neuronCount = 60;           // Nombre de neurones (électrodes MEA)
        double learningRate = 0.01;     // Taux d'apprentissage
        double stimulationThreshold = 0.5; // Seuil de stimulation
        double adaptationRate = 0.1;    // Vitesse d'adaptation
        int memoryDepth = 1000;        // Profondeur mémoire des patterns
        bool useReinforcementLearning = true;
        
        // Configuration complète pour compatibilité
        int inputSize = 60;         // Nombre d'électrodes MEA
        int outputSize = 32;        // Bits de sortie pour nonce
        QVector<int> hiddenLayers = QVector<int>{128, 64, 32}; // Architecture des couches cachées
        bool enablePlasticity = true;  // Plasticité synaptique
        bool enableAdaptation = true;  // Adaptation dynamique
        double momentum = 0.9;         // Facteur de momentum
        double decayRate = 0.995;      // Taux de décroissance
        double adaptiveThreshold = 0.1; // Seuil adaptatif
        int maxEpochs = 10000;         // Nombre maximum d'époques
    };

    /**
     * @brief Résultat de prédiction de nonce
     */
    struct NoncePredicition {
        uint64_t suggestedNonce;        // Nonce suggéré par le réseau
        double confidence;              // Confiance de la prédiction (0-1)
        QVector<uint64_t> candidateRange; // Plage de nonces candidats
        double expectedEfficiency;      // Efficacité attendue
        QString reasoning;              // Explication du choix
    };

    /**
     * @brief Données d'apprentissage
     */
    struct LearningData {
        QVector<double> inputSignals;   // Signaux d'entrée MEA
        uint64_t targetNonce;          // Nonce cible
        QString blockHeader;           // En-tête du bloc
        uint64_t difficulty;           // Difficulté courante
        bool wasSuccessful;            // Succès du mining
        int attempts;                  // Nombre de tentatives
        double computeTime;            // Temps de calcul
        QDateTime timestamp;           // Horodatage
    };
    
public:
    // Méthodes rendues publiques pour accès externe
    bool initialize();
    bool configureNetwork(const NetworkConfig &config);
    bool initializeLearning(const NetworkConfig &config);
    void setAdaptiveLearning(bool enable);
    void updateInputSignals(const QVector<double> &signals);
    void updateWeights();
    QVector<double> getOutputValues();
    void forwardPropagation(const QVector<double> &inputs);
    void backPropagation(const QVector<double> &targets);

};

    explicit BiologicalNetwork(QObject *parent = nullptr);
    ~BiologicalNetwork();

    // Configuration
    void setNetworkConfig(const NetworkConfig &config);
    NetworkConfig getNetworkConfig() const;
    
    // État du réseau
    LearningState getLearningState() const;
    double getTrainingProgress() const;
    int getTrainingEpochs() const;
    
    // Apprentissage initial
    bool startInitialLearning(int trainingCycles = 100);
    void stopLearning();
    bool isLearningComplete() const;
    
    // Prédiction de nonce
    NoncePredicition predictOptimalNonce(const QString &blockHeader, 
                                        uint64_t difficulty,
                                        const QVector<double> &currentSignals);
    
    // Rétro-apprentissage
    void addLearningExample(const LearningData &data);
    void performRetroLearning();
    void optimizeFromFeedback(bool success, const LearningData &context);
    
    // Persistance
    bool saveNetwork(const QString &filepath = "");
    bool loadNetwork(const QString &filepath = "");
    QJsonObject exportNetworkState() const;
    bool importNetworkState(const QJsonObject &state);
    
    // Diagnostic
    QString getNetworkDiagnostic() const;
    QVector<double> getLayerActivations(int layer) const;
    double getNetworkEfficiency() const;

signals:
    void learningStateChanged(LearningState newState);
    void trainingProgress(double percentage);
    void noncePredictionReady(const NoncePredicition &prediction);
    void learningCompleted(bool success);
    void networkOptimized(double newEfficiency);
    void diagnosticUpdate(const QString &info);

public slots:
    void onMEASignalsReceived(const QVector<double> &signalData);
    void onMiningResult(bool success, uint64_t nonce, int attempts);

private slots:
    void onLearningCycle();
    void onOptimizationTimer();

private:
    // Réseau biologique
    struct BiologicalNeuron {
        double activation;              // Niveau d'activation
        double threshold;               // Seuil d'activation
        QVector<double> weights;        // Poids des connexions
        QVector<double> dendrites;      // Entrées dendritiques
        double lastStimulation;         // Dernière stimulation
        bool isActive;                  // État actif/inactif
        int connectionCount;            // Nombre de connexions
        double adaptationFactor;        // Facteur d'adaptation
    };

    struct NetworkLayer {
        QVector<BiologicalNeuron> neurons;
        QString layerType;              // "input", "hidden", "output"
        double layerActivation;         // Activation globale de couche
        QVector<QVector<double>> synapses; // Connexions synaptiques
    };

    // Architecture du réseau
    void initializeNetwork();
    void createNetworkTopology();
    void establishSynapticConnections();
    
    // Fonctions d'activation biologique
    double sigmoidActivation(double input) const;
    double biologicalActivation(double input, double threshold) const;
    double adaptiveThreshold(const BiologicalNeuron &neuron) const;
    
    // Propagation des signaux
    void forwardPropagation(const QVector<double> &inputs);
    void backPropagation(const QVector<double> &targets);
    QVector<double> getNetworkOutput() const;
    
    // Apprentissage biologique
    void performLearningCycle(const QVector<double> &inputs, const QVector<double> &targets);
    void adjustSynapticWeights(double learningSignal);
    void stimulateNeuronGrowth();
    void pruneWeakConnections();
    
    // Optimisation de nonce
    uint64_t biologicalNonceSearch(const QString &blockHeader, uint64_t difficulty);
    QVector<uint64_t> generateNonceCandidates(const QVector<double> &networkOutput);
    double evaluateNoncePromise(uint64_t nonce, const QString &blockHeader);
    
    // Renforcement et adaptation
    void reinforceSuccessfulPaths(const LearningData &successData);
    void weakenFailurePaths(const LearningData &failureData);
    void adaptToNewDifficulty(uint64_t newDifficulty);
    
    // Mémoire et patterns
    void storePatternMemory(const LearningData &data);
    QVector<LearningData> retrieveSimilarPatterns(const QVector<double> &currentSignals);
    double calculatePatternSimilarity(const QVector<double> &a, const QVector<double> &b);
    
    // État du réseau
    NetworkConfig m_config;
    LearningState m_learningState;
    QVector<NetworkLayer> m_layers;
    
    // Apprentissage
    int m_currentEpoch;
    int m_totalEpochs;
    double m_trainingProgress;
    std::unique_ptr<QTimer> m_learningTimer;
    std::unique_ptr<QTimer> m_optimizationTimer;
    
    // Mémoire et historique
    QVector<LearningData> m_learningHistory;
    QVector<LearningData> m_patternMemory;
    QVector<double> m_lastMEASignals;
    
    // Métriques de performance
    double m_networkEfficiency;
    int m_successfulPredictions;
    int m_totalPredictions;
    double m_averageConfidence;
    
    // Thread safety
    mutable QMutex m_networkMutex;
    
    // Constantes biologiques
    static constexpr double DEFAULT_ACTIVATION_THRESHOLD = 0.6;
    static constexpr double SYNAPTIC_PLASTICITY_RATE = 0.05;
    static constexpr double NEURON_FATIGUE_FACTOR = 0.95;
    static constexpr double PATTERN_SIMILARITY_THRESHOLD = 0.8;
    static constexpr int MAX_PATTERN_MEMORY = 10000;
    static constexpr double MIN_CONNECTION_STRENGTH = 0.01;

    // Méthodes ajoutées pour corriger les erreurs de compilation
private:
    void generateTrainingData();
    void calculateNetworkEfficiency();  
    double calculateNetworkStability() const;
    double sigmoidDerivative(double sigmoidOutput) const;
    double estimateNonceEfficiency(uint64_t nonce, uint64_t difficulty);
    void adaptNetworkThresholds();
    
    // Méthodes ajoutées pour corriger les erreurs de compilation supplémentaires
    bool initialize();
    bool configureNetwork(const NetworkConfig &config);
    bool initializeLearning(const NetworkConfig &config);
    void setAdaptiveLearning(bool enable);
    void updateInputSignals(const QVector<double> &currentSignals);
    void updateWeights();
    QVector<double> getOutputValues();
    
    // Alias pour la compatibilité
    typedef NetworkConfig LearningConfig;

    // Note: adaptiveThreshold déjà déclaré ligne 161

};

#endif // BIOLOGICAL_NETWORK_H
