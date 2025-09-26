#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QSplitter>
#include <QTabWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_meaInterface(std::make_unique<MEAInterface>(this))
    , m_bitcoinMiner(std::make_unique<BitcoinMiner>(this))
    , m_updateTimer(std::make_unique<QTimer>(this))
    , m_sessionStartTime(QDateTime::currentSecsSinceEpoch())
    , m_totalMiningTime(0)
    , m_bestHashrate(0.0)
    , m_totalSuccesses(0)
{
    setWindowTitle("Plateforme Hybride Bio-Mining Bitcoin v1.0");
    setMinimumSize(1200, 800);
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    
    // Configuration des chemins de fichiers
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_configFilePath = dataDir + "/config.ini";
    m_calibrationFilePath = dataDir + "/calibration.json";
    
    // Timer de mise à jour UI
    m_updateTimer->start(1000); // 1 seconde
    
    log("Plateforme Bio-Mining initialisée", "SYSTEM");
    updateStatusDisplay();
}

MainWindow::~MainWindow()
{
    if (m_meaInterface) {
        m_meaInterface->disconnect();
    }
    if (m_bitcoinMiner) {
        m_bitcoinMiner->stopMining();
        m_bitcoinMiner->stopContinuousMining();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    // Splitter principal horizontal
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Onglets principaux
    m_tabWidget = new QTabWidget;
    
    // === TAB MEA CONTROL ===
    m_meaTab = new QWidget;
    setupMEATab();
    m_tabWidget->addTab(m_meaTab, "Contrôle MEA");
    
    // === TAB MINING CONTROL ===
    m_miningTab = new QWidget;
    setupMiningTab();
    m_tabWidget->addTab(m_miningTab, "Mining Bitcoin");
    
    // === TAB MONITORING ===
    m_monitorTab = new QWidget;
    setupMonitorTab();
    m_tabWidget->addTab(m_monitorTab, "Monitoring");
    
    // Configuration du splitter
    m_mainSplitter->addWidget(m_tabWidget);
    
    // Layout principal
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_mainSplitter);
}

void MainWindow::setupMEATab()
{
    QVBoxLayout *layout = new QVBoxLayout(m_meaTab);
    
    // Groupe Contrôle MEA
    m_meaControlGroup = new QGroupBox("Contrôle MEA");
    QGridLayout *controlLayout = new QGridLayout(m_meaControlGroup);
    
    m_btnConnectMEA = new QPushButton("Connecter MEA");
    m_btnDisconnectMEA = new QPushButton("Déconnecter");
    m_btnStartAcquisition = new QPushButton("Démarrer Acquisition");
    m_btnStopAcquisition = new QPushButton("Arrêter Acquisition");
    m_btnStartLearning = new QPushButton("Apprentissage Initial");
    m_btnCalibrate = new QPushButton("Calibrer");
    m_btnSaveCalibration = new QPushButton("Sauvegarder Calibration");
    m_btnLoadCalibration = new QPushButton("Charger Calibration");
    
    controlLayout->addWidget(m_btnConnectMEA, 0, 0);
    controlLayout->addWidget(m_btnDisconnectMEA, 0, 1);
    controlLayout->addWidget(m_btnStartAcquisition, 1, 0);
    controlLayout->addWidget(m_btnStopAcquisition, 1, 1);
    controlLayout->addWidget(m_btnStartLearning, 2, 0);
    controlLayout->addWidget(m_btnCalibrate, 2, 1);
    controlLayout->addWidget(m_btnSaveCalibration, 3, 0);
    controlLayout->addWidget(m_btnLoadCalibration, 3, 1);
    
    // Groupe Statut MEA
    m_meaStatusGroup = new QGroupBox("Statut MEA");
    QGridLayout *statusLayout = new QGridLayout(m_meaStatusGroup);
    
    m_lblMEAStatus = new QLabel("Déconnecté");
    m_lblElectrodeCount = new QLabel("0");
    m_lblSignalQuality = new QLabel("0%");
    m_lblCalibrationFactor = new QLabel("1.0");
    m_progressSignalQuality = new QProgressBar;
    m_progressSignalQuality->setRange(0, 100);
    
    statusLayout->addWidget(new QLabel("Statut:"), 0, 0);
    statusLayout->addWidget(m_lblMEAStatus, 0, 1);
    statusLayout->addWidget(new QLabel("Électrodes:"), 1, 0);
    statusLayout->addWidget(m_lblElectrodeCount, 1, 1);
    statusLayout->addWidget(new QLabel("Qualité Signal:"), 2, 0);
    statusLayout->addWidget(m_lblSignalQuality, 2, 1);
    statusLayout->addWidget(m_progressSignalQuality, 2, 2);
    statusLayout->addWidget(new QLabel("Facteur Calibration:"), 3, 0);
    statusLayout->addWidget(m_lblCalibrationFactor, 3, 1);
    
    // Groupe Signaux MEA
    m_meaSignalsGroup = new QGroupBox("Signaux Biologiques");
    QVBoxLayout *signalsLayout = new QVBoxLayout(m_meaSignalsGroup);
    // Ici on ajouterait un widget de visualisation des signaux (QCustomPlot par exemple)
    QLabel *signalsPlaceholder = new QLabel("Visualisation des signaux en temps réel");
    signalsPlaceholder->setAlignment(Qt::AlignCenter);
    signalsPlaceholder->setStyleSheet("border: 1px solid gray; padding: 20px;");
    signalsLayout->addWidget(signalsPlaceholder);
    
    layout->addWidget(m_meaControlGroup);
    layout->addWidget(m_meaStatusGroup);
    layout->addWidget(m_meaSignalsGroup);
}

void MainWindow::setupMiningTab()
{
    QVBoxLayout *layout = new QVBoxLayout(m_miningTab);
    
    // Groupe Contrôle Mining
    m_miningControlGroup = new QGroupBox("Contrôle Mining");
    QHBoxLayout *controlLayout = new QHBoxLayout(m_miningControlGroup);
    
    m_btnStartMining = new QPushButton("Démarrer Mining");
    m_btnStopMining = new QPushButton("Arrêter Mining");
    m_btnContinuousMining = new QPushButton("Mode Continu");
    m_btnConfigureMining = new QPushButton("Configuration");
    
    m_btnStartMining->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    m_btnStopMining->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    
    controlLayout->addWidget(m_btnStartMining);
    controlLayout->addWidget(m_btnStopMining);
    controlLayout->addWidget(m_btnContinuousMining);
    controlLayout->addWidget(m_btnConfigureMining);
    
    // Groupe Statistiques Mining
    m_miningStatsGroup = new QGroupBox("Statistiques Mining");
    QGridLayout *statsLayout = new QGridLayout(m_miningStatsGroup);
    
    m_lblHashrate = new QLabel("0 H/s");
    m_lblTotalAttempts = new QLabel("0");
    m_lblSuccessCount = new QLabel("0");
    m_lblSuccessRate = new QLabel("0%");
    m_lblCurrentDifficulty = new QLabel("N/A");
    m_progressMining = new QProgressBar;
    
    statsLayout->addWidget(new QLabel("Hashrate:"), 0, 0);
    statsLayout->addWidget(m_lblHashrate, 0, 1);
    statsLayout->addWidget(new QLabel("Tentatives Totales:"), 1, 0);
    statsLayout->addWidget(m_lblTotalAttempts, 1, 1);
    statsLayout->addWidget(new QLabel("Succès:"), 2, 0);
    statsLayout->addWidget(m_lblSuccessCount, 2, 1);
    statsLayout->addWidget(new QLabel("Taux Succès:"), 3, 0);
    statsLayout->addWidget(m_lblSuccessRate, 3, 1);
    statsLayout->addWidget(new QLabel("Difficulté:"), 4, 0);
    statsLayout->addWidget(m_lblCurrentDifficulty, 4, 1);
    statsLayout->addWidget(new QLabel("Progrès:"), 5, 0);
    statsLayout->addWidget(m_progressMining, 5, 1);
    
    // Groupe Configuration Mining
    m_miningConfigGroup = new QGroupBox("Configuration Mining");
    QGridLayout *configLayout = new QGridLayout(m_miningConfigGroup);
    
    m_editBlockHeader = new QLineEdit("000000000000000000000000000000000000000000000000000000000000000000000000");
    m_spinMaxAttempts = new QSpinBox;
    m_spinMaxAttempts->setRange(1000, 100000);
    m_spinMaxAttempts->setValue(10000);
    m_spinSignalWeight = new QDoubleSpinBox;
    m_spinSignalWeight->setRange(0.1, 10.0);
    m_spinSignalWeight->setValue(1.0);
    m_spinSignalWeight->setSingleStep(0.1);
    m_spinThreadCount = new QSpinBox;
    m_spinThreadCount->setRange(1, 16);
    m_spinThreadCount->setValue(QThread::idealThreadCount());
    m_comboOptimization = new QComboBox;
    m_comboOptimization->addItems({"Standard", "Quantique", "Hybride"});
    
    configLayout->addWidget(new QLabel("Block Header:"), 0, 0);
    configLayout->addWidget(m_editBlockHeader, 0, 1);
    configLayout->addWidget(new QLabel("Max Tentatives:"), 1, 0);
    configLayout->addWidget(m_spinMaxAttempts, 1, 1);
    configLayout->addWidget(new QLabel("Poids Signaux:"), 2, 0);
    configLayout->addWidget(m_spinSignalWeight, 2, 1);
    configLayout->addWidget(new QLabel("Threads:"), 3, 0);
    configLayout->addWidget(m_spinThreadCount, 3, 1);
    configLayout->addWidget(new QLabel("Optimisation:"), 4, 0);
    configLayout->addWidget(m_comboOptimization, 4, 1);
    
    layout->addWidget(m_miningControlGroup);
    layout->addWidget(m_miningStatsGroup);
    layout->addWidget(m_miningConfigGroup);
}

void MainWindow::setupMonitorTab()
{
    QVBoxLayout *layout = new QVBoxLayout(m_monitorTab);
    
    m_console = new QPlainTextEdit;
    m_console->setReadOnly(true);
    m_console->setFont(QFont("Consolas", 9));
    m_console->setStyleSheet("QPlainTextEdit { background-color: #1e1e1e; color: #ffffff; }");
    
    layout->addWidget(new QLabel("Console de Log:"));
    layout->addWidget(m_console);
    
    // Boutons de contrôle console
    QHBoxLayout *consoleControls = new QHBoxLayout;
    QPushButton *btnClearConsole = new QPushButton("Effacer");
    QPushButton *btnSaveLogs = new QPushButton("Sauvegarder Logs");
    QPushButton *btnExportStats = new QPushButton("Exporter Stats");
    
    consoleControls->addWidget(btnClearConsole);
    consoleControls->addWidget(btnSaveLogs);
    consoleControls->addWidget(btnExportStats);
    consoleControls->addStretch();
    
    layout->addLayout(consoleControls);
    
    connect(btnClearConsole, &QPushButton::clicked, m_console, &QPlainTextEdit::clear);
    connect(btnSaveLogs, &QPushButton::clicked, this, &MainWindow::onExportData);
    connect(btnExportStats, &QPushButton::clicked, this, &MainWindow::onExportData);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // Menu Fichier
    QMenu *fileMenu = menuBar->addMenu("&Fichier");
    
    QAction *actImportConfig = new QAction("&Importer Configuration", this);
    QAction *actExportData = new QAction("&Exporter Données", this);
    QAction *actExit = new QAction("&Quitter", this);
    
    fileMenu->addAction(actImportConfig);
    fileMenu->addAction(actExportData);
    fileMenu->addSeparator();
    fileMenu->addAction(actExit);
    
    // Menu Outils
    QMenu *toolsMenu = menuBar->addMenu("&Outils");
    
    QAction *actSettings = new QAction("&Paramètres", this);
    QAction *actCalibrate = new QAction("&Calibration MEA", this);
    
    toolsMenu->addAction(actSettings);
    toolsMenu->addAction(actCalibrate);
    
    // Menu Aide
    QMenu *helpMenu = menuBar->addMenu("&Aide");
    
    QAction *actAbout = new QAction("À &propos", this);
    
    helpMenu->addAction(actAbout);
    
    // Connexions
    connect(actImportConfig, &QAction::triggered, this, &MainWindow::onImportConfig);
    connect(actExportData, &QAction::triggered, this, &MainWindow::onExportData);
    connect(actExit, &QAction::triggered, this, &QWidget::close);
    connect(actSettings, &QAction::triggered, this, &MainWindow::onSettings);
    connect(actCalibrate, &QAction::triggered, this, &MainWindow::onCalibrate);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Prêt");
    m_connectionLabel = new QLabel("MEA: Déconnecté");
    m_miningLabel = new QLabel("Mining: Arrêté");
    
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_connectionLabel);
    statusBar()->addPermanentWidget(m_miningLabel);
}

void MainWindow::setupConnections()
{
    // Connexions MEA
    connect(m_btnConnectMEA, &QPushButton::clicked, this, &MainWindow::onConnectMEA);
    connect(m_btnDisconnectMEA, &QPushButton::clicked, this, &MainWindow::onDisconnectMEA);
    connect(m_btnStartAcquisition, &QPushButton::clicked, this, &MainWindow::onStartAcquisition);
    connect(m_btnStopAcquisition, &QPushButton::clicked, this, &MainWindow::onStopAcquisition);
    connect(m_btnStartLearning, &QPushButton::clicked, this, &MainWindow::onStartLearning);
    connect(m_btnCalibrate, &QPushButton::clicked, this, &MainWindow::onCalibrate);
    connect(m_btnSaveCalibration, &QPushButton::clicked, this, &MainWindow::onSaveCalibration);
    connect(m_btnLoadCalibration, &QPushButton::clicked, this, &MainWindow::onLoadCalibration);
    
    // Signaux MEA
    connect(m_meaInterface.get(), &MEAInterface::statusChanged,
            this, &MainWindow::onMEAStatusChanged);
    connect(m_meaInterface.get(), &MEAInterface::signalsAcquired,
            this, &MainWindow::onMEASignalsReceived);
    connect(m_meaInterface.get(), &MEAInterface::errorOccurred,
            this, &MainWindow::onMEAError);
    connect(m_meaInterface.get(), &MEAInterface::calibrationChanged,
            this, &MainWindow::onCalibrationChanged);
    
    // Connexions Mining
    connect(m_btnStartMining, &QPushButton::clicked, this, &MainWindow::onStartMining);
    connect(m_btnStopMining, &QPushButton::clicked, this, &MainWindow::onStopMining);
    connect(m_btnContinuousMining, &QPushButton::clicked, this, &MainWindow::onStartContinuousMining);
    connect(m_btnConfigureMining, &QPushButton::clicked, this, &MainWindow::onConfigureMining);
    
    // Signaux Mining
    connect(m_bitcoinMiner.get(), &BitcoinMiner::miningComplete,
            this, &MainWindow::onMiningComplete);
    connect(m_bitcoinMiner.get(), &BitcoinMiner::hashRateUpdated,
            this, &MainWindow::onHashRateUpdated);
    connect(m_bitcoinMiner.get(), &BitcoinMiner::progressUpdate,
            this, &MainWindow::onMiningProgress);
    connect(m_bitcoinMiner.get(), &BitcoinMiner::difficultyAdjusted,
            this, &MainWindow::onDifficultyAdjusted);
    
    // Connexion MEA -> Mining pour le mode continu
    connect(m_meaInterface.get(), &MEAInterface::signalsAcquired,
            m_bitcoinMiner.get(), &BitcoinMiner::onBioSignalsReceived);
    
    // Timer de mise à jour
    connect(m_updateTimer.get(), &QTimer::timeout, this, &MainWindow::onUpdateTimer);
}