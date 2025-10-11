#include "crypto/bitcoin_miner.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace BioMining {
namespace Crypto {

BitcoinMiner::BitcoinMiner(QObject *parent)
    : QObject(parent)
    , m_workerThread(std::make_unique<QThread>())
    , m_worker(std::make_unique<MiningWorker>(this))
    , m_totalAttempts(0)
    , m_successCount(0)
    , m_currentHashrate(0.0)
    , m_isMining(false)
    , m_continuousMode(false)
{
    // Configuration par défaut
    m_config.blockHeader = "000000000000000000000000000000000000000000000000000000000000000000000000";
    m_config.difficulty = DEFAULT_DIFFICULTY;
    m_config.maxAttempts = DEFAULT_MAX_ATTEMPTS;
    m_config.signalWeight = DEFAULT_SIGNAL_WEIGHT;
    m_config.useQuantumOptimization = false;
    m_config.threadCount = QThread::idealThreadCount();
    
    // Configuration du worker thread
    m_worker->moveToThread(m_workerThread.get());
    
    connect(m_worker.get(), &MiningWorker::miningComplete,
            this, &BitcoinMiner::miningComplete);
    connect(m_worker.get(), &MiningWorker::progressUpdate,
            this, &BitcoinMiner::progressUpdate);
    
    connect(m_workerThread.get(), &QThread::started,
            m_worker.get(), &MiningWorker::startMining);
    connect(m_workerThread.get(), &QThread::finished,
            this, &BitcoinMiner::onMiningThreadFinished);
    
    m_startTime = QDateTime::currentDateTime();
    
    qDebug() << "[MINER] Initialisé avec" << m_config.threadCount << "threads";
}

BitcoinMiner::~BitcoinMiner()
{
    stopMining();
    stopContinuousMining();
    
    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait(5000);
    }
}

void BitcoinMiner::setMiningConfig(const MiningConfig &config)
{
    QMutexLocker locker(&m_statsMutex);
    m_config = config;
    m_worker->setConfig(config);
    
    qDebug() << "[MINER] Configuration mise à jour - Difficulté:" 
             << QString::number(config.difficulty, 16);
}

BitcoinMiner::MiningConfig BitcoinMiner::getMiningConfig() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_config;
}

BitcoinMiner::MiningResult BitcoinMiner::mine(const QVector<double> &bioSignals)
{
    if (m_isMining) {
        qWarning() << "[MINER] Mining déjà en cours";
        return {false, "", "", 0.0, 0, 0.0};
    }
    
    QElapsedTimer timer;
    timer.start();
    
    MiningResult result;
    result.success = false;
    result.attempts = 0;
    result.signalContribution = calculateSignalEntropy(bioSignals);
    
    qDebug() << "[MINER] Démarrage mining avec" << bioSignals.size() 
             << "signaux, entropie:" << result.signalContribution;
    
    for (int attempt = 0; attempt < m_config.maxAttempts; ++attempt) {
        // Génération du nonce basé sur les signaux biologiques
        uint64_t nonce = generateNonceFromSignals(bioSignals, attempt);
        
        // Construction des données à hasher
        QString data = m_config.blockHeader + QString::number(nonce);
        
        // Calcul du hash
        QString hash = computeHash(data);
        
        result.attempts++;
        
        // Vérification de la difficulté
        if (checkDifficulty(hash, m_config.difficulty)) {
            result.success = true;
            result.nonce = QString::number(nonce);
            result.hash = hash;
            break;
        }
        
        // Émission du progrès périodique
        if (attempt % 1000 == 0) {
            double progress = static_cast<double>(attempt) / m_config.maxAttempts;
            emit progressUpdate(attempt, progress);
        }
    }
    
    result.computeTime = timer.elapsed() / 1000.0; // en secondes
    
    // Mise à jour des statistiques
    {
        QMutexLocker locker(&m_statsMutex);
        m_totalAttempts += result.attempts;
        if (result.success) {
            m_successCount++;
        }
        
        // Calcul du hashrate
        double hashrate = result.attempts / result.computeTime;
        m_currentHashrate = hashrate;
        emit hashRateUpdated(hashrate);
    }
    
    qDebug() << "[MINER] Mining terminé -" << (result.success ? "SUCCESS" : "FAILED")
             << "- Attempts:" << result.attempts 
             << "- Time:" << result.computeTime << "s"
             << "- Hashrate:" << (result.attempts / result.computeTime) << "H/s";
    
    // Optimisation adaptative
    optimizeFromResult(result);
    
    return result;
}

void BitcoinMiner::startMiningAsync(const QVector<double> &bioSignals)
{
    if (m_isMining) {
        qWarning() << "[MINER] Mining déjà en cours";
        return;
    }
    
    m_isMining = true;
    m_worker->setBioSignals(bioSignals);
    m_workerThread->start();
    
    qDebug() << "[MINER] Mining asynchrone démarré";
}

void BitcoinMiner::stopMining()
{
    if (!m_isMining) {
        return;
    }
    
    m_isMining = false;
    m_worker->stopMining();
    
    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait(3000);
    }
    
    qDebug() << "[MINER] Mining arrêté";
}

void BitcoinMiner::startContinuousMining()
{
    m_continuousMode = true;
    qDebug() << "[MINER] Mode continu activé";
}

void BitcoinMiner::stopContinuousMining()
{
    m_continuousMode = false;
    stopMining();
    qDebug() << "[MINER] Mode continu désactivé";
}

double BitcoinMiner::getHashrate() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_currentHashrate;
}

int BitcoinMiner::getTotalAttempts() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_totalAttempts;
}

int BitcoinMiner::getSuccessCount() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_successCount;
}

double BitcoinMiner::getSuccessRate() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_totalAttempts > 0 ? static_cast<double>(m_successCount) / m_totalAttempts : 0.0;
}

void BitcoinMiner::updateDifficulty(double adjustmentFactor)
{
    QMutexLocker locker(&m_statsMutex);
    
    uint64_t newDifficulty = static_cast<uint64_t>(m_config.difficulty * adjustmentFactor);
    
    // Limites de sécurité
    const uint64_t MIN_DIFFICULTY = 0x000000FFFFFFFFFF;
    const uint64_t MAX_DIFFICULTY = 0xFFFFFFFFFFFFFFFF;
    
    newDifficulty = qBound(MIN_DIFFICULTY, newDifficulty, MAX_DIFFICULTY);
    
    if (newDifficulty != m_config.difficulty) {
        m_config.difficulty = newDifficulty;
        m_worker->setConfig(m_config);
        
        qDebug() << "[MINER] Difficulté ajustée:" << QString::number(newDifficulty, 16);
        emit difficultyAdjusted(newDifficulty);
    }
}

void BitcoinMiner::optimizeFromResult(const MiningResult &result)
{
    m_recentResults.append(result);
    
    // Garde seulement les résultats récents
    while (m_recentResults.size() > MAX_RESULT_HISTORY) {
        m_recentResults.removeFirst();
    }
    
    // Ajustement adaptatif de la difficulté basé sur les performances
    if (m_recentResults.size() >= 10) {
        double avgTime = 0.0;
        double successRate = 0.0;
        
        for (const auto &res : m_recentResults) {
            avgTime += res.computeTime;
            if (res.success) successRate += 1.0;
        }
        
        avgTime /= m_recentResults.size();
        successRate /= m_recentResults.size();
        
        // Ajustement intelligent
        double targetTime = 10.0; // 10 secondes cible
        double targetSuccessRate = 0.1; // 10% de succès cible
        
        if (avgTime < targetTime * 0.5 && successRate > targetSuccessRate * 1.5) {
            // Trop facile, augmenter la difficulté
            updateDifficulty(1.1);
        } else if (avgTime > targetTime * 2.0 || successRate < targetSuccessRate * 0.5) {
            // Trop difficile, réduire la difficulté
            updateDifficulty(0.9);
        }
    }
}

void BitcoinMiner::onBioSignalsReceived(const QVector<double> &signalData)
{
    if (m_continuousMode && !m_isMining) {
        startMiningAsync(signalData);
    }
}

void BitcoinMiner::onMiningThreadFinished()
{
    m_isMining = false;
}

bool BitcoinMiner::initialize()
{
    qDebug() << "[MINER] BitcoinMiner::initialize() called.";
    return true;
}

bool BitcoinMiner::setMiningParameters(const MiningConfig& config)
{
    QMutexLocker locker(&m_statsMutex);
    m_config = config;
    m_worker->setConfig(config);
    qDebug() << "[MINER] BitcoinMiner::setMiningParameters() called.";
    return true;
}

bool BitcoinMiner::mineBlock(const QString& blockHeader, uint32_t& nonce, QString& blockHash)
{
    qDebug() << "[MINER] BitcoinMiner::mineBlock() called.";
    // Placeholder implementation for now
    Q_UNUSED(blockHeader);
    nonce = QRandomGenerator::global()->generate();
    blockHash = computeHash(blockHeader + QString::number(nonce));
    return true;
}

uint64_t BitcoinMiner::generateNonceFromSignals(const QVector<double> &signalData, int seed)
{
    if (signalData.isEmpty()) {
        return QRandomGenerator::global()->generate64();
    }
    
    // Combinaison des signaux avec pondération biologique
    double signalSum = 0.0;
    double weightedSum = 0.0;
    
    for (int i = 0; i < signalData.size(); ++i) {
        double weight = 1.0 + sin(i * 0.1) * m_config.signalWeight;
        signalSum += signalData[i];
        weightedSum += signalData[i] * weight;
    }
    
    // Incorporation de l'entropie temporelle
    uint64_t timeComponent = QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFF;
    
    // Génération du nonce
    uint64_t baseNonce = static_cast<uint64_t>(abs(signalSum * weightedSum * 1e6));
    uint64_t nonce = (baseNonce ^ timeComponent ^ seed) % (1ULL << 32);
    
    return nonce;
}

QString BitcoinMiner::computeHash(const QString &data)
{
    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
    
    // Double SHA-256 comme Bitcoin
    hash = QCryptographicHash::hash(hash, QCryptographicHash::Sha256);
    
    return hash.toHex();
}

bool BitcoinMiner::checkDifficulty(const QString &hash, uint64_t difficulty)
{
    // Conversion du hash en valeur numérique pour comparaison
    bool ok;
    uint64_t hashValue = hash.left(16).toULongLong(&ok, 16);
    
    if (!ok) {
        return false;
    }
    
    return hashValue < difficulty;
}

double BitcoinMiner::calculateSignalEntropy(const QVector<double> &signalData)
{
    if (signalData.isEmpty()) {
        return 0.0;
    }
    
    // Calcul de l'entropie de Shannon des signaux
    QMap<int, int> histogram;
    
    // Quantification des signaux en bins
    const int numBins = 256;
    double minVal = *std::min_element(signalData.begin(), signalData.end());
    double maxVal = *std::max_element(signalData.begin(), signalData.end());
    double range = maxVal - minVal;
    
    if (range == 0.0) {
        return 0.0;
    }
    
    for (double signal : signalData) {
        int bin = static_cast<int>((signal - minVal) / range * (numBins - 1));
        histogram[bin]++;
    }
    
    // Calcul de l'entropie
    double entropy = 0.0;
    int totalSamples = signalData.size();
    
    for (auto it = histogram.begin(); it != histogram.end(); ++it) {
        double probability = static_cast<double>(it.value()) / totalSamples;
        if (probability > 0.0) {
            entropy -= probability * log2(probability);
        }
    }
    
    return entropy;
}

QVector<int> BitcoinMiner::selectBestElectrodes(const QVector<double> &signalData, int count)
{
    QVector<QPair<double, int>> signalPairs;
    
    for (int i = 0; i < signalData.size(); ++i) {
        signalPairs.append({abs(signalData[i]), i});
    }
    
    // Tri par amplitude décroissante
    std::sort(signalPairs.begin(), signalPairs.end(),
              [](const QPair<double, int> &a, const QPair<double, int> &b) {
                  return a.first > b.first;
              });
    
    QVector<int> bestElectrodes;
    for (int i = 0; i < qMin(count, signalPairs.size()); ++i) {
        bestElectrodes.append(signalPairs[i].second);
    }
    
    return bestElectrodes;
}

double BitcoinMiner::calculateSignalCorrelation(const QVector<double> &signalData)
{
    if (signalData.size() < 2) {
        return 0.0;
    }
    
    // Autocorrélation moyenne
    double totalCorrelation = 0.0;
    int pairCount = 0;
    
    for (int i = 0; i < signalData.size() - 1; ++i) {
        for (int j = i + 1; j < signalData.size(); ++j) {
            totalCorrelation += signalData[i] * signalData[j];
            pairCount++;
        }
    }
    
    return pairCount > 0 ? totalCorrelation / pairCount : 0.0;
}

// MiningWorker implementation
BitcoinMiner::MiningWorker::MiningWorker(BitcoinMiner *parent)
    : QObject(nullptr), m_parent(parent), m_shouldStop(false)
{
}

void BitcoinMiner::MiningWorker::setConfig(const MiningConfig &config)
{
    m_config = config;
}

void BitcoinMiner::MiningWorker::setBioSignals(const QVector<double> &signalData)
{
    m_bioSignals = signalData;
}

void BitcoinMiner::MiningWorker::startMining()
{
    if (!m_parent) {
        return;
    }
    
    m_shouldStop = false;
    
    // Utilise la fonction de mining du parent
    MiningResult result = m_parent->mine(m_bioSignals);
    
    if (!m_shouldStop) {
        emit miningComplete(result);
    }
}

void BitcoinMiner::MiningWorker::stopMining()
{
    m_shouldStop = true;
}

} // namespace Crypto
} // namespace BioMining

#include "bitcoin_miner.moc"
