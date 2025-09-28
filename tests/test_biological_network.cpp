#include <QtTest/QtTest>
#include <QCoreApplication>
#include "bio/biological_network.h"
#include "bio/mea_interface.h"

using namespace BioMining::Bio;

class TestBiologicalNetwork : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Tests de base
    void testInitialization();
    void testNetworkConfiguration();
    void testLearningStateTransition();
    void testNoncePrediction();

private:
    BioMining::Bio::BiologicalNetwork* m_network;
    QCoreApplication* m_app;
};

void TestBiologicalNetwork::initTestCase()
{
    // Configuration globale pour tous les tests
    int argc = 0;
    char* argv[] = {nullptr};
    if (!QCoreApplication::instance()) {
        m_app = new QCoreApplication(argc, argv);
    }
}

void TestBiologicalNetwork::cleanupTestCase()
{
    // Nettoyage global après tous les tests
}

void TestBiologicalNetwork::init()
{
    // Préparation avant chaque test
    m_network = new BioMining::Bio::BiologicalNetwork(this);
}

void TestBiologicalNetwork::cleanup()
{
    // Nettoyage après chaque test
    if (m_network) {
        delete m_network;
        m_network = nullptr;
    }
}

void TestBiologicalNetwork::testInitialization()
{
    QVERIFY(m_network->initialize());
}

void TestBiologicalNetwork::testNetworkConfiguration()
{
    BioMining::Bio::BiologicalNetwork::NetworkConfig config;
    config.neuronCount = 64;
    config.learningRate = 0.02;
    
    m_network->setNetworkConfig(config);
    auto retrievedConfig = m_network->getNetworkConfig();
    
    QCOMPARE(retrievedConfig.neuronCount, 64);
    QCOMPARE(retrievedConfig.learningRate, 0.02);
}

void TestBiologicalNetwork::testLearningStateTransition()
{
    QCOMPARE(m_network->getLearningState(), BioMining::Bio::BiologicalNetwork::LearningState::Untrained);
    
    // Start learning
    QVERIFY(m_network->startInitialLearning(10));
    
    // Stop learning
    m_network->stopLearning();
}

void TestBiologicalNetwork::testNoncePrediction()
{
    // Initialize and configure network
    QVERIFY(m_network->initialize());
    
    QString blockHeader = "0000000000000000000000000000000000000000000000000000000000000000";
    uint64_t difficulty = 1000;
    QVector<double> signalData = {0.1, 0.2, 0.3, 0.4, 0.5};
    
    auto prediction = m_network->predictOptimalNonce(blockHeader, difficulty, signalData);
    
    QVERIFY(prediction.confidence > 0.0);
    QVERIFY(prediction.confidence <= 1.0);
    QVERIFY(prediction.suggestedNonce > 0);
}

QTEST_MAIN(TestBiologicalNetwork)
#include "test_biological_network.moc"