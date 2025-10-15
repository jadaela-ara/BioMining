#ifndef IBIO_COMPUTE_INTERFACE_H
#define IBIO_COMPUTE_INTERFACE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <cstdint>

namespace BioMining {
namespace Bio {

/**
 * @brief Interface abstraite pour les systèmes de calcul biologique
 * 
 * Cette interface permet d'unifier RealMEA (vraies cellules) et 
 * BiologicalNetwork (simulation) sous une API commune.
 */
class IBioComputeInterface : public QObject
{
    Q_OBJECT

public:
    explicit IBioComputeInterface(QObject *parent = nullptr);

    /**
     * @brief Mode de calcul biologique
     */
    enum class ComputeMode {
        RealMEA,              // MEA avec vraies cellules biologiques
        SimulatedNetwork      // Réseau neuronal simulé
    };

    /**
     * @brief Pattern de stimulation (entrée)
     */
    struct StimulusPattern {
        QVector<double> amplitudes;     // 60 valeurs d'amplitude
        QVector<double> frequencies;    // 60 valeurs de fréquence (optionnel)
        int durationMs;                 // Durée de la stimulation
    };

    /**
     * @brief Réponse du système biologique (sortie)
     */
    struct BioResponse {
        QVector<double> rsignals;        // 60 signaux de réponse
        double responseStrength;        // Force globale de la réponse
        double signalQuality;           // Qualité du signal [0,1]
        qint64 responseTime;            // Temps de réponse [μs]
        bool isValid;                   // Réponse valide ou non
    };

    virtual ~IBioComputeInterface() = default;

    /**
     * @brief Retourne le mode de calcul
     */
    virtual ComputeMode getComputeMode() const = 0;

    /**
     * @brief Initialise le système de calcul biologique
     */
    virtual bool initialize() = 0;

    /**
     * @brief Vérifie si le système est prêt
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Applique un pattern de stimulation
     * @param pattern Pattern d'entrée
     * @return true si la stimulation a réussi
     */
    virtual bool applyStimulus(const StimulusPattern &pattern) = 0;

    /**
     * @brief Capture la réponse du système biologique
     * @param waitTimeMs Temps d'attente pour la réponse
     * @return Réponse du système
     */
    virtual BioResponse captureResponse(int waitTimeMs = 100) = 0;

    /**
     * @brief Exécute une stimulation complète (stimulus + capture)
     * @param pattern Pattern d'entrée
     * @return Réponse du système
     */
    virtual BioResponse stimulateAndCapture(const StimulusPattern &pattern) = 0;

    /**
     * @brief Applique un renforcement positif (apprentissage)
     * @param pattern Pattern qui a produit un bon résultat
     * @param nonce Nonce valide trouvé
     * @param reward Signal de récompense [0,1]
     */
    virtual void reinforcePattern(const StimulusPattern &pattern, 
                                  uint32_t nonce, 
                                  double reward) = 0;

    /**
     * @brief Obtient des informations de diagnostic
     */
    virtual QString getDiagnosticInfo() const = 0;

    /**
     * @brief Réinitialise le système
     */
    virtual void reset() = 0;

signals:
    void stimulusApplied();
    void responseReady(const BioResponse &response);
    void patternReinforced(double confidence);
    void errorOccurred(const QString &error);
};

} // namespace Bio
} // namespace BioMining

// Déclarer les métadonnées Qt pour les types
Q_DECLARE_METATYPE(BioMining::Bio::IBioComputeInterface::StimulusPattern)
Q_DECLARE_METATYPE(BioMining::Bio::IBioComputeInterface::BioResponse)
Q_DECLARE_METATYPE(BioMining::Bio::IBioComputeInterface::ComputeMode)

#endif // IBIO_COMPUTE_INTERFACE_H
