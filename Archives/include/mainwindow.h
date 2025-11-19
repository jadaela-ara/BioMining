#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QVector>
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QSplitter>
#include <QTabWidget>
#include <memory>

#include "bio/mea_interface.h"
#include "crypto/bitcoin_miner.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

/**
 * @brief Interface principale de la plateforme hybride bio-mining
 * 
 * Fournit une interface utilisateur complète pour contrôler le mining Bitcoin
 * hybride avec interface MEA et optimisation biologique.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // MEA Control
    void onConnectMEA();
    void onDisconnectMEA();
    void onStartAcquisition();
    void onStopAcquisition();
    void onStartLearning();
    void onCalibrate();
    void onSaveCalibration();
    void onLoadCalibration();

    // Mining Control
    void onStartMining();
    void onStopMining();
    void onStartContinuousMining();
    void onConfigureMining();

    // MEA Events
    void onMEAStatusChanged(BioMining::Bio::MEAInterface::ConnectionStatus status);
    void onMEASignalsReceived(const QVector<double> &currentSignals);
    void onMEAError(const QString &error);
    void onCalibrationChanged(double factor);

    // Mining Events
    void onMiningComplete(const BioMining::Crypto::BitcoinMiner::MiningResult &result);
    void onHashRateUpdated(double hashrate);
    void onMiningProgress(int attempts, double progress);
    void onDifficultyAdjusted(uint64_t difficulty);

    // UI Updates
    void updateStatusDisplay();
    void updateStatistics();
    void onUpdateTimer();

    // Menu Actions
    void onAbout();
    void onSettings();
    void onExportData();
    void onImportConfig();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    
    void setupMEATab(); // Declared here
    void setupMiningTab(); // Declared here
    void setupMonitorTab(); // Declared here
    
    void log(const QString &message, const QString &level = "INFO");
    void updateMEADisplay(const QVector<double> &currentSignals);
    void updateMiningStats();
    
    // Core components
    std::unique_ptr<BioMining::Bio::MEAInterface> m_meaInterface;
    std::unique_ptr<BioMining::Crypto::BitcoinMiner> m_bitcoinMiner;
    
    // UI Components - Main Layout
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;
    QTabWidget *m_tabWidget;
    
    // MEA Control Tab
    QWidget *m_meaTab;
    QGroupBox *m_meaControlGroup;
    QGroupBox *m_meaStatusGroup;
    QGroupBox *m_meaSignalsGroup;
    
    QPushButton *m_btnConnectMEA;
    QPushButton *m_btnDisconnectMEA;
    QPushButton *m_btnStartAcquisition;
    QPushButton *m_btnStopAcquisition;
    QPushButton *m_btnStartLearning;
    QPushButton *m_btnCalibrate;
    QPushButton *m_btnSaveCalibration;
    QPushButton *m_btnLoadCalibration;
    
    QLabel *m_lblMEAStatus;
    QLabel *m_lblElectrodeCount;
    QLabel *m_lblSignalQuality;
    QLabel *m_lblCalibrationFactor;
    QProgressBar *m_progressSignalQuality;
    
    // Mining Control Tab
    QWidget *m_miningTab;
    QGroupBox *m_miningControlGroup;
    QGroupBox *m_miningStatsGroup;
    QGroupBox *m_miningConfigGroup;
    
    QPushButton *m_btnStartMining;
    QPushButton *m_btnStopMining;
    QPushButton *m_btnContinuousMining;
    QPushButton *m_btnConfigureMining;
    
    QLabel *m_lblHashrate;
    QLabel *m_lblTotalAttempts;
    QLabel *m_lblSuccessCount;
    QLabel *m_lblSuccessRate;
    QLabel *m_lblCurrentDifficulty;
    QProgressBar *m_progressMining;
    
    QLineEdit *m_editBlockHeader;
    QSpinBox *m_spinMaxAttempts;
    QDoubleSpinBox *m_spinSignalWeight;
    QSpinBox *m_spinThreadCount;
    QComboBox *m_comboOptimization;
    
    // Monitoring Tab
    QWidget *m_monitorTab;
    QPlainTextEdit *m_console;
    
    // Status Bar
    QLabel *m_statusLabel;
    QLabel *m_connectionLabel;
    QLabel *m_miningLabel;
    
    // Timers
    std::unique_ptr<QTimer> m_updateTimer;
    
    // Statistics
    int m_sessionStartTime;
    int m_totalMiningTime;
    double m_bestHashrate;
    int m_totalSuccesses;
    
    // Configuration
    QString m_configFilePath;
    QString m_calibrationFilePath;
};

#endif // MAINWINDOW_H
