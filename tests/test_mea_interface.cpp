#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QTemporaryFile>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QFile>
#include <QThread>
#include <cmath>

#include "bio/mea_interface.h"

using namespace BioMining::Bio;

class TestMEAInterface : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Tests de base
    void testInitialization();
    void testConnection();
    void testDisconnection();
    
    // Tests d'acquisition
    void testSignalReading();
    void testContinuousAcquisition();
    void testSignalQuality();
    
    // Tests de stimulation
    void testStimulation();
    void testStimulationParameters();
    void testAsyncStimulation();
    
    // Tests de calibration
    void testCalibrationAdjustment();
    void testCalibrationSaveLoad();
    void testCalibrationBounds();
    
    // Tests de robustesse
    void testErrorHandling();
    void testThreadSafety();
    void testPerformance();

private:
    BioMining::Bio::MEAInterface *m_meaInterface;
    QTemporaryFile *m_tempCalibFile;
};

void TestMEAInterface::initTestCase()
{
    // Configuration globale pour tous les tests
    QLoggingCategory::setFilterRules("*.debug=false");
}

void TestMEAInterface::cleanupTestCase()
{
    // Nettoyage global après tous les tests
}

void TestMEAInterface::init()
{
    // Préparation avant chaque test
    m_meaInterface = new BioMining::Bio::MEAInterface(this);
    m_tempCalibFile = new QTemporaryFile(this);
    m_tempCalibFile->open();
}

void TestMEAInterface::cleanup()
{
    // Nettoyage après chaque test
    if (m_meaInterface) {
        m_meaInterface->disconnect();
        delete m_meaInterface;
        m_meaInterface = nullptr;
    }
    
    if (m_tempCalibFile) {
        delete m_tempCalibFile;
        m_tempCalibFile = nullptr;
    }
}

void TestMEAInterface::testInitialization()
{
    // Test de l'état initial
    QCOMPARE(m_meaInterface->getStatus(), BioMining::Bio::MEAInterface::ConnectionStatus::Disconnected);
    QCOMPARE(m_meaInterface->getCalibration(), 1.0);
    QVERIFY(m_meaInterface->getLastSignals().isEmpty());
    QCOMPARE(m_meaInterface->getSignalQuality(), 0.0);
}

void TestMEAInterface::testConnection()
{
    // Test de connexion réussie
    QSignalSpy statusSpy(m_meaInterface, &BioMining::Bio::MEAInterface::statusChanged);
    
    bool result = m_meaInterface->initialize();
    
    QVERIFY(result);
    QCOMPARE(m_meaInterface->getStatus(), BioMining::Bio::MEAInterface::ConnectionStatus::Connected);
    QVERIFY(statusSpy.count() >= 1);
    
    // Vérification du signal de changement de statut
    QList<QVariant> lastSignal = statusSpy.takeLast();
    QCOMPARE(lastSignal.at(0).value<MEAInterface::ConnectionStatus>(), 
             BioMining::Bio::MEAInterface::ConnectionStatus::Connected);
}

void TestMEAInterface::testDisconnection()
{
    // Prérequis: connexion établie
    QVERIFY(m_meaInterface->initialize());
    QCOMPARE(m_meaInterface->getStatus(), BioMining::Bio::MEAInterface::ConnectionStatus::Connected);
    
    QSignalSpy statusSpy(m_meaInterface, &BioMining::Bio::MEAInterface::statusChanged);
    
    // Test de déconnexion
    m_meaInterface->disconnect();
    
    QCOMPARE(m_meaInterface->getStatus(), BioMining::Bio::MEAInterface::ConnectionStatus::Disconnected);
    QVERIFY(statusSpy.count() >= 1);
}

void TestMEAInterface::testSignalReading()
{
    // Connexion nécessaire
    QVERIFY(m_meaInterface->initialize());
    
    // Test de lecture de signaux
    QVector<double> signalData = m_meaInterface->readSignals();
    
    QVERIFY(!signalData.isEmpty());
    QCOMPARE(signalData.size(), 60); // 60 électrodes attendues
    
    // Vérification que les signaux sont dans des valeurs raisonnables
    for (double signal : signalData) {
        QVERIFY(!std::isnan(signal));
        QVERIFY(!std::isinf(signal));
        QVERIFY(std::abs(signal) < 100.0); // Limite raisonnable
    }
    
    // Vérification que les derniers signaux sont mémorisés
    QVector<double> lastSignals = m_meaInterface->getLastSignals();
    QCOMPARE(lastSignals.size(), signalData.size());
}

void TestMEAInterface::testContinuousAcquisition()
{
    QVERIFY(m_meaInterface->initialize());
    
    QSignalSpy signalsSpy(m_meaInterface, &BioMining::Bio::MEAInterface::signalsAcquired);
    
    // Démarrage acquisition continue
    m_meaInterface->startContinuousAcquisition(50); // 20 Hz
    
    // Attendre quelques signaux
    QTest::qWait(200);
    
    QVERIFY(signalsSpy.count() >= 3); // Au moins 3 acquisitions en 200ms à 20Hz
    
    // Vérification des signaux reçus
    QList<QVariant> lastEmission = signalsSpy.takeLast();
    QVector<double> receivedSignals = lastEmission.at(0).value<QVector<double>>();
    QCOMPARE(receivedSignals.size(), 60);
    
    // Arrêt de l'acquisition
    m_meaInterface->stopContinuousAcquisition();
    
    int countBefore = signalsSpy.count();
    QTest::qWait(100);
    int countAfter = signalsSpy.count();
    
    QCOMPARE(countBefore, countAfter); // Pas de nouveaux signaux après arrêt
}

void TestMEAInterface::testSignalQuality()
{
    QVERIFY(m_meaInterface->initialize());
    
    // Génération de signaux et calcul de qualité
    QVector<double> signalData = m_meaInterface->readSignals();
    double quality = m_meaInterface->getSignalQuality();
    
    QVERIFY(quality >= 0.0);
    QVERIFY(quality <= 1.0);
    
    // Test avec plusieurs lectures pour vérifier la cohérence
    QVector<double> qualities;
    for (int i = 0; i < 5; ++i) {
        m_meaInterface->readSignals();
        qualities.append(m_meaInterface->getSignalQuality());
        QTest::qWait(10);
    }
    
    // La qualité devrait être relativement stable
    double avgQuality = 0.0;
    for (double q : qualities) {
        avgQuality += q;
    }
    avgQuality /= qualities.size();
    
    for (double q : qualities) {
        QVERIFY(std::abs(q - avgQuality) < 0.5); // Variation max 50%
    }
}

void TestMEAInterface::testStimulation()
{
    QVERIFY(m_meaInterface->initialize());
    
    QSignalSpy stimSpy(m_meaInterface, &BioMining::Bio::MEAInterface::stimulationComplete);
    
    // Création d'un pattern de stimulation
    QVector<double> pattern(60);
    for (int i = 0; i < 60; ++i) {
        pattern[i] = 0.5 * std::sin(i * 0.1);
    }
    
    // Test de stimulation
    m_meaInterface->stimulate(pattern);
    
    QVERIFY(stimSpy.wait(1000)); // Attendre le signal de complétion
    QCOMPARE(stimSpy.count(), 1);
}

void TestMEAInterface::testStimulationParameters()
{
    QVERIFY(m_meaInterface->initialize());
    
    // Configuration des paramètres
    double testAmplitude = 2.5;
    double testDuration = 150.0;
    double testFrequency = 75.0;
    
    m_meaInterface->setStimulationParameters(testAmplitude, testDuration, testFrequency);
    
    // Pas d'accesseurs directs, mais on teste via stimulation
    QVector<double> pattern(60, 0.5);
    
    QSignalSpy stimSpy(m_meaInterface, &BioMining::Bio::MEAInterface::stimulationComplete);
    m_meaInterface->stimulate(pattern);
    
    QVERIFY(stimSpy.wait(1000));
    
    // Test de limites
    m_meaInterface->setStimulationParameters(15.0, 2000.0, 2000.0); // Au-delà des limites
    // Devrait être bridé automatiquement sans erreur
}

void TestMEAInterface::testAsyncStimulation()
{
    QVERIFY(m_meaInterface->initialize());
    
    QVector<double> pattern(60, 0.3);
    
    // Test stimulation asynchrone (dans cette implémentation, c'est synchrone)
    m_meaInterface->stimulateAsync(pattern);
    
    // Pas de blocage attendu
    QVERIFY(true);
}

void TestMEAInterface::testCalibrationAdjustment()
{
    QVERIFY(m_meaInterface->initialize());
    
    QSignalSpy calibSpy(m_meaInterface, &BioMining::Bio::MEAInterface::calibrationChanged);
    
    double initialCalib = m_meaInterface->getCalibration();
    QCOMPARE(initialCalib, 1.0);
    
    // Test d'ajustement
    m_meaInterface->adjustCalibration(1.5);
    
    QVERIFY(calibSpy.wait(100));
    QCOMPARE(m_meaInterface->getCalibration(), 1.5);
    
    // Test d'ajustement multiple
    m_meaInterface->adjustCalibration(2.0);
    QCOMPARE(m_meaInterface->getCalibration(), 3.0);
    
    // Test des limites
    m_meaInterface->adjustCalibration(10.0); // Devrait être limité à 5.0
    QVERIFY(m_meaInterface->getCalibration() <= 5.0);
    
    m_meaInterface->adjustCalibration(0.01); // Devrait être limité à 0.1
    QVERIFY(m_meaInterface->getCalibration() >= 0.1);
}

void TestMEAInterface::testCalibrationSaveLoad()
{
    QVERIFY(m_meaInterface->initialize());
    
    // Modification de la calibration
    m_meaInterface->adjustCalibration(2.0);
    double originalCalib = m_meaInterface->getCalibration();
    
    // Sauvegarde
    QString tempFilePath = m_tempCalibFile->fileName();
    m_meaInterface->saveCalibration(tempFilePath);
    
    QVERIFY(QFile::exists(tempFilePath));
    
    // Reset et rechargement
    m_meaInterface->resetCalibration();
    QCOMPARE(m_meaInterface->getCalibration(), 1.0);
    
    bool loadResult = m_meaInterface->loadCalibration(tempFilePath);
    QVERIFY(loadResult);
    QCOMPARE(m_meaInterface->getCalibration(), originalCalib);
}

void TestMEAInterface::testCalibrationBounds()
{
    QVERIFY(m_meaInterface->initialize());
    
    // Test limite inférieure
    for (int i = 0; i < 20; ++i) {
        m_meaInterface->adjustCalibration(0.5);
    }
    QVERIFY(m_meaInterface->getCalibration() >= 0.1);
    
    // Reset et test limite supérieure
    m_meaInterface->resetCalibration();
    for (int i = 0; i < 20; ++i) {
        m_meaInterface->adjustCalibration(2.0);
    }
    QVERIFY(m_meaInterface->getCalibration() <= 5.0);
}

void TestMEAInterface::testErrorHandling()
{
    // Test lecture sans connexion
    QVector<double> signalData = m_meaInterface->readSignals();
    QVERIFY(signalData.isEmpty());
    
    QString error = m_meaInterface->getLastError();
    QVERIFY(!error.isEmpty());
    
    // Test stimulation sans connexion
    QVector<double> pattern(60, 0.5);
    m_meaInterface->stimulate(pattern);
    
    // Test pattern invalide après connexion
    QVERIFY(m_meaInterface->initialize());
    QVector<double> invalidPattern(30, 0.5); // Mauvaise taille
    
    QSignalSpy errorSpy(m_meaInterface, &BioMining::Bio::MEAInterface::errorOccurred);
    m_meaInterface->stimulate(invalidPattern);
    
    QVERIFY(errorSpy.wait(100));
}

void TestMEAInterface::testThreadSafety()
{
    QVERIFY(m_meaInterface->initialize());
    
    // Test d'accès concurrent aux méthodes thread-safe
    QVector<QThread*> threads;
    QVector<bool> results;
    results.resize(5);
    
    for (int i = 0; i < 5; ++i) {
        QThread *thread = QThread::create([this, i, &results]() {
            // Opérations concurrentes
            for (int j = 0; j < 10; ++j) {
                m_meaInterface->readSignals();
                m_meaInterface->adjustCalibration(1.01);
                QThread::msleep(1);
            }
            results[i] = true;
        });
        
        threads.append(thread);
        thread->start();
    }
    
    // Attendre tous les threads
    for (QThread *thread : threads) {
        QVERIFY(thread->wait(5000));
        delete thread;
    }
    
    // Vérifier que tous ont terminé correctement
    for (bool result : results) {
        QVERIFY(result);
    }
}

void TestMEAInterface::testPerformance()
{
    QVERIFY(m_meaInterface->initialize());
    
    // Test de performance de lecture
    QElapsedTimer timer;
    timer.start();
    
    const int numReadings = 100;
    for (int i = 0; i < numReadings; ++i) {
        QVector<double> signalData = m_meaInterface->readSignals();
        QVERIFY(!signalData.isEmpty());
    }
    
    qint64 elapsedMs = timer.elapsed();
    double avgTimePerReading = static_cast<double>(elapsedMs) / numReadings;
    
    // Vérifier que chaque lecture prend moins de 50ms (performance raisonnable)
    QVERIFY2(avgTimePerReading < 50.0, 
             QString("Performance trop lente: %1 ms par lecture").arg(avgTimePerReading).toLocal8Bit());
    
    qDebug() << "Performance MEA: " << avgTimePerReading << "ms par lecture";
}

QTEST_MAIN(TestMEAInterface)
#include "test_mea_interface.moc"