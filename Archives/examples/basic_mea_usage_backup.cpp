#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>

#include "bio/mea_interface.h"

/**
 * @brief Exemple basique d'utilisation de l'interface MEA
 * 
 * Cet exemple montre comment:
 * - Initialiser l'interface MEA
 * - Lire des signaux biologiques
 * - Effectuer des stimulations
 * - Gérer la calibration
 */

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Exemple d'utilisation MEA basique ===";
    
    // Création de l'interface MEA
    MEAInterface meaInterface;
    
    // === ÉTAPE 1: INITIALISATION ===
    qDebug() << "\n1. Initialisation de l'interface MEA...";
    
    if (!meaInterface.initialize()) {
        qCritical() << "Erreur lors de l'initialisation MEA:" << meaInterface.getLastError();
        return 1;
    }
    
    qDebug() << "MEA initialisé avec succès!";
    qDebug() << "Statut:" << static_cast<int>(meaInterface.getStatus());
    qDebug() << "Calibration initiale:" << meaInterface.getCalibration();
    
    // === ÉTAPE 2: LECTURE DE SIGNAUX ===
    qDebug() << "\n2. Lecture de signaux biologiques...";
    
    for (int cycle = 0; cycle < 3; ++cycle) {
        QElapsedTimer timer;
        timer.start();
        
        QVector<double> signals = meaInterface.readSignals();
        
        if (signals.isEmpty()) {
            qWarning() << "Aucun signal reçu au cycle" << cycle + 1;
            continue;
        }
        
        // Analyse basique des signaux
        double sum = 0.0, min = signals[0], max = signals[0];
        for (double signal : signals) {
            sum += signal;
            if (signal < min) min = signal;
            if (signal > max) max = signal;
        }
        
        double average = sum / signals.size();
        double quality = meaInterface.getSignalQuality();
        
        qDebug() << QString("Cycle %1 - Électrodes: %2, Moyenne: %3, Min: %4, Max: %5, Qualité: %6%, Temps: %7ms")
                    .arg(cycle + 1)
                    .arg(signals.size())
                    .arg(average, 0, 'f', 4)
                    .arg(min, 0, 'f', 4)
                    .arg(max, 0, 'f', 4)
                    .arg(quality * 100, 0, 'f', 1)
                    .arg(timer.elapsed());
        
        QThread::msleep(500); // Délai entre les lectures
    }
    
    // === ÉTAPE 3: CONFIGURATION DES PARAMÈTRES DE STIMULATION ===
    qDebug() << "\n3. Configuration des paramètres de stimulation...";
    
    meaInterface.setStimulationParameters(
        2.0,    // Amplitude: 2.0V
        150.0,  // Durée: 150ms  
        40.0    // Fréquence: 40Hz
    );
    
    qDebug() << "Paramètres de stimulation configurés (2.0V, 150ms, 40Hz)";
    
    // === ÉTAPE 4: STIMULATION ===
    qDebug() << "\n4. Test de stimulation...";
    
    // Création d'un pattern de stimulation
    QVector<double> stimulationPattern(60);
    for (int i = 0; i < 60; ++i) {
        // Pattern sinusoïdal avec variation spatiale
        stimulationPattern[i] = 0.5 * (1.0 + sin(i * 0.1)) * 0.8;
    }
    
    // Connexion au signal de completion
    QObject::connect(&meaInterface, &MEAInterface::stimulationComplete, [&]() {
        qDebug() << "Stimulation terminée avec succès!";
    });
    
    // Envoi de la stimulation
    meaInterface.stimulate(stimulationPattern);
    
    // Attente courte pour la completion
    QTimer::singleShot(200, [&]() {
        // Lecture de la réponse après stimulation
        qDebug() << "\n5. Lecture de la réponse post-stimulation...";
        
        QVector<double> responseSignals = meaInterface.readSignals();
        if (!responseSignals.isEmpty()) {
            double responseQuality = meaInterface.getSignalQuality();
            
            double responseSum = 0.0;
            for (double signal : responseSignals) {
                responseSum += signal;
            }
            double responseAvg = responseSum / responseSignals.size();
            
            qDebug() << QString("Réponse - Moyenne: %1, Qualité: %2%")
                        .arg(responseAvg, 0, 'f', 4)
                        .arg(responseQuality * 100, 0, 'f', 1);
        }
        
        // === ÉTAPE 6: CALIBRATION ADAPTATIVE ===
        qDebug() << "\n6. Test de calibration adaptative...";
        
        double originalCalibration = meaInterface.getCalibration();
        qDebug() << "Calibration originale:" << originalCalibration;
        
        // Connexion au signal de calibration
        QObject::connect(&meaInterface, &MEAInterface::calibrationChanged, [](double newFactor) {
            qDebug() << "Calibration mise à jour:" << newFactor;
        });
        
        // Ajustements progressifs basés sur la qualité
        for (int adjustment = 0; adjustment < 3; ++adjustment) {
            QVector<double> testSignals = meaInterface.readSignals();
            double currentQuality = meaInterface.getSignalQuality();
            
            // Algorithme d'ajustement simple
            double adjustmentFactor = 1.0;
            if (currentQuality > 0.7) {
                adjustmentFactor = 1.05; // Amélioration légère
            } else if (currentQuality < 0.3) {
                adjustmentFactor = 0.95; // Réduction
            } else {
                adjustmentFactor = 1.02; // Ajustement minimal
            }
            
            meaInterface.adjustCalibration(adjustmentFactor);
            
            qDebug() << QString("Ajustement %1 - Qualité: %2%, Facteur: %3, Nouvelle calibration: %4")
                        .arg(adjustment + 1)
                        .arg(currentQuality * 100, 0, 'f', 1)
                        .arg(adjustmentFactor, 0, 'f', 3)
                        .arg(meaInterface.getCalibration(), 0, 'f', 3);
            
            QThread::msleep(300);
        }
        
        // === ÉTAPE 7: ACQUISITION CONTINUE ===
        qDebug() << "\n7. Test d'acquisition continue...";
        
        QObject::connect(&meaInterface, &MEAInterface::signalsAcquired, [](const QVector<double> &signals) {
            static int acquisitionCount = 0;
            acquisitionCount++;
            
            if (acquisitionCount <= 5) { // Log seulement les 5 premières
                double avg = 0.0;
                for (double signal : signals) {
                    avg += signal;
                }
                avg /= signals.size();
                
                qDebug() << QString("Acquisition continue #%1 - Moyenne: %2")
                            .arg(acquisitionCount)
                            .arg(avg, 0, 'f', 4);
            }
        });
        
        // Démarrage de l'acquisition continue (5 Hz)
        meaInterface.startContinuousAcquisition(200);
        qDebug() << "Acquisition continue démarrée (5 Hz)...";
        
        // Laisser tourner 2 secondes
        QTimer::singleShot(2000, [&]() {
            meaInterface.stopContinuousAcquisition();
            qDebug() << "Acquisition continue arrêtée.";
            
            // === ÉTAPE 8: SAUVEGARDE DE CALIBRATION ===
            qDebug() << "\n8. Sauvegarde de la calibration...";
            
            QString calibrationFile = "example_mea_calibration.json";
            meaInterface.saveCalibration(calibrationFile);
            qDebug() << "Calibration sauvegardée dans:" << calibrationFile;
            
            // Test de rechargement
            meaInterface.resetCalibration();
            qDebug() << "Calibration réinitialisée à:" << meaInterface.getCalibration();
            
            if (meaInterface.loadCalibration(calibrationFile)) {
                qDebug() << "Calibration rechargée avec succès:" << meaInterface.getCalibration();
            } else {
                qWarning() << "Erreur lors du rechargement:" << meaInterface.getLastError();
            }
            
            // === FINALISATION ===
            qDebug() << "\n=== Exemple terminé ===";
            qDebug() << "Déconnexion de l'interface MEA...";
            
            meaInterface.disconnect();
            qDebug() << "Statut final:" << static_cast<int>(meaInterface.getStatus());
            
            app.quit();
        });
    });
    
    return app.exec();
}