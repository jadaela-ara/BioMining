#include "crypto/bio_entropy_generator.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>
#include <QDateTime>
#include <QJsonObject>
#include <cstring>

namespace BioMining {
namespace Crypto {

BioEntropyGenerator::BioEntropyGenerator(QObject *parent)
    : QObject(parent)
{
    qDebug() << "[BioEntropyGenerator] Générateur d'entropie biologique créé";
}

BioEntropyGenerator::~BioEntropyGenerator()
{
    qDebug() << "[BioEntropyGenerator] Générateur détruit";
}

BioEntropyGenerator::BlockHeaderFeatures BioEntropyGenerator::extractHeaderFeatures(
    const QString &blockHeader, uint64_t difficulty)
{
    BlockHeaderFeatures features;
    
    // Parser le header (format: version|prevHash|merkleRoot|timestamp|bits|nonce)
    QStringList parts = blockHeader.split('|');
    
    if (parts.size() < 5) {
        qWarning() << "[BioEntropyGenerator] Format de header invalide";
        return features;
    }
    
    QString prevHash = parts.size() > 1 ? parts[1] : "";
    QString merkleRoot = parts.size() > 2 ? parts[2] : "";
    uint32_t timestamp = parts.size() > 3 ? parts[3].toUInt() : 0;
    
    // 1. Timestamp normalisé [0, 1]
    // Utiliser une plage raisonnable (2009-2030)
    const uint32_t MIN_TIMESTAMP = 1230768000; // 2009-01-01
    const uint32_t MAX_TIMESTAMP = 1893456000; // 2030-01-01
    features.timestampNorm = static_cast<double>(timestamp - MIN_TIMESTAMP) / 
                             (MAX_TIMESTAMP - MIN_TIMESTAMP);
    features.timestampNorm = qBound(0.0, features.timestampNorm, 1.0);
    
    // 2. Difficulty level (échelle log)
    features.difficultyLevel = std::log10(static_cast<double>(difficulty + 1));
    
    // 3. Entropy du previous hash
    features.prevHashEntropy = calculateShannonEntropy(prevHash);
    
    // 4. Leading zeros
    features.prevHashLeadingZeros = countLeadingZeros(prevHash);
    
    // 5. Entropy du merkle root
    features.merkleEntropy = calculateShannonEntropy(merkleRoot);
    
    // 6. Bytes du previous hash (premiers 20)
    features.prevHashBytes = hexToNormalizedBytes(prevHash, 20);
    
    // 7. Bytes du merkle root (premiers 20)
    features.merkleBytes = hexToNormalizedBytes(merkleRoot, 20);
    
    // 8. Version normalisée
    uint32_t version = parts.size() > 0 ? parts[0].toUInt() : 1;
    features.versionNorm = static_cast<double>(version) / 4.0; // Versions 1-4
    
    // 9. Difficulty bits normalisé
    uint32_t diffBits = parts.size() > 4 ? parts[4].toUInt() : 0;
    features.difficultyBitsNorm = static_cast<double>(diffBits) / 0xFFFFFFFF;
    
    qDebug() << QString("[BioEntropyGenerator] Features extraites: entropy=%1, zeros=%2, diff=%3")
                .arg(features.prevHashEntropy)
                .arg(features.prevHashLeadingZeros)
                .arg(features.difficultyLevel);
    
    return features;
}

BioEntropyGenerator::StimulusPattern BioEntropyGenerator::featuresToStimulus(
    const BlockHeaderFeatures &features, double maxVoltage)
{
    StimulusPattern pattern;
    pattern.amplitudes.resize(60);
    pattern.frequencies.resize(60);
    pattern.durationMs = 100; // Durée standard
    
    // Distribuer les features sur 60 électrodes
    int idx = 0;
    
    // Électrodes 0-9: Difficulty et timestamp
    for (int i = 0; i < 10 && idx < 60; ++i, ++idx) {
        double value = (i % 2 == 0) ? features.difficultyLevel / 10.0 : features.timestampNorm;
        pattern.amplitudes[idx] = value * maxVoltage;
        pattern.frequencies[idx] = 50.0 + value * 100.0; // 50-150 Hz
    }
    
    // Électrodes 10-19: Previous hash bytes
    for (int i = 0; i < 10 && idx < 60 && i < features.prevHashBytes.size(); ++i, ++idx) {
        pattern.amplitudes[idx] = features.prevHashBytes[i] * maxVoltage;
        pattern.frequencies[idx] = 100.0 + features.prevHashBytes[i] * 50.0;
    }
    
    // Électrodes 20-29: Merkle root bytes
    for (int i = 0; i < 10 && idx < 60 && i < features.merkleBytes.size(); ++i, ++idx) {
        pattern.amplitudes[idx] = features.merkleBytes[i] * maxVoltage;
        pattern.frequencies[idx] = 75.0 + features.merkleBytes[i] * 75.0;
    }
    
    // Électrodes 30-39: Entropy et leading zeros
    for (int i = 0; i < 10 && idx < 60; ++i, ++idx) {
        double value = (i < 5) ? features.prevHashEntropy : features.merkleEntropy;
        pattern.amplitudes[idx] = value * maxVoltage;
        pattern.frequencies[idx] = 60.0 + value * 90.0;
    }
    
    // Électrodes 40-49: Remaining previous hash bytes
    for (int i = 10; i < 20 && idx < 60 && i < features.prevHashBytes.size(); ++i, ++idx) {
        pattern.amplitudes[idx] = features.prevHashBytes[i] * maxVoltage;
        pattern.frequencies[idx] = 80.0 + features.prevHashBytes[i] * 70.0;
    }
    
    // Électrodes 50-59: Remaining merkle bytes
    for (int i = 10; i < 20 && idx < 60 && i < features.merkleBytes.size(); ++i, ++idx) {
        pattern.amplitudes[idx] = features.merkleBytes[i] * maxVoltage;
        pattern.frequencies[idx] = 90.0 + features.merkleBytes[i] * 60.0;
    }
    
    // Calculer l'énergie totale
    pattern.totalEnergy = 0.0;
    for (double amp : pattern.amplitudes) {
        pattern.totalEnergy += amp * amp;
    }
    
    qDebug() << QString("[BioEntropyGenerator] Stimulus généré: énergie=%1, durée=%2ms")
                .arg(pattern.totalEnergy).arg(pattern.durationMs);
    
    return pattern;
}

BioEntropyGenerator::BioEntropySeed BioEntropyGenerator::generateEntropySeed(
    const QVector<double> &meaResponse, const BlockHeaderFeatures &features)
{
    BioEntropySeed seed;
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    if (meaResponse.size() != 60) {
        qWarning() << "[BioEntropyGenerator] Réponse MEA invalide";
        seed.confidence = 0.0;
        return seed;
    }
    
    // Stocker la réponse brute
    seed.rawResponse = meaResponse;
    
    // 1. Générer le seed principal par mixing
    seed.primarySeed = mixEntropyComponents(meaResponse, features);
    
    // 2. Calculer la force de réponse
    double totalAmplitude = 0.0;
    for (double value : meaResponse) {
        totalAmplitude += std::abs(value);
    }
    seed.responseStrength = totalAmplitude / meaResponse.size();
    
    // 3. Calculer la confiance (basée sur variance et force)
    double mean = 0.0;
    for (double value : meaResponse) {
        mean += value;
    }
    mean /= meaResponse.size();
    
    double variance = 0.0;
    for (double value : meaResponse) {
        variance += (value - mean) * (value - mean);
    }
    variance /= meaResponse.size();
    
    seed.confidence = seed.responseStrength / (1.0 + variance);
    seed.confidence = qBound(0.0, seed.confidence, 1.0);
    
    // 4. Générer seeds diversifiés
    seed.diverseSeeds = generateDiverseSeeds(seed.primarySeed, 16);
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch() * 1000;
    seed.generationTime = endTime - startTime;
    
    // Mettre à jour les stats
    {
        QMutexLocker locker(&m_statsMutex);
        m_stats.totalSeedsGenerated++;
        m_stats.averageConfidence = 
            (m_stats.averageConfidence * (m_stats.totalSeedsGenerated - 1) + seed.confidence) / 
            m_stats.totalSeedsGenerated;
        m_stats.averageResponseStrength = 
            (m_stats.averageResponseStrength * (m_stats.totalSeedsGenerated - 1) + seed.responseStrength) / 
            m_stats.totalSeedsGenerated;
        m_stats.totalGenerationTime += seed.generationTime;
    }
    
    emit entropyGenerated(seed);
    
    qDebug() << QString("[BioEntropyGenerator] Seed généré: 0x%1, confiance=%2, force=%3")
                .arg(seed.primarySeed, 16, 16, QChar('0'))
                .arg(seed.confidence)
                .arg(seed.responseStrength);
    
    return seed;
}

BioEntropyGenerator::SmartStartingPoints BioEntropyGenerator::generateStartingPoints(
    const BioEntropySeed &seed, int pointCount, uint32_t windowSize)
{
    SmartStartingPoints points;
    points.windowSize = windowSize;
    points.expectedCoverage = static_cast<double>(pointCount * windowSize) / 0x100000000ULL;
    
    // Sélectionner la stratégie selon la confiance
    if (seed.confidence > 0.7) {
        // Haute confiance: utiliser guidage biologique
        points.nonceStarts = strategyBioGuided(seed.primarySeed, seed.rawResponse, pointCount);
        points.strategy = "BioGuided";
    } else if (seed.confidence > 0.4) {
        // Moyenne confiance: Fibonacci
        points.nonceStarts = strategyFibonacci(seed.primarySeed, pointCount);
        points.strategy = "Fibonacci";
    } else {
        // Faible confiance: uniforme
        points.nonceStarts = strategyUniform(seed.primarySeed, pointCount);
        points.strategy = "Uniform";
    }
    
    emit startingPointsReady(points);
    
    qDebug() << QString("[BioEntropyGenerator] %1 points générés (stratégie: %2, couverture=%3%)")
                .arg(pointCount)
                .arg(points.strategy)
                .arg(points.expectedCoverage * 100, 0, 'f', 2);
    
    return points;
}

void BioEntropyGenerator::reinforceSuccessfulPattern(
    const BlockHeaderFeatures &features,
    const QVector<double> &meaResponse,
    uint32_t validNonce)
{
    QMutexLocker locker(&m_patternMutex);
    
    PatternMemory memory;
    memory.features = features;
    memory.response = meaResponse;
    memory.nonce = validNonce;
    memory.successScore = 1.0;
    memory.timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
    
    m_successfulPatterns.append(memory);
    
    // Limiter la taille de la mémoire
    if (m_successfulPatterns.size() > 100) {
        m_successfulPatterns.removeFirst();
    }
    
    // Mettre à jour les stats
    {
        QMutexLocker statsLocker(&m_statsMutex);
        m_stats.successfulSeeds++;
    }
    
    emit patternReinforced(1.0);
    
    qDebug() << QString("[BioEntropyGenerator] Pattern renforcé: nonce=0x%1, mémoire=%2")
                .arg(validNonce, 8, 16, QChar('0'))
                .arg(m_successfulPatterns.size());
}

QJsonObject BioEntropyGenerator::getEntropyStats() const
{
    QMutexLocker locker(&m_statsMutex);
    
    QJsonObject stats;
    stats["totalSeedsGenerated"] = m_stats.totalSeedsGenerated;
    stats["successfulSeeds"] = m_stats.successfulSeeds;
    stats["averageConfidence"] = m_stats.averageConfidence;
    stats["averageResponseStrength"] = m_stats.averageResponseStrength;
    stats["totalGenerationTime"] = static_cast<qint64>(m_stats.totalGenerationTime);
    stats["averageGenerationTime"] = m_stats.totalSeedsGenerated > 0 ?
        static_cast<qint64>(m_stats.totalGenerationTime / m_stats.totalSeedsGenerated) : 0;
    stats["successRate"] = m_stats.totalSeedsGenerated > 0 ?
        static_cast<double>(m_stats.successfulSeeds) / m_stats.totalSeedsGenerated : 0.0;
    
    return stats;
}

void BioEntropyGenerator::resetStats()
{
    QMutexLocker locker(&m_statsMutex);
    m_stats = EntropyStats();
    qDebug() << "[BioEntropyGenerator] Statistiques réinitialisées";
}

// Private methods
double BioEntropyGenerator::calculateShannonEntropy(const QString &hexString)
{
    if (hexString.isEmpty()) {
        return 0.0;
    }
    
    QMap<QChar, int> frequency;
    for (QChar c : hexString) {
        frequency[c]++;
    }
    
    double entropy = 0.0;
    int total = hexString.length();
    
    for (int count : frequency.values()) {
        double probability = static_cast<double>(count) / total;
        entropy -= probability * std::log2(probability);
    }
    
    return entropy;
}

int BioEntropyGenerator::countLeadingZeros(const QString &hexString)
{
    int zeros = 0;
    for (QChar c : hexString) {
        if (c == '0') {
            zeros++;
        } else {
            break;
        }
    }
    return zeros;
}

QVector<double> BioEntropyGenerator::hexToNormalizedBytes(const QString &hexString, int maxBytes)
{
    QVector<double> bytes;
    
    // Convertir hex en bytes normalisés [0, 1]
    for (int i = 0; i < hexString.length() - 1 && bytes.size() < maxBytes; i += 2) {
        QString byteStr = hexString.mid(i, 2);
        bool ok;
        uint8_t byteValue = byteStr.toUInt(&ok, 16);
        if (ok) {
            bytes.append(static_cast<double>(byteValue) / 255.0);
        }
    }
    
    // Compléter si nécessaire
    while (bytes.size() < maxBytes) {
        bytes.append(0.0);
    }
    
    return bytes;
}

QVector<uint32_t> BioEntropyGenerator::generateDiverseSeeds(uint64_t primarySeed, int count)
{
    QVector<uint32_t> seeds;
    
    for (int i = 0; i < count; ++i) {
        // Utiliser le seed primaire + index pour générer diversité
        uint64_t mixed = primarySeed ^ (static_cast<uint64_t>(i) << 32);
        mixed = mixed * 0x5DEECE66DLL + 0xBLL; // Linear congruential
        seeds.append(static_cast<uint32_t>(mixed >> 16));
    }
    
    return seeds;
}

uint64_t BioEntropyGenerator::mixEntropyComponents(
    const QVector<double> &response, const BlockHeaderFeatures &features)
{
    // Mélanger les composantes pour générer un seed 64-bit
    QCryptographicHash hash(QCryptographicHash::Sha256);
    
    // Ajouter la réponse MEA
    for (double value : response) {
        //uint64_t intValue = *reinterpret_cast<const uint64_t*>(&value);
        uint64_t intValue;
        std::memcpy(&intValue, &value, sizeof(uint64_t));
        hash.addData(reinterpret_cast<const char*>(&intValue), sizeof(intValue));
    }
    
    // Ajouter les features
    hash.addData(reinterpret_cast<const char*>(&features.difficultyLevel), sizeof(double));
    hash.addData(reinterpret_cast<const char*>(&features.timestampNorm), sizeof(double));
    hash.addData(reinterpret_cast<const char*>(&features.prevHashEntropy), sizeof(double));
    
    QByteArray result = hash.result();
    
    // Extraire les premiers 8 bytes comme seed 64-bit
    uint64_t seed = 0;
    for (int i = 0; i < 8 && i < result.size(); ++i) {
        seed |= (static_cast<uint64_t>(static_cast<uint8_t>(result[i])) << (i * 8));
    }
    
    return seed;
}

QVector<uint32_t> BioEntropyGenerator::strategyUniform(uint64_t seed, int count)
{
    QVector<uint32_t> points;
    
    // Distribution uniforme dans l'espace [0, 2^32)
    uint64_t step = 0x100000000ULL / count;
    uint64_t offset = seed % step;
    
    for (int i = 0; i < count; ++i) {
        points.append(static_cast<uint32_t>((offset + i * step) & 0xFFFFFFFFULL));
    }
    
    return points;
}

QVector<uint32_t> BioEntropyGenerator::strategyFibonacci(uint64_t seed, int count)
{
    QVector<uint32_t> points;
    
    // Distribution Fibonacci (nombre d'or pour espacement)
    const double PHI = 1.618033988749895;
    double offset = static_cast<double>(seed % 0x100000000ULL) / 0x100000000ULL;
    
    for (int i = 0; i < count; ++i) {
        double position = std::fmod(offset + i / PHI, 1.0);
        points.append(static_cast<uint32_t>(position * 0x100000000ULL));
    }
    
    return points;
}

QVector<uint32_t> BioEntropyGenerator::strategyBioGuided(
    uint64_t seed, const QVector<double> &response, int count)
{
    QVector<uint32_t> points;
    
    // Utiliser la réponse biologique pour guider la génération
    // Chaque pic dans la réponse définit une zone d'intérêt
    
    // Trouver les pics dans la réponse
    QVector<int> peaks;
    for (int i = 1; i < response.size() - 1; ++i) {
        if (response[i] > response[i-1] && response[i] > response[i+1] && 
            std::abs(response[i]) > 0.1) {
            peaks.append(i);
        }
    }
    
    if (peaks.isEmpty()) {
        // Fallback à Fibonacci si pas de pics
        return strategyFibonacci(seed, count);
    }
    
    // Distribuer les points autour des pics
    int pointsPerPeak = count / peaks.size();
    int remaining = count % peaks.size();
    
    for (int peakIdx = 0; peakIdx < peaks.size(); ++peakIdx) {
        int peakPosition = peaks[peakIdx];
        double peakAmplitude = response[peakPosition];
        
        // Centre de la zone basé sur le pic
        uint64_t center = static_cast<uint64_t>(
            (static_cast<double>(peakPosition) / 60.0) * 0x100000000ULL
        );
        
        // Largeur de la zone basée sur l'amplitude
        uint64_t width = static_cast<uint64_t>(
            std::abs(peakAmplitude) * 0x10000000ULL
        );
        
        // Générer points autour de ce pic
        int numPoints = pointsPerPeak + (peakIdx < remaining ? 1 : 0);
        for (int i = 0; i < numPoints; ++i) {
            uint64_t offset = (seed ^ (peakIdx << 16) ^ (i << 8)) % width;
            uint32_t point = static_cast<uint32_t>((center + offset - width/2) & 0xFFFFFFFFULL);
            points.append(point);
        }
    }
    
    return points;
}

} // namespace Crypto
} // namespace BioMining
