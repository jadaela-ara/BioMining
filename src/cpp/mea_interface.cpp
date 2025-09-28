#include "bio/mea_interface.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

namespace BioMining {
namespace Bio {

MEAInterface::MEAInterface(QObject *parent)
    : QObject(parent)
    , m_status(ConnectionStatus::Disconnected)
    , m_calibrationFactor(1.0)
    , m_stimAmplitude(1.0)
    , m_stimDuration(100.0)
    , m_stimFrequency(50.0)
    , m_acquisitionTimer(std::make_unique<QTimer>(this))
    , m_continuousMode(false)
{
    connect(m_acquisitionTimer.get(), &QTimer::timeout, 
            this, &MEAInterface::onAcquisitionTimer);
    
    qDebug() << "[MEA] Interface initialisée";
}

MEAInterface::~MEAInterface()
{
    if (m_status == ConnectionStatus::Connected) {
        disconnect();
    }
}

bool MEAInterface::initialize(const QString &configPath)
{
    QMutexLocker locker(&m_mutex);
    
    setStatus(ConnectionStatus::Connecting);
    
    try {
        // Simulation d'initialisation hardware
        // Dans un vrai système, ici on connecterait à l'API MEA réelle
        
        if (!configPath.isEmpty()) {
            if (!loadCalibration(configPath)) {
                qWarning() << "[MEA] Impossible de charger la configuration:" << configPath;
            }
        }
        
        // Simulation d'un délai de connexion
        QThread::msleep(500);
        
        m_lastSignals.resize(ELECTRODE_COUNT);
        
        setStatus(ConnectionStatus::Connected);
        qDebug() << "[MEA] Connexion établie avec" << ELECTRODE_COUNT << "électrodes";
        
        return true;
        
    } catch (const std::exception &e) {
        setError(QString("Erreur d'initialisation: %1").arg(e.what()));
        setStatus(ConnectionStatus::Error);
        return false;
    }
}

void MEAInterface::disconnect()
{
    QMutexLocker locker(&m_mutex);
    
    stopContinuousAcquisition();
    
    // Simulation déconnexion hardware
    setStatus(ConnectionStatus::Disconnected);
    qDebug() << "[MEA] Déconnecté";
}

QVector<double> MEAInterface::readSignals()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != ConnectionStatus::Connected) {
        setError("MEA non connecté");
        return QVector<double>();
    }
    
    QVector<double> signalData(ELECTRODE_COUNT);
    
    // Simulation de lecture de signaux biologiques
    // Génération de signaux avec caractéristiques biologiques réalistes
    for (int i = 0; i < ELECTRODE_COUNT; ++i) {
        // Signal de base avec bruit biologique
        double baseSignal = QRandomGenerator::global()->generateDouble() - 0.5;
        
        // Ajout de composantes fréquentielles biologiques
        double time = QDateTime::currentMSecsSinceEpoch() / 1000.0;
        double bioComponent = 0.1 * std::sin(2 * M_PI * 10.0 * time + i * 0.1); // ~10Hz neural activity
        
        // Application de la calibration
        signalData[i] = (baseSignal + bioComponent) * m_calibrationFactor;
        
        // Simulation d'artéfacts occasionnels
        if (QRandomGenerator::global()->generateDouble() < 0.05) {
            signalData[i] *= 2.0; // Spike occasionnel
        }
    }
    
    if (!validateSignals(signalData)) {
        setError("Signaux invalides détectés");
        return QVector<double>();
    }
    
    m_lastSignals = signalData;
    return signalData;
}

QVector<double> MEAInterface::readSignalsAsync()
{
    // Version non-bloquante - délègue à un thread worker dans un vrai système
    return readSignals();
}

void MEAInterface::startContinuousAcquisition(int intervalMs)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != ConnectionStatus::Connected) {
        setError("MEA non connecté pour acquisition continue");
        return;
    }
    
    m_continuousMode = true;
    m_acquisitionTimer->start(intervalMs);
    qDebug() << "[MEA] Acquisition continue démarrée, intervalle:" << intervalMs << "ms";
}

void MEAInterface::stopContinuousAcquisition()
{
    QMutexLocker locker(&m_mutex);
    
    m_acquisitionTimer->stop();
    m_continuousMode = false;
    qDebug() << "[MEA] Acquisition continue arrêtée";
}

void MEAInterface::stimulate(const QVector<double> &pattern)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != ConnectionStatus::Connected) {
        setError("MEA non connecté pour stimulation");
        return;
    }
    
    if (pattern.size() != ELECTRODE_COUNT) {
        setError(QString("Pattern de stimulation invalide: %1 valeurs au lieu de %2")
                 .arg(pattern.size()).arg(ELECTRODE_COUNT));
        return;
    }
    
    // Simulation d'envoi de pattern de stimulation
    qDebug() << "[MEA] Stimulation envoyée - Amplitude:" << m_stimAmplitude 
             << "V, Durée:" << m_stimDuration << "ms, Fréquence:" << m_stimFrequency << "Hz";
    
    // Dans un vrai système, ici on enverrait les données à l'API hardware
    // hardware_api->sendStimulation(pattern, m_stimAmplitude, m_stimDuration, m_stimFrequency);
    
    emit stimulationComplete();
}

void MEAInterface::stimulateAsync(const QVector<double> &pattern)
{
    // Dans un vrai système, cette méthode utiliserait un thread séparé
    stimulate(pattern);
}

void MEAInterface::setStimulationParameters(double amplitude, double duration, double frequency)
{
    QMutexLocker locker(&m_mutex);
    
    m_stimAmplitude = qBound(0.1, amplitude, 10.0);
    m_stimDuration = qBound(1.0, duration, 1000.0);
    m_stimFrequency = qBound(1.0, frequency, 1000.0);
    
    qDebug() << "[MEA] Paramètres stimulation mis à jour - A:" << m_stimAmplitude 
             << "V, D:" << m_stimDuration << "ms, F:" << m_stimFrequency << "Hz";
}

void MEAInterface::adjustCalibration(double factor)
{
    QMutexLocker locker(&m_mutex);
    
    double newFactor = m_calibrationFactor * factor;
    m_calibrationFactor = qBound(MIN_CALIBRATION, newFactor, MAX_CALIBRATION);
    
    qDebug() << "[MEA] Calibration ajustée:" << m_calibrationFactor;
    emit calibrationChanged(m_calibrationFactor);
}

double MEAInterface::getCalibration() const
{
    QMutexLocker locker(&m_mutex);
    return m_calibrationFactor;
}

void MEAInterface::resetCalibration()
{
    QMutexLocker locker(&m_mutex);
    m_calibrationFactor = 1.0;
    qDebug() << "[MEA] Calibration réinitialisée";
    emit calibrationChanged(m_calibrationFactor);
}

void MEAInterface::saveCalibration(const QString &filepath)
{
    QMutexLocker locker(&m_mutex);
    
    QString path = filepath;
    if (path.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        path = dataDir + "/mea_calibration.json";
    }
    
    QJsonObject config;
    config["calibration_factor"] = m_calibrationFactor;
    config["stim_amplitude"] = m_stimAmplitude;
    config["stim_duration"] = m_stimDuration;
    config["stim_frequency"] = m_stimFrequency;
    config["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(config);
    
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qDebug() << "[MEA] Calibration sauvegardée:" << path;
    } else {
        setError("Impossible de sauvegarder la calibration: " + path);
    }
}

bool MEAInterface::loadCalibration(const QString &filepath)
{
    QMutexLocker locker(&m_mutex);
    
    QString path = filepath;
    if (path.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        path = dataDir + "/mea_calibration.json";
    }
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError("Impossible de charger la calibration: " + path);
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        setError("Fichier de calibration corrompu: " + error.errorString());
        return false;
    }
    
    QJsonObject config = doc.object();
    
    if (config.contains("calibration_factor")) {
        m_calibrationFactor = config["calibration_factor"].toDouble();
    }
    if (config.contains("stim_amplitude")) {
        m_stimAmplitude = config["stim_amplitude"].toDouble();
    }
    if (config.contains("stim_duration")) {
        m_stimDuration = config["stim_duration"].toDouble();
    }
    if (config.contains("stim_frequency")) {
        m_stimFrequency = config["stim_frequency"].toDouble();
    }
    
    qDebug() << "[MEA] Calibration chargée:" << path;
    emit calibrationChanged(m_calibrationFactor);
    return true;
}

QVector<double> MEAInterface::getLastSignals() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastSignals;
}

double MEAInterface::getSignalQuality() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_lastSignals.isEmpty()) {
        return 0.0;
    }
    
    // Calcul de qualité basé sur le rapport signal/bruit
    double mean = 0.0;
    for (double signal : m_lastSignals) {
        mean += signal;
    }
    mean /= m_lastSignals.size();
    
    double variance = 0.0;
    for (double signal : m_lastSignals) {
        variance += (signal - mean) * (signal - mean);
    }
    variance /= m_lastSignals.size();
    
    // Qualité inversement proportionnelle à la variance
    return std::max(0.0, 1.0 - variance);
}

QString MEAInterface::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

void MEAInterface::onAcquisitionTimer()
{
    if (!m_continuousMode || m_status != ConnectionStatus::Connected) {
        return;
    }
    
    QVector<double> signalData = readSignals();
    if (!signalData.isEmpty()) {
        emit signalsAcquired(signalData);
    }
}

void MEAInterface::setStatus(ConnectionStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void MEAInterface::setError(const QString &error)
{
    m_lastError = error;
    qWarning() << "[MEA ERROR]" << error;
    emit errorOccurred(error);
}

bool MEAInterface::validateSignals(const QVector<double> &signalData) const
{
    if (signalData.size() != ELECTRODE_COUNT) {
        return false;
    }
    
    // Vérification de valeurs aberrantes
    for (double signal : signalData) {
        if (std::isnan(signal) || std::isinf(signal) || std::abs(signal) > 100.0) {
            return false;
        }
    }
    
    return true;
}


// Implémentations des fonctions manquantes ajoutées pour corriger les erreurs de compilation

bool MEAInterface::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_status == ConnectionStatus::Connected;
}

bool MEAInterface::connectToDevice()
{
    QMutexLocker locker(&m_mutex);
    
    qDebug() << "[MEA] Tentative de connexion au périphérique MEA...";
    
    if (m_status == ConnectionStatus::Connected) {
        qDebug() << "[MEA] Déjà connecté";
        return true;
    }
    
    setStatus(ConnectionStatus::Connecting);
    
    // Simulation de connexion au périphérique MEA
    // Dans un système réel, ici on initialiserait la communication USB/série/Ethernet
    try {
        // Simulation d'une détection de périphérique
        QThread::msleep(100); // Simule le temps de connexion
        
        // Initialisation des paramètres par défaut
        m_calibrationFactor = 1.0;
        m_lastSignals.clear();
        m_lastSignals.resize(ELECTRODE_COUNT);
        
        // Test de communication basique
        QVector<double> testSignals = readSignals();
        if (testSignals.isEmpty()) {
            setError("Échec du test de communication avec le périphérique MEA");
            setStatus(ConnectionStatus::Error);
            return false;
        }
        
        setStatus(ConnectionStatus::Connected);
        qDebug() << "[MEA] Connexion établie avec succès";
        return true;
        
    } catch (...) {
        setError("Exception lors de la connexion au périphérique MEA");
        setStatus(ConnectionStatus::Error);
        return false;
    }
}

bool MEAInterface::startAcquisition()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != ConnectionStatus::Connected) {
        setError("Impossible de démarrer l'acquisition : périphérique non connecté");
        return false;
    }
    
    qDebug() << "[MEA] Démarrage de l'acquisition de données...";
    
    try {
        // Démarrage de l'acquisition continue
        m_continuousMode = true;
        
        // Configuration du timer d'acquisition (par défaut 10Hz = 100ms)
        int acquisitionInterval = 100; // ms
        if (!m_acquisitionTimer->isActive()) {
            m_acquisitionTimer->start(acquisitionInterval);
        }
        
        qDebug() << "[MEA] Acquisition démarrée avec intervalle de" << acquisitionInterval << "ms";
        return true;
        
    } catch (...) {
        setError("Exception lors du démarrage de l'acquisition");
        m_continuousMode = false;
        return false;
    }
}

void MEAInterface::stopAcquisition()
{
    QMutexLocker locker(&m_mutex);
    
    qDebug() << "[MEA] Arrêt de l'acquisition de données...";
    
    try {
        // Arrêt de l'acquisition continue
        m_continuousMode = false;
        
        // Arrêt du timer d'acquisition
        if (m_acquisitionTimer->isActive()) {
            m_acquisitionTimer->stop();
        }
        
        qDebug() << "[MEA] Acquisition arrêtée avec succès";
        
    } catch (...) {
        setError("Exception lors de l'arrêt de l'acquisition");
    }
}

QVector<double> MEAInterface::getCurrentElectrodeData()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != ConnectionStatus::Connected) {
        qWarning() << "[MEA] Impossible de lire les données : périphérique non connecté";
        return QVector<double>();
    }
    
    try {
        // Lecture des données actuelles des électrodes
        QVector<double> currentData;
        currentData.resize(ELECTRODE_COUNT);
        
        // Simulation de lecture des données réelles
        // Dans un système réel, ici on lirait directement depuis le hardware MEA
        for (int i = 0; i < ELECTRODE_COUNT; ++i) {
            // Génération de signaux biologiques simulés avec:
            // - Activité de base (bruit thermique)
            // - Signaux neuronaux simulés
            // - Artefacts occasionnels
            
            double baseActivity = QRandomGenerator::global()->generateDouble() * 0.1 - 0.05; // ±50µV
            double neuronalSignal = 0.0;
            
            // 20% de chance d'activité neuronale sur chaque électrode
            if (QRandomGenerator::global()->generateDouble() < 0.2) {
                // Spike neuronal typique (amplitude 50-500µV, durée ~1ms)
                double spikeAmplitude = (QRandomGenerator::global()->generateDouble() * 0.45 + 0.05); // 50-500µV
                neuronalSignal = spikeAmplitude * (QRandomGenerator::global()->generateDouble() > 0.5 ? 1.0 : -1.0);
            }
            
            // Application de la calibration
            double rawSignal = (baseActivity + neuronalSignal) * m_calibrationFactor;
            
            // Filtrage basique (simulation d'un filtre passe-bande 300Hz-3kHz)
            currentData[i] = rawSignal;
        }
        
        // Validation des signaux
        if (!validateSignals(currentData)) {
            qWarning() << "[MEA] Données d'électrodes invalides détectées";
            return QVector<double>();
        }
        
        // Mise à jour du cache des derniers signaux
        m_lastSignals = currentData;
        
        // Émission du signal pour les listeners
        emit signalsAcquired(currentData);
        
        // Émission du signal de compatibilité std::vector
        std::vector<double> stdData(currentData.begin(), currentData.end());
        emit dataReady(stdData);
        
        return currentData;
        
    } catch (...) {
        setError("Exception lors de la lecture des données d'électrodes");
        return QVector<double>();
    }
}

} // namespace Bio
} // namespace BioMining