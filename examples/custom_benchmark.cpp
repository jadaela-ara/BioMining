#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../include/crypto/hybrid_bitcoin_miner.h"
#include "../include/bio/biological_network.h"
#include "../include/bio/mea_interface.h"

/**
 * @brief Custom benchmark example for the BioMining platform
 * 
 * This example demonstrates:
 * 1. Performance benchmarking of different configurations
 * 2. Comparing traditional vs biological mining
 * 3. Measuring network adaptation efficiency
 * 4. Custom optimization algorithms
 * 5. Statistical analysis and reporting
 */

class CustomBenchmarkExample : public QObject
{
    Q_OBJECT

private:
    struct BenchmarkConfig {
        QString name;
        int neuronCount;
        double learningRate;
        double stimulationThreshold;
        bool useBiological;
        int miningRounds;
    };
    
    struct BenchmarkResult {
        QString configName;
        double averageTime;
        uint64_t totalAttempts;
        int successfulMines;
        double biologicalContribution;
        double networkEfficiency;
    };
    
    QVector<BenchmarkConfig> m_configs;
    QVector<BenchmarkResult> m_results;
    int m_currentConfigIndex;
    int m_currentRound;
    
    BiologicalNetwork* m_network;
    MEAInterface* m_meaInterface;
    HybridBitcoinMiner* m_miner;
    
    QElapsedTimer m_roundTimer;
    BenchmarkResult m_currentResult;
    
public:
    CustomBenchmarkExample(QObject *parent = nullptr) 
        : QObject(parent), m_currentConfigIndex(0), m_currentRound(0),
          m_network(nullptr), m_meaInterface(nullptr), m_miner(nullptr) {
        
        setupBenchmarkConfigs();
    }

private:
    void setupBenchmarkConfigs() {
        qDebug() << "Setting up benchmark configurations...";
        
        // Config 1: Traditional mining (no biological)
        m_configs.append({
            "Traditional Mining",
            0, 0.0, 0.0, false, 20
        });
        
        // Config 2: Basic biological network
        m_configs.append({
            "Basic Biological (30 neurons)",
            30, 0.01, 0.5, true, 20
        });
        
        // Config 3: Standard biological network
        m_configs.append({
            "Standard Biological (60 neurons)",
            60, 0.01, 0.5, true, 20
        });
        
        // Config 4: High-performance biological network
        m_configs.append({
            "High-Performance (100 neurons)",
            100, 0.015, 0.4, true, 20
        });
        
        // Config 5: Adaptive biological network
        m_configs.append({
            "Adaptive Learning",
            60, 0.02, 0.3, true, 25
        });
        
        qDebug() << "✓ Configured" << m_configs.size() << "benchmark tests";
    }

public slots:
    void run() {
        qDebug() << "=== BioMining Platform - Custom Benchmark Suite ===";
        qDebug() << "";
        qDebug() << "This benchmark will test multiple configurations:";
        
        for (int i = 0; i < m_configs.size(); ++i) {
            const auto& config = m_configs[i];
            qDebug() << QString("  %1. %2 (%3 rounds)")
                       .arg(i + 1)
                       .arg(config.name)
                       .arg(config.miningRounds);
        }
        
        qDebug() << "";
        qDebug() << "Starting benchmark in 3 seconds...";
        
        QTimer::singleShot(3000, this, &CustomBenchmarkExample::startNextConfiguration);
    }

private slots:
    void startNextConfiguration() {
        if (m_currentConfigIndex >= m_configs.size()) {
            generateFinalReport();
            return;
        }
        
        const auto& config = m_configs[m_currentConfigIndex];
        
        qDebug() << QString("\n=== Configuration %1/%2: %3 ===")
                   .arg(m_currentConfigIndex + 1)
                   .arg(m_configs.size())
                   .arg(config.name);
        
        // Initialize current result tracking
        m_currentResult = BenchmarkResult();
        m_currentResult.configName = config.name;
        m_currentResult.averageTime = 0.0;
        m_currentResult.totalAttempts = 0;
        m_currentResult.successfulMines = 0;
        m_currentResult.biologicalContribution = 0.0;
        m_currentResult.networkEfficiency = 0.0;
        
        m_currentRound = 0;
        
        // Initialize system for this configuration
        initializeForConfiguration(config);
    }
    
    void initializeForConfiguration(const BenchmarkConfig& config) {
        qDebug() << "Initializing system for configuration:" << config.name;
        
        // Clean up previous instances
        if (m_network) delete m_network;
        if (m_meaInterface) delete m_meaInterface;
        if (m_miner) delete m_miner;
        
        if (config.useBiological) {
            // Initialize biological components
            m_network = new BiologicalNetwork(this);
            
            BiologicalNetwork::NetworkConfig networkConfig;
            networkConfig.neuronCount = config.neuronCount;
            networkConfig.learningRate = config.learningRate;
            networkConfig.stimulationThreshold = config.stimulationThreshold;
            networkConfig.adaptationRate = 0.1;
            networkConfig.memoryDepth = 1000;
            networkConfig.useReinforcementLearning = true;
            
            m_network->setNetworkConfig(networkConfig);
            
            if (!m_network->initialize()) {
                qDebug() << "ERROR: Failed to initialize biological network";
                QCoreApplication::quit();
                return;
            }
            
            // Initialize MEA interface
            m_meaInterface = new MEAInterface(this);
            MEAInterface::MEAConfig meaConfig;
            meaConfig.electrodeCount = config.neuronCount;
            meaConfig.samplingRate = 25000.0;
            meaConfig.amplification = 1200.0;
            
            m_meaInterface->initialize(meaConfig);
            
            // Connect MEA to network
            connect(m_meaInterface, &MEAInterface::signalDataReceived,
                    m_network, &BiologicalNetwork::onMEASignalsReceived);
        }
        
        // Initialize miner
        m_miner = new HybridBitcoinMiner(this);
        
        if (config.useBiological) {
            m_miner->initializeWithBiologicalNetwork(m_network);
        } else {
            m_miner->initializeTraditionalMining();
        }
        
        qDebug() << "✓ System initialized for" << config.name;
        
        // Start training if biological
        if (config.useBiological && m_network) {
            qDebug() << "Starting network training...";
            
            connect(m_network, &BiologicalNetwork::learningCompleted,
                    this, &CustomBenchmarkExample::onTrainingCompleted);
            
            m_network->startInitialLearning(50);  // Quick training for benchmark
        } else {
            startBenchmarkRounds();
        }
    }
    
    void onTrainingCompleted(bool success) {
        if (success) {
            qDebug() << "✓ Training completed successfully";
            startBenchmarkRounds();
        } else {
            qDebug() << "✗ Training failed, skipping configuration";
            m_currentConfigIndex++;
            startNextConfiguration();
        }
    }
    
    void startBenchmarkRounds() {
        qDebug() << "Starting benchmark rounds...";
        performBenchmarkRound();
    }
    
    void performBenchmarkRound() {
        const auto& config = m_configs[m_currentConfigIndex];
        
        if (m_currentRound >= config.miningRounds) {
            // Configuration completed
            finalizeBenchmarkConfiguration();
            return;
        }
        
        m_currentRound++;
        
        qDebug() << QString("Round %1/%2 for %3")
                   .arg(m_currentRound)
                   .arg(config.miningRounds)
                   .arg(config.name);
        
        // Generate test block
        QString blockHeader = QString("benchmark%1_%2d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f")
                             .arg(m_currentConfigIndex, 2, 10, QChar('0'))
                             .arg(m_currentRound, 4, 10, QChar('0'));
        
        uint64_t difficulty = 486604799 + (m_currentRound * 10000);
        
        // Start timing
        m_roundTimer.start();
        
        // Perform mining
        auto result = m_miner->mineBlock(blockHeader, difficulty);
        
        // Record results
        double roundTime = m_roundTimer.elapsed();
        
        m_currentResult.averageTime += roundTime;
        m_currentResult.totalAttempts += result.hashAttempts;
        
        if (result.success) {
            m_currentResult.successfulMines++;
            qDebug() << QString("  ✓ Success in %1ms (%2 attempts)")
                       .arg(roundTime, 0, 'f', 2)
                       .arg(result.hashAttempts);
        } else {
            qDebug() << QString("  ✗ Failed after %1ms (%2 attempts)")
                       .arg(roundTime, 0, 'f', 2)
                       .arg(result.hashAttempts);
        }
        
        if (config.useBiological && m_network) {
            m_currentResult.biologicalContribution += result.biologicalContribution;
            m_currentResult.networkEfficiency = m_network->getNetworkEfficiency();
            
            // Add learning example
            BiologicalNetwork::LearningData learningData;
            learningData.blockHeader = blockHeader;
            learningData.difficulty = difficulty;
            learningData.wasSuccessful = result.success;
            learningData.attempts = result.hashAttempts;
            learningData.computeTime = roundTime;
            learningData.targetNonce = result.nonce;
            learningData.timestamp = QDateTime::currentDateTime();
            
            m_network->addLearningExample(learningData);
        }
        
        // Continue with next round
        QTimer::singleShot(100, this, &CustomBenchmarkExample::performBenchmarkRound);
    }
    
    void finalizeBenchmarkConfiguration() {
        const auto& config = m_configs[m_currentConfigIndex];
        
        // Calculate averages
        if (config.miningRounds > 0) {
            m_currentResult.averageTime /= config.miningRounds;
            
            if (config.useBiological && config.miningRounds > 0) {
                m_currentResult.biologicalContribution /= config.miningRounds;
            }
        }
        
        m_results.append(m_currentResult);
        
        qDebug() << QString("Configuration '%1' completed:").arg(config.name);
        qDebug() << QString("  Average time: %1ms").arg(m_currentResult.averageTime, 0, 'f', 2);
        qDebug() << QString("  Success rate: %1/%2 (%3%)")
                   .arg(m_currentResult.successfulMines)
                   .arg(config.miningRounds)
                   .arg((100.0 * m_currentResult.successfulMines) / config.miningRounds, 0, 'f', 1);
        qDebug() << QString("  Total attempts: %1").arg(m_currentResult.totalAttempts);
        
        if (config.useBiological) {
            qDebug() << QString("  Biological contribution: %1%")
                       .arg(m_currentResult.biologicalContribution, 0, 'f', 1);
            qDebug() << QString("  Network efficiency: %1%")
                       .arg(m_currentResult.networkEfficiency, 0, 'f', 1);
        }
        
        // Move to next configuration
        m_currentConfigIndex++;
        QTimer::singleShot(1000, this, &CustomBenchmarkExample::startNextConfiguration);
    }
    
    void generateFinalReport() {
        qDebug() << "\n";
        qDebug() << "=================================================================";
        qDebug() << "                    FINAL BENCHMARK REPORT";
        qDebug() << "=================================================================";
        qDebug() << "";
        
        // Summary table
        qDebug() << QString("%-25s %10s %10s %8s %12s %12s")
                   .arg("Configuration")
                   .arg("Avg Time")
                   .arg("Success")
                   .arg("Attempts")
                   .arg("Bio Contrib")
                   .arg("Efficiency");
        qDebug() << "-------------------------------------------------------------------------";
        
        for (const auto& result : m_results) {
            qDebug() << QString("%-25s %9.2fms %8.1f%% %8llu %10.1f%% %10.1f%%")
                       .arg(result.configName)
                       .arg(result.averageTime)
                       .arg((100.0 * result.successfulMines) / 20.0)  // Assuming 20 rounds
                       .arg(result.totalAttempts)
                       .arg(result.biologicalContribution)
                       .arg(result.networkEfficiency);
        }
        
        qDebug() << "";
        
        // Find best configurations
        if (!m_results.isEmpty()) {
            auto fastest = std::min_element(m_results.begin(), m_results.end(),
                [](const BenchmarkResult& a, const BenchmarkResult& b) {
                    return a.averageTime < b.averageTime;
                });
                
            auto mostSuccessful = std::max_element(m_results.begin(), m_results.end(),
                [](const BenchmarkResult& a, const BenchmarkResult& b) {
                    return a.successfulMines < b.successfulMines;
                });
            
            qDebug() << "BEST PERFORMERS:";
            qDebug() << QString("  Fastest: %1 (%2ms average)")
                       .arg(fastest->configName)
                       .arg(fastest->averageTime, 0, 'f', 2);
            qDebug() << QString("  Most successful: %1 (%2 successes)")
                       .arg(mostSuccessful->configName)
                       .arg(mostSuccessful->successfulMines);
        }
        
        qDebug() << "";
        qDebug() << "=== Benchmark completed successfully ===";
        
        QTimer::singleShot(2000, QCoreApplication::instance(), &QCoreApplication::quit);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "BioMining Platform v1.0.0";
    qDebug() << "Custom Benchmark Suite";
    qDebug() << "Performance testing and optimization analysis\n";

    CustomBenchmarkExample benchmark;
    
    // Start the benchmark after the event loop begins
    QTimer::singleShot(0, &benchmark, &CustomBenchmarkExample::run);

    return app.exec();
}

#include "custom_benchmark.moc"