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
// ... reste du fichier inchangé ...

// --- Correction dans performFullCalibration ---
m_calibrationFactors.resize(m_config.electrodeCount);
std::fill(m_calibrationFactors.begin(), m_calibrationFactors.end(), 1.0);

// --- Correction dans performImpedanceTest ---
int oldSize = m_electrodeImpedances.size();
if (oldSize < electrodeId + 1) {
    m_electrodeImpedances.resize(electrodeId + 1);
    std::fill(m_electrodeImpedances.begin() + oldSize, m_electrodeImpedances.end(), impedance);
}
m_electrodeImpedances[electrodeId] = impedance;
// ... reste du fichier inchangé ...
