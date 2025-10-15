#ifndef BIOLOGICAL_NETWORK_ADAPTER_H
#define BIOLOGICAL_NETWORK_ADAPTER_H

#include "bio/ibio_compute_interface.h"
#include "bio/biological_network.h"
#include <QSharedPointer>
#include <QMutex>

namespace BioMining {
namespace Bio {

/**
 * @brief Adapteur pour BiologicalNetwork vers IBioComputeInterface
 * 
 * Permet d'utiliser le réseau biologique simulé via l'interface commune.
 */
class BiologicalNetworkAdapter : public IBioComputeInterface
{
    Q_OBJECT

public:
    explicit BiologicalNetworkAdapter(QObject *parent = nullptr);
    explicit BiologicalNetworkAdapter(QSharedPointer<Network::BiologicalNetwork> network, QObject *parent = nullptr);
    ~BiologicalNetworkAdapter() override;

    // IBioComputeInterface implementation
    ComputeMode getComputeMode() const override { return ComputeMode::SimulatedNetwork; }
    bool initialize() override;
    bool isReady() const override;
    bool applyStimulus(const StimulusPattern &pattern) override;
    BioResponse captureResponse(int waitTimeMs = 100) override;
    BioResponse stimulateAndCapture(const StimulusPattern &pattern) override;
    void reinforcePattern(const StimulusPattern &pattern, uint32_t nonce, double reward) override;
    QString getDiagnosticInfo() const override;
    void reset() override;

    // Configuration spécifique Network
    void setNetworkConfig(const Network::BiologicalNetwork::NetworkConfig &config);
    Network::BiologicalNetwork* getNetwork() const { return m_network.data(); }

private slots:
    void onNetworkLearningStateChanged(Network::BiologicalNetwork::LearningState state);

private:
    QSharedPointer<Network::BiologicalNetwork> m_network;
    mutable QMutex m_mutex;
    bool m_initialized;
    
    // Stockage temporaire des signaux de stimulation
    QVector<double> m_lastStimulusSignals;
    
    // Conversion des réponses du réseau
    BioResponse convertNetworkOutputToResponse(const QVector<double> &output);
    double calculateNetworkResponseStrength(const QVector<double> &output);
    double calculateNetworkSignalQuality(const QVector<double> &output);
    
    // Simuler un délai biologique réaliste
    void simulateBiologicalDelay(int durationMs);
};

} // namespace Bio
} // namespace BioMining

#endif // BIOLOGICAL_NETWORK_ADAPTER_H
