#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QMessageBox>

#include "mainwindow.h"

// Configuration du logging Qt
Q_LOGGING_CATEGORY(bioMining, "biomining")

void setupApplicationProperties()
{
    QApplication::setOrganizationName("BioLab Research");
    QApplication::setOrganizationDomain("biolab.research");
    QApplication::setApplicationName("Bio-Crypto Mining Platform");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setApplicationDisplayName("Plateforme Bio-Mining Bitcoin");
}

void setupApplicationStyle(QApplication &app)
{
    // Style sombre moderne
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
    
    // Style CSS additionnel pour les éléments spécifiques
    app.setStyleSheet(
        "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"
        "QGroupBox { font-weight: bold; border: 2px solid #2a82da; border-radius: 5px; margin-top: 1ex; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; }"
        "QPushButton { border: 1px solid #2a82da; border-radius: 3px; padding: 3px; min-width: 80px; }"
        "QPushButton:hover { background-color: #2a82da; }"
        "QPushButton:pressed { background-color: #1e6ba8; }"
        "QProgressBar { border: 1px solid #2a82da; border-radius: 3px; text-align: center; }"
        "QProgressBar::chunk { background-color: #2a82da; border-radius: 2px; }"
    );
}

QPixmap createSplashScreen()
{
    // Création d'un splash screen simple
    QPixmap splashPixmap(600, 300);
    splashPixmap.fill(QColor(25, 25, 25));
    
    QPainter painter(&splashPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Titre
    QFont titleFont("Arial", 24, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(Qt::white);
    painter.drawText(QRect(0, 80, 600, 40), Qt::AlignCenter, "Bio-Crypto Mining Platform");
    
    // Sous-titre
    QFont subFont("Arial", 12);
    painter.setFont(subFont);
    painter.setPen(QColor(42, 130, 218));
    painter.drawText(QRect(0, 130, 600, 20), Qt::AlignCenter, "Plateforme Hybride Bio-Informatique");
    
    // Version
    painter.setPen(Qt::gray);
    painter.drawText(QRect(0, 160, 600, 20), Qt::AlignCenter, "Version 1.0.0 - Research Edition");
    
    // Logo/icône stylisée
    painter.setPen(QPen(QColor(42, 130, 218), 3));
    painter.setBrush(Qt::NoBrush);
    
    // Cercle principal
    painter.drawEllipse(QRect(260, 200, 80, 80));
    
    // Éléments de design (électrodes)
    for (int i = 0; i < 8; ++i) {
        double angle = i * M_PI / 4;
        int x1 = 300 + 30 * cos(angle);
        int y1 = 240 + 30 * sin(angle);
        int x2 = 300 + 40 * cos(angle);
        int y2 = 240 + 40 * sin(angle);
        
        painter.drawLine(x1, y1, x2, y2);
        painter.drawEllipse(x2-2, y2-2, 4, 4);
    }
    
    return splashPixmap;
}

bool checkSystemRequirements()
{
    // Vérifications système de base
    bool allOk = true;
    QStringList issues;
    
    // Vérification des répertoires de données
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(dataDir)) {
        issues << "Impossible de créer le répertoire de données: " + dataDir;
        allOk = false;
    }
    
    // Vérification des permissions d'écriture
    QString tempFile = dataDir + "/test_write.tmp";
    QFile testFile(tempFile);
    if (!testFile.open(QIODevice::WriteOnly)) {
        issues << "Permissions d'écriture insuffisantes: " + dataDir;
        allOk = false;
    } else {
        testFile.close();
        testFile.remove();
    }
    
    // Log des vérifications
    if (allOk) {
        qCInfo(bioMining) << "Vérifications système: OK";
        qCInfo(bioMining) << "Répertoire données:" << dataDir;
    } else {
        qCWarning(bioMining) << "Problèmes système détectés:";
        for (const QString &issue : issues) {
            qCWarning(bioMining) << " -" << issue;
        }
    }
    
    return allOk;
}

void setupLogging()
{
    // Configuration du système de logging
    QLoggingCategory::setFilterRules("biomining.debug=true\n"
                                   "biomining.info=true\n"
                                   "biomining.warning=true\n"
                                   "biomining.critical=true");
    
    // Optionnel: rediriger les logs vers un fichier
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);
    
    // Note: Dans une vraie application, on pourrait utiliser une solution
    // de logging plus avancée comme spdlog ou Qt's built-in logging framework
}

int main(int argc, char *argv[])
{
    // Création de l'application Qt
    QApplication app(argc, argv);
    
    // Configuration des propriétés de l'application
    setupApplicationProperties();
    
    // Parser les arguments de ligne de commande
    QCommandLineParser parser;
    parser.setApplicationDescription("Plateforme Hybride Bio-Informatique pour Mining Bitcoin");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption debugOption("debug", "Activer le mode debug");
    parser.addOption(debugOption);
    
    QCommandLineOption noSplashOption("no-splash", "Désactiver l'écran de démarrage");
    parser.addOption(noSplashOption);
    
    QCommandLineOption configOption("config", "Fichier de configuration", "config-file");
    parser.addOption(configOption);
    
    parser.process(app);
    
    // Configuration du logging
    setupLogging();
    
    // Mode debug
    if (parser.isSet(debugOption)) {
        QLoggingCategory::setFilterRules("*.debug=true");
        qCInfo(bioMining) << "Mode debug activé";
    }
    
    qCInfo(bioMining) << "=== DÉMARRAGE PLATEFORME BIO-MINING ===";
    qCInfo(bioMining) << "Version:" << QApplication::applicationVersion();
    qCInfo(bioMining) << "Qt Version:" << qVersion();
    qCInfo(bioMining) << "Plateforme:" << QSysInfo::prettyProductName();
    
    // Vérifications système
    if (!checkSystemRequirements()) {
        QMessageBox::critical(nullptr, "Erreur Système",
                             "Les vérifications système ont échoué.\n"
                             "Vérifiez les logs pour plus de détails.\n"
                             "L'application peut ne pas fonctionner correctement.");
        // On continue quand même, mais avec un avertissement
    }
    
    // Configuration du style
    setupApplicationStyle(app);
    
    // Splash Screen
    std::unique_ptr<QSplashScreen> splash;
    if (!parser.isSet(noSplashOption)) {
        QPixmap splashPixmap = createSplashScreen();
        splash = std::make_unique<QSplashScreen>(splashPixmap);
        splash->show();
        splash->showMessage("Initialisation des composants bio-mining...", 
                           Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        
        app.processEvents();
        QThread::msleep(1000); // Pause pour montrer le splash
    }
    
    try {
        // Création de la fenêtre principale
        if (splash) {
            splash->showMessage("Chargement de l'interface utilisateur...", 
                               Qt::AlignBottom | Qt::AlignCenter, Qt::white);
            app.processEvents();
        }
        
        MainWindow window;
        
        // Configuration optionnelle depuis fichier de config
        if (parser.isSet(configOption)) {
            QString configFile = parser.value(configOption);
            qCInfo(bioMining) << "Chargement configuration:" << configFile;
            // Implémentation du chargement de config à ajouter
        }
        
        // Affichage de la fenêtre
        if (splash) {
            splash->showMessage("Finalisation du démarrage...", 
                               Qt::AlignBottom | Qt::AlignCenter, Qt::white);
            app.processEvents();
            QThread::msleep(500);
            
            splash->finish(&window);
        }
        
        window.show();
        
        qCInfo(bioMining) << "Interface utilisateur chargée avec succès";
        qCInfo(bioMining) << "Plateforme Bio-Mining prête à l'utilisation";
        
        // Boucle d'événements principale
        int result = app.exec();
        
        qCInfo(bioMining) << "=== ARRÊT PLATEFORME BIO-MINING ===";
        qCInfo(bioMining) << "Code de sortie:" << result;
        
        return result;
        
    } catch (const std::exception &e) {
        qCCritical(bioMining) << "Exception fatale:" << e.what();
        
        if (splash) {
            splash->close();
        }
        
        QMessageBox::critical(nullptr, "Erreur Fatale",
                             QString("Une erreur fatale s'est produite:\n%1\n\n"
                                   "L'application va se fermer.").arg(e.what()));
        
        return -1;
        
    } catch (...) {
        qCCritical(bioMining) << "Exception fatale inconnue";
        
        if (splash) {
            splash->close();
        }
        
        QMessageBox::critical(nullptr, "Erreur Fatale",
                             "Une erreur fatale inconnue s'est produite.\n"
                             "L'application va se fermer.");
        
        return -2;
    }
}