/**
 * @file test_triple_mining.cpp
 * @brief Programme de test pour le système de mining triple hybride révolutionnaire
 * 
 * Ce programme teste l'intégration des trois systèmes :
 * - SHA-256 traditionnel
 * - Réseau biologique (BiologicalNetwork) 
 * - Neurones biologiques réels (RealMEAInterface)
 * 
 * Avec fusion intelligente, apprentissage croisé et optimisation dynamique.
 */

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QDateTime>
#include "crypto/hybrid_bitcoin_miner.h"
#include "bio/real_mea_interface.h"
#include "bio/biological_network.h"

using namespace BioMining::HCrypto;
using namespace BioMining::Bio;
using namespace BioMining::Network;

class TripleSystemTester : public QObject
{
    Q_OBJECT

public:
    explicit TripleSystemTester(QObject *parent = nullptr)
        : QObject(parent)
        , m_hybridMiner(new HybridBitcoinMiner(this))
        , m_realMea(std::make_shared<RealMEAInterface>())
    {
        // Connexions des signaux pour monitoring
        connect(m_hybridMiner, &HybridBitcoinMiner::triplePredictionReady,
                this, &TripleSystemTester::onTriplePrediction);
        connect(m_hybridMiner, &HybridBitcoinMiner::systemWeightsUpdated,
                this, &TripleSystemTester::onWeightsUpdated);
        connect(m_hybridMiner, &HybridBitcoinMiner::optimalMethodSelected,
                this, &TripleSystemTester::onMethodSelected);
        connect(m_hybridMiner, &HybridBitcoinMiner::crossLearningCompleted,
                this, &TripleSystemTester::onCrossLearning);
        connect(m_hybridMiner, &HybridBitcoinMiner::blockFound,
                this, &TripleSystemTester::onBlockFound);
        connect(m_hybridMiner, &HybridBitcoinMiner::errorOccurred,
                this, &TripleSystemTester::onError);
    }

public slots:
    void runTripleSystemTest()
    {
        qDebug() << "\n=== TEST SYSTÈME TRIPLE RÉVOLUTIONNAIRE ===";
        qDebug() << "🔥 SHA-256 + Réseau Biologique + Neurones MEA Réels 🔥\n";
        
        // 1. Initialisation des composants
        if (!initializeComponents()) {
            qCritical() << "❌ Échec d'initialisation";
            QCoreApplication::exit(1);
            return;
        }
        
        // 2. Configuration du système triple
        if (!configureTripleSystem()) {
            qCritical() << "❌ Échec configuration système triple";
            QCoreApplication::exit(1);
            return;
        }
        
        // 3. Tests de prédiction
        qDebug() << "🧠 Début des tests de prédiction triple...";
        testTriplePredictions();
        
        // 4. Test d'apprentissage croisé
        QTimer::singleShot(3000, this, &TripleSystemTester::testCrossLearning);
        
        // 5. Test d'optimisation dynamique
        QTimer::singleShot(6000, this, &TripleSystemTester::testDynamicOptimization);
        
        // 6. Mining complet
        QTimer::singleShot(9000, this, &TripleSystemTester::testFullMining);
        
        // 7. Arrêt automatique
        QTimer::singleShot(15000, []() {
            qDebug() << "\n🎯 FIN DU TEST SYSTÈME TRIPLE";
            QCoreApplication::quit();
        });
    }

private slots:
    void onTriplePrediction(const TripleSystemPrediction& prediction)
    {
        qDebug() << QString("🔮 PRÉDICTION TRIPLE:");
        qDebug() << QString("   SHA256: %1 (conf: %2%)")
                    .arg(prediction.sha256Nonce)
                    .arg(prediction.sha256Confidence * 100, 0, 'f', 1);
        qDebug() << QString("   Network: %1 (conf: %2%)")
                    .arg(prediction.networkNonce)
                    .arg(prediction.networkConfidence * 100, 0, 'f', 1);
        qDebug() << QString("   MEA: %1 (conf: %2%)")
                    .arg(prediction.meaNonce)
                    .arg(prediction.meaConfidence * 100, 0, 'f', 1);
        qDebug() << QString("   🎯 FUSION: %1 (conf: %2%)")
                    .arg(prediction.fusedNonce)
                    .arg(prediction.fusedConfidence * 100, 0, 'f', 1);
    }
    
    void onWeightsUpdated(double sha256Weight, double networkWeight, double meaWeight)
    {
        qDebug() << QString("⚖️ POIDS OPTIMISÉS - SHA256: %1%, Network: %2%, MEA: %3%")
                    .arg(sha256Weight * 100, 0, 'f', 1)
                    .arg(networkWeight * 100, 0, 'f', 1)
                    .arg(meaWeight * 100, 0, 'f', 1);
    }
    
    void onMethodSelected(MiningMethod method, double confidence)
    {
        QString methodName;
        switch (method) {
            case MiningMethod::TraditionalSHA256: methodName = "SHA-256"; break;
            case MiningMethod::BiologicalNetwork: methodName = "Réseau Bio"; break;
            case MiningMethod::RealMEANeurons: methodName = "MEA Réel"; break;
            case MiningMethod::HybridFusion: methodName = "Fusion Hybride"; break;
        }
        
        qDebug() << QString("🎯 MÉTHODE SÉLECTIONNÉE: %1 (conf: %2%)")
                    .arg(methodName)
                    .arg(confidence * 100, 0, 'f', 1);
    }
    
    void onCrossLearning(bool success, double improvement)
    {
        qDebug() << QString("🔄 APPRENTISSAGE CROISÉ: %1 (amélioration: %2%)")
                    .arg(success ? "SUCCÈS" : "ÉCHEC")
                    .arg(improvement * 100, 0, 'f', 2);
    }
    
    void onBlockFound(const QString& blockHash, uint32_t nonce, double biologicalContribution)
    {
        qDebug() << QString("🏆 BLOC TROUVÉ ! Hash: %1, Nonce: %2, Bio: %3%")
                    .arg(blockHash.left(16))
                    .arg(nonce)
                    .arg(biologicalContribution * 100, 0, 'f', 1);
    }
    
    void onError(const QString& error)
    {
        qWarning() << QString("⚠️ ERREUR: %1").arg(error);
    }
    
    void testTriplePredictions()
    {
        qDebug() << "\n🧪 TEST DE PRÉDICTIONS MULTIPLES:";
        
        QStringList testHeaders = {
            "block_header_test_1_revolutionary_mining",
            "block_header_test_2_biological_neurons", 
            "block_header_test_3_mea_integration",
            "block_header_test_4_hybrid_fusion",
            "block_header_test_5_cross_learning"
        };
        
        for (int i = 0; i < testHeaders.size(); ++i) {
            QTimer::singleShot(i * 500, [this, testHeaders, i]() {
                qDebug() << QString("\n📋 Test %1/5: %2").arg(i+1).arg(testHeaders[i]);
                
                TripleSystemPrediction prediction = m_hybridMiner->predictNonceTriple(testHeaders[i]);
                
                // Validation simulée (30% de succès)
                bool success = (QRandomGenerator::global()->bounded(100) < 30);
                
                QTimer::singleShot(200, [this, prediction, success]() {
                    m_hybridMiner->validateTriplePrediction(prediction, success);
                });
            });
        }
    }
    
    void testCrossLearning()
    {
        qDebug() << "\n🔄 TEST APPRENTISSAGE CROISÉ:";
        m_hybridMiner->performCrossSystemLearning();
    }
    
    void testDynamicOptimization()
    {
        qDebug() << "\n⚡ TEST OPTIMISATION DYNAMIQUE:";
        m_hybridMiner->optimizeSystemWeights();
        
        // Affichage des métriques
        HybridMiningMetrics metrics = m_hybridMiner->getMetrics();
        qDebug() << QString("📊 MÉTRIQUES - Total: %1, Bio: %2, Accuracy: %3%")
                    .arg(metrics.totalHashes.load())
                    .arg(metrics.biologicalPredictions.load())
                    .arg(metrics.biologicalAccuracy.load() * 100, 0, 'f', 1);
    }
    
    void testFullMining()
    {
        qDebug() << "\n⛏️ TEST MINING COMPLET:";
        
        if (!m_hybridMiner->startHybridMining()) {
            qCritical() << "❌ Échec démarrage mining";
            return;
        }
        
        qDebug() << "✅ Mining hybride triple démarré !";
        
        // Arrêt du mining après 5 secondes
        QTimer::singleShot(5000, [this]() {
            m_hybridMiner->stopHybridMining();
            qDebug() << "⏹️ Mining arrêté";
        });
    }

private:
    bool initializeComponents()
    {
        qDebug() << "🔧 Initialisation des composants...";
        
        // 1. Initialisation HybridBitcoinMiner de base
        if (!m_hybridMiner->initialize()) {
            qCritical() << "Échec initialisation HybridBitcoinMiner";
            return false;
        }
        
        // 2. Configuration MEA réel (simulation si hardware absent)
        RealMEAConfig meaConfig;
        meaConfig.deviceType = MEADeviceType::Custom_Serial;
        meaConfig.electrodeCount = 60;
        meaConfig.samplingRate = 25000.0;
        meaConfig.autoCalibration = false; // Skip pour test rapide
        
        if (!m_realMea->initialize(meaConfig)) {
            qWarning() << "MEA hardware non disponible, mode simulation";
        }
        
        // 3. Connexion RealMEA au système hybride
        if (!m_hybridMiner->connectToRealMEA(m_realMea)) {
            qCritical() << "Échec connexion RealMEA";
            return false;
        }
        
        qDebug() << "✅ Composants initialisés";
        return true;
    }
    
    bool configureTripleSystem()
    {
        qDebug() << "⚙️ Configuration système triple...";
        
        // Configuration apprentissage biologique
        BiologicalLearningParams learningParams;
        learningParams.initialLearningRate = 0.02;
        learningParams.retroLearningRate = 0.01;
        learningParams.maxIterations = 200;
        learningParams.enablePlasticity = true;
        learningParams.enableAdaptation = true;
        
        if (!m_hybridMiner->configureBiologicalNetwork(learningParams)) {
            qCritical() << "Échec configuration réseau biologique";
            return false;
        }
        
        // Configuration optimisation triple
        TripleOptimizationConfig tripleConfig;
        tripleConfig.sha256Weight = 0.4;
        tripleConfig.networkWeight = 0.3; 
        tripleConfig.meaWeight = 0.3;
        tripleConfig.adaptationRate = 0.05;
        tripleConfig.enableDynamicWeighting = true;
        tripleConfig.enableCrossLearning = true;
        tripleConfig.preferHighConfidence = true;
        
        if (!m_hybridMiner->initializeTripleSystem(tripleConfig)) {
            qCritical() << "Échec initialisation système triple";
            return false;
        }
        
        // Configuration apprentissage triple
        if (!m_hybridMiner->configureTripleSystemLearning()) {
            qCritical() << "Échec configuration apprentissage triple";
            return false;
        }
        
        qDebug() << "✅ Système triple configuré";
        return true;
    }

    HybridBitcoinMiner* m_hybridMiner;
    std::shared_ptr<RealMEAInterface> m_realMea;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("TripleSystemMiningTest");
    app.setApplicationVersion("1.0");
    
    qDebug() << "🚀🧠⚡ TEST SYSTÈME TRIPLE RÉVOLUTIONNAIRE ⚡🧠🚀";
    qDebug() << "==================================================";
    qDebug() << "• SHA-256 Traditionnel + Réseau Biologique + MEA Réel";
    qDebug() << "• Fusion Intelligente + Apprentissage Croisé";
    qDebug() << "• Optimisation Dynamique + Sélection Adaptative";
    qDebug() << "==================================================\n";
    
    TripleSystemTester tester;
    
    // Démarrer le test
    QTimer::singleShot(1000, &tester, &TripleSystemTester::runTripleSystemTest);
    
    return app.exec();
}

#include "test_triple_mining.moc"