#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>

#include "bio/mea_interface.h"

using namespace BioMining::Bio;

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
        
        QVector<double> signalData = meaInterface.readSignals();
        
        if (signalData.isEmpty()) {
            qWarning() << "Aucun signal reçu au cycle" << cycle + 1;
            continue;
        }
        
        // Analyse basique des signaux
        double sum = 0.0, minVal = signalData[0], maxVal = signalData[0];
        for (double signal : signalData) {
            sum += signal;
            if (signal < minVal) minVal = signal;
            if (signal > maxVal) maxVal = signal;
        }
        
        double average = sum / signalData.size();
        double quality = meaInterface.getSignalQuality();
        
        qDebug() << QString("Cycle %1 - Électrodes: %2, Moyenne: %3, Min: %4, Max: %5, Qualité: %6%, Temps: %7ms")
                    .arg(cycle + 1)
                    .arg(signalData.size())
                    .arg(average, 0, 'f', 4)
                    .arg(minVal, 0, 'f', 4)
                    .arg(maxVal, 0, 'f', 4)
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
    
    // Envoi de la stimulation
    meaInterface.stimulate(stimulationPattern);
    qDebug() << "Stimulation envoyée avec succès!";
    
    // Attente courte pour la completion
    QThread::msleep(200);
    
    // === ÉTAPE 5: LECTURE POST-STIMULATION ===
    qDebug() << "\n5. Lecture de la réponse post-stimulation...";
    
    QVector<double> responseSignalData = meaInterface.readSignals();
    if (!responseSignalData.isEmpty()) {
        double responseQuality = meaInterface.getSignalQuality();
        
        double responseSum = 0.0;
        for (double signal : responseSignalData) {
            responseSum += signal;
        }
        double responseAvg = responseSum / responseSignalData.size();
        
        qDebug() << QString("Réponse - Moyenne: %1, Qualité: %2%")
                    .arg(responseAvg, 0, 'f', 4)
                    .arg(responseQuality * 100, 0, 'f', 1);
    }
    
    // === ÉTAPE 6: CALIBRATION ADAPTATIVE ===
    qDebug() << "\n6. Test de calibration adaptative...";
    
    double originalCalibration = meaInterface.getCalibration();
    qDebug() << "Calibration originale:" << originalCalibration;
    
    // Ajustements progressifs basés sur la qualité
    for (int adjustment = 0; adjustment < 3; ++adjustment) {
        QVector<double> testSignalData = meaInterface.readSignals();
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
    
    qDebug() << "\n=== Test MEA terminé avec succès! ===";
    
    return 0;
}