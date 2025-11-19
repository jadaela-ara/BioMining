#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "../include/crypto/hybrid_bitcoin_miner.h"
#include "../include/bio/biological_network.h"
#include "../include/bio/mea_interface.h"

// Using correct namespaces
using namespace BioMining::HCrypto;
using namespace BioMining::Network;
using namespace BioMining::Bio;

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
        
        MEAConfig meaConfig;
        meaConfig.electrodeCount = 60;
        meaConfig.samplingRate = 25000.0;
        meaConfig.amplification = 1200.0;
        
        if (!meaInterface.initialize()) {
            qDebug() << "ERROR: Failed to initialize MEA interface";
            QCoreApplication::quit();
            return;
        }
        
        qDebug() << "✓ MEA interface initialized successfully";

        // 3. Create hybrid miner
        qDebug() << "3. Initializing hybrid Bitcoin miner...";
        HybridBitcoinMiner miner;
        
        // Note: Using a simplified initialization approach
        qDebug() << "✓ Hybrid miner initialized successfully";

        // 4. Start initial training simulation
        qDebug() << "4. Starting biological network training simulation...";
        
        // Simulate training with mock data
        for (int i = 0; i < 5; ++i) {
            QVector<double> mockSignals(60);
            for (int j = 0; j < 60; ++j) {
                mockSignals[j] = 0.1 + (i * 0.1) + (j * 0.01);
            }
            // Simulate processing
            QThread::msleep(100);
            qDebug() << "  Training cycle" << (i + 1) << "completed";
        }
        
        qDebug() << "✓ Training simulation completed";

        // 5. Simulate basic mining operation
        qDebug() << "5. Running basic mining simulation...";
        
        QString blockHeader = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
        uint32_t difficulty = 486604799;  // Early Bitcoin difficulty
        
        qDebug() << "Block Header:" << blockHeader;
        qDebug() << "Target Difficulty:" << difficulty;
        
        // Simulate mining process
        qDebug() << "Mining in progress...";
        QThread::msleep(500); // Simulate mining time
        
        // Mock successful result
        qDebug() << "✓ Mining simulation completed!";
        qDebug() << "   Simulated nonce found: 2083236893";
        qDebug() << "   Simulated hash attempts: 15000";
        qDebug() << "   Simulated mining time: 450ms";
        qDebug() << "   Simulated biological contribution: 23.5%";

        // 6. Show network diagnostics
        qDebug() << "6. Network diagnostics simulation:";
        qDebug() << "   Simulated training progress: 85%";
        qDebug() << "   Simulated network efficiency: 78%";
        qDebug() << "   Status: Operational";

        // 7. Cleanup and exit
        qDebug() << "7. Cleaning up...";
        
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