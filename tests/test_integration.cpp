#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QElapsedTimer>
#include <QTemporaryFile>
#include <cmath>

#include "bio/mea_interface.h"
#include "crypto/bitcoin_miner.h"

using namespace BioMining::Bio;
using namespace BioMining::Crypto;

/**
 * @brief Tests d'intégration pour la plateforme bio-mining complète
 * 
 * Ces tests vérifient le fonctionnement coordonné entre les interfaces MEA
 * et les modules de mining Bitcoin, simulant des scénarios d'utilisation réels.
 */
class TestIntegration : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Tests d'intégration de base
    void testMEAToMinerPipeline();
    void testContinuousOperation();
    void testAdaptiveCalibration();
    
    // Tests de scénarios d'utilisation
    void testFullWorkflowSimulation();
    void testLongTermStability();
    void testErrorRecovery();
    
    // Tests de performance intégrée
    void testEndToEndPerformance();
    void testConcurrentOperations();
    void testResourceManagement();
    
    // Tests de cas d'usage réels
    void testLaboratoryWorkflow();
    void testBenchmarkMode();
    void testMaintenanceMode();

private:
    BioMining::Bio::MEAInterface *m_meaInterface;
    BioMining::Crypto::BitcoinMiner *m_bitcoinMiner;
    
    void setupRealisticMEA();
    void simulateLabConditions();
    QVector<double> simulateNeuralActivity(int duration_ms = 1000);
    QVector<double> simulateMuscleTension(int electrodes = 60);
    QVector<double> createPatternnedSignals(int size);
    void verifyMiningQuality(const BioMining::Crypto::BitcoinMiner::MiningResult &result);
};

void TestIntegration::initTestCase()
{
    qDebug() << "=== Tests d'Intégration Bio-Mining ===";
    QLoggingCategory::setFilterRules("*.debug=false");
}

void TestIntegration::cleanupTestCase()
{
    qDebug() << "=== Fin des tests d'intégration ===";
}

void TestIntegration::init()
{
    m_meaInterface = new BioMining::Bio::MEAInterface(this);
    m_bitcoinMiner = new BioMining::Crypto::BitcoinMiner(this);
    
    // Connexion du pipeline MEA -> Miner
    connect(m_meaInterface, &BioMining::Bio::MEAInterface::signalsAcquired,
            m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::onBioSignalsReceived);
}

void TestIntegration::cleanup()
{
    if (m_meaInterface) {
        m_meaInterface->stopContinuousAcquisition();
        m_meaInterface->disconnect();
        delete m_meaInterface;
        m_meaInterface = nullptr;
    }
    
    if (m_bitcoinMiner) {
        m_bitcoinMiner->stopMining();
        m_bitcoinMiner->stopContinuousMining();
        delete m_bitcoinMiner;
        m_bitcoinMiner = nullptr;
    }
}

void TestIntegration::setupRealisticMEA()
{
    // Configuration MEA avec paramètres réalistes
    QVERIFY(m_meaInterface->initialize());
    
    // Paramètres de stimulation biologiques typiques
    m_meaInterface->setStimulationParameters(
        1.5,    // 1.5V amplitude
        200.0,  // 200ms durée
        25.0    // 25Hz fréquence
    );
    
    // Calibration initiale
    m_meaInterface->adjustCalibration(1.2);
}

void TestIntegration::simulateLabConditions()
{
    // Configuration mining pour conditions de laboratoire
    BioMining::Crypto::BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    
    // Paramètres optimisés pour signaux biologiques
    config.maxAttempts = 5000;
    config.signalWeight = 1.8;
    config.difficulty = 0x0000FFFFFFFFFFFF; // Difficulté modérée
    config.threadCount = 2; // Modéré pour éviter interférences
    config.useQuantumOptimization = false; // Désactivé pour tests
    
    m_bitcoinMiner->setMiningConfig(config);
}

QVector<double> TestIntegration::simulateNeuralActivity(int duration_ms)
{
    QVector<double> signalData(60);
    
    // Simulation d'activité neurale réaliste
    double time_s = duration_ms / 1000.0;
    
    for (int electrode = 0; electrode < 60; ++electrode) {
        double signal = 0.0;
        
        // Activité de base (1-4 Hz)
        signal += 0.1 * std::sin(2 * M_PI * 2.0 * time_s + electrode * 0.1);
        
        // Activité alpha (8-12 Hz)
        signal += 0.05 * std::sin(2 * M_PI * 10.0 * time_s + electrode * 0.2);
        
        // Activité beta (13-30 Hz)
        signal += 0.02 * std::sin(2 * M_PI * 20.0 * time_s + electrode * 0.3);
        
        // Spikes occasionnels
        if (QRandomGenerator::global()->generateDouble() < 0.02) {
            signal += 0.5 * std::exp(-std::pow(time_s - electrode * 0.01, 2) * 100);
        }
        
        // Bruit biologique
        signal += 0.01 * (QRandomGenerator::global()->generateDouble() - 0.5);
        
        signalData[electrode] = signal;
    }
    
    return signalData;
}

QVector<double> TestIntegration::simulateMuscleTension(int electrodes)
{
    QVector<double> signalData(electrodes);
    
    // Simulation de tension musculaire avec EMG
    for (int i = 0; i < electrodes; ++i) {
        double tension = 0.0;
        
        // Tension de base
        tension += 0.05;
        
        // Contraction musculaire (50-150 Hz)
        double contraction_freq = 50 + (i % 100);
        tension += 0.2 * std::sin(2 * M_PI * contraction_freq * i / 1000.0);
        
        // Fatigue progressive
        tension *= (1.0 - i * 0.001);
        
        // Variabilité inter-électrode
        tension *= (0.8 + 0.4 * QRandomGenerator::global()->generateDouble());
        
        signalData[i] = tension;
    }
    
    return signalData;
}

void TestIntegration::verifyMiningQuality(const BioMining::Crypto::BitcoinMiner::MiningResult &result)
{
    // Vérifications de qualité pour mining bio-assisté
    QVERIFY(result.attempts > 0);
    QVERIFY(result.computeTime > 0.0);
    QVERIFY(result.signalContribution >= 0.0);
    QVERIFY(result.signalContribution <= 1.0);
    
    if (result.success) {
        QVERIFY(!result.nonce.isEmpty());
        QVERIFY(!result.hash.isEmpty());
        QCOMPARE(result.hash.length(), 64); // SHA-256 hex
        
        // Vérification que le nonce est numérique
        bool ok;
        result.nonce.toULongLong(&ok);
        QVERIFY(ok);
    }
    
    // Efficacité minimale attendue avec signaux biologiques
    double hashrate = result.attempts / result.computeTime;
    QVERIFY(hashrate > 50.0); // Au moins 50 H/s
    
    // Contribution biologique significative
    QVERIFY(result.signalContribution > 0.01); // Au moins 1%
}

void TestIntegration::testMEAToMinerPipeline()
{
    qDebug() << "Test pipeline MEA -> Miner";
    
    setupRealisticMEA();
    simulateLabConditions();
    
    QSignalSpy miningCompleteSpy(m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::miningComplete);
    QSignalSpy signalsAcquiredSpy(m_meaInterface, &BioMining::Bio::MEAInterface::signalsAcquired);
    
    // Démarrage du pipeline
    m_bitcoinMiner->startContinuousMining();
    m_meaInterface->startContinuousAcquisition(200); // 5 Hz
    
    // Attendre plusieurs cycles
    QTest::qWait(2000);
    
    // Vérifications
    QVERIFY(signalsAcquiredSpy.count() >= 8); // Au moins 8 acquisitions en 2s à 5Hz
    QVERIFY(miningCompleteSpy.count() >= 3); // Plusieurs mining complétés
    
    // Vérifier la qualité du dernier résultat
    if (miningCompleteSpy.count() > 0) {
        QList<QVariant> lastResult = miningCompleteSpy.takeLast();
        BioMining::Crypto::BitcoinMiner::MiningResult result = lastResult.at(0).value<BioMining::Crypto::BitcoinMiner::MiningResult>();
        verifyMiningQuality(result);
    }
    
    m_meaInterface->stopContinuousAcquisition();
    m_bitcoinMiner->stopContinuousMining();
}

void TestIntegration::testContinuousOperation()
{
    qDebug() << "Test opération continue";
    
    setupRealisticMEA();
    simulateLabConditions();
    
    QSignalSpy miningCompleteSpy(m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::miningComplete);
    QSignalSpy calibrationSpy(m_meaInterface, &BioMining::Bio::MEAInterface::calibrationChanged);
    QSignalSpy hashrateSpy(m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::hashRateUpdated);
    
    // Opération continue avec calibration adaptative
    m_bitcoinMiner->startContinuousMining();
    m_meaInterface->startContinuousAcquisition(100); // 10 Hz
    
    // Simulation d'une session de 5 secondes
    QElapsedTimer sessionTimer;
    sessionTimer.start();
    
    int calibrationAdjustments = 0;
    while (sessionTimer.elapsed() < 5000) {
        QTest::qWait(500);
        
        // Ajustement périodique de calibration (simule adaptation)
        if (calibrationAdjustments < 3) {
            double signalQuality = m_meaInterface->getSignalQuality();
            double adjustment = 1.0 + (signalQuality - 0.5) * 0.1;
            m_meaInterface->adjustCalibration(adjustment);
            calibrationAdjustments++;
        }
    }
    
    // Arrêt propre
    m_meaInterface->stopContinuousAcquisition();
    m_bitcoinMiner->stopContinuousMining();
    
    // Vérifications de l'opération continue
    QVERIFY(miningCompleteSpy.count() >= 3); // Plusieurs cycles complétés
    QVERIFY(calibrationSpy.count() >= 3); // Calibrations ajustées
    QVERIFY(hashrateSpy.count() >= 3); // Hashrate mis à jour
    
    // Statistiques finales
    QVERIFY(m_bitcoinMiner->getTotalAttempts() > 1000);
    QVERIFY(m_bitcoinMiner->getHashrate() > 0.0);
    
    qDebug() << "Session stats - Attempts:" << m_bitcoinMiner->getTotalAttempts()
             << "Successes:" << m_bitcoinMiner->getSuccessCount()
             << "Hashrate:" << m_bitcoinMiner->getHashrate();
}

void TestIntegration::testAdaptiveCalibration()
{
    qDebug() << "Test calibration adaptative";
    
    setupRealisticMEA();
    simulateLabConditions();
    
    QSignalSpy calibrationSpy(m_meaInterface, &BioMining::Bio::MEAInterface::calibrationChanged);
    QSignalSpy miningCompleteSpy(m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::miningComplete);
    
    // Démarrage avec calibration sous-optimale
    m_meaInterface->adjustCalibration(0.5); // Facteur faible
    
    double initialCalibration = m_meaInterface->getCalibration();
    
    // Plusieurs cycles d'apprentissage adaptatif
    for (int cycle = 0; cycle < 5; ++cycle) {
        // Acquisition et mining
        QVector<double> signalData = m_meaInterface->readSignals();
        BioMining::Crypto::BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(signalData);
        
        // Adaptation basée sur la performance
        double signalQuality = m_meaInterface->getSignalQuality();
        double performance = result.signalContribution;
        
        // Algorithme d'adaptation simple
        double adjustmentFactor = 1.0;
        if (performance > 0.7 && signalQuality > 0.6) {
            adjustmentFactor = 1.05; // Amélioration
        } else if (performance < 0.3 || signalQuality < 0.3) {
            adjustmentFactor = 0.95; // Dégradation
        }
        
        m_meaInterface->adjustCalibration(adjustmentFactor);
        
        qDebug() << "Cycle" << cycle << "- Quality:" << signalQuality 
                 << "Performance:" << performance 
                 << "Calibration:" << m_meaInterface->getCalibration();
        
        QTest::qWait(100);
    }
    
    double finalCalibration = m_meaInterface->getCalibration();
    
    // Vérifications de l'adaptation
    QVERIFY(calibrationSpy.count() >= 6); // 1 initial + 5 ajustements
    QVERIFY(miningCompleteSpy.count() == 5); // Un par cycle
    
    // La calibration devrait s'être améliorée
    QVERIFY(finalCalibration != initialCalibration);
    
    qDebug() << "Calibration evolution:" << initialCalibration << "->" << finalCalibration;
}

void TestIntegration::testFullWorkflowSimulation()
{
    qDebug() << "Test workflow complet";
    
    // === PHASE 1: INITIALISATION ===
    QVERIFY(m_meaInterface->initialize());
    
    // Chargement de configuration (simulé)
    QTemporaryFile configFile;
    configFile.open();
    m_meaInterface->saveCalibration(configFile.fileName());
    
    // === PHASE 2: APPRENTISSAGE INITIAL ===
    QSignalSpy stimulationSpy(m_meaInterface, &BioMining::Bio::MEAInterface::stimulationComplete);
    
    // Séquence d'apprentissage de 3 cycles
    for (int learningCycle = 0; learningCycle < 3; ++learningCycle) {
        // Pattern de stimulation adaptatif
        QVector<double> stimPattern(60);
        for (int i = 0; i < 60; ++i) {
            stimPattern[i] = 0.3 + 0.2 * std::sin(i * 0.1 + learningCycle * 0.5);
        }
        
        m_meaInterface->stimulate(stimPattern);
        QTest::qWait(200); // Temps de réponse biologique
        
        // Acquisition de la réponse
        QVector<double> response = m_meaInterface->readSignals();
        double quality = m_meaInterface->getSignalQuality();
        
        // Ajustement adaptatif
        double adjustment = 1.0 + (quality - 0.5) * 0.2;
        m_meaInterface->adjustCalibration(adjustment);
        
        qDebug() << "Learning cycle" << learningCycle << "- Quality:" << quality;
    }
    
    QVERIFY(stimulationSpy.count() == 3);
    
    // === PHASE 3: MINING OPÉRATIONNEL ===
    simulateLabConditions();
    
    QSignalSpy miningCompleteSpy(m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::miningComplete);
    
    // Session de mining avec différents types de signaux
    QVector<QVector<double>> signalTypes = {
        simulateNeuralActivity(1000),
        simulateMuscleTension(60),
        simulateNeuralActivity(1500)
    };
    
    QVector<BioMining::Crypto::BitcoinMiner::MiningResult> results;
    
    for (const auto &signalData : signalTypes) {
        BioMining::Crypto::BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(signalData);
        results.append(result);
        verifyMiningQuality(result);
        
        QTest::qWait(100);
    }
    
    QCOMPARE(results.size(), 3);
    QCOMPARE(miningCompleteSpy.count(), 3);
    
    // === PHASE 4: ANALYSE DES PERFORMANCES ===
    double avgHashrate = 0.0;
    double avgSignalContrib = 0.0;
    int successCount = 0;
    
    for (const auto &result : results) {
        avgHashrate += result.attempts / result.computeTime;
        avgSignalContrib += result.signalContribution;
        if (result.success) successCount++;
    }
    
    avgHashrate /= results.size();
    avgSignalContrib /= results.size();
    
    qDebug() << "Workflow results - Avg Hashrate:" << avgHashrate
             << "Avg Signal Contrib:" << avgSignalContrib
             << "Success rate:" << (successCount * 100.0 / results.size()) << "%";
    
    // Vérifications finales
    QVERIFY(avgHashrate > 100.0); // Performance minimale
    QVERIFY(avgSignalContrib > 0.1); // Contribution significative
    QVERIFY(successCount >= 1); // Au moins un succès
    
    // === PHASE 5: SAUVEGARDE ===
    m_meaInterface->saveCalibration(configFile.fileName());
}

void TestIntegration::testLongTermStability()
{
    qDebug() << "Test stabilité long terme";
    
    setupRealisticMEA();
    simulateLabConditions();
    
    QSignalSpy miningCompleteSpy(m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::miningComplete);
    QSignalSpy errorSpy(m_meaInterface, &BioMining::Bio::MEAInterface::errorOccurred);
    
    // Opération continue pour test de stabilité
    m_bitcoinMiner->startContinuousMining();
    m_meaInterface->startContinuousAcquisition(50); // 20 Hz
    
    // Simulation de 10 secondes d'opération continue
    QElapsedTimer stabilityTimer;
    stabilityTimer.start();
    
    int cycleCount = 0;
    QVector<double> hashrates;
    QVector<double> signalQualities;
    
    while (stabilityTimer.elapsed() < 10000) {
        QTest::qWait(500);
        cycleCount++;
        
        // Collecte des métriques de stabilité
        double currentHashrate = m_bitcoinMiner->getHashrate();
        double currentQuality = m_meaInterface->getSignalQuality();
        
        if (currentHashrate > 0.0) {
            hashrates.append(currentHashrate);
        }
        if (currentQuality > 0.0) {
            signalQualities.append(currentQuality);
        }
        
        // Simulation de perturbations occasionnelles
        if (cycleCount % 5 == 0) {
            m_meaInterface->adjustCalibration(1.02); // Micro-ajustement
        }
    }
    
    m_meaInterface->stopContinuousAcquisition();
    m_bitcoinMiner->stopContinuousMining();
    
    // Analyse de la stabilité
    QVERIFY(errorSpy.count() == 0); // Aucune erreur
    QVERIFY(miningCompleteSpy.count() >= 5); // Plusieurs cycles
    QVERIFY(hashrates.size() >= 5);
    QVERIFY(signalQualities.size() >= 5);
    
    // Calcul de la stabilité (coefficient de variation)
    double avgHashrate = std::accumulate(hashrates.begin(), hashrates.end(), 0.0) / hashrates.size();
    double avgQuality = std::accumulate(signalQualities.begin(), signalQualities.end(), 0.0) / signalQualities.size();
    
    double hashrateVariance = 0.0;
    double qualityVariance = 0.0;
    
    for (double hr : hashrates) {
        hashrateVariance += (hr - avgHashrate) * (hr - avgHashrate);
    }
    for (double q : signalQualities) {
        qualityVariance += (q - avgQuality) * (q - avgQuality);
    }
    
    hashrateVariance /= hashrates.size();
    qualityVariance /= signalQualities.size();
    
    double hashrateCV = std::sqrt(hashrateVariance) / avgHashrate;
    double qualityCV = std::sqrt(qualityVariance) / avgQuality;
    
    qDebug() << "Stability metrics - Hashrate CV:" << hashrateCV << "Quality CV:" << qualityCV;
    
    // La stabilité devrait être raisonnable (CV < 50%)
    QVERIFY(hashrateCV < 0.5);
    QVERIFY(qualityCV < 0.5);
}

void TestIntegration::testErrorRecovery()
{
    qDebug() << "Test récupération d'erreur";
    
    setupRealisticMEA();
    simulateLabConditions();
    
    QSignalSpy errorSpy(m_meaInterface, &BioMining::Bio::MEAInterface::errorOccurred);
    QSignalSpy statusSpy(m_meaInterface, &BioMining::Bio::MEAInterface::statusChanged);
    
    // === TEST 1: DÉCONNEXION/RECONNEXION ===
    QCOMPARE(m_meaInterface->getStatus(), MEAInterface::ConnectionStatus::Connected);
    
    m_meaInterface->disconnect();
    QCOMPARE(m_meaInterface->getStatus(), MEAInterface::ConnectionStatus::Disconnected);
    
    // Tentative de mining pendant déconnexion
    QVector<double> signalData = m_meaInterface->readSignals();
    QVERIFY(signalData.isEmpty());
    
    // Reconnexion
    QVERIFY(m_meaInterface->initialize());
    QCOMPARE(m_meaInterface->getStatus(), MEAInterface::ConnectionStatus::Connected);
    
    // Vérification fonctionnement normal après reconnexion
    signalData = m_meaInterface->readSignals();
    QVERIFY(!signalData.isEmpty());
    
    // === TEST 2: GESTION D'ERREUR DE STIMULATION ===
    QVector<double> invalidPattern(30, 0.5); // Mauvaise taille
    m_meaInterface->stimulate(invalidPattern);
    
    // Une erreur devrait être émise
    QVERIFY(errorSpy.count() > 0);
    
    // Le système devrait continuer à fonctionner
    QVector<double> validPattern(60, 0.3);
    QSignalSpy stimSpy(m_meaInterface, &BioMining::Bio::MEAInterface::stimulationComplete);
    m_meaInterface->stimulate(validPattern);
    QVERIFY(stimSpy.wait(1000));
    
    // === TEST 3: RÉCUPÉRATION DE CALIBRATION ===
    double originalCalibration = m_meaInterface->getCalibration();
    
    // Calibration extrême (devrait être bridée)
    m_meaInterface->adjustCalibration(100.0);
    QVERIFY(m_meaInterface->getCalibration() <= 5.0); // Maximum autorisé
    
    // Reset et vérification
    m_meaInterface->resetCalibration();
    QCOMPARE(m_meaInterface->getCalibration(), 1.0);
    
    qDebug() << "Error recovery tests completed - Errors caught:" << errorSpy.count();
}

void TestIntegration::testEndToEndPerformance()
{
    qDebug() << "Test performance end-to-end";
    
    setupRealisticMEA();
    simulateLabConditions();
    
    // Benchmark avec métriques détaillées
    QElapsedTimer benchmarkTimer;
    benchmarkTimer.start();
    
    // === MESURE: LATENCE ACQUISITION ===
    QElapsedTimer acquisitionTimer;
    acquisitionTimer.start();
    
    QVector<double> signalData = m_meaInterface->readSignals();
    qint64 acquisitionLatency = acquisitionTimer.elapsed();
    
    // === MESURE: TEMPS DE MINING ===
    QElapsedTimer miningTimer;
    miningTimer.start();
    
    BioMining::Crypto::BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(signalData);
    qint64 miningLatency = miningTimer.elapsed();
    
    // === MESURE: CYCLE COMPLET ===
    qint64 totalLatency = benchmarkTimer.elapsed();
    
    // Métriques de performance
    double hashrate = result.attempts / result.computeTime;
    double throughput = 1000.0 / totalLatency; // Cycles par seconde
    
    qDebug() << "Performance metrics:";
    qDebug() << " - Acquisition latency:" << acquisitionLatency << "ms";
    qDebug() << " - Mining latency:" << miningLatency << "ms";
    qDebug() << " - Total latency:" << totalLatency << "ms";
    qDebug() << " - Hashrate:" << hashrate << "H/s";
    qDebug() << " - Throughput:" << throughput << "cycles/s";
    
    // Vérifications de performance
    QVERIFY(acquisitionLatency < 100); // < 100ms pour acquisition
    QVERIFY(miningLatency < 5000); // < 5s pour mining
    QVERIFY(hashrate > 100.0); // > 100 H/s
    QVERIFY(throughput > 0.1); // > 0.1 cycles/s
    
    verifyMiningQuality(result);
}

void TestIntegration::testConcurrentOperations()
{
    qDebug() << "Test opérations concurrentes";
    
    setupRealisticMEA();
    simulateLabConditions();
    
    QSignalSpy acquisitionSpy(m_meaInterface, &BioMining::Bio::MEAInterface::signalsAcquired);
    QSignalSpy miningSpy(m_bitcoinMiner, &BioMining::Crypto::BitcoinMiner::miningComplete);
    
    // Démarrage des opérations concurrentes
    m_meaInterface->startContinuousAcquisition(100); // 10 Hz
    m_bitcoinMiner->startContinuousMining();
    
    // Opérations additionnelles pendant le fonctionnement
    for (int i = 0; i < 5; ++i) {
        QTest::qWait(200);
        
        // Calibrations concurrentes
        m_meaInterface->adjustCalibration(1.01);
        
        // Lecture directe pendant l'acquisition continue
        QVector<double> directSignals = m_meaInterface->readSignals();
        QVERIFY(!directSignals.isEmpty());
        
        // Stimulation pendant le mining
        QVector<double> stimPattern(60, 0.2);
        m_meaInterface->stimulate(stimPattern);
    }
    
    QTest::qWait(1000); // Laisser les opérations se terminer
    
    m_meaInterface->stopContinuousAcquisition();
    m_bitcoinMiner->stopContinuousMining();
    
    // Vérifications des opérations concurrentes
    QVERIFY(acquisitionSpy.count() >= 8); // Acquisition continue
    QVERIFY(miningSpy.count() >= 2); // Mining concurrent
    
    // Pas d'erreur de concurrence
    QCOMPARE(m_meaInterface->getStatus(), MEAInterface::ConnectionStatus::Connected);
    QVERIFY(m_bitcoinMiner->getHashrate() >= 0.0);
    
    qDebug() << "Concurrent operations - Acquisitions:" << acquisitionSpy.count()
             << "Mining cycles:" << miningSpy.count();
}

void TestIntegration::testResourceManagement()
{
    qDebug() << "Test gestion des ressources";
    
    // Test de création/destruction multiple
    for (int iteration = 0; iteration < 5; ++iteration) {
        MEAInterface *mea = new MEAInterface(this);
        BitcoinMiner *miner = new BitcoinMiner(this);
        
        // Connexion et utilisation
        QVERIFY(mea->initialize());
        
        BioMining::Crypto::BitcoinMiner::MiningConfig config = miner->getMiningConfig();
        config.maxAttempts = 500; // Rapide
        miner->setMiningConfig(config);
        
        QVector<double> testSignals = simulateNeuralActivity();
        BioMining::Crypto::BitcoinMiner::MiningResult result = miner->mine(testSignals);
        
        QVERIFY(result.attempts > 0);
        
        // Nettoyage explicite
        mea->disconnect();
        miner->stopMining();
        
        delete mea;
        delete miner;
        
        // Petit délai pour vérifier la libération des ressources
        QTest::qWait(50);
    }
    
    // Si on arrive ici sans crash/fuite, c'est bon signe
    QVERIFY(true);
}

void TestIntegration::testLaboratoryWorkflow()
{
    qDebug() << "Test workflow laboratoire";
    
    // Simulation d'une session de laboratoire réaliste
    setupRealisticMEA();
    
    // === ÉTAPE 1: CALIBRATION ÉLECTRODES ===
    QSignalSpy calibrationSpy(m_meaInterface, &BioMining::Bio::MEAInterface::calibrationChanged);
    
    // Test de chaque électrode (simulé rapide)
    for (int electrode = 0; electrode < 10; electrode += 10) { // Test par groupes
        QVector<double> testPattern(60, 0.0);
        for (int i = electrode; i < electrode + 10 && i < 60; ++i) {
            testPattern[i] = 0.5; // Stimulation sélective
        }
        
        m_meaInterface->stimulate(testPattern);
        QTest::qWait(100);
        
        QVector<double> response = m_meaInterface->readSignals();
        double quality = m_meaInterface->getSignalQuality();
        
        if (quality < 0.5) {
            m_meaInterface->adjustCalibration(1.1); // Amélioration
        }
    }
    
    // === ÉTAPE 2: PROTOCOLE EXPÉRIMENTAL ===
    simulateLabConditions();
    
    QVector<QString> experimentPhases = {
        "baseline", "stimulation", "recovery"
    };
    
    QVector<BioMining::Crypto::BitcoinMiner::MiningResult> phaseResults;
    
    for (const QString &phase : experimentPhases) {
        qDebug() << "Phase expérimentale:" << phase;
        
        QVector<double> phaseSignals;
        if (phase == "baseline") {
            phaseSignals = simulateNeuralActivity(500);
        } else if (phase == "stimulation") {
            phaseSignals = simulateNeuralActivity(1000);
            // Simulation d'activation
            for (double &signal : phaseSignals) {
                signal *= 1.5;
            }
        } else { // recovery
            phaseSignals = simulateNeuralActivity(800);
            // Simulation de retour à la normale
            for (double &signal : phaseSignals) {
                signal *= 0.8;
            }
        }
        
        BioMining::Crypto::BitcoinMiner::MiningResult phaseResult = m_bitcoinMiner->mine(phaseSignals);
        phaseResults.append(phaseResult);
        verifyMiningQuality(phaseResult);
        
        QTest::qWait(200); // Délai inter-phase
    }
    
    // === ÉTAPE 3: ANALYSE DES RÉSULTATS ===
    QCOMPARE(phaseResults.size(), 3);
    
    // Vérification de la cohérence expérimentale
    for (int i = 0; i < phaseResults.size(); ++i) {
        qDebug() << "Phase" << experimentPhases[i]
                 << "- Contrib:" << phaseResults[i].signalContribution
                 << "- Success:" << phaseResults[i].success;
    }
    
    // La phase de stimulation devrait avoir la plus haute contribution
    double baselineContrib = phaseResults[0].signalContribution;
    double stimulationContrib = phaseResults[1].signalContribution;
    double recoveryContrib = phaseResults[2].signalContribution;
    
    QVERIFY(stimulationContrib >= baselineContrib);
    QVERIFY(stimulationContrib >= recoveryContrib);
}

void TestIntegration::testBenchmarkMode()
{
    qDebug() << "Test mode benchmark";
    
    setupRealisticMEA();
    
    // Configuration benchmark
    BioMining::Crypto::BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 2000; // Standardisé pour benchmark
    config.difficulty = 0x0001FFFFFFFFFFFF; // Difficulté benchmark
    m_bitcoinMiner->setMiningConfig(config);
    
    // Série de tests benchmark standardisés
    QVector<QVector<double>> benchmarkSignals = {
        simulateNeuralActivity(1000),   // Neural standard
        simulateMuscleTension(60),      // EMG standard
        QVector<double>(60, 0.0),       // Signal null (référence)
        createPatternnedSignals(60),    // Pattern synthétique
        simulateNeuralActivity(2000)    // Neural long
    };
    
    QStringList benchmarkNames = {
        "Neural-1s", "EMG-Standard", "Null-Reference", "Synthetic", "Neural-2s"
    };
    
    QVector<BioMining::Crypto::BitcoinMiner::MiningResult> benchmarkResults;
    
    for (int i = 0; i < benchmarkSignals.size(); ++i) {
        qDebug() << "Benchmark test:" << benchmarkNames[i];
        
        QElapsedTimer benchTimer;
        benchTimer.start();
        
        BioMining::Crypto::BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(benchmarkSignals[i]);
        
        qint64 benchTime = benchTimer.elapsed();
        
        benchmarkResults.append(result);
        verifyMiningQuality(result);
        
        double hashrate = result.attempts / result.computeTime;
        
        qDebug() << " - Hashrate:" << hashrate << "H/s"
                 << "Contribution:" << result.signalContribution
                 << "Wall time:" << benchTime << "ms";
    }
    
    // Analyse comparative
    QCOMPARE(benchmarkResults.size(), 5);
    
    // Le signal null devrait avoir la plus faible contribution
    double nullContribution = benchmarkResults[2].signalContribution;
    for (int i = 0; i < benchmarkResults.size(); ++i) {
        if (i != 2) { // Pas le signal null
            QVERIFY(benchmarkResults[i].signalContribution >= nullContribution);
        }
    }
    
    // Cohérence des hashrates
    double avgHashrate = 0.0;
    for (const auto &result : benchmarkResults) {
        avgHashrate += result.attempts / result.computeTime;
    }
    avgHashrate /= benchmarkResults.size();
    
    QVERIFY(avgHashrate > 200.0); // Performance minimale benchmark
    
    qDebug() << "Benchmark complete - Average hashrate:" << avgHashrate << "H/s";
}

void TestIntegration::testMaintenanceMode()
{
    qDebug() << "Test mode maintenance";
    
    setupRealisticMEA();
    
    // === TEST DE DIAGNOSTIC ===
    
    // 1. Test de toutes les électrodes
    QVector<double> allElectrodes = m_meaInterface->readSignals();
    QCOMPARE(allElectrodes.size(), 60);
    
    int workingElectrodes = 0;
    for (double signal : allElectrodes) {
        if (!std::isnan(signal) && !std::isinf(signal)) {
            workingElectrodes++;
        }
    }
    
    double electrodeHealth = static_cast<double>(workingElectrodes) / 60.0;
    qDebug() << "Electrode health:" << (electrodeHealth * 100) << "%";
    
    QVERIFY(electrodeHealth > 0.8); // Au moins 80% d'électrodes fonctionnelles
    
    // 2. Test de qualité de signal
    QVector<double> qualityMeasures;
    for (int i = 0; i < 10; ++i) {
        m_meaInterface->readSignals();
        qualityMeasures.append(m_meaInterface->getSignalQuality());
        QTest::qWait(50);
    }
    
    double avgQuality = std::accumulate(qualityMeasures.begin(), qualityMeasures.end(), 0.0) / qualityMeasures.size();
    qDebug() << "Average signal quality:" << (avgQuality * 100) << "%";
    
    QVERIFY(avgQuality > 0.3); // Qualité minimale acceptable
    
    // 3. Test de calibration
    double originalCalibration = m_meaInterface->getCalibration();
    
    QVector<double> calibrationTest = {0.5, 1.0, 1.5, 2.0};
    for (double testCalib : calibrationTest) {
        m_meaInterface->resetCalibration();
        m_meaInterface->adjustCalibration(testCalib);
        
        double actualCalib = m_meaInterface->getCalibration();
        QVERIFY(std::abs(actualCalib - testCalib) < 0.01 || 
                actualCalib == 5.0 || actualCalib == 0.1); // Limites respectées
    }
    
    // 4. Test de performance mining
    BioMining::Crypto::BitcoinMiner::MiningConfig testConfig = m_bitcoinMiner->getMiningConfig();
    testConfig.maxAttempts = 1000; // Test rapide
    m_bitcoinMiner->setMiningConfig(testConfig);
    
    QVector<double> testSignals = simulateNeuralActivity();
    BioMining::Crypto::BitcoinMiner::MiningResult maintenanceResult = m_bitcoinMiner->mine(testSignals);
    
    verifyMiningQuality(maintenanceResult);
    
    double maintenanceHashrate = maintenanceResult.attempts / maintenanceResult.computeTime;
    qDebug() << "Maintenance hashrate:" << maintenanceHashrate << "H/s";
    
    QVERIFY(maintenanceHashrate > 100.0); // Performance minimale
    
    // === SAUVEGARDE D'ÉTAT ===
    QTemporaryFile maintenanceLog;
    maintenanceLog.open();
    
    QTextStream logStream(&maintenanceLog);
    logStream << "=== MAINTENANCE REPORT ===" << Qt::endl;
    logStream << "Electrode Health: " << (electrodeHealth * 100) << "%" << Qt::endl;
    logStream << "Average Signal Quality: " << (avgQuality * 100) << "%" << Qt::endl;
    logStream << "Mining Performance: " << maintenanceHashrate << " H/s" << Qt::endl;
    logStream << "System Status: " << (electrodeHealth > 0.8 && avgQuality > 0.3 ? "OK" : "ATTENTION") << Qt::endl;
    
    m_meaInterface->saveCalibration(maintenanceLog.fileName() + ".calib");
    
    qDebug() << "Maintenance report saved";
}

QVector<double> TestIntegration::createPatternnedSignals(int size)
{
    QVector<double> signalData(size);
    for (int i = 0; i < size; ++i) {
        signalData[i] = 0.5 * std::sin(i * 0.2) + 0.3 * std::cos(i * 0.1);
    }
    return signalData;
}

QTEST_MAIN(TestIntegration)
#include "test_integration.moc"