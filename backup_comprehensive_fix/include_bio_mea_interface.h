#ifndef MEA_INTERFACE_H
#define MEA_INTERFACE_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <memory>

// Namespace pour éviter les conflits
namespace BioMining {
namespace Bio {

/**
 * @brief Interface pour la gestion des Multi-Electrode Arrays (MEA)
 * 
 * Cette classe fournit une interface standardisée pour l'acquisition et la stimulation
 * des signaux biologiques via des électrodes multiples.
 */
class MEAInterface : public QObject
{
    Q_OBJECT

public:
    enum class ConnectionStatus {
        Disconnected,
        Connecting,
        Connected,
        Error
    };

    explicit MEAInterface(QObject *parent = nullptr);
    ~MEAInterface();

    // Configuration et connexion
    bool initialize(const QString &configPath = "");
    void disconnect();
    ConnectionStatus getStatus() const { return m_status; }

    // Acquisition de données
    QVector<double> readSignals();
    QVector<double> readSignalsAsync(); // Version non-bloquante
    void startContinuousAcquisition(int intervalMs = 100);
    void stopContinuousAcquisition();

    // Stimulation
    void stimulate(const QVector<double> &pattern);
    void stimulateAsync(const QVector<double> &pattern);
    void setStimulationParameters(double amplitude, double duration, double frequency);

    // Calibration
    void adjustCalibration(double factor);
    double getCalibration() const;
    void resetCalibration();
    void saveCalibration(const QString &filepath = "");
    bool loadCalibration(const QString &filepath = "");

    // Diagnostic et monitoring
    QVector<double> getLastSignals() const;
    double getSignalQuality() const;
    QString getLastError() const;
    
signals:
    void signalsAcquired(const QVector<double> &signalData);
    void stimulationComplete();
    void statusChanged(ConnectionStatus status);
    void errorOccurred(const QString &error);
    void calibrationChanged(double factor);

private slots:
    void onAcquisitionTimer();

private:
    void setStatus(ConnectionStatus status);
    void setError(const QString &error);
    bool validateSignals(const QVector<double> &signalData) const;

    ConnectionStatus m_status;
    double m_calibrationFactor;
    QVector<double> m_lastSignals;
    QString m_lastError;
    
    // Configuration stimulation
    double m_stimAmplitude;
    double m_stimDuration;
    double m_stimFrequency;
    
    // Acquisition continue
    std::unique_ptr<QTimer> m_acquisitionTimer;
    bool m_continuousMode;
    
    // Thread safety
    mutable QMutex m_mutex;
    
    // Simulation parameters (remplacé par vraie API hardware)
    static constexpr int ELECTRODE_COUNT = 60;
    static constexpr double MIN_CALIBRATION = 0.1;
    static constexpr double MAX_CALIBRATION = 5.0;
};

} // namespace Bio
} // namespace BioMining

#endif // MEA_INTERFACE_H