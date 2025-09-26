#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QElapsedTimer>

#include "crypto/bitcoin_miner.h"

class TestBitcoinMiner : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Tests de configuration
    void testInitialization();
    void testMiningConfiguration();
    void testDifficultyAdjustment();
    
    // Tests de mining
    void testSingleMining();
    void testAsyncMining();
    void testContinuousMining();
    
    // Tests avec signaux biologiques
    void testBioSignalIntegration();
    void testSignalEntropyCalculation();
    void testNonceGeneration();
    
    // Tests de performance
    void testHashrateCalculation();
    void testPerformanceBenchmark();
    void testThreadedMining();
    
    // Tests de robustesse
    void testErrorHandling();
    void testLongRunning();
    void testMemoryUsage();

private:
    BitcoinMiner *m_bitcoinMiner;
    QVector<double> createTestSignals(int size = 60);
    QVector<double> createRandomSignals(int size = 60);
    QVector<double> createPatternnedSignals(int size = 60);
};

void TestBitcoinMiner::initTestCase()
{
    // Configuration globale pour tous les tests
    QLoggingCategory::setFilterRules("*.debug=false");
    qDebug() << "=== Tests Bitcoin Miner ===";
}

void TestBitcoinMiner::cleanupTestCase()
{
    // Nettoyage global
    qDebug() << "=== Fin des tests Bitcoin Miner ===";
}

void TestBitcoinMiner::init()
{
    m_bitcoinMiner = new BitcoinMiner(this);
}

void TestBitcoinMiner::cleanup()
{
    if (m_bitcoinMiner) {
        m_bitcoinMiner->stopMining();
        m_bitcoinMiner->stopContinuousMining();
        delete m_bitcoinMiner;
        m_bitcoinMiner = nullptr;
    }
}

QVector<double> TestBitcoinMiner::createTestSignals(int size)
{
    QVector<double> signals(size);
    for (int i = 0; i < size; ++i) {
        signals[i] = 0.5 * std::sin(i * 0.1) + 0.3 * std::cos(i * 0.05);
    }
    return signals;
}

QVector<double> TestBitcoinMiner::createRandomSignals(int size)
{
    QVector<double> signals(size);
    for (int i = 0; i < size; ++i) {
        signals[i] = QRandomGenerator::global()->generateDouble() - 0.5;
    }
    return signals;
}

QVector<double> TestBitcoinMiner::createPatternnedSignals(int size)
{
    QVector<double> signals(size);
    for (int i = 0; i < size; ++i) {
        // Pattern biologique réaliste avec différentes fréquences
        double neural = 0.2 * std::sin(2 * M_PI * 10.0 * i / size); // 10 Hz neural
        double muscle = 0.1 * std::sin(2 * M_PI * 50.0 * i / size); // 50 Hz muscle
        double noise = 0.05 * (QRandomGenerator::global()->generateDouble() - 0.5);
        
        signals[i] = neural + muscle + noise;
    }
    return signals;
}

void TestBitcoinMiner::testInitialization()
{
    // Test de l'état initial
    QCOMPARE(m_bitcoinMiner->getHashrate(), 0.0);
    QCOMPARE(m_bitcoinMiner->getTotalAttempts(), 0);
    QCOMPARE(m_bitcoinMiner->getSuccessCount(), 0);
    QCOMPARE(m_bitcoinMiner->getSuccessRate(), 0.0);
    
    // Vérification de la configuration par défaut
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    QVERIFY(!config.blockHeader.isEmpty());
    QVERIFY(config.difficulty > 0);
    QVERIFY(config.maxAttempts > 0);
    QVERIFY(config.signalWeight > 0.0);
    QVERIFY(config.threadCount > 0);
}

void TestBitcoinMiner::testMiningConfiguration()
{
    BitcoinMiner::MiningConfig config;
    config.blockHeader = "test_block_header";
    config.difficulty = 0x0000FFFFFFFFFFFF;
    config.maxAttempts = 5000;
    config.signalWeight = 2.0;
    config.useQuantumOptimization = true;
    config.threadCount = 4;
    
    m_bitcoinMiner->setMiningConfig(config);
    
    BitcoinMiner::MiningConfig retrievedConfig = m_bitcoinMiner->getMiningConfig();
    
    QCOMPARE(retrievedConfig.blockHeader, config.blockHeader);
    QCOMPARE(retrievedConfig.difficulty, config.difficulty);
    QCOMPARE(retrievedConfig.maxAttempts, config.maxAttempts);
    QCOMPARE(retrievedConfig.signalWeight, config.signalWeight);
    QCOMPARE(retrievedConfig.useQuantumOptimization, config.useQuantumOptimization);
    QCOMPARE(retrievedConfig.threadCount, config.threadCount);
}

void TestBitcoinMiner::testDifficultyAdjustment()
{
    QSignalSpy difficultySpy(m_bitcoinMiner, &BitcoinMiner::difficultyAdjusted);
    
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    uint64_t originalDifficulty = config.difficulty;
    
    // Test d'augmentation de difficulté
    m_bitcoinMiner->updateDifficulty(2.0);
    
    QVERIFY(difficultySpy.wait(100));
    QCOMPARE(difficultySpy.count(), 1);
    
    BitcoinMiner::MiningConfig newConfig = m_bitcoinMiner->getMiningConfig();
    QVERIFY(newConfig.difficulty != originalDifficulty);
    
    // Test de réduction de difficulté
    difficultySpy.clear();
    m_bitcoinMiner->updateDifficulty(0.5);
    
    QVERIFY(difficultySpy.wait(100));
    QCOMPARE(difficultySpy.count(), 1);
}

void TestBitcoinMiner::testSingleMining()
{
    // Configuration pour test rapide
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 1000; // Limité pour test rapide
    config.difficulty = 0x00FFFFFFFFFFFFFF; // Difficulté élevée pour succès probable
    m_bitcoinMiner->setMiningConfig(config);
    
    QVector<double> testSignals = createTestSignals();
    
    QElapsedTimer timer;
    timer.start();
    
    BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(testSignals);
    
    qint64 elapsedMs = timer.elapsed();
    
    // Vérifications du résultat
    QVERIFY(result.attempts > 0);
    QVERIFY(result.attempts <= config.maxAttempts);
    QVERIFY(result.computeTime > 0.0);
    QVERIFY(result.signalContribution >= 0.0);
    QVERIFY(result.signalContribution <= 1.0);
    
    if (result.success) {
        QVERIFY(!result.nonce.isEmpty());
        QVERIFY(!result.hash.isEmpty());
        QVERIFY(result.hash.length() == 64); // SHA-256 hex = 64 chars
    }
    
    // Vérification que le temps mesuré correspond approximativement
    QVERIFY(std::abs(elapsedMs / 1000.0 - result.computeTime) < 0.1);
    
    qDebug() << "Mining result:" << (result.success ? "SUCCESS" : "FAILURE")
             << "- Attempts:" << result.attempts 
             << "- Time:" << result.computeTime << "s"
             << "- Signal contribution:" << result.signalContribution;
}

void TestBitcoinMiner::testAsyncMining()
{
    QSignalSpy miningSpy(m_bitcoinMiner, &BitcoinMiner::miningComplete);
    QSignalSpy progressSpy(m_bitcoinMiner, &BitcoinMiner::progressUpdate);
    
    // Configuration pour test
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 2000;
    m_bitcoinMiner->setMiningConfig(config);
    
    QVector<double> testSignals = createTestSignals();
    
    // Démarrage mining asynchrone
    m_bitcoinMiner->startMiningAsync(testSignals);
    
    // Attendre la complétion
    QVERIFY(miningSpy.wait(10000)); // 10 secondes max
    QCOMPARE(miningSpy.count(), 1);
    
    // Vérifier qu'on a reçu des mises à jour de progrès
    QVERIFY(progressSpy.count() > 0);
    
    // Récupérer le résultat
    QList<QVariant> miningArgs = miningSpy.takeFirst();
    BitcoinMiner::MiningResult result = miningArgs.at(0).value<BitcoinMiner::MiningResult>();
    
    QVERIFY(result.attempts > 0);
    QVERIFY(result.computeTime > 0.0);
}

void TestBitcoinMiner::testContinuousMining()
{
    QSignalSpy miningSpy(m_bitcoinMiner, &BitcoinMiner::miningComplete);
    
    // Démarrage du mode continu
    m_bitcoinMiner->startContinuousMining();
    
    // Simulation d'arrivée de signaux biologiques
    QVector<double> signals1 = createRandomSignals();
    QVector<double> signals2 = createPatternnedSignals();
    
    // Envoi des signaux (simule MEA)
    QMetaObject::invokeMethod(m_bitcoinMiner, "onBioSignalsReceived", 
                             Q_ARG(QVector<double>, signals1));
    
    // Attendre le premier mining
    QVERIFY(miningSpy.wait(5000));
    
    // Envoi d'un second set de signaux
    QMetaObject::invokeMethod(m_bitcoinMiner, "onBioSignalsReceived", 
                             Q_ARG(QVector<double>, signals2));
    
    // Attendre le second mining
    QVERIFY(miningSpy.wait(5000));
    
    QVERIFY(miningSpy.count() >= 2);
    
    // Arrêt du mode continu
    m_bitcoinMiner->stopContinuousMining();
}

void TestBitcoinMiner::testBioSignalIntegration()
{
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 500; // Test rapide
    m_bitcoinMiner->setMiningConfig(config);
    
    // Test avec différents types de signaux
    QVector<double> randomSignals = createRandomSignals();
    QVector<double> patternedSignals = createPatternnedSignals();
    QVector<double> zeroSignals(60, 0.0);
    
    BitcoinMiner::MiningResult result1 = m_bitcoinMiner->mine(randomSignals);
    BitcoinMiner::MiningResult result2 = m_bitcoinMiner->mine(patternedSignals);
    BitcoinMiner::MiningResult result3 = m_bitcoinMiner->mine(zeroSignals);
    
    // Les contributions des signaux devraient être différentes
    QVERIFY(result1.signalContribution != result2.signalContribution);
    QVERIFY(result2.signalContribution != result3.signalContribution);
    
    // Les signaux zéro devraient avoir la plus faible contribution
    QVERIFY(result3.signalContribution <= result1.signalContribution);
    QVERIFY(result3.signalContribution <= result2.signalContribution);
    
    qDebug() << "Signal contributions - Random:" << result1.signalContribution
             << "Patterned:" << result2.signalContribution 
             << "Zero:" << result3.signalContribution;
}

void TestBitcoinMiner::testSignalEntropyCalculation()
{
    // Test indirect via les résultats de mining
    QVector<double> lowEntropySignals(60, 1.0); // Tous identiques
    QVector<double> highEntropySignals = createRandomSignals(); // Aléatoires
    
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 100; // Test très rapide
    m_bitcoinMiner->setMiningConfig(config);
    
    BitcoinMiner::MiningResult lowEntropyResult = m_bitcoinMiner->mine(lowEntropySignals);
    BitcoinMiner::MiningResult highEntropyResult = m_bitcoinMiner->mine(highEntropySignals);
    
    // Les signaux haute entropie devraient avoir une meilleure contribution
    QVERIFY(highEntropyResult.signalContribution > lowEntropyResult.signalContribution);
    
    qDebug() << "Entropy test - Low:" << lowEntropyResult.signalContribution
             << "High:" << highEntropyResult.signalContribution;
}

void TestBitcoinMiner::testNonceGeneration()
{
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 10; // Juste pour générer quelques nonces
    m_bitcoinMiner->setMiningConfig(config);
    
    QVector<double> testSignals = createTestSignals();
    
    // Plusieurs runs avec les mêmes signaux devraient générer des nonces différents
    // (à cause de la composante temporelle)
    QStringList nonces;
    
    for (int i = 0; i < 3; ++i) {
        QThread::msleep(10); // Assurer une différence temporelle
        BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(testSignals);
        
        if (result.success) {
            QVERIFY(!nonces.contains(result.nonce));
            nonces.append(result.nonce);
        }
    }
    
    qDebug() << "Generated nonces:" << nonces;
}

void TestBitcoinMiner::testHashrateCalculation()
{
    QSignalSpy hashrateSpy(m_bitcoinMiner, &BitcoinMiner::hashRateUpdated);
    
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 1000;
    m_bitcoinMiner->setMiningConfig(config);
    
    QVector<double> testSignals = createTestSignals();
    
    BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(testSignals);
    
    // Vérification du signal hashrate
    QVERIFY(hashrateSpy.count() > 0);
    
    double reportedHashrate = m_bitcoinMiner->getHashrate();
    double calculatedHashrate = result.attempts / result.computeTime;
    
    // Les deux calculs devraient être proches
    QVERIFY(std::abs(reportedHashrate - calculatedHashrate) < calculatedHashrate * 0.1);
    
    qDebug() << "Hashrate - Reported:" << reportedHashrate 
             << "Calculated:" << calculatedHashrate;
}

void TestBitcoinMiner::testPerformanceBenchmark()
{
    // Test de performance avec différentes configurations
    QVector<int> threadCounts = {1, 2, 4};
    QVector<int> attemptCounts = {1000, 2000, 5000};
    
    for (int threads : threadCounts) {
        for (int attempts : attemptCounts) {
            BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
            config.threadCount = threads;
            config.maxAttempts = attempts;
            m_bitcoinMiner->setMiningConfig(config);
            
            QVector<double> testSignals = createTestSignals();
            
            QElapsedTimer timer;
            timer.start();
            
            BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(testSignals);
            
            qint64 elapsedMs = timer.elapsed();
            double hashrate = result.attempts / result.computeTime;
            
            qDebug() << QString("Performance - Threads: %1, Attempts: %2, Time: %3ms, Hashrate: %4 H/s")
                        .arg(threads).arg(attempts).arg(elapsedMs).arg(hashrate, 0, 'f', 0);
            
            // Vérifications de base
            QVERIFY(result.attempts <= attempts);
            QVERIFY(result.computeTime > 0.0);
            QVERIFY(hashrate > 0.0);
            
            // Performance minimale attendue (ajuster selon le matériel)
            QVERIFY(hashrate > 100.0); // Au moins 100 H/s
        }
    }
}

void TestBitcoinMiner::testThreadedMining()
{
    // Test avec plusieurs threads
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.threadCount = 4;
    config.maxAttempts = 2000;
    m_bitcoinMiner->setMiningConfig(config);
    
    QVector<double> testSignals = createTestSignals();
    
    QElapsedTimer timer;
    timer.start();
    
    BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(testSignals);
    
    qint64 multiThreadTime = timer.elapsed();
    
    // Comparaison avec single thread
    config.threadCount = 1;
    m_bitcoinMiner->setMiningConfig(config);
    
    timer.restart();
    BitcoinMiner::MiningResult singleResult = m_bitcoinMiner->mine(testSignals);
    qint64 singleThreadTime = timer.elapsed();
    
    qDebug() << "Threading test - Single:" << singleThreadTime << "ms Multi:" << multiThreadTime << "ms";
    
    // Le multi-threading devrait être plus efficace (ou au moins pas plus lent)
    // Note: sur certains systèmes légers, le overhead peut annuler le bénéfice
    QVERIFY(multiThreadTime <= singleThreadTime * 1.5); // Max 50% plus lent
}

void TestBitcoinMiner::testErrorHandling()
{
    // Test avec signaux vides
    QVector<double> emptySignals;
    
    BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(emptySignals);
    
    QVERIFY(result.attempts > 0); // Devrait quand même fonctionner
    QVERIFY(result.computeTime > 0.0);
    QCOMPARE(result.signalContribution, 0.0); // Pas de contribution
    
    // Test avec signaux invalides
    QVector<double> invalidSignals(60);
    for (int i = 0; i < 60; ++i) {
        invalidSignals[i] = (i % 3 == 0) ? std::numeric_limits<double>::quiet_NaN() : 1.0;
    }
    
    BitcoinMiner::MiningResult invalidResult = m_bitcoinMiner->mine(invalidSignals);
    QVERIFY(invalidResult.attempts > 0);
    // Devrait gérer les valeurs NaN gracieusement
}

void TestBitcoinMiner::testLongRunning()
{
    // Test de stabilité sur une période plus longue
    QSignalSpy miningSpy(m_bitcoinMiner, &BitcoinMiner::miningComplete);
    
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 1000; // Cycles rapides
    m_bitcoinMiner->setMiningConfig(config);
    
    m_bitcoinMiner->startContinuousMining();
    
    // Simulation de plusieurs cycles de signaux
    for (int i = 0; i < 10; ++i) {
        QVector<double> signals = (i % 2 == 0) ? createRandomSignals() : createPatternnedSignals();
        
        QMetaObject::invokeMethod(m_bitcoinMiner, "onBioSignalsReceived", 
                                 Q_ARG(QVector<double>, signals));
        
        QTest::qWait(100);
    }
    
    // Attendre plusieurs complétions
    QTest::qWait(5000);
    
    QVERIFY(miningSpy.count() >= 5); // Au moins la moitié des cycles
    
    // Vérifier que les statistiques sont cohérentes
    QVERIFY(m_bitcoinMiner->getTotalAttempts() > 0);
    QVERIFY(m_bitcoinMiner->getHashrate() > 0.0);
    
    m_bitcoinMiner->stopContinuousMining();
}

void TestBitcoinMiner::testMemoryUsage()
{
    // Test de base pour détecter des fuites mémoire évidentes
    // Note: Un vrai test de fuite mémoire nécessiterait des outils spécialisés
    
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 500;
    m_bitcoinMiner->setMiningConfig(config);
    
    // Plusieurs cycles pour vérifier qu'il n'y a pas d'accumulation excessive
    for (int i = 0; i < 20; ++i) {
        QVector<double> signals = createRandomSignals();
        BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(signals);
        
        QVERIFY(result.attempts > 0);
        
        // Petit délai pour permettre le nettoyage
        QTest::qWait(10);
    }
    
    // Si on arrive ici sans crash, c'est un bon signe
    QVERIFY(true);
}

QTEST_MAIN(TestBitcoinMiner)
#include "test_bitcoin_miner.moc"