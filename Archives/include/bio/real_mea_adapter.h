#ifndef REAL_MEA_ADAPTER_H
#define REAL_MEA_ADAPTER_H

#include "bio/ibio_compute_interface.h"
#include "bio/real_mea_interface.h"
#include <QSharedPointer>
#include <QMutex>

namespace BioMining {
namespace Bio {

/**
 * @brief Adapteur pour RealMEAInterface vers IBioComputeInterface
 * 
 * Permet d'utiliser le vrai MEA (cellules biologiques) via l'interface commune.
 */
class RealMEAAdapter : public IBioComputeInterface
{
    Q_OBJECT

public:
    explicit RealMEAAdapter(QObject *parent = nullptr);
    explicit RealMEAAdapter(QSharedPointer<RealMEAInterface> meaInterface, QObject *parent = nullptr);
    ~RealMEAAdapter() override;

    // IBioComputeInterface implementation
    ComputeMode getComputeMode() const override { return ComputeMode::RealMEA; }
    bool initialize() override;
    bool isReady() const override;
    bool applyStimulus(const StimulusPattern &pattern) override;
    BioResponse captureResponse(int waitTimeMs = 100) override;
    BioResponse stimulateAndCapture(const StimulusPattern &pattern) override;
    void reinforcePattern(const StimulusPattern &pattern, uint32_t nonce, double reward) override;
    QString getDiagnosticInfo() const override;
    void reset() override;

    // Configuration spécifique MEA
    void setMEAConfig(const RealMEAConfig &config);
    RealMEAInterface* getMEAInterface() const { return m_meaInterface.data(); }
    
    /**
     * @brief Configure le système d'apprentissage Bitcoin du MEA
     */
    bool initializeBitcoinLearning(const BitcoinLearningConfig &config);

private slots:
    void onMEAStatusChanged(RealMEAInterface::ConnectionStatus status);
    void onMEAError(const QString &error);

private:
    QSharedPointer<RealMEAInterface> m_meaInterface;
    mutable QMutex m_mutex;
    bool m_initialized;
    
    // Conversion des réponses MEA
    BioResponse convertElectrodeDataToResponse(const QVector<ElectrodeData> &electrodeData);
    double calculateResponseStrength(const QVector<ElectrodeData> &data);
    double calculateSignalQuality(const QVector<ElectrodeData> &data);
};

} // namespace Bio
} // namespace BioMining

#endif // REAL_MEA_ADAPTER_H
