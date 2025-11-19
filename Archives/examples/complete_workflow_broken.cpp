#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include "../include/crypto/hybrid_bitcoin_miner.h"
#include "../include/bio/biological_network.h"
#include "../include/bio/mea_interface.h"
#include "../include/crypto/bitcoin_miner.h" // Added for BitcoinMiner

namespace BioMining {
    enum class MiningMode {
        Hybrid,      // Utilize HybridBitcoinMiner
        Biological   // Utilize BitcoinMiner directly with MEA data
    };
}

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
    // BiologicalNetwork m_network; // HybridBitcoinMiner manages its own BiologicalNetwork
    std::shared_ptr<MEAInterface> m_meaInterface;
    std::unique_ptr<HybridBitcoinMiner> m_hybridMiner;
    std::unique_ptr<BioMining::Crypto::BitcoinMiner> m_biologicalMiner;
    QTimer* m_miningTimer;
    int m_miningRounds;
    
    BioMining::MiningMode m_miningMode;
    
public:
    explicit CompleteWorkflowExample(BioMining::MiningMode mode, QObject *parent = nullptr) 
        : QObject(parent), m_meaInterface(std::make_shared<MEAInterface>(this)), 
          m_hybridMiner(nullptr), m_biologicalMiner(nullptr), 
          m_miningTimer(nullptr), m_miningRounds(0), m_miningMode(mode) {}

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

        // Initialize MEA interface
        qDebug() << "Initializing MEA interface...";
        if (!m_meaInterface->initialize("")) {
            qDebug() << "ERROR: Failed to initialize MEA interface";
            QCoreApplication::quit();
            return;
        }
        qDebug() << "✓ MEA interface ready";

        // Initialize miner based on mode
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            qDebug() << "Initializing hybrid Bitcoin miner...";
            m_hybridMiner = std::make_unique<HybridBitcoinMiner>(this);
            if (!m_hybridMiner->initialize()) {
                qDebug() << "ERROR: Failed to initialize hybrid miner";
                QCoreApplication::quit();
                return;
            }
            if (!m_hybridMiner->connectToMEA(m_meaInterface)) {
                qDebug() << "ERROR: Failed to connect MEA to hybrid miner";
                QCoreApplication::quit();
                return;
            }
            qDebug() << "✓ Hybrid miner ready";
        } else if (m_miningMode == BioMining::MiningMode::Biological) {
            qDebug() << "Initializing biological Bitcoin miner...";
            m_biologicalMiner = std::make_unique<BioMining::Crypto::BitcoinMiner>(this);
            if (!m_biologicalMiner->initialize()) {
                qDebug() << "ERROR: Failed to initialize biological miner";
                QCoreApplication::quit();
                return;
            }
            // Set some default mining config for the biological miner
            BioMining::Crypto::BitcoinMiner::MiningConfig config;
            config.blockHeader = "0000000000000000000000000000000000000000000000000000000000000000";
            config.difficulty = 1;
            config.maxAttempts = 100000;
            config.signalWeight = 2.0;
            config.threadCount = QThread::idealThreadCount();
            m_biologicalMiner->setMiningConfig(config);
            qDebug() << "✓ Biological miner ready";
        }
        
        qDebug() << "System initialization complete!";
    }

    void setupConnections() {
        qDebug() << "\n2. Setting up data connections";
        qDebug() << "==============================";

        // Connect MEA to appropriate miner
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            connect(m_meaInterface.get(), &MEAInterface::signalsAcquired,
                    m_hybridMiner->getBiologicalNetwork(), &BiologicalNetwork::onMEASignalsReceived);
            
            // Connect biological network state changes (from within the hybrid miner)
            connect(m_hybridMiner->getBiologicalNetwork(), &BiologicalNetwork::learningStateChanged,
                    this, &CompleteWorkflowExample::onLearningStateChanged);
            
            connect(m_hybridMiner->getBiologicalNetwork(), &BiologicalNetwork::trainingProgress,
                    this, &CompleteWorkflowExample::onTrainingProgress);
            
            connect(m_hybridMiner->getBiologicalNetwork(), &BiologicalNetwork::learningCompleted,
                    this, &CompleteWorkflowExample::onLearningCompleted);
            
            // Connect mining results (HybridBitcoinMiner uses miningComplete)
            connect(m_hybridMiner.get(), &HybridBitcoinMiner::miningComplete,
                    this, &CompleteWorkflowExample::onBlockMined);
        } else if (m_miningMode == BioMining::MiningMode::Biological) {
            connect(m_meaInterface.get(), &MEAInterface::signalsAcquired,
                    m_biologicalMiner.get(), &BioMining::Crypto::BitcoinMiner::onBioSignalsReceived);
            
            // Connect mining results (BitcoinMiner uses miningComplete)
            connect(m_biologicalMiner.get(), &BioMining::Crypto::BitcoinMiner::miningComplete,
                    this, &CompleteWorkflowExample::onBlockMined);
        }
        
        qDebug() << "✓ Data connections established";
    }

    void startTraining() {
        qDebug() << "\n3. Starting biological network training";
        qDebug() << "=======================================";
        
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            // Initialize biological learning within the hybrid miner
            if (!m_hybridMiner->initializeBiologicalLearning()) {
                qDebug() << "ERROR: Failed to initialize biological learning in hybrid miner";
                QCoreApplication::quit();
                return;
            }
            
            // Create dummy training data
            std::vector<BioMining::BiologicalNetwork::BiologicalTrainingData> trainingData;
            for (int i = 0; i < 100; ++i) {
                BioMining::BiologicalNetwork::BiologicalTrainingData data;
                data.blockHeader = QString("dummy_header_%1").arg(i);
                data.difficulty = 1000 + (i * 10);
                data.nonce = QRandomGenerator::global()->generate();
                data.wasSuccessful = (QRandomGenerator::global()->generateDouble() > 0.5);
                data.reward = data.wasSuccessful ? 1.0 : -0.5;
                data.inputSignals.resize(MEAInterface::ELECTRODE_COUNT);
                for (int j = 0; j < MEAInterface::ELECTRODE_COUNT; ++j) {
                    data.inputSignals[j] = QRandomGenerator::global()->generateDouble();
                }
                trainingData.push_back(data);
            }
            
            if (!m_hybridMiner->performInitialLearning(trainingData)) {
                qDebug() << "ERROR: Failed to start initial learning";
                QCoreApplication::quit();
                return;
            }
            qDebug() << "✓ Training started (with initial learning via HybridBitcoinMiner)";
        } else if (m_miningMode == BioMining::MiningMode::Biological) {
            qDebug() << "No specific training phase for Biological Mining Mode. Proceeding...";
        }
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
        
        // Start continuous MEA acquisition
        m_meaInterface->startContinuousAcquisition(100); // Start acquisition at 100ms interval
        qDebug() << "✓ MEA continuous acquisition started";
        
        // Start mining based on mode
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            if (!m_hybridMiner->startHybridMining()) {
                qDebug() << "ERROR: Failed to start hybrid mining";
                QCoreApplication::quit();
                return;
            }
            qDebug() << "✓ Hybrid mining started";
        } else if (m_miningMode == BioMining::MiningMode::Biological) {
            m_biologicalMiner->startContinuousMining();
            qDebug() << "✓ Biological mining started";
        }
        
        // Setup mining timer for regular logging/monitoring
        m_miningTimer = new QTimer(this);
        connect(m_miningTimer, &QTimer::timeout, this, &CompleteWorkflowExample::performMiningRound);
        m_miningTimer->start(5000);  // Report status every 5 seconds
        
        qDebug() << "✓ Adaptive mining process initiated";
    }

    void performMiningRound() {
        m_miningRounds++;
        
        qDebug() << "\n--- Mining Round" << m_miningRounds << "---";
        
        // Generate test block header
        QString blockHeader = QString("00000000%1d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f")
                             .arg(m_miningRounds, 8, 16, QChar('0'));
        
        // Simulate difficulty
        uint64_t difficulty = 486604799 + (m_miningRounds * 1000);
        
        qDebug() << "Block Header:" << blockHeader;
        qDebug() << "Difficulty:" << difficulty;
        
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            // Predict nonce using hybrid miner
            BioMining::Crypto::BiologicalNoncePrediction prediction = m_hybridMiner->predictNonce(blockHeader);
            qDebug() << "Predicted Nonce:" << prediction.predictedNonce 
                     << "(Confidence:" << QString::number(prediction.confidence, 'f', 2) << ")";

            // Simulate mining outcome based on prediction confidence (for demonstration)
            bool wasSuccessful = (QRandomGenerator::global()->generateDouble() < prediction.confidence);
            
            // Validate prediction and provide feedback to the miner
            prediction.isValidated = true;
            m_hybridMiner->validatePrediction(prediction, wasSuccessful);
            
            qDebug() << "Mining simulated. Outcome:" << (wasSuccessful ? "SUCCESS" : "FAILURE");
        } else if (m_miningMode == BioMining::MiningMode::Biological) {
            qDebug() << "Biological mining running continuously via signals. Monitoring...";
        }
        
        if (m_miningRounds >= 10) {
            finishWorkflow();
        }
    }

    void onBlockMined(const BioMining::Crypto::HybridBitcoinMiner::MiningResult& result) {
        qDebug() << "✓ Block mined successfully!";
        qDebug() << "   Nonce:" << result.nonce;
        qDebug() << "   Attempts:" << result.hashAttempts;
        
        // Trigger biological network optimization
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            m_hybridMiner->performRetroLearning();
        }
    }

    void finishWorkflow() {
        qDebug() << "\n6. Completing workflow and saving state";
        qDebug() << "=======================================";
        
        // Stop mining
        if (m_miningTimer) {
            m_miningTimer->stop();
        }
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            m_hybridMiner->stopHybridMining();
        } else if (m_miningMode == BioMining::MiningMode::Biological) {
            m_biologicalMiner->stopMining();
            m_biologicalMiner->stopContinuousMining();
        }
        
        // Stop MEA recording
        m_meaInterface->stopContinuousAcquisition();
        
        // Save network state (only for hybrid mode where there's a biological network managed by the miner)
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            if (m_hybridMiner->getBiologicalNetwork()->saveNetwork("biomining_trained_network.json")) {
                qDebug() << "✓ Network state saved to biomining_trained_network.json";
            }
        }
        
        qDebug() << "\n7. Final Performance Report";
        qDebug() << "===========================";
        qDebug() << "Mining rounds completed:" << m_miningRounds;
        
        if (m_miningMode == BioMining::MiningMode::Hybrid) {
            qDebug() << "Network efficiency:" << m_hybridMiner->getBiologicalNetwork()->getNetworkEfficiency() << "%";
            qDebug() << "Training epochs:" << m_hybridMiner->getBiologicalNetwork()->getTrainingEpochs();
            qDebug() << "Final learning state:" << (int)m_hybridMiner->getBiologicalNetwork()->getLearningState();
            QString diagnostic = m_hybridMiner->getBiologicalNetwork()->getNetworkDiagnostic();
            qDebug() << "Network diagnostic:" << diagnostic;
            
            // Export network diagnostics
            QJsonObject networkState = m_hybridMiner->getBiologicalNetwork()->exportNetworkState();
            QJsonDocument doc(networkState);
        } else if (m_miningMode == BioMining::MiningMode::Biological) {
            qDebug() << "Biological miner has no internal network metrics like efficiency, epochs or learning state.";
            qDebug() << "Hashrate: " << QString::number(m_biologicalMiner->getHashrate(), 'f', 2) << " H/s";
            qDebug() << "Total Attempts: " << m_biologicalMiner->getTotalAttempts();
            qDebug() << "Successful Mines: " << m_biologicalMiner->getSuccessCount();
            qDebug() << "Success Rate: " << QString::number(m_biologicalMiner->getSuccessRate() * 100, 'f', 2) << "%";
        }
        
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

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription("BioMining Platform Complete Workflow Example");

    QCommandLineOption modeOption({"m", "mode"}, "Mining mode: hybrid or biological", "mode", "hybrid");
    parser.addOption(modeOption);

    parser.process(app);

    BioMining::MiningMode selectedMode = BioMining::MiningMode::Hybrid;
    if (parser.isSet(modeOption)) {
        QString modeString = parser.value(modeOption).toLower();
        if (modeString == "biological") {
            selectedMode = BioMining::MiningMode::Biological;
            qDebug() << "Running in Biological Mining Mode";
        } else if (modeString == "hybrid") {
            selectedMode = BioMining::MiningMode::Hybrid;
            qDebug() << "Running in Hybrid Mining Mode";
        } else {
            qWarning() << "Invalid mining mode specified. Defaulting to Hybrid Mode.";
        }
    }

    CompleteWorkflowExample example(selectedMode);
    
    // Start the example after the event loop begins
    QTimer::singleShot(0, &example, &CompleteWorkflowExample::run);

    return app.exec();
}

#include "complete_workflow.moc"