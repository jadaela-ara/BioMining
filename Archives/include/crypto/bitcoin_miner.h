#ifndef BITCOIN_MINER_H
#define BITCOIN_MINER_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QMutex>
#include <QDateTime>
#include <QCryptographicHash>
#include <memory>

namespace BioMining {
namespace Crypto {

/**
 * @brief Moteur de mining Bitcoin hybride bio-informatique
 * 
 * Cette classe implémente un algorithme de mining Bitcoin adapté aux signaux biologiques,
 * utilisant les données MEA pour générer des nonces et optimiser le processus de hachage.
 */
class BitcoinMiner : public QObject
{
    Q_OBJECT

public:
    struct MiningResult {
        bool success;
        QString nonce;
        QString hash;
        double computeTime;
        int attempts;
        double signalContribution;
    };

    struct MiningConfig {
        QString blockHeader;
        uint64_t difficulty;
        int maxAttempts;
        double signalWeight;
        bool useQuantumOptimization;
        int threadCount;

        double targetDifficulty = 1.0;    // Compatibilité avec HybridMiner
        uint32_t threads = 1;             // Nombre de threads
        bool useGPU = false;              // Utilisation GPU
        double targetEfficiency = 0.5;    // Efficacité cible

    };

    explicit BitcoinMiner(QObject *parent = nullptr);
    ~BitcoinMiner();

    // Configuration
    void setMiningConfig(const MiningConfig &config);
    MiningConfig getMiningConfig() const;

    // Mining principal
    MiningResult mine(const QVector<double> &bioSignals);
    void startMiningAsync(const QVector<double> &bioSignals);
    void stopMining();

    // Mining continu avec feed MEA
    void startContinuousMining();
    void stopContinuousMining();

    // Statistiques et monitoring
    double getHashrate() const;
    int getTotalAttempts() const;
    int getSuccessCount() const;
    double getSuccessRate() const;
    
    // Optimisation adaptative
    void updateDifficulty(double adjustmentFactor);
    void optimizeFromResult(const MiningResult &result);

signals:
    void miningComplete(const MiningResult &result);
    void hashRateUpdated(double hashrate);
    void difficultyAdjusted(uint64_t newDifficulty);
    void progressUpdate(int attempts, double progress);

public slots:
    void onBioSignalsReceived(const QVector<double> &signalData);

//private slots:
    void onMiningThreadFinished();


    // Méthodes ajoutées pour corriger les erreurs de compilation
    bool initialize();
    bool setMiningParameters(const MiningConfig& config);
    bool mineBlock(const QString& blockHeader, uint32_t& nonce, QString& blockHash);

private:
    // Fonctions de hachage et nonce generation
    uint64_t generateNonceFromSignals(const QVector<double> &signalData, int seed = 0);
    QString computeHash(const QString &data);
    bool checkDifficulty(const QString &hash, uint64_t difficulty);
    
    // Optimisations biologiques
    double calculateSignalEntropy(const QVector<double> &signalData);
    QVector<int> selectBestElectrodes(const QVector<double> &signalData, int count);
    double calculateSignalCorrelation(const QVector<double> &signalData);
    
    // Gestion des threads
    class MiningWorker;
    std::unique_ptr<QThread> m_workerThread;
    std::unique_ptr<MiningWorker> m_worker;
    
    MiningConfig m_config;
    
    // Statistiques
    mutable QMutex m_statsMutex;
    QDateTime m_startTime;
    int m_totalAttempts;
    int m_successCount;
    double m_currentHashrate;
    
    // État du mining
    bool m_isMining;
    bool m_continuousMode;
    
    // Optimisation adaptative
    QVector<MiningResult> m_recentResults;
    static constexpr int MAX_RESULT_HISTORY = 100;
    
    // Constantes
    static constexpr uint64_t DEFAULT_DIFFICULTY = 0x0000FFFFFFFFFFFF;
    static constexpr int DEFAULT_MAX_ATTEMPTS = 10000;
    static constexpr double DEFAULT_SIGNAL_WEIGHT = 1.0;
};

/**
 * @brief Worker thread pour le mining Bitcoin
 */
class BitcoinMiner::MiningWorker : public QObject
{
    Q_OBJECT

public:
    MiningWorker(BitcoinMiner *parent = nullptr);
    void setConfig(const MiningConfig &config);
    void setBioSignals(const QVector<double> &signalData);

public slots:
    void startMining();
    void stopMining();

signals:
    void miningComplete(const BitcoinMiner::MiningResult &result);
    void progressUpdate(int attempts, double progress);


    // Méthodes ajoutées pour corriger les erreurs de compilation
    bool initialize();
    bool setMiningParameters(const MiningConfig& config);
    bool mineBlock(const QString& blockHeader, uint32_t& nonce, QString& blockHash);

private:
    BitcoinMiner *m_parent;
    MiningConfig m_config;
    QVector<double> m_bioSignals;
    bool m_shouldStop;
};

} // namespace Crypto
} // namespace BioMining

#endif // BITCOIN_MINER_H
