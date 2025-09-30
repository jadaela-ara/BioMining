#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QJsonObject>
#include "../include/bio/real_mea_interface.h"

using namespace BioMining::Bio;

/**
 * @brief Exemple d'utilisation de l'interface MEA réelle
 * 
 * Cet exemple démontre :
 * - Configuration et connexion à un dispositif MEA réel
 * - Acquisition de données en temps réel
 * - Stimulation contrôlée des électrodes
 * - Calibration et tests d'impédance
 * - Détection de spikes
 */

class RealMEAExample : public QObject
{
    Q_OBJECT

public:
    explicit RealMEAExample(QObject *parent = nullptr) 
        : QObject(parent)
        , m_meaInterface(std::make_unique<RealMEAInterface>(this))
        , m_acquisitionCount(0)
        , m_maxAcquisitions(50)
    {
        setupConnections();
    }

public slots:
    void run() {
        qDebug() << "=== Interface MEA Réelle - Exemple Complet ===";
        qDebug() << "";
        
        // 1. Détecter les dispositifs disponibles
        detectAvailableDevices();
        
        // 2. Configurer et connecter
        if (!configureAndConnect()) {
            qDebug() << "❌ Échec de la configuration, arrêt du programme";
            QCoreApplication::quit();
            return;
        }
        
        // 3. Effectuer les tests de base
        performBasicTests();
    }

private slots:
    void onStatusChanged(RealMEAInterface::ConnectionStatus status) {
        QString statusStr;
        switch (status) {
            case RealMEAInterface::ConnectionStatus::Disconnected:
                statusStr = "Disconnected"; break;
            case RealMEAInterface::ConnectionStatus::Connecting:
                statusStr = "Connecting"; break;
            case RealMEAInterface::ConnectionStatus::Connected:
                statusStr = "Connected"; break;
            case RealMEAInterface::ConnectionStatus::Streaming:
                statusStr = "Streaming"; break;
            case RealMEAInterface::ConnectionStatus::Calibrating:
                statusStr = "Calibrating"; break;
            case RealMEAInterface::ConnectionStatus::Error:
                statusStr = "Error"; break;
        }
        
        qDebug() << "[STATUS] MEA:" << statusStr;
    }
    
    void onElectrodeDataReady(const QVector<ElectrodeData> &data) {
        ++m_acquisitionCount;
        
        if (m_acquisitionCount <= 10 || m_acquisitionCount % 10 == 0) {
            // Analyser les données
            double avgVoltage = 0.0;
            double avgQuality = 0.0;
            int activeCount = 0;
            
            for (const ElectrodeData &electrode : data) {
                if (electrode.isActive) {
                    avgVoltage += electrode.voltage;
                    avgQuality += electrode.signalQuality;
                    ++activeCount;
                }
            }
            
            if (activeCount > 0) {
                avgVoltage /= activeCount;
                avgQuality /= activeCount;
            }
            
            qDebug() << QString("[DATA %1] %2 électrodes actives, Voltage moyen: %3 mV, Qualité: %4%")
                        .arg(m_acquisitionCount, 3)
                        .arg(activeCount)
                        .arg(avgVoltage * 1000, 0, 'f', 2)
                        .arg(avgQuality * 100, 0, 'f', 1);
        }
        
        // Arrêter après un certain nombre d'acquisitions
        if (m_acquisitionCount >= m_maxAcquisitions) {
            m_meaInterface->stopContinuousAcquisition();
            
            QTimer::singleShot(1000, this, &RealMEAExample::performStimulationTest);
        }
    }
    
    void onSpikeDetected(const SpikeEvent &spike) {
        qDebug() << QString("[SPIKE] Électrode %1: %2 μV à t=%3μs")
                    .arg(spike.electrodeId)
                    .arg(spike.amplitude)
                    .arg(spike.timestamp);
    }
    
    void onErrorOccurred(const QString &error) {
        qDebug() << "[ERROR]" << error;
    }
    
    void onCalibrationProgress(int electrode, double progress) {
        if (electrode == 0 || electrode % 10 == 0) {
            qDebug() << QString("[CALIBRATION] Électrode %1, Progrès: %2%")
                        .arg(electrode)
                        .arg(progress * 100, 0, 'f', 1);
        }
    }
    
    void onCalibrationComplete() {
        qDebug() << "[CALIBRATION] Calibration terminée avec succès!";
        
        // Afficher les résultats d'impédance
        QVector<double> impedances = m_meaInterface->getElectrodeImpedances();
        
        double avgImpedance = 0.0;
        int validCount = 0;
        
        for (int i = 0; i < impedances.size(); ++i) {
            if (impedances[i] > 0 && impedances[i] < 1e9) { // Valeurs valides
                avgImpedance += impedances[i];
                ++validCount;
                
                if (i % 10 == 0) { // Afficher chaque 10ème électrode
                    qDebug() << QString("  Électrode %1: %2 MΩ")
                                .arg(i, 2)
                                .arg(impedances[i] / 1e6, 0, 'f', 2);
                }
            }
        }
        
        if (validCount > 0) {
            avgImpedance /= validCount;
            qDebug() << QString("[CALIBRATION] Impédance moyenne: %1 MΩ (%2 électrodes valides)")
                        .arg(avgImpedance / 1e6, 0, 'f', 2)
                        .arg(validCount);
        }
        
        // Passer au test d'acquisition
        QTimer::singleShot(500, this, &RealMEAExample::startAcquisitionTest);
    }

private:
    void setupConnections() {
        connect(m_meaInterface.get(), &RealMEAInterface::statusChanged,
                this, &RealMEAExample::onStatusChanged);
        connect(m_meaInterface.get(), &RealMEAInterface::electrodeDataReady,
                this, &RealMEAExample::onElectrodeDataReady);
        connect(m_meaInterface.get(), &RealMEAInterface::spikeDetected,
                this, &RealMEAExample::onSpikeDetected);
        connect(m_meaInterface.get(), &RealMEAInterface::errorOccurred,
                this, &RealMEAExample::onErrorOccurred);
        connect(m_meaInterface.get(), &RealMEAInterface::calibrationProgress,
                this, &RealMEAExample::onCalibrationProgress);
        connect(m_meaInterface.get(), &RealMEAInterface::calibrationComplete,
                this, &RealMEAExample::onCalibrationComplete);
    }
    
    void detectAvailableDevices() {
        qDebug() << "🔍 Détection des dispositifs MEA disponibles...";
        
        QStringList supportedDevices = MEAInterfaceFactory::getSupportedDevices();
        qDebug() << "📋 Dispositifs supportés:";
        for (int i = 0; i < supportedDevices.size(); ++i) {
            qDebug() << QString("  %1. %2").arg(i + 1).arg(supportedDevices[i]);
        }
        
        // Test de disponibilité pour quelques dispositifs courants
        QStringList testPaths = {"/dev/ttyUSB0", "/dev/ttyACM0", "/dev/cu.usbserial", "COM1", "COM2"};
        
        qDebug() << "";
        qDebug() << "🔌 Test de disponibilité:";
        for (const QString &path : testPaths) {
            bool available = MEAInterfaceFactory::isDeviceAvailable(MEADeviceType::Custom_Serial, path);
            qDebug() << QString("  %1: %2").arg(path).arg(available ? "✅ Disponible" : "❌ Non disponible");
        }
        qDebug() << "";
    }
    
    bool configureAndConnect() {
        qDebug() << "⚙️  Configuration de l'interface MEA...";
        
        // Configuration pour dispositif série personnalisé
        RealMEAConfig config;
        config.deviceType = MEADeviceType::Custom_Serial;
        config.protocol = CommunicationProtocol::SerialPort;
        config.devicePath = "/dev/ttyUSB0";  // Ajuster selon votre système
        config.baudRate = 115200;
        config.electrodeCount = 60;
        config.samplingRate = 25000.0;
        config.amplification = 1200.0;
        config.filterLowCut = 300.0;
        config.filterHighCut = 8000.0;
        config.bufferSize = 1024;
        config.autoCalibration = true;
        config.spikeDetection = true;
        config.spikeThreshold = -50.0;
        config.connectionTimeoutMs = 5000;
        config.readTimeoutMs = 1000;
        
        // Configurer les électrodes actives (toutes par défaut)
        config.activeElectrodes.clear();
        for (int i = 0; i < config.electrodeCount; ++i) {
            config.activeElectrodes.append(i);
        }
        
        qDebug() << "📊 Configuration:";
        qDebug() << QString("  Dispositif: %1").arg(config.devicePath);
        qDebug() << QString("  Électrodes: %1").arg(config.electrodeCount);
        qDebug() << QString("  Échantillonnage: %1 Hz").arg(config.samplingRate);
        qDebug() << QString("  Amplification: %1x").arg(config.amplification);
        qDebug() << QString("  Calibration auto: %1").arg(config.autoCalibration ? "Oui" : "Non");
        
        // Tentative de connexion
        qDebug() << "";
        qDebug() << "🔗 Connexion au MEA...";
        
        if (m_meaInterface->initialize(config)) {
            qDebug() << "✅ Connexion réussie!";
            qDebug() << "📋 Informations du dispositif:" << m_meaInterface->getDeviceInfo();
            return true;
        } else {
            qDebug() << "❌ Connexion échouée:" << m_meaInterface->getLastError();
            qDebug() << "";
            qDebug() << "💡 Suggestions:";
            qDebug() << "  - Vérifiez que le dispositif MEA est connecté";
            qDebug() << "  - Vérifiez le chemin du dispositif (/dev/ttyUSB0, COM1, etc.)";
            qDebug() << "  - Vérifiez les permissions (sudo usermod -a -G dialout $USER)";
            qDebug() << "  - Vérifiez que le taux de bauds est correct";
            return false;
        }
    }
    
    void performBasicTests() {
        qDebug() << "";
        qDebug() << "🧪 Tests de base du MEA...";
        
        // Test 1: Lecture simple des données
        qDebug() << "📊 Test 1: Lecture des données d'électrodes...";
        QVector<ElectrodeData> data = m_meaInterface->readElectrodeData();
        
        if (!data.isEmpty()) {
            qDebug() << QString("✅ %1 électrodes lues avec succès").arg(data.size());
            
            // Afficher quelques électrodes d'exemple
            for (int i = 0; i < qMin(5, data.size()); ++i) {
                const ElectrodeData &electrode = data[i];
                qDebug() << QString("  Électrode %1: %2 mV, Qualité: %3%")
                            .arg(electrode.electrodeId, 2)
                            .arg(electrode.voltage * 1000, 0, 'f', 2)
                            .arg(electrode.signalQuality * 100, 0, 'f', 1);
            }
        } else {
            qDebug() << "❌ Aucune donnée lue";
        }
        
        // Test 2: Vérification de la qualité des signaux
        qDebug() << "";
        qDebug() << "📊 Test 2: Qualité des signaux...";
        double overallQuality = m_meaInterface->getSignalQuality();
        qDebug() << QString("  Qualité globale: %1%").arg(overallQuality * 100, 0, 'f', 1);
        
        if (overallQuality > 0.7) {
            qDebug() << "✅ Qualité des signaux excellente";
        } else if (overallQuality > 0.5) {
            qDebug() << "⚠️  Qualité des signaux correcte";
        } else {
            qDebug() << "❌ Qualité des signaux faible - vérifiez les connexions";
        }
        
        // Démarrer le test d'acquisition continue (la calibration sera faite automatiquement si configurée)
        if (m_meaInterface->getStatus() == RealMEAInterface::ConnectionStatus::Connected) {
            QTimer::singleShot(1000, this, &RealMEAExample::startAcquisitionTest);
        }
    }
    
    void startAcquisitionTest() {
        qDebug() << "";
        qDebug() << "🔄 Test 3: Acquisition continue...";
        qDebug() << QString("  Acquisition de %1 échantillons...").arg(m_maxAcquisitions);
        
        // Activer la détection de spikes
        m_meaInterface->enableSpikeDetection(true);
        m_meaInterface->setSpikeThreshold(-50.0);
        
        // Démarrer l'acquisition continue
        if (m_meaInterface->startContinuousAcquisition()) {
            qDebug() << "✅ Acquisition continue démarrée";
        } else {
            qDebug() << "❌ Impossible de démarrer l'acquisition continue";
            QTimer::singleShot(1000, this, &RealMEAExample::performStimulationTest);
        }
    }
    
    void performStimulationTest() {
        qDebug() << "";
        qDebug() << "⚡ Test 4: Stimulation des électrodes...";
        
        // Test de stimulation sur quelques électrodes
        QVector<int> testElectrodes = {0, 10, 20, 30, 40, 50};
        
        for (int electrodeId : testElectrodes) {
            if (electrodeId < m_meaInterface->getActiveElectrodes().size()) {
                qDebug() << QString("  Stimulation électrode %1...").arg(electrodeId);
                
                // Stimulation douce : 0.1V pendant 100ms
                bool success = m_meaInterface->stimulateElectrode(electrodeId, 0.1, 100.0);
                
                if (success) {
                    qDebug() << QString("  ✅ Électrode %1 stimulée").arg(electrodeId);
                } else {
                    qDebug() << QString("  ❌ Échec stimulation électrode %1").arg(electrodeId);
                }
                
                // Attendre un peu entre les stimulations
                QThread::msleep(200);
            }
        }
        
        // Test de pattern de stimulation
        qDebug() << "";
        qDebug() << "🎭 Test de pattern de stimulation...";
        
        QVector<double> pattern(60, 0.0);
        // Créer un pattern sinusoïdal
        for (int i = 0; i < pattern.size(); ++i) {
            pattern[i] = 0.05 * std::sin(2 * M_PI * i / pattern.size()); // 50mV amplitude
        }
        
        if (m_meaInterface->stimulatePattern(pattern)) {
            qDebug() << "✅ Pattern de stimulation appliqué avec succès";
        } else {
            qDebug() << "❌ Échec d'application du pattern de stimulation";
        }
        
        // Finaliser les tests
        QTimer::singleShot(2000, this, &RealMEAExample::finalizeTests);
    }
    
    void finalizeTests() {
        qDebug() << "";
        qDebug() << "📊 Résumé des tests MEA:";
        qDebug() << "========================";
        
        // Statistiques d'acquisition
        qDebug() << QString("  Échantillons acquis: %1").arg(m_acquisitionCount);
        
        // Statistiques de spikes
        QVector<SpikeEvent> recentSpikes = m_meaInterface->getRecentSpikes(100);
        qDebug() << QString("  Spikes détectés: %1").arg(recentSpikes.size());
        
        // Status du système
        QJsonObject systemStatus = m_meaInterface->getSystemStatus();
        qDebug() << "  Status système:";
        for (auto it = systemStatus.begin(); it != systemStatus.end(); ++it) {
            qDebug() << QString("    %1: %2").arg(it.key()).arg(it.value().toString());
        }
        
        qDebug() << "";
        qDebug() << "🎉 Tests MEA terminés avec succès!";
        qDebug() << "";
        qDebug() << "💡 Prochaines étapes possibles:";
        qDebug() << "  - Intégrer avec l'algorithme de mining BioMining";
        qDebug() << "  - Configurer l'enregistrement de données longue durée";
        qDebug() << "  - Optimiser les paramètres de stimulation";
        qDebug() << "  - Implémenter des protocoles de stimulation complexes";
        
        // Déconnexion propre
        QTimer::singleShot(2000, [this]() {
            m_meaInterface->disconnect();
            QTimer::singleShot(1000, QCoreApplication::instance(), &QCoreApplication::quit);
        });
    }

private:
    std::unique_ptr<RealMEAInterface> m_meaInterface;
    int m_acquisitionCount;
    int m_maxAcquisitions;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "BioMining Platform v1.0.0";
    qDebug() << "Interface MEA Réelle - Exemple Complet";
    qDebug() << "======================================";
    qDebug() << "";

    RealMEAExample example;
    
    // Démarrer l'exemple après le démarrage de la boucle d'événements
    QTimer::singleShot(100, &example, &RealMEAExample::run);

    return app.exec();
}

#include "real_mea_example.moc"