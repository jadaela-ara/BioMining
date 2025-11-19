#!/bin/bash

# Script de correction pour BioMining dans Cloud Shell
# Exécutez ce script dans le répertoire racine de BioMining

echo "🔧 Application des corrections BioMining..."

# 1. Correction du namespace dans biological_network.cpp
echo "Correction du namespace dans biological_network.cpp..."
if ! grep -q "using namespace BioMining::Network;" src/cpp/biological_network.cpp; then
    sed -i '14a\\nusing namespace BioMining::Network;' src/cpp/biological_network.cpp
    echo "✅ Namespace ajouté dans biological_network.cpp"
fi

# 2. Correction des références BiologicalNetwork dans hybrid_bitcoin_miner.cpp
echo "Correction des références dans hybrid_bitcoin_miner.cpp..."
sed -i 's/BiologicalNetwork::NetworkConfig/BioMining::Network::BiologicalNetwork::NetworkConfig/g' src/cpp/hybrid_bitcoin_miner.cpp
echo "✅ Références BiologicalNetwork corrigées"

# 3. Correction des conflits avec le mot-clé 'signals' dans les tests
echo "Correction des conflits 'signals' dans les tests..."
find tests/ -name "*.cpp" -exec sed -i 's/QVector<double> signals/QVector<double> signalData/g' {} \;
find tests/ -name "*.cpp" -exec sed -i 's/signals\[/signalData[/g' {} \;
find tests/ -name "*.cpp" -exec sed -i 's/return signals;/return signalData;/g' {} \;
echo "✅ Conflits 'signals' corrigés"

# 4. Ajout des includes manquants dans les tests
echo "Ajout des includes manquants..."
for test_file in tests/test_*.cpp; do
    if ! grep -q "#include <cmath>" "$test_file"; then
        sed -i '1i#include <cmath>' "$test_file"
    fi
    if ! grep -q "#include <QElapsedTimer>" "$test_file"; then
        sed -i '/QtTest/a#include <QElapsedTimer>' "$test_file"
    fi
    if ! grep -q "#include <QLoggingCategory>" "$test_file"; then
        sed -i '/QtTest/a#include <QLoggingCategory>' "$test_file"
    fi
done
echo "✅ Includes ajoutés"

# 5. Conversion Google Test vers Qt Test (si nécessaire)
if grep -q "gtest/gtest.h" tests/test_biological_network.cpp 2>/dev/null; then
    echo "Conversion de Google Test vers Qt Test..."
    cat > tests/test_biological_network.cpp << 'EOF'
#include <QtTest/QtTest>
#include <QCoreApplication>
#include "bio/biological_network.h"

using namespace BioMining::Network;

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
    BioMining::Network::BiologicalNetwork* m_network;
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
    m_network = new BioMining::Network::BiologicalNetwork(this);
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
    BioMining::Network::BiologicalNetwork::NetworkConfig config;
    config.neuronCount = 64;
    config.learningRate = 0.02;
    
    m_network->setNetworkConfig(config);
    auto retrievedConfig = m_network->getNetworkConfig();
    
    QCOMPARE(retrievedConfig.neuronCount, 64);
    QCOMPARE(retrievedConfig.learningRate, 0.02);
}

void TestBiologicalNetwork::testLearningStateTransition()
{
    QCOMPARE(m_network->getLearningState(), BioMining::Network::BiologicalNetwork::LearningState::Untrained);
    
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
EOF
    echo "✅ Conversion Qt Test terminée"
fi

# 6. Correction des namespaces dans tous les fichiers
echo "Vérification finale des namespaces..."
# Correction des références MEAInterface
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/\bMEAInterface\b/BioMining::Bio::MEAInterface/g' 2>/dev/null || true
# Correction des références BitcoinMiner
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/\([^:]:\)BitcoinMiner\b/\1BioMining::Crypto::BitcoinMiner/g' 2>/dev/null || true

echo "🎉 Toutes les corrections ont été appliquées !"
echo ""
echo "📋 Prochaines étapes :"
echo "1. mkdir -p build && cd build"  
echo "2. cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON"
echo "3. make -j\$(nproc)"
echo "4. make test (optionnel)"