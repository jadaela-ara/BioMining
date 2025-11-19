#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "../include/crypto/hybrid_bitcoin_miner.h"
#include "../include/bio/biological_network.h"
#include "../include/bio/mea_interface.h"

/**
 * @brief Basic usage example of the BioMining platform
 * 
 * This example demonstrates:
 * 1. Initializing the biological network
 * 2. Setting up MEA interface
 * 3. Running basic hybrid mining operations
 * 4. Monitoring results
 */

class BasicMiningExample : public QObject
{
    Q_OBJECT

public:
    BasicMiningExample(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void run() {
        qDebug() << "=== BioMining Platform - Basic Usage Example ===";

        // 1. Initialize biological network
        qDebug() << "1. Initializing biological network...";
        BiologicalNetwork network;
        
        BiologicalNetwork::NetworkConfig config;
        config.neuronCount = 60;
        config.learningRate = 0.01;
        config.stimulationThreshold = 0.5;
        
        network.setNetworkConfig(config);
        
        if (!network.initialize()) {
            qDebug() << "ERROR: Failed to initialize biological network";
            QCoreApplication::quit();
            return;
        }
        
        qDebug() << "✓ Biological network initialized successfully";

        // 2. Setup MEA Interface
        qDebug() << "2. Setting up MEA interface...";
        MEAInterface meaInterface;
        
        MEAInterface::MEAConfig meaConfig;
        meaConfig.electrodeCount = 60;
        meaConfig.samplingRate = 25000.0;
        meaConfig.amplification = 1200.0;
        
        if (!meaInterface.initialize(meaConfig)) {
            qDebug() << "ERROR: Failed to initialize MEA interface";
            QCoreApplication::quit();
            return;
        }
        
        qDebug() << "✓ MEA interface initialized successfully";

        // 3. Create hybrid miner
        qDebug() << "3. Initializing hybrid Bitcoin miner...";
        HybridBitcoinMiner miner;
        
        if (!miner.initializeWithBiologicalNetwork(&network)) {
            qDebug() << "ERROR: Failed to initialize hybrid miner";
            QCoreApplication::quit();
            return;
        }
        
        qDebug() << "✓ Hybrid miner initialized successfully";

        // 4. Connect MEA to biological network
        connect(&meaInterface, &MEAInterface::signalDataReceived,
                &network, &BiologicalNetwork::onMEASignalsReceived);

        // 5. Start initial training
        qDebug() << "4. Starting biological network training...";
        if (!network.startInitialLearning(50)) {
            qDebug() << "ERROR: Failed to start network training";
            QCoreApplication::quit();
            return;
        }
        
        qDebug() << "✓ Training started (50 cycles)";

        // 6. Simulate basic mining operation
        qDebug() << "5. Running basic mining simulation...";
        
        QString blockHeader = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
        uint64_t difficulty = 486604799;  // Early Bitcoin difficulty
        
        qDebug() << "Block Header:" << blockHeader;
        qDebug() << "Difficulty:" << difficulty;
        
        // Start mining
        auto result = miner.mineBlock(blockHeader, difficulty);
        
        if (result.success) {
            qDebug() << "✓ Mining successful!";
            qDebug() << "   Nonce found:" << result.nonce;
            qDebug() << "   Hash attempts:" << result.hashAttempts;
            qDebug() << "   Mining time:" << result.miningTime << "ms";
            qDebug() << "   Biological contribution:" << result.biologicalContribution << "%";
        } else {
            qDebug() << "✗ Mining failed after" << result.hashAttempts << "attempts";
        }

        // 7. Show network diagnostics
        qDebug() << "6. Network diagnostics:";
        qDebug() << "   Training progress:" << network.getTrainingProgress() << "%";
        qDebug() << "   Learning state:" << (int)network.getLearningState();
        qDebug() << "   Network efficiency:" << network.getNetworkEfficiency() << "%";
        
        QString diagnostic = network.getNetworkDiagnostic();
        qDebug() << "   Diagnostic:" << diagnostic;

        // 8. Cleanup and exit
        qDebug() << "7. Cleaning up...";
        network.stopLearning();
        meaInterface.disconnect();
        
        qDebug() << "=== Example completed successfully ===";
        
        QTimer::singleShot(1000, QCoreApplication::instance(), &QCoreApplication::quit);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "BioMining Platform v1.0.0";
    qDebug() << "Basic Mining Usage Example";
    qDebug() << "";

    BasicMiningExample example;
    
    // Start the example after the event loop begins
    QTimer::singleShot(0, &example, &BasicMiningExample::run);

    return app.exec();
}

#include "basic_mining_usage.moc"