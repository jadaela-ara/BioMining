#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include "../include/crypto/hybrid_bitcoin_miner.h"
#include "../include/bio/biological_network.h"
#include "../include/bio/mea_interface.h"

/**
 * @brief Complete workflow example of the BioMining platform
 * 
 * This example demonstrates the full workflow:
 * 1. System initialization and configuration
 * 2. Biological network training and optimization
 * 3. Real-time MEA data integration
 * 4. Adaptive mining with biological feedback
 * 5. Performance monitoring and optimization
 * 6. Network state persistence
 */

class CompleteWorkflowExample : public QObject
{
    Q_OBJECT

private:
    BiologicalNetwork* m_network;
    MEAInterface* m_meaInterface;
    HybridBitcoinMiner* m_miner;
    QTimer* m_miningTimer;
    int m_miningRounds;
    
public:
    CompleteWorkflowExample(QObject *parent = nullptr) 
        : QObject(parent), m_network(nullptr), m_meaInterface(nullptr), 
          m_miner(nullptr), m_miningTimer(nullptr), m_miningRounds(0) {}

public slots:
    void run() {
        qDebug() << "=== BioMining Platform - Complete Workflow Example ===";

        initializeSystem();
        setupConnections();
        startTraining();
    }

private slots:
    void initializeSystem() {
        qDebug() << "\n1. System Initialization";
        qDebug() << "========================";

        // Initialize biological network
        qDebug() << "Initializing biological network...";
        m_network = new BiologicalNetwork(this);
        
        BiologicalNetwork::NetworkConfig networkConfig;
        networkConfig.neuronCount = 60;
        networkConfig.learningRate = 0.015;
        networkConfig.stimulationThreshold = 0.4;
        networkConfig.adaptationRate = 0.12;
        networkConfig.memoryDepth = 2000;
        networkConfig.useReinforcementLearning = true;
        
        m_network->setNetworkConfig(networkConfig);
        
        if (!m_network->initialize()) {
            qDebug() << "ERROR: Failed to initialize biological network";
            QCoreApplication::quit();
            return;
        }
        qDebug() << "✓ Biological network ready";

        // Initialize MEA interface
        qDebug() << "Initializing MEA interface...";
        m_meaInterface = new MEAInterface(this);
        
        MEAInterface::MEAConfig meaConfig;
        meaConfig.electrodeCount = 60;
        meaConfig.samplingRate = 25000.0;
        meaConfig.amplification = 1200.0;
        meaConfig.filterLowCut = 300.0;
        meaConfig.filterHighCut = 8000.0;
        meaConfig.thresholdDetection = true;
        
        if (!m_meaInterface->initialize(meaConfig)) {
            qDebug() << "ERROR: Failed to initialize MEA interface";
            QCoreApplication::quit();
            return;
        }
        qDebug() << "✓ MEA interface ready";

        // Initialize hybrid miner
        qDebug() << "Initializing hybrid Bitcoin miner...";
        m_miner = new HybridBitcoinMiner(this);
        
        if (!m_miner->initializeWithBiologicalNetwork(m_network)) {
            qDebug() << "ERROR: Failed to initialize hybrid miner";
            QCoreApplication::quit();
            return;
        }
        qDebug() << "✓ Hybrid miner ready";
        
        qDebug() << "System initialization complete!";
    }

    void setupConnections() {
        qDebug() << "\n2. Setting up data connections";
        qDebug() << "==============================";

        // Connect MEA to biological network
        connect(m_meaInterface, &MEAInterface::signalDataReceived,
                m_network, &BiologicalNetwork::onMEASignalsReceived);

        // Connect biological network state changes
        connect(m_network, &BiologicalNetwork::learningStateChanged,
                this, &CompleteWorkflowExample::onLearningStateChanged);
        
        connect(m_network, &BiologicalNetwork::trainingProgress,
                this, &CompleteWorkflowExample::onTrainingProgress);
        
        connect(m_network, &BiologicalNetwork::learningCompleted,
                this, &CompleteWorkflowExample::onLearningCompleted);

        // Connect mining results
        connect(m_miner, &HybridBitcoinMiner::blockMined,
                this, &CompleteWorkflowExample::onBlockMined);
        
        qDebug() << "✓ Data connections established";
    }

    void startTraining() {
        qDebug() << "\n3. Starting biological network training";
        qDebug() << "=======================================";
        
        if (!m_network->startInitialLearning(200)) {
            qDebug() << "ERROR: Failed to start training";
            QCoreApplication::quit();
            return;
        }
        
        qDebug() << "✓ Training started (200 cycles)";
    }

    void onLearningStateChanged(BiologicalNetwork::LearningState newState) {
        QString stateStr;
        switch(newState) {
            case BiologicalNetwork::LearningState::Untrained: stateStr = "Untrained"; break;
            case BiologicalNetwork::LearningState::InitialLearning: stateStr = "Initial Learning"; break;
            case BiologicalNetwork::LearningState::Trained: stateStr = "Trained"; break;
            case BiologicalNetwork::LearningState::Retraining: stateStr = "Retraining"; break;
            case BiologicalNetwork::LearningState::Optimizing: stateStr = "Optimizing"; break;
        }
        
        qDebug() << "Learning state changed to:" << stateStr;
    }

    void onTrainingProgress(double percentage) {
        static int lastReported = -1;
        int current = static_cast<int>(percentage);
        
        if (current != lastReported && current % 10 == 0) {
            qDebug() << "Training progress:" << percentage << "%";
            lastReported = current;
        }
    }

    void onLearningCompleted(bool success) {
        if (success) {
            qDebug() << "\n4. Training completed successfully!";
            qDebug() << "==================================";
            startAdaptiveMining();
        } else {
            qDebug() << "ERROR: Training failed";
            QCoreApplication::quit();
        }
    }

    void startAdaptiveMining() {
        qDebug() << "\n5. Starting adaptive mining operations";
        qDebug() << "======================================";
        
        // Start continuous MEA monitoring
        if (!m_meaInterface->startContinuousRecording()) {
            qDebug() << "WARNING: Could not start MEA recording (simulation mode)";
        }
        
        // Setup mining timer for regular mining attempts
        m_miningTimer = new QTimer(this);
        connect(m_miningTimer, &QTimer::timeout, this, &CompleteWorkflowExample::performMiningRound);
        m_miningTimer->start(5000);  // Mine every 5 seconds
        
        qDebug() << "✓ Adaptive mining started";
    }

    void performMiningRound() {
        m_miningRounds++;
        
        qDebug() << "\n--- Mining Round" << m_miningRounds << "---";
        
        // Generate test block header
        QString blockHeader = QString("00000000%1d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f")
                             .arg(m_miningRounds, 8, 16, QChar('0'));
        
        uint64_t difficulty = 486604799 + (m_miningRounds * 1000);
        
        qDebug() << "Block Header:" << blockHeader;
        qDebug() << "Difficulty:" << difficulty;
        
        // Start mining with biological assistance
        auto result = m_miner->mineBlock(blockHeader, difficulty);
        
        // Add learning example to biological network
        BiologicalNetwork::LearningData learningData;
        learningData.blockHeader = blockHeader;
        learningData.difficulty = difficulty;
        learningData.wasSuccessful = result.success;
        learningData.attempts = result.hashAttempts;
        learningData.computeTime = result.miningTime;
        learningData.targetNonce = result.nonce;
        learningData.timestamp = QDateTime::currentDateTime();
        
        m_network->addLearningExample(learningData);
        
        if (m_miningRounds >= 10) {
            finishWorkflow();
        }
    }

    void onBlockMined(const QString& blockHeader, uint64_t nonce, uint32_t attempts) {
        qDebug() << "✓ Block mined successfully!";
        qDebug() << "   Nonce:" << nonce;
        qDebug() << "   Attempts:" << attempts;
        
        // Trigger biological network optimization
        m_network->performRetroLearning();
    }

    void finishWorkflow() {
        qDebug() << "\n6. Completing workflow and saving state";
        qDebug() << "=======================================";
        
        // Stop mining
        if (m_miningTimer) {
            m_miningTimer->stop();
        }
        
        // Stop MEA recording
        m_meaInterface->stopRecording();
        
        // Save network state
        if (m_network->saveNetwork("biomining_trained_network.json")) {
            qDebug() << "✓ Network state saved to biomining_trained_network.json";
        }
        
        // Export network diagnostics
        QJsonObject networkState = m_network->exportNetworkState();
        QJsonDocument doc(networkState);
        
        qDebug() << "\n7. Final Performance Report";
        qDebug() << "===========================";
        qDebug() << "Mining rounds completed:" << m_miningRounds;
        qDebug() << "Network efficiency:" << m_network->getNetworkEfficiency() << "%";
        qDebug() << "Training epochs:" << m_network->getTrainingEpochs();
        qDebug() << "Final learning state:" << (int)m_network->getLearningState();
        
        QString diagnostic = m_network->getNetworkDiagnostic();
        qDebug() << "Network diagnostic:" << diagnostic;
        
        qDebug() << "\n=== Workflow completed successfully ===";
        
        // Cleanup and exit
        QTimer::singleShot(2000, QCoreApplication::instance(), &QCoreApplication::quit);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "BioMining Platform v1.0.0";
    qDebug() << "Complete Workflow Example";
    qDebug() << "Demonstrates full system integration and adaptive learning\n";

    CompleteWorkflowExample example;
    
    // Start the example after the event loop begins
    QTimer::singleShot(0, &example, &CompleteWorkflowExample::run);

    return app.exec();
}

#include "complete_workflow.moc"