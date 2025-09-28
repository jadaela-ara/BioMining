#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QVector> // Moved for QVector
#include <QFile> // Added for QFile
#include <QIODevice> // Added for QIODevice
#include <iostream>
#include <iomanip>
#include <numeric> // For std::accumulate, std::max_element, std::min_element
#include <algorithm> // For std::accumulate, std::max_element, std::min_element
#include <QRandomGenerator> // For QRandomGenerator

#include "bio/mea_interface.h"
#include "crypto/bitcoin_miner.h"

class BiominingCLI : public QObject
{
    Q_OBJECT

public:
    BiominingCLI(QObject *parent = nullptr);
    ~BiominingCLI();

    int run(const QStringList &arguments);

private slots:
    void onMEASignalsReceived(const QVector<double> &currentSignals);
    void onMiningComplete(const BioMining::Crypto::BitcoinMiner::MiningResult &result);
    void onTimeout();

private:
    void printHeader();
    void printUsage();
    void printStatus();
    void printMiningStats();
    void runBenchmark();
    void runAutomaticMode();
    void runSingleMining();
    void setupMEA();
    void setupMiner();
    void saveResultsToFile(); // Declared here

    std::unique_ptr<BioMining::Bio::MEAInterface> m_meaInterface;
    std::unique_ptr<BioMining::Crypto::BitcoinMiner> m_bitcoinMiner;
    QTextStream m_out;
    QTextStream m_err;
    
    // Options de ligne de commande
    bool m_verbose;
    bool m_benchmark;
    bool m_automaticMode;
    int m_maxIterations;
    QString m_outputFile;
    QString m_configFile;
    
    // Statistiques
    int m_currentIteration;
    QElapsedTimer m_sessionTimer;
    QVector<BioMining::Crypto::BitcoinMiner::MiningResult> m_results; // Corrected type
};

BiominingCLI::BiominingCLI(QObject *parent)
    : QObject(parent)
    , m_out(stdout)
    , m_err(stderr)
    , m_verbose(false)
    , m_benchmark(false)
    , m_automaticMode(false)
    , m_maxIterations(10)
    , m_currentIteration(0)
{
    m_meaInterface = std::make_unique<BioMining::Bio::MEAInterface>(this);
    m_bitcoinMiner = std::make_unique<BioMining::Crypto::BitcoinMiner>(this);
    
    // Connexions pour le mode automatique
    connect(m_meaInterface.get(), &BioMining::Bio::MEAInterface::signalsAcquired,
            this, &BiominingCLI::onMEASignalsReceived);
    connect(m_bitcoinMiner.get(), &BioMining::Crypto::BitcoinMiner::miningComplete,
            this, &BiominingCLI::onMiningComplete);
}

BiominingCLI::~BiominingCLI()
{
    if (m_meaInterface) {
        m_meaInterface->disconnect();
    }
    if (m_bitcoinMiner) {
        m_bitcoinMiner->stopMining();
    }
}

int BiominingCLI::run(const QStringList &arguments)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Bio-Crypto Mining Platform - Interface Ligne de Commande");
    
    // Options disponibles
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption verboseOption({"v", "verbose"}, "Mode verbeux");
    parser.addOption(verboseOption);
    
    QCommandLineOption benchmarkOption({"b", "benchmark"}, "Mode benchmark");
    parser.addOption(benchmarkOption);
    
    QCommandLineOption automaticOption({"a", "automatic"}, "Mode automatique continu");
    parser.addOption(automaticOption);
    
    QCommandLineOption iterationsOption({"i", "iterations"}, 
                                       "Nombre d'itérations (défaut: 10)", "count", "10");
    parser.addOption(iterationsOption);
    
    QCommandLineOption outputOption({"o", "output"}, 
                                   "Fichier de sortie pour les résultats", "file");
    parser.addOption(outputOption);
    
    QCommandLineOption configOption({"c", "config"}, 
                                   "Fichier de configuration", "file");
    parser.addOption(configOption);
    
    QCommandLineOption difficultyOption({"d", "difficulty"}, 
                                       "Niveau de difficulté (hex)", "value");
    parser.addOption(difficultyOption);
    
    QCommandLineOption threadsOption({"t", "threads"}, 
                                    "Nombre de threads", "count");
    parser.addOption(threadsOption);
    
    // Parse des arguments
    parser.process(arguments);
    
    // Configuration des options
    m_verbose = parser.isSet(verboseOption);
    m_benchmark = parser.isSet(benchmarkOption);
    m_automaticMode = parser.isSet(automaticOption);
    m_maxIterations = parser.value(iterationsOption).toInt();
    m_outputFile = parser.value(outputOption);
    m_configFile = parser.value(configOption);
    
    // Affichage de l'en-tête
    printHeader();
    
    // Configuration des composants
    setupMEA();
    setupMiner();
    
    // Configuration spécifique depuis les arguments
    if (parser.isSet(difficultyOption)) {
        bool ok;
        uint64_t difficulty = parser.value(difficultyOption).toULongLong(&ok, 16);
        if (ok) {
            BioMining::Crypto::BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
            config.difficulty = difficulty;
            m_bitcoinMiner->setMiningConfig(config);
            
            if (m_verbose) {
                m_out << "Difficulté configurée: 0x" << Qt::hex << difficulty << Qt::dec << Qt::endl;
            }
        } else {
            m_err << "Erreur: Valeur de difficulté invalide" << Qt::endl;
            return 1;
        }
    }
    
    if (parser.isSet(threadsOption)) {
        int threads = parser.value(threadsOption).toInt();
        if (threads > 0 && threads <= 32) {
            BioMining::Crypto::BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
            config.threadCount = threads;
            m_bitcoinMiner->setMiningConfig(config);
            
            if (m_verbose) {
                m_out << "Threads configurés: " << threads << Qt::endl;
            }
        } else {
            m_err << "Erreur: Nombre de threads invalide (1-32)" << Qt::endl;
            return 1;
        }
    }
    
    // Démarrage du timer de session
    m_sessionTimer.start();
    
    // Exécution selon le mode choisi
    try {
        if (m_benchmark) {
            runBenchmark();
        } else if (m_automaticMode) {
            runAutomaticMode();
        } else {
            runSingleMining();
        }
        
        // Statistiques finales
        printMiningStats();
        
        return 0;
        
    } catch (const std::exception &e) {
        m_err << "Erreur: " << e.what() << Qt::endl;
        return 1;
    }
}

void BiominingCLI::printHeader()
{
    m_out << "=====================================================" << Qt::endl;
    m_out << "  BIO-CRYPTO MINING PLATFORM - CLI v1.0.0" << Qt::endl;
    m_out << "  Plateforme Hybride Bio-Informatique" << Qt::endl;
    m_out << "=====================================================" << Qt::endl;
    m_out << Qt::endl;
}

void BiominingCLI::printStatus()
{
    m_out << "--- État Système ---" << Qt::endl;
    
    // État MEA
    QString meaStatus;
    switch (m_meaInterface->getStatus()) {
        case BioMining::Bio::MEAInterface::ConnectionStatus::Connected:
            meaStatus = "Connecté";
            break;
        case BioMining::Bio::MEAInterface::ConnectionStatus::Connecting:
            meaStatus = "Connexion...";
            break;
        case BioMining::Bio::MEAInterface::ConnectionStatus::Disconnected:
            meaStatus = "Déconnecté";
            break;
        case BioMining::Bio::MEAInterface::ConnectionStatus::Error:
            meaStatus = "Erreur";
            break;
    }
    
    m_out << "MEA: " << meaStatus << Qt::endl;
    m_out << "Calibration: " << QString::number(m_meaInterface->getCalibration(), 'f', 3) << Qt::endl;
    m_out << "Qualité Signal: " << QString::number(m_meaInterface->getSignalQuality() * 100, 'f', 1) << "%" << Qt::endl;
    
    // État Mining
    m_out << "Hashrate: " << QString::number(m_bitcoinMiner->getHashrate(), 'f', 0) << " H/s" << Qt::endl;
    m_out << "Succès: " << m_bitcoinMiner->getSuccessCount() << "/" << m_bitcoinMiner->getTotalAttempts() << Qt::endl;
    
    BioMining::Crypto::BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    m_out << "Difficulté: 0x" << Qt::hex << config.difficulty << Qt::dec << Qt::endl;
    m_out << "Threads: " << config.threadCount << Qt::endl;
    
    m_out << Qt::endl;
}

void BiominingCLI::runBenchmark()
{
    m_out << "=== MODE BENCHMARK ===" << Qt::endl;
    m_out << "Exécution de " << m_maxIterations << " cycles de mining..." << Qt::endl;
    m_out << Qt::endl;
    
    // Configuration pour benchmark
    BioMining::Crypto::BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.maxAttempts = 5000; // Limité pour benchmark
    m_bitcoinMiner->setMiningConfig(config);
    
    for (int i = 0; i < m_maxIterations; ++i) {
        m_out << "Cycle " << (i + 1) << "/" << m_maxIterations << "... ";
        m_out.flush();
        
        // Génération de signaux de test
        QVector<double> testSignals(60);
        for (int j = 0; j < 60; ++j) {
            testSignals[j] = sin(j * 0.1 + i * 0.05) * cos(i * 0.02);
        }
        
        QElapsedTimer cycleTimer;
        cycleTimer.start();
        
        BioMining::Crypto::BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(testSignals);
        m_results.append(result);
        
        m_out << (result.success ? "SUCCÈS" : "ÉCHEC") 
              << " (" << cycleTimer.elapsed() << "ms, "
              << QString::number(result.attempts / result.computeTime, 'f', 0) << " H/s)" << Qt::endl;
        
        if (m_verbose && result.success) {
            m_out << "  Nonce: " << result.nonce << Qt::endl;
            m_out << "  Hash: " << result.hash.left(32) << "..." << Qt::endl;
        }
    }
}

void BiominingCLI::runAutomaticMode()
{
    m_out << "=== MODE AUTOMATIQUE ===" << Qt::endl;
    m_out << "Connexion MEA et démarrage mining continu..." << Qt::endl;
    
    // Connexion MEA
    if (!m_meaInterface->initialize()) {
        m_out << "ATTENTION: Connexion MEA échouée, utilisation de signaux simulés" << Qt::endl;
    }
    
    // Démarrage acquisition continue
    m_meaInterface->startContinuousAcquisition(200); // 5 Hz
    m_bitcoinMiner->startContinuousMining();
    
    m_out << "Mining automatique démarré. Appuyez sur Ctrl+C pour arrêter..." << Qt::endl;
    m_out << Qt::endl;
    
    // Timer pour affichage périodique des statistiques
    QTimer *statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &BiominingCLI::onTimeout);
    statusTimer->start(10000); // Chaque 10 secondes
    
    // Timer d'arrêt automatique si maxIterations est défini
    if (m_maxIterations > 0) {
        QTimer::singleShot(m_maxIterations * 1000, [this]() {
            m_out << Qt::endl << "Arrêt automatique après " << m_maxIterations << " secondes." << Qt::endl;
            QCoreApplication::quit();
        });
    }
}

void BiominingCLI::runSingleMining()
{
    m_out << "=== MINING UNIQUE ===" << Qt::endl;
    
    printStatus();
    
    // Acquisition des signaux
    m_out << "Acquisition des signaux biologiques..." << Qt::endl;
    QVector<double> bioSignals = m_meaInterface->readSignals();
    
    if (bioSignals.isEmpty()) {
        m_out << "Aucun signal MEA - génération de signaux simulés" << Qt::endl;
        bioSignals.resize(60);
        for (int i = 0; i < 60; ++i) {
            bioSignals[i] = QRandomGenerator::global()->generateDouble() - 0.5;
        }
    }
    
    m_out << "Signaux acquis: " << bioSignals.size() << " électrodes" << Qt::endl;
    
    // Mining
    m_out << "Démarrage mining..." << Qt::endl;
    BioMining::Crypto::BitcoinMiner::MiningResult result = m_bitcoinMiner->mine(bioSignals);
    
    m_out << Qt::endl;
    m_out << "=== RÉSULTAT ===" << Qt::endl;
    m_out << "Succès: " << (result.success ? "OUI" : "NON") << Qt::endl;
    m_out << "Tentatives: " << result.attempts << Qt::endl;
    m_out << "Temps: " << QString::number(result.computeTime, 'f', 3) << " secondes" << Qt::endl;
    m_out << "Hashrate: " << QString::number(result.attempts / result.computeTime, 'f', 0) << " H/s" << Qt::endl;
    m_out << "Contribution Signal: " << QString::number(result.signalContribution * 100, 'f', 1) << "%" << Qt::endl;
    
    if (result.success) {
        m_out << "Nonce: " << result.nonce << Qt::endl;
        m_out << "Hash: " << result.hash << Qt::endl;
    }
    
    m_results.append(result);
}

void BiominingCLI::setupMEA()
{
    if (m_verbose) {
        m_out << "Configuration MEA..." << Qt::endl;
    }
    
    // Tentative de connexion (non bloquante)
    if (!m_configFile.isEmpty()) {
        m_meaInterface->loadCalibration(m_configFile);
    }
    
    // La connexion sera tentée lors de l'utilisation
}

void BiominingCLI::setupMiner()
{
    if (m_verbose) {
        m_out << "Configuration Bitcoin Miner..." << Qt::endl;
    }
    
    BioMining::Crypto::BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    
    // Configuration par défaut optimisée pour CLI
    config.maxAttempts = 20000;
    config.signalWeight = 1.5;
    config.threadCount = QThread::idealThreadCount();
    
    m_bitcoinMiner->setMiningConfig(config);
}

void BiominingCLI::printMiningStats()
{
    if (m_results.isEmpty()) {
        return;
    }
    
    m_out << Qt::endl;
    m_out << "=== STATISTIQUES FINALES ===" << Qt::endl;
    
    int totalAttempts = 0;
    int totalSuccesses = 0;
    double totalTime = 0.0;
    double maxHashrate = 0.0;
    double totalSignalContrib = 0.0;
    
    for (const auto &result : m_results) {
        totalAttempts += result.attempts;
        if (result.success) totalSuccesses++;
        totalTime += result.computeTime;
        
        double hashrate = result.attempts / result.computeTime;
        if (hashrate > maxHashrate) {
            maxHashrate = hashrate;
        }
        
        totalSignalContrib += result.signalContribution;
    }
    
    double avgHashrate = totalAttempts / totalTime;
    double successRate = static_cast<double>(totalSuccesses) / m_results.size();
    double avgSignalContrib = totalSignalContrib / m_results.size();
    double sessionTime = m_sessionTimer.elapsed() / 1000.0;
    
    m_out << "Cycles exécutés: " << m_results.size() << Qt::endl;
    m_out << "Durée session: " << QString::number(sessionTime, 'f', 1) << " secondes" << Qt::endl;
    m_out << "Tentatives totales: " << totalAttempts << Qt::endl;
    m_out << "Succès: " << totalSuccesses << " (" << QString::number(successRate * 100, 'f', 1) << "%)" << Qt::endl;
    m_out << "Hashrate moyen: " << QString::number(avgHashrate, 'f', 0) << " H/s" << Qt::endl;
    m_out << "Hashrate maximum: " << QString::number(maxHashrate, 'f', 0) << " H/s" << Qt::endl;
    m_out << "Contribution signal moyenne: " << QString::number(avgSignalContrib * 100, 'f', 1) << "%" << Qt::endl;
    
    // Sauvegarde dans fichier si demandé
    if (!m_outputFile.isEmpty()) {
        saveResultsToFile();
    }
}

void BiominingCLI::saveResultsToFile()
{
    if (m_outputFile.isEmpty()) {
        m_err << "Erreur: Aucun fichier de sortie spécifié." << Qt::endl;
        return;
    }

    QFile file(m_outputFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_err << "Erreur: Impossible d'ouvrir le fichier de sortie: " << file.errorString() << Qt::endl;
        return;
    }

    QTextStream out(&file);
    out << "[";
    for (int i = 0; i < m_results.size(); ++i) {
        const auto &result = m_results.at(i);
        QJsonObject obj;
        obj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        obj["success"] = result.success;
        obj["attempts"] = (qint64)result.attempts;
        obj["nonce"] = (qint64)result.nonce;
        obj["hash"] = result.hash;
        obj["difficulty"] = (qint64)result.difficulty;
        obj["computeTime"] = (qint64)result.computeTime;
        obj["signalInfluence"] = result.signalInfluence;
        out << QJsonDocument(obj).toJson(QJsonDocument::Indented);
        if (i < m_results.size() - 1) {
            out << ",\n";
        } else {
            out << "\n";
        }
    }
    out << "]\n";
    file.close();
    m_out << "Résultats sauvegardés dans: " << m_outputFile << Qt::endl;
}

void BiominingCLI::onMEASignalsReceived(const QVector<double> &currentSignals)
{
    if (m_automaticMode && m_verbose) {
        static int signalCount = 0;
        if (++signalCount % 10 == 0) {
            double avg = std::accumulate(currentSignals.begin(), currentSignals.end(), 0.0) / currentSignals.size();
            m_out << "[" << QTime::currentTime().toString("hh:mm:ss") 
                  << "] Signaux MEA reçus (" << currentSignals.size() 
                  << " électrodes, moyenne: " << QString::number(avg, 'f', 4) << ")" << Qt::endl;
        }
    }
}

void BiominingCLI::onMiningComplete(const BioMining::Crypto::BitcoinMiner::MiningResult &result)
{
    m_results.append(result);
    m_currentIteration++;
    
    if (m_automaticMode) {
        m_out << "[" << QTime::currentTime().toString("hh:mm:ss") 
              << "] Mining " << m_currentIteration 
              << " - " << (result.success ? "SUCCÈS" : "ÉCHEC")
              << " (" << result.attempts << " attempts, "
              << QString::number(result.computeTime, 'f', 2) << "s, "
              << QString::number(result.attempts / result.computeTime, 'f', 0) << " H/s)" << Qt::endl;
        
        if (m_maxIterations > 0 && m_currentIteration >= m_maxIterations) {
            m_out << "Maximum d'itérations atteint (" << m_maxIterations << ")" << Qt::endl;
            QCoreApplication::quit();
        }
    }
}

void BiominingCLI::onTimeout()
{
    if (m_automaticMode) {
        m_out << Qt::endl << "--- Status Update ---" << Qt::endl;
        printStatus();
    }
}

// Point d'entrée principal pour l'application CLI
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    app.setApplicationName("BioCrypto Mining CLI");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("BioLab Research");
    
    BiominingCLI cli;
    
    // Gestion du signal Ctrl+C pour arrêt propre
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&cli]() {
        std::cout << std::endl << "Arrêt en cours..." << std::endl;
    });
    
    int result = cli.run(app.arguments());
    
    return result;
}
