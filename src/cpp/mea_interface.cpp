#include "bio/mea_interface.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

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
    
    QVector<double> signals(ELECTRODE_COUNT);
    
    // Simulation de lecture de signaux biologiques
    // Génération de signaux avec caractéristiques biologiques réalistes
    for (int i = 0; i < ELECTRODE_COUNT; ++i) {
        // Signal de base avec bruit biologique
        double baseSignal = QRandomGenerator::global()->generateDouble() - 0.5;
        
        // Ajout de composantes fréquentielles biologiques
        double time = QDateTime::currentMSecsSinceEpoch() / 1000.0;
        double bioComponent = 0.1 * std::sin(2 * M_PI * 10.0 * time + i * 0.1); // ~10Hz neural activity
        
        // Application de la calibration
        signals[i] = (baseSignal + bioComponent) * m_calibrationFactor;
        
        // Simulation d'artéfacts occasionnels
        if (QRandomGenerator::global()->generateDouble() < 0.05) {
            signals[i] *= 2.0; // Spike occasionnel
        }
    }
    
    if (!validateSignals(signals)) {
        setError("Signaux invalides détectés");
        return QVector<double>();
    }
    
    m_lastSignals = signals;
    return signals;
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
    
    QVector<double> signals = readSignals();
    if (!signals.isEmpty()) {
        emit signalsAcquired(signals);
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

bool MEAInterface::validateSignals(const QVector<double> &signals) const
{
    if (signals.size() != ELECTRODE_COUNT) {
        return false;
    }
    
    // Vérification de valeurs aberrantes
    for (double signal : signals) {
        if (std::isnan(signal) || std::isinf(signal) || std::abs(signal) > 100.0) {
            return false;
        }
    }
    
    return true;
}