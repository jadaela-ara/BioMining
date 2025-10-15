#ifndef BIO_ENTROPY_GENERATOR_H
#define BIO_ENTROPY_GENERATOR_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QMutex>
#include <cstdint>

namespace BioMining {
namespace Crypto {

/**
 * @brief Générateur d'entropie biologique pour mining Bitcoin optimisé
 * 
 * Utilise les réponses des cellules biologiques du MEA pour générer
 * des seeds d'entropie de haute qualité qui guident la recherche GPU.
 */
class BioEntropyGenerator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Features extraites du block header Bitcoin
     */
    struct BlockHeaderFeatures {
        double timestampNorm;           // Timestamp normalisé [0,1]
        double difficultyLevel;         // Log10(difficulty)
        double prevHashEntropy;         // Entropie Shannon du previous hash
        double prevHashLeadingZeros;    // Nombre de zéros initiaux
        double merkleEntropy;           // Entropie du merkle root
        QVector<double> prevHashBytes;  // Premiers 20 bytes du prev hash
        QVector<double> merkleBytes;    // Premiers 20 bytes du merkle
        double versionNorm;             // Version normalisée
        double difficultyBitsNorm;      // Difficulty bits normalisé
        
        // Total: ~60 features pour correspondre aux 60 électrodes
    };

    /**
     * @brief Pattern de stimulation électrique pour le MEA
     */
    struct StimulusPattern {
        QVector<double> amplitudes;     // Amplitude par électrode (60) [0, maxVoltage]
        QVector<double> frequencies;    // Fréquence par électrode (60) [Hz]
        int durationMs;                 // Durée de stimulation [ms]
        double totalEnergy;             // Énergie totale du pattern
    };

    /**
     * @brief Seed d'entropie généré par les cellules biologiques
     */
    struct BioEntropySeed {
        uint64_t primarySeed;           // Seed principal 64-bit
        QVector<uint32_t> diverseSeeds; // Seeds secondaires pour diversité
        double confidence;              // Confiance dans le seed [0,1]
        double responseStrength;        // Force de la réponse bio
        QVector<double> rawResponse;    // Réponse MEA brute (60 valeurs)
        qint64 generationTime;          // Temps de génération [μs]
    };

    /**
     * @brief Points de départ intelligents pour la recherche
     */
    struct SmartStartingPoints {
        QVector<uint32_t> nonceStarts;  // Points de départ (ex: 1000 points)
        uint32_t windowSize;            // Taille de fenêtre à explorer par point
        double expectedCoverage;        // Couverture attendue de l'espace
        QString strategy;               // Stratégie utilisée
    };

    explicit BioEntropyGenerator(QObject *parent = nullptr);
    ~BioEntropyGenerator();

    /**
     * @brief Extrait les features d'un block header Bitcoin
     */
    BlockHeaderFeatures extractHeaderFeatures(const QString &blockHeader, uint64_t difficulty);

    /**
     * @brief Convertit les features en pattern de stimulation électrique
     */
    StimulusPattern featuresToStimulus(const BlockHeaderFeatures &features, 
                                       double maxVoltage = 3.0);

    /**
     * @brief Génère un seed d'entropie à partir de la réponse MEA
     */
    BioEntropySeed generateEntropySeed(const QVector<double> &meaResponse, 
                                       const BlockHeaderFeatures &features);

    /**
     * @brief Génère des points de départ intelligents depuis le seed bio
     */
    SmartStartingPoints generateStartingPoints(const BioEntropySeed &seed, 
                                               int pointCount = 1000,
                                               uint32_t windowSize = 4194304);

    /**
     * @brief Renforce les patterns qui ont produit des nonces valides
     */
    void reinforceSuccessfulPattern(const BlockHeaderFeatures &features,
                                    const QVector<double> &meaResponse,
                                    uint32_t validNonce);

    /**
     * @brief Obtient les statistiques d'entropie
     */
    QJsonObject getEntropyStats() const;

    /**
     * @brief Réinitialise les statistiques
     */
    void resetStats();

signals:
    void entropyGenerated(const BioEntropySeed &seed);
    void startingPointsReady(const SmartStartingPoints &points);
    void patternReinforced(double newConfidence);

private:
    // Calculs d'entropie
    double calculateShannonEntropy(const QString &hexString);
    int countLeadingZeros(const QString &hexString);
    
    // Conversion hex → bytes
    QVector<double> hexToNormalizedBytes(const QString &hexString, int maxBytes = 20);
    
    // Génération de diversité
    QVector<uint32_t> generateDiverseSeeds(uint64_t primarySeed, int count);
    
    // Hashing et mixing
    uint64_t mixEntropyComponents(const QVector<double> &response, 
                                  const BlockHeaderFeatures &features);
    
    // Stratégies de génération de points
    QVector<uint32_t> strategyUniform(uint64_t seed, int count);
    QVector<uint32_t> strategyFibonacci(uint64_t seed, int count);
    QVector<uint32_t> strategyBioGuided(uint64_t seed, const QVector<double> &response, int count);
    
    // Renforcement
    struct PatternMemory {
        BlockHeaderFeatures features;
        QVector<double> response;
        uint32_t nonce;
        double successScore;
        qint64 timestamp;
    };
    
    QVector<PatternMemory> m_successfulPatterns;
    QMutex m_patternMutex;
    
    // Statistiques
    struct EntropyStats {
        int totalSeedsGenerated = 0;
        int successfulSeeds = 0;
        double averageConfidence = 0.0;
        double averageResponseStrength = 0.0;
        qint64 totalGenerationTime = 0;
    } m_stats;
    
    mutable QMutex m_statsMutex;
};

} // namespace Crypto
} // namespace BioMining

#endif // BIO_ENTROPY_GENERATOR_H
