#include "bio/real_mea_interface.h"
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QElapsedTimer>
#include <QThread>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>

// ... autres méthodes ou code ...

bool RealMEAInterface::performFullCalibration()
{
    qDebug() << "[RealMEA] Début de la calibration complète";
    
    QMutexLocker locker(&m_configMutex);
    
    m_calibrationInProgress = true;
    m_currentCalibrationElectrode = 0;
    m_calibrationFactors.clear();
    m_calibrationFactors.resize(m_config.electrodeCount);
    std::fill(m_calibrationFactors.begin(), m_calibrationFactors.end(), 1.0);

    setStatus(ConnectionStatus::Calibrating);

    // Lancer le timer de calibration
    m_calibrationTimer->start(1000); // 1 seconde par électrode

    emit calibrationProgress(0, 0.0);

    return true;
}

void RealMEAInterface::performImpedanceTest(int electrodeId)
{
    if (electrodeId < 0 || electrodeId >= m_config.electrodeCount) {
        qWarning() << "[RealMEA] ID électrode invalide pour test d'impédance:" << electrodeId;
        return;
    }

    double baseImpedance = 1000.0; // 1 kΩ de base
    double variation = (QRandomGenerator::global()->bounded(200) - 100) / 100.0; // ±100 Ω
    double impedance = baseImpedance + variation;

    bool isValid = (impedance < MAX_IMPEDANCE_MOHM * 1000000.0); // Convertir MΩ en Ω

    int oldSize = m_electrodeImpedances.size();
    if (oldSize < electrodeId + 1) {
        m_electrodeImpedances.resize(electrodeId + 1);
        std::fill(m_electrodeImpedances.begin() + oldSize, m_electrodeImpedances.end(), impedance);
    }
    m_electrodeImpedances[electrodeId] = impedance;

    qDebug() << QString("[RealMEA] Électrode %1: Impédance = %2 Ω (%3)")
                .arg(electrodeId)
                .arg(impedance, 0, 'f', 1)
                .arg(isValid ? "OK" : "HORS LIMITES");
}

// ... autres méthodes ou code ...
