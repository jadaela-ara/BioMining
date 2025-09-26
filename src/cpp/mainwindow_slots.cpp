// Continuation de mainwindow.cpp - Implémentation des slots

#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

// === MEA CONTROL SLOTS ===

void MainWindow::onConnectMEA()
{
    log("Tentative de connexion MEA...");
    
    if (m_meaInterface->initialize()) {
        log("MEA connecté avec succès", "SUCCESS");
        m_btnConnectMEA->setEnabled(false);
        m_btnDisconnectMEA->setEnabled(true);
        m_btnStartAcquisition->setEnabled(true);
        m_btnStartLearning->setEnabled(true);
    } else {
        log("Échec de connexion MEA: " + m_meaInterface->getLastError(), "ERROR");
        QMessageBox::warning(this, "Erreur MEA", "Impossible de connecter au MEA: " + m_meaInterface->getLastError());
    }
}

void MainWindow::onDisconnectMEA()
{
    log("Déconnexion MEA...");
    
    m_meaInterface->stopContinuousAcquisition();
    m_meaInterface->disconnect();
    
    m_btnConnectMEA->setEnabled(true);
    m_btnDisconnectMEA->setEnabled(false);
    m_btnStartAcquisition->setEnabled(false);
    m_btnStopAcquisition->setEnabled(false);
    m_btnStartLearning->setEnabled(false);
    
    log("MEA déconnecté", "INFO");
}

void MainWindow::onStartAcquisition()
{
    log("Démarrage acquisition continue...");
    
    m_meaInterface->startContinuousAcquisition(100); // 10 Hz
    
    m_btnStartAcquisition->setEnabled(false);
    m_btnStopAcquisition->setEnabled(true);
    
    log("Acquisition continue démarrée (10 Hz)", "SUCCESS");
}

void MainWindow::onStopAcquisition()
{
    log("Arrêt acquisition continue...");
    
    m_meaInterface->stopContinuousAcquisition();
    
    m_btnStartAcquisition->setEnabled(true);
    m_btnStopAcquisition->setEnabled(false);
    
    log("Acquisition continue arrêtée", "INFO");
}

void MainWindow::onStartLearning()
{
    log("Démarrage apprentissage initial...");
    
    m_btnStartLearning->setEnabled(false);
    
    // Séquence d'apprentissage adaptative
    for (int cycle = 0; cycle < 5; ++cycle) {
        log(QString("Cycle d'apprentissage %1/5").arg(cycle + 1));
        
        // Génération de pattern de stimulation pseudo-aléatoire
        QVector<double> stimPattern(60); // 60 électrodes
        for (int i = 0; i < 60; ++i) {
            // Pattern biologique réaliste
            double amplitude = 0.5 + 0.3 * sin(i * 0.1 + cycle);
            stimPattern[i] = amplitude;
        }
        
        // Envoi de la stimulation
        m_meaInterface->stimulate(stimPattern);
        
        // Attente et acquisition
        QThread::msleep(500);
        QVector<double> response = m_meaInterface->readSignals();
        
        // Ajustement adaptatif basé sur la qualité de la réponse
        double signalQuality = m_meaInterface->getSignalQuality();
        double adjustmentFactor = 1.0 + (signalQuality - 0.5) * 0.1;
        m_meaInterface->adjustCalibration(adjustmentFactor);
        
        log(QString("  Qualité signal: %1%, Ajustement: %2")
            .arg(signalQuality * 100, 0, 'f', 1)
            .arg(adjustmentFactor, 0, 'f', 3));
    }
    
    m_btnStartLearning->setEnabled(true);
    log("Apprentissage initial terminé", "SUCCESS");
}

void MainWindow::onCalibrate()
{
    bool ok;
    double factor = QInputDialog::getDouble(this, 
                                           "Calibration MEA",
                                           "Facteur de calibration:",
                                           m_meaInterface->getCalibration(),
                                           0.1, 5.0, 3, &ok);
    
    if (ok) {
        double currentFactor = m_meaInterface->getCalibration();
        double adjustmentFactor = factor / currentFactor;
        m_meaInterface->adjustCalibration(adjustmentFactor);
        
        log(QString("Calibration manuelle: %1").arg(factor, 0, 'f', 3));
    }
}

void MainWindow::onSaveCalibration()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                   "Sauvegarder Calibration",
                                                   m_calibrationFilePath,
                                                   "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        m_meaInterface->saveCalibration(fileName);
        log("Calibration sauvegardée: " + fileName, "SUCCESS");
    }
}

void MainWindow::onLoadCalibration()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                   "Charger Calibration",
                                                   m_calibrationFilePath,
                                                   "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        if (m_meaInterface->loadCalibration(fileName)) {
            log("Calibration chargée: " + fileName, "SUCCESS");
        } else {
            log("Erreur chargement calibration: " + fileName, "ERROR");
            QMessageBox::warning(this, "Erreur", "Impossible de charger la calibration");
        }
    }
}

// === MINING CONTROL SLOTS ===

void MainWindow::onStartMining()
{
    log("Démarrage mining Bitcoin...");
    
    // Vérification de la connexion MEA
    if (m_meaInterface->getStatus() != MEAInterface::ConnectionStatus::Connected) {
        QMessageBox::warning(this, "Attention", "MEA non connecté. Mining avec signaux simulés.");
    }
    
    // Configuration du mining
    BitcoinMiner::MiningConfig config = m_bitcoinMiner->getMiningConfig();
    config.blockHeader = m_editBlockHeader->text();
    config.maxAttempts = m_spinMaxAttempts->value();
    config.signalWeight = m_spinSignalWeight->value();
    config.threadCount = m_spinThreadCount->value();
    config.useQuantumOptimization = (m_comboOptimization->currentText() != "Standard");
    
    m_bitcoinMiner->setMiningConfig(config);
    
    // Acquisition des signaux biologiques
    QVector<double> bioSignals = m_meaInterface->readSignals();
    
    if (bioSignals.isEmpty()) {
        log("Aucun signal biologique - génération de signaux simulés", "WARNING");
        // Génération de signaux simulés pour test
        bioSignals.resize(60);
        for (int i = 0; i < 60; ++i) {
            bioSignals[i] = QRandomGenerator::global()->generateDouble() - 0.5;
        }
    }
    
    // Démarrage mining asynchrone
    m_bitcoinMiner->startMiningAsync(bioSignals);
    
    m_btnStartMining->setEnabled(false);
    m_btnStopMining->setEnabled(true);
    m_progressMining->setValue(0);
    
    log(QString("Mining démarré - %1 signaux biologiques, %2 tentatives max")
        .arg(bioSignals.size())
        .arg(config.maxAttempts));
}

void MainWindow::onStopMining()
{
    log("Arrêt du mining...");
    
    m_bitcoinMiner->stopMining();
    
    m_btnStartMining->setEnabled(true);
    m_btnStopMining->setEnabled(false);
    m_progressMining->setValue(0);
    
    log("Mining arrêté", "INFO");
}

void MainWindow::onStartContinuousMining()
{
    if (m_bitcoinMiner->getTotalAttempts() == 0) {
        // Premier démarrage du mode continu
        m_bitcoinMiner->startContinuousMining();
        m_btnContinuousMining->setText("Arrêter Mode Continu");
        m_btnContinuousMining->setStyleSheet("QPushButton { background-color: #ff9800; color: white; }");
        
        // Démarrage automatique de l'acquisition MEA si nécessaire
        if (m_meaInterface->getStatus() == MEAInterface::ConnectionStatus::Connected) {
            onStartAcquisition();
        }
        
        log("Mode continu activé - Mining automatique avec signaux MEA", "SUCCESS");
    } else {
        // Arrêt du mode continu
        m_bitcoinMiner->stopContinuousMining();
        m_btnContinuousMining->setText("Mode Continu");
        m_btnContinuousMining->setStyleSheet("");
        
        log("Mode continu désactivé", "INFO");
    }
}

void MainWindow::onConfigureMining()
{
    // Dialogue de configuration avancée (à implémenter)
    QMessageBox::information(this, "Configuration", 
                            "Dialogue de configuration avancée à implémenter.\n"
                            "Utilisez les contrôles de la section Configuration Mining pour l'instant.");
}

// === MEA EVENT SLOTS ===

void MainWindow::onMEAStatusChanged(MEAInterface::ConnectionStatus status)
{
    QString statusText;
    QString styleSheet;
    
    switch (status) {
        case MEAInterface::ConnectionStatus::Disconnected:
            statusText = "Déconnecté";
            styleSheet = "color: red;";
            break;
        case MEAInterface::ConnectionStatus::Connecting:
            statusText = "Connexion...";
            styleSheet = "color: orange;";
            break;
        case MEAInterface::ConnectionStatus::Connected:
            statusText = "Connecté";
            styleSheet = "color: green;";
            break;
        case MEAInterface::ConnectionStatus::Error:
            statusText = "Erreur";
            styleSheet = "color: red; font-weight: bold;";
            break;
    }
    
    m_lblMEAStatus->setText(statusText);
    m_lblMEAStatus->setStyleSheet(styleSheet);
    m_connectionLabel->setText("MEA: " + statusText);
    
    updateStatusDisplay();
}

void MainWindow::onMEASignalsReceived(const QVector<double> &signals)
{
    updateMEADisplay(signals);
    
    // Mise à jour de l'affichage des électrodes
    m_lblElectrodeCount->setText(QString::number(signals.size()));
    
    // Calcul et affichage de la qualité du signal
    double quality = m_meaInterface->getSignalQuality();
    m_lblSignalQuality->setText(QString("%1%").arg(quality * 100, 0, 'f', 1));
    m_progressSignalQuality->setValue(static_cast<int>(quality * 100));
}

void MainWindow::onMEAError(const QString &error)
{
    log("Erreur MEA: " + error, "ERROR");
    QMessageBox::critical(this, "Erreur MEA", error);
}

void MainWindow::onCalibrationChanged(double factor)
{
    m_lblCalibrationFactor->setText(QString::number(factor, 'f', 3));
    log(QString("Calibration mise à jour: %1").arg(factor, 0, 'f', 3));
}

// === MINING EVENT SLOTS ===

void MainWindow::onMiningComplete(const BitcoinMiner::MiningResult &result)
{
    QString resultText;
    QString logLevel;
    
    if (result.success) {
        resultText = QString("SUCCÈS - Nonce: %1, Hash: %2...")
                    .arg(result.nonce)
                    .arg(result.hash.left(16));
        logLevel = "SUCCESS";
        m_totalSuccesses++;
    } else {
        resultText = QString("ÉCHEC - %1 tentatives en %2s")
                    .arg(result.attempts)
                    .arg(result.computeTime, 0, 'f', 2);
        logLevel = "INFO";
    }
    
    log("Mining terminé: " + resultText, logLevel);
    
    // Détails additionnels
    log(QString("  Temps: %1s, Hashrate: %2 H/s, Contribution signal: %3%")
        .arg(result.computeTime, 0, 'f', 2)
        .arg(result.attempts / result.computeTime, 0, 'f', 0)
        .arg(result.signalContribution * 100, 0, 'f', 1));
    
    // Réactivation des boutons si pas en mode continu
    if (!m_bitcoinMiner->getTotalAttempts() || 
        m_btnContinuousMining->text() == "Mode Continu") {
        m_btnStartMining->setEnabled(true);
        m_btnStopMining->setEnabled(false);
    }
    
    m_progressMining->setValue(100);
    
    updateMiningStats();
}

void MainWindow::onHashRateUpdated(double hashrate)
{
    m_lblHashrate->setText(QString("%1 H/s").arg(hashrate, 0, 'f', 0));
    
    if (hashrate > m_bestHashrate) {
        m_bestHashrate = hashrate;
        log(QString("Nouveau record hashrate: %1 H/s").arg(hashrate, 0, 'f', 0), "SUCCESS");
    }
}

void MainWindow::onMiningProgress(int attempts, double progress)
{
    m_progressMining->setValue(static_cast<int>(progress * 100));
    
    // Log périodique du progrès (chaque 25%)
    static int lastProgressPercent = -1;
    int currentPercent = static_cast<int>(progress * 100);
    if (currentPercent >= lastProgressPercent + 25) {
        log(QString("Progrès mining: %1% (%2 tentatives)")
            .arg(currentPercent)
            .arg(attempts));
        lastProgressPercent = currentPercent;
    }
}

void MainWindow::onDifficultyAdjusted(uint64_t difficulty)
{
    m_lblCurrentDifficulty->setText(QString("0x%1").arg(difficulty, 16, 16, QChar('0')));
    log(QString("Difficulté ajustée: 0x%1").arg(difficulty, 16, 16, QChar('0')));
}

// === UI UPDATE SLOTS ===

void MainWindow::updateStatusDisplay()
{
    // Mise à jour du statut général
    QString status = "Prêt";
    
    if (m_meaInterface->getStatus() == MEAInterface::ConnectionStatus::Connected &&
        m_bitcoinMiner->getTotalAttempts() > 0) {
        status = "Opérationnel";
    } else if (m_meaInterface->getStatus() == MEAInterface::ConnectionStatus::Connected) {
        status = "MEA Connecté";
    } else if (m_bitcoinMiner->getTotalAttempts() > 0) {
        status = "Mining Actif";
    }
    
    m_statusLabel->setText(status);
}

void MainWindow::updateStatistics()
{
    // Statistiques de session
    int sessionTime = QDateTime::currentSecsSinceEpoch() - m_sessionStartTime;
    
    log(QString("=== STATISTIQUES SESSION ==="));
    log(QString("Durée session: %1h %2m %3s")
        .arg(sessionTime / 3600)
        .arg((sessionTime % 3600) / 60)
        .arg(sessionTime % 60));
    log(QString("Tentatives totales: %1").arg(m_bitcoinMiner->getTotalAttempts()));
    log(QString("Succès totaux: %1").arg(m_bitcoinMiner->getSuccessCount()));
    log(QString("Taux de succès: %1%").arg(m_bitcoinMiner->getSuccessRate() * 100, 0, 'f', 2));
    log(QString("Meilleur hashrate: %1 H/s").arg(m_bestHashrate, 0, 'f', 0));
    log(QString("Calibration MEA: %1").arg(m_meaInterface->getCalibration(), 0, 'f', 3));
}

void MainWindow::onUpdateTimer()
{
    // Mise à jour périodique des statistiques d'affichage
    updateMiningStats();
}

// === MENU ACTION SLOTS ===

void MainWindow::onAbout()
{
    QMessageBox::about(this, "À propos",
                      "<h3>Plateforme Hybride Bio-Mining Bitcoin</h3>"
                      "<p>Version 1.0</p>"
                      "<p>Système révolutionnaire combinant biologie cellulaire et "
                      "informatique quantique pour le mining Bitcoin avec des "
                      "Multi-Electrode Arrays (MEA).</p>"
                      "<p><b>Technologies:</b></p>"
                      "<ul>"
                      "<li>Interface MEA 60 électrodes</li>"
                      "<li>Algorithmes de mining adaptatifs</li>"
                      "<li>Optimisation bio-informatique</li>"
                      "<li>Architecture Qt/C++</li>"
                      "</ul>"
                      "<p>© 2024 - Recherche & Développement</p>");
}

void MainWindow::onSettings()
{
    // Dialogue de paramètres avancés (à implémenter)
    QMessageBox::information(this, "Paramètres", 
                            "Dialogue de paramètres avancés à implémenter.");
}

void MainWindow::onExportData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                   "Exporter Données",
                                                   "biomining_export.txt",
                                                   "Text Files (*.txt)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            
            out << "=== EXPORT DONNÉES BIO-MINING ===" << Qt::endl;
            out << "Date: " << QDateTime::currentDateTime().toString() << Qt::endl;
            out << Qt::endl;
            
            out << "=== CONFIGURATION MEA ===" << Qt::endl;
            out << "Statut: " << m_lblMEAStatus->text() << Qt::endl;
            out << "Électrodes: " << m_lblElectrodeCount->text() << Qt::endl;
            out << "Qualité Signal: " << m_lblSignalQuality->text() << Qt::endl;
            out << "Calibration: " << m_lblCalibrationFactor->text() << Qt::endl;
            out << Qt::endl;
            
            out << "=== STATISTIQUES MINING ===" << Qt::endl;
            out << "Hashrate: " << m_lblHashrate->text() << Qt::endl;
            out << "Tentatives: " << m_lblTotalAttempts->text() << Qt::endl;
            out << "Succès: " << m_lblSuccessCount->text() << Qt::endl;
            out << "Taux Succès: " << m_lblSuccessRate->text() << Qt::endl;
            out << "Difficulté: " << m_lblCurrentDifficulty->text() << Qt::endl;
            out << Qt::endl;
            
            out << "=== LOGS CONSOLE ===" << Qt::endl;
            out << m_console->toPlainText();
            
            log("Données exportées: " + fileName, "SUCCESS");
        } else {
            log("Erreur export: " + fileName, "ERROR");
        }
    }
}

void MainWindow::onImportConfig()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                   "Importer Configuration",
                                                   "",
                                                   "INI Files (*.ini);;JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        // Implémentation de l'import de configuration
        log("Import configuration: " + fileName, "INFO");
        QMessageBox::information(this, "Import", "Fonctionnalité d'import à implémenter.");
    }
}

// === HELPER METHODS ===

void MainWindow::log(const QString &message, const QString &level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString colorCode;
    
    if (level == "ERROR") {
        colorCode = "<font color='#ff4444'>";
    } else if (level == "WARNING") {
        colorCode = "<font color='#ffaa00'>";
    } else if (level == "SUCCESS") {
        colorCode = "<font color='#44ff44'>";
    } else if (level == "SYSTEM") {
        colorCode = "<font color='#4488ff'>";
    } else {
        colorCode = "<font color='#ffffff'>";
    }
    
    QString logEntry = QString("[%1] %2%3: %4</font>")
                      .arg(timestamp)
                      .arg(colorCode)
                      .arg(level)
                      .arg(message);
    
    m_console->appendHtml(logEntry);
    
    // Auto-scroll
    QTextCursor cursor = m_console->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_console->setTextCursor(cursor);
}

void MainWindow::updateMEADisplay(const QVector<double> &signals)
{
    // Mise à jour de l'affichage des signaux MEA
    // Dans une vraie implémentation, ici on mettrait à jour un graphique en temps réel
    
    if (!signals.isEmpty()) {
        double avg = std::accumulate(signals.begin(), signals.end(), 0.0) / signals.size();
        double max = *std::max_element(signals.begin(), signals.end());
        double min = *std::min_element(signals.begin(), signals.end());
        
        static int signalCount = 0;
        if (++signalCount % 50 == 0) { // Log chaque 50 acquisitions
            log(QString("Signaux MEA - Moy: %1, Min: %2, Max: %3")
                .arg(avg, 0, 'f', 4)
                .arg(min, 0, 'f', 4)
                .arg(max, 0, 'f', 4), "DEBUG");
        }
    }
}

void MainWindow::updateMiningStats()
{
    // Mise à jour des statistiques de mining
    m_lblTotalAttempts->setText(QString::number(m_bitcoinMiner->getTotalAttempts()));
    m_lblSuccessCount->setText(QString::number(m_bitcoinMiner->getSuccessCount()));
    
    double successRate = m_bitcoinMiner->getSuccessRate() * 100;
    m_lblSuccessRate->setText(QString("%1%").arg(successRate, 0, 'f', 2));
    
    // Mise à jour du label de statut mining
    QString miningStatus = m_bitcoinMiner->getTotalAttempts() > 0 ? "Actif" : "Arrêté";
    m_miningLabel->setText("Mining: " + miningStatus);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Nettoyage avant fermeture
    if (m_meaInterface && m_meaInterface->getStatus() == MEAInterface::ConnectionStatus::Connected) {
        m_meaInterface->stopContinuousAcquisition();
        m_meaInterface->disconnect();
    }
    
    if (m_bitcoinMiner) {
        m_bitcoinMiner->stopMining();
        m_bitcoinMiner->stopContinuousMining();
    }
    
    // Sauvegarde des statistiques finales
    updateStatistics();
    
    log("Fermeture de la plateforme bio-mining", "SYSTEM");
    
    event->accept();
}