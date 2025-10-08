#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/functional.h>

// Include the C++ headers
#include "../include/crypto/hybrid_bitcoin_miner.h"
#include "../include/bio/real_mea_interface.h"

namespace py = pybind11;

PYBIND11_MODULE(biomining_cpp, m) {
    m.doc() = "BioMining C++ Python bindings";
    
    // ==========================================================================
    // HYBRID BITCOIN MINER MODULE
    // ==========================================================================
    
    py::module m_crypto = m.def_submodule("crypto", "Cryptographic mining module");
    
    // Mining Configuration
    py::class_<BioMining::HCrypto::MiningConfig>(m_crypto, "MiningConfig")
        .def(py::init<>())
        .def_readwrite("difficulty", &BioMining::HCrypto::MiningConfig::difficulty)
        .def_readwrite("threads", &BioMining::HCrypto::MiningConfig::threads)
        .def_readwrite("useGPU", &BioMining::HCrypto::MiningConfig::useGPU)
        .def_readwrite("targetEfficiency", &BioMining::HCrypto::MiningConfig::targetEfficiency)
        .def_readwrite("targetDifficulty", &BioMining::HCrypto::MiningConfig::targetDifficulty);
    
    // Biological Learning Parameters
    py::class_<BioMining::HCrypto::BiologicalLearningParams>(m_crypto, "BiologicalLearningParams")
        .def(py::init<>())
        .def_readwrite("initialLearningRate", &BioMining::HCrypto::BiologicalLearningParams::initialLearningRate)
        .def_readwrite("retroLearningRate", &BioMining::HCrypto::BiologicalLearningParams::retroLearningRate)
        .def_readwrite("decayRate", &BioMining::HCrypto::BiologicalLearningParams::decayRate)
        .def_readwrite("momentumFactor", &BioMining::HCrypto::BiologicalLearningParams::momentumFactor)
        .def_readwrite("adaptationThreshold", &BioMining::HCrypto::BiologicalLearningParams::adaptationThreshold)
        .def_readwrite("maxIterations", &BioMining::HCrypto::BiologicalLearningParams::maxIterations)
        .def_readwrite("enablePlasticity", &BioMining::HCrypto::BiologicalLearningParams::enablePlasticity)
        .def_readwrite("enableAdaptation", &BioMining::HCrypto::BiologicalLearningParams::enableAdaptation);
    
    // Hybrid Mining Metrics
    py::class_<BioMining::HCrypto::HybridMiningMetrics>(m_crypto, "HybridMiningMetrics")
        .def(py::init<>())
        .def("reset", &BioMining::HCrypto::HybridMiningMetrics::reset)
        .def_property_readonly("totalHashes", [](const BioMining::HCrypto::HybridMiningMetrics& m) { 
            return m.totalHashes.load(); 
        })
        .def_property_readonly("biologicalPredictions", [](const BioMining::HCrypto::HybridMiningMetrics& m) { 
            return m.biologicalPredictions.load(); 
        })
        .def_property_readonly("successfulPredictions", [](const BioMining::HCrypto::HybridMiningMetrics& m) { 
            return m.successfulPredictions.load(); 
        })
        .def_property_readonly("biologicalAccuracy", [](const BioMining::HCrypto::HybridMiningMetrics& m) { 
            return m.biologicalAccuracy.load(); 
        })
        .def_property_readonly("hybridHashRate", [](const BioMining::HCrypto::HybridMiningMetrics& m) { 
            return m.hybridHashRate.load(); 
        });
    
    // Learning States
    py::enum_<BioMining::HCrypto::HybridLearningState>(m_crypto, "HybridLearningState")
        .value("Uninitialized", BioMining::HCrypto::HybridLearningState::Uninitialized)
        .value("InitialLearning", BioMining::HCrypto::HybridLearningState::InitialLearning)
        .value("ActiveMining", BioMining::HCrypto::HybridLearningState::ActiveMining)
        .value("RetroLearning", BioMining::HCrypto::HybridLearningState::RetroLearning)
        .value("Optimizing", BioMining::HCrypto::HybridLearningState::Optimizing)
        .value("Adapting", BioMining::HCrypto::HybridLearningState::Adapting)
        .value("Error", BioMining::HCrypto::HybridLearningState::Error);
    
    // Mining Methods
    py::enum_<BioMining::HCrypto::MiningMethod>(m_crypto, "MiningMethod")
        .value("TraditionalSHA256", BioMining::HCrypto::MiningMethod::TraditionalSHA256)
        .value("BiologicalNetwork", BioMining::HCrypto::MiningMethod::BiologicalNetwork)
        .value("RealMEANeurons", BioMining::HCrypto::MiningMethod::RealMEANeurons)
        .value("HybridFusion", BioMining::HCrypto::MiningMethod::HybridFusion);
    
    // Biological Nonce Prediction
    py::class_<BioMining::HCrypto::BiologicalNoncePrediction>(m_crypto, "BiologicalNoncePrediction")
        .def(py::init<>())
        .def_readwrite("predictedNonce", &BioMining::HCrypto::BiologicalNoncePrediction::predictedNonce)
        .def_readwrite("confidence", &BioMining::HCrypto::BiologicalNoncePrediction::confidence)
        .def_readwrite("biologicalEntropy", &BioMining::HCrypto::BiologicalNoncePrediction::biologicalEntropy)
        .def_readwrite("isValidated", &BioMining::HCrypto::BiologicalNoncePrediction::isValidated);
    
    // Triple System Prediction
    py::class_<BioMining::HCrypto::TripleSystemPrediction>(m_crypto, "TripleSystemPrediction")
        .def(py::init<>())
        .def_readwrite("sha256Nonce", &BioMining::HCrypto::TripleSystemPrediction::sha256Nonce)
        .def_readwrite("networkNonce", &BioMining::HCrypto::TripleSystemPrediction::networkNonce)
        .def_readwrite("meaNonce", &BioMining::HCrypto::TripleSystemPrediction::meaNonce)
        .def_readwrite("sha256Confidence", &BioMining::HCrypto::TripleSystemPrediction::sha256Confidence)
        .def_readwrite("networkConfidence", &BioMining::HCrypto::TripleSystemPrediction::networkConfidence)
        .def_readwrite("meaConfidence", &BioMining::HCrypto::TripleSystemPrediction::meaConfidence)
        .def_readwrite("fusedNonce", &BioMining::HCrypto::TripleSystemPrediction::fusedNonce)
        .def_readwrite("fusedConfidence", &BioMining::HCrypto::TripleSystemPrediction::fusedConfidence)
        .def_readwrite("selectedMethod", &BioMining::HCrypto::TripleSystemPrediction::selectedMethod)
        .def_readwrite("isValidated", &BioMining::HCrypto::TripleSystemPrediction::isValidated)
        .def_readwrite("wasSuccessful", &BioMining::HCrypto::TripleSystemPrediction::wasSuccessful);
    
    // Main HybridBitcoinMiner class
    py::class_<BioMining::HCrypto::HybridBitcoinMiner>(m_crypto, "HybridBitcoinMiner")
        .def(py::init<>())
        .def("initialize", &BioMining::HCrypto::HybridBitcoinMiner::initialize,
             "Initialize the hybrid mining system")
        .def("configureBiologicalNetwork", &BioMining::HCrypto::HybridBitcoinMiner::configureBiologicalNetwork,
             "Configure biological network parameters")
        .def("setMiningParameters", &BioMining::HCrypto::HybridBitcoinMiner::setMiningParameters,
             "Set mining configuration parameters")
        .def("startHybridMining", &BioMining::HCrypto::HybridBitcoinMiner::startHybridMining,
             "Start the hybrid mining process")
        .def("stopHybridMining", &BioMining::HCrypto::HybridBitcoinMiner::stopHybridMining,
             "Stop the hybrid mining process")
        .def("isMining", &BioMining::HCrypto::HybridBitcoinMiner::isMining,
             "Check if mining is active")
        .def("pauseMining", &BioMining::HCrypto::HybridBitcoinMiner::pauseMining,
             "Pause mining")
        .def("resumeMining", &BioMining::HCrypto::HybridBitcoinMiner::resumeMining,
             "Resume mining")
        .def("initializeBiologicalLearning", &BioMining::HCrypto::HybridBitcoinMiner::initializeBiologicalLearning,
             "Initialize biological learning system")
        .def("performRetroLearning", &BioMining::HCrypto::HybridBitcoinMiner::performRetroLearning,
             "Perform retro-learning optimization")
        .def("enableAdaptiveLearning", &BioMining::HCrypto::HybridBitcoinMiner::enableAdaptiveLearning,
             "Enable or disable adaptive learning")
        .def("predictNonce", &BioMining::HCrypto::HybridBitcoinMiner::predictNonce,
             "Predict nonce using biological network")
        .def("validatePrediction", &BioMining::HCrypto::HybridBitcoinMiner::validatePrediction,
             "Validate a biological prediction")
        .def("getBiologicalAccuracy", &BioMining::HCrypto::HybridBitcoinMiner::getBiologicalAccuracy,
             "Get biological prediction accuracy")
        .def("getMetrics", &BioMining::HCrypto::HybridBitcoinMiner::getMetrics,
             "Get mining metrics")
        .def("getLearningState", &BioMining::HCrypto::HybridBitcoinMiner::getLearningState,
             "Get current learning state")
        .def("getNetworkAdaptationScore", &BioMining::HCrypto::HybridBitcoinMiner::getNetworkAdaptationScore,
             "Get network adaptation score")
        .def("getStatusReport", &BioMining::HCrypto::HybridBitcoinMiner::getStatusReport,
             "Get detailed status report")
        .def("setBiologicalWeight", &BioMining::HCrypto::HybridBitcoinMiner::setBiologicalWeight,
             "Set biological vs traditional weight")
        .def("getBiologicalWeight", &BioMining::HCrypto::HybridBitcoinMiner::getBiologicalWeight,
             "Get current biological weight")
        // Triple System methods
        .def("initializeTripleSystem", &BioMining::HCrypto::HybridBitcoinMiner::initializeTripleSystem,
             "Initialize triple system optimization")
        .def("configureTripleSystemLearning", &BioMining::HCrypto::HybridBitcoinMiner::configureTripleSystemLearning,
             "Configure triple system learning")
        .def("predictNonceTriple", &BioMining::HCrypto::HybridBitcoinMiner::predictNonceTriple,
             "Predict nonce with triple system")
        .def("fuseTriplePredictions", &BioMining::HCrypto::HybridBitcoinMiner::fuseTriplePredictions,
             "Fuse triple system predictions")
        .def("selectOptimalMethod", &BioMining::HCrypto::HybridBitcoinMiner::selectOptimalMethod,
             "Select optimal mining method")
        .def("performTripleOptimizedMining", &BioMining::HCrypto::HybridBitcoinMiner::performTripleOptimizedMining,
             "Perform triple-optimized mining")
        .def("optimizeSystemWeights", &BioMining::HCrypto::HybridBitcoinMiner::optimizeSystemWeights,
             "Optimize system weights dynamically");
    
    // ==========================================================================
    // REAL MEA INTERFACE MODULE
    // ==========================================================================
    
    py::module m_bio = m.def_submodule("bio", "Biological interface module");
    
    // MEA Device Types
    py::enum_<BioMining::Bio::MEADeviceType>(m_bio, "MEADeviceType")
        .value("MultiChannelSystems_MCS", BioMining::Bio::MEADeviceType::MultiChannelSystems_MCS)
        .value("AlphaOmega_AlphaMap", BioMining::Bio::MEADeviceType::AlphaOmega_AlphaMap)
        .value("Blackrock_CerePlex", BioMining::Bio::MEADeviceType::Blackrock_CerePlex)
        .value("Plexon_OmniPlex", BioMining::Bio::MEADeviceType::Plexon_OmniPlex)
        .value("Intan_RHD", BioMining::Bio::MEADeviceType::Intan_RHD)
        .value("OpenEphys_ONIX", BioMining::Bio::MEADeviceType::OpenEphys_ONIX)
        .value("NeuroNexus_SmartBox", BioMining::Bio::MEADeviceType::NeuroNexus_SmartBox)
        .value("Custom_Serial", BioMining::Bio::MEADeviceType::Custom_Serial)
        .value("Custom_Network", BioMining::Bio::MEADeviceType::Custom_Network)
        .value("Custom_SharedMemory", BioMining::Bio::MEADeviceType::Custom_SharedMemory);
    
    // Communication Protocols
    py::enum_<BioMining::Bio::CommunicationProtocol>(m_bio, "CommunicationProtocol")
        .value("SerialPort", BioMining::Bio::CommunicationProtocol::SerialPort)
        .value("TCP", BioMining::Bio::CommunicationProtocol::TCP)
        .value("UDP", BioMining::Bio::CommunicationProtocol::UDP)
        .value("SharedMemory", BioMining::Bio::CommunicationProtocol::SharedMemory)
        .value("NamedPipe", BioMining::Bio::CommunicationProtocol::NamedPipe)
        .value("HDF5_File", BioMining::Bio::CommunicationProtocol::HDF5_File)
        .value("Custom_API", BioMining::Bio::CommunicationProtocol::Custom_API);
    
    // Connection Status
    py::enum_<BioMining::Bio::RealMEAInterface::ConnectionStatus>(m_bio, "ConnectionStatus")
        .value("Disconnected", BioMining::Bio::RealMEAInterface::ConnectionStatus::Disconnected)
        .value("Connecting", BioMining::Bio::RealMEAInterface::ConnectionStatus::Connecting)
        .value("Connected", BioMining::Bio::RealMEAInterface::ConnectionStatus::Connected)
        .value("Error", BioMining::Bio::RealMEAInterface::ConnectionStatus::Error)
        .value("Calibrating", BioMining::Bio::RealMEAInterface::ConnectionStatus::Calibrating)
        .value("Streaming", BioMining::Bio::RealMEAInterface::ConnectionStatus::Streaming);
    
    // MEA Configuration
    py::class_<BioMining::Bio::RealMEAConfig>(m_bio, "RealMEAConfig")
        .def(py::init<>())
        .def_readwrite("deviceType", &BioMining::Bio::RealMEAConfig::deviceType)
        .def_readwrite("protocol", &BioMining::Bio::RealMEAConfig::protocol)
        .def_readwrite("electrodeCount", &BioMining::Bio::RealMEAConfig::electrodeCount)
        .def_readwrite("samplingRate", &BioMining::Bio::RealMEAConfig::samplingRate)
        .def_readwrite("amplification", &BioMining::Bio::RealMEAConfig::amplification)
        .def_readwrite("filterLowCut", &BioMining::Bio::RealMEAConfig::filterLowCut)
        .def_readwrite("filterHighCut", &BioMining::Bio::RealMEAConfig::filterHighCut)
        .def_readwrite("bufferSize", &BioMining::Bio::RealMEAConfig::bufferSize)
        .def_readwrite("stimMaxVoltage", &BioMining::Bio::RealMEAConfig::stimMaxVoltage)
        .def_readwrite("stimMaxCurrent", &BioMining::Bio::RealMEAConfig::stimMaxCurrent)
        .def_readwrite("bidirectionalStim", &BioMining::Bio::RealMEAConfig::bidirectionalStim)
        .def_readwrite("spikeDetection", &BioMining::Bio::RealMEAConfig::spikeDetection)
        .def_readwrite("spikeThreshold", &BioMining::Bio::RealMEAConfig::spikeThreshold)
        .def_readwrite("connectionTimeoutMs", &BioMining::Bio::RealMEAConfig::connectionTimeoutMs)
        .def_readwrite("readTimeoutMs", &BioMining::Bio::RealMEAConfig::readTimeoutMs)
        .def_readwrite("maxRetries", &BioMining::Bio::RealMEAConfig::maxRetries)
        .def_readwrite("autoCalibration", &BioMining::Bio::RealMEAConfig::autoCalibration);
    
    // Electrode Data
    py::class_<BioMining::Bio::ElectrodeData>(m_bio, "ElectrodeData")
        .def(py::init<>())
        .def_readwrite("electrodeId", &BioMining::Bio::ElectrodeData::electrodeId)
        .def_readwrite("voltage", &BioMining::Bio::ElectrodeData::voltage)
        .def_readwrite("current", &BioMining::Bio::ElectrodeData::current)
        .def_readwrite("impedance", &BioMining::Bio::ElectrodeData::impedance)
        .def_readwrite("isActive", &BioMining::Bio::ElectrodeData::isActive)
        .def_readwrite("signalQuality", &BioMining::Bio::ElectrodeData::signalQuality)
        .def_readwrite("timestamp", &BioMining::Bio::ElectrodeData::timestamp);
    
    // Spike Event
    py::class_<BioMining::Bio::SpikeEvent>(m_bio, "SpikeEvent")
        .def(py::init<>())
        .def_readwrite("electrodeId", &BioMining::Bio::SpikeEvent::electrodeId)
        .def_readwrite("amplitude", &BioMining::Bio::SpikeEvent::amplitude)
        .def_readwrite("timestamp", &BioMining::Bio::SpikeEvent::timestamp);
    
    // Bitcoin Learning Pattern
    py::class_<BioMining::Bio::BitcoinLearningPattern>(m_bio, "BitcoinLearningPattern")
        .def(py::init<>())
        .def_readwrite("targetNonce", &BioMining::Bio::BitcoinLearningPattern::targetNonce)
        .def_readwrite("difficulty", &BioMining::Bio::BitcoinLearningPattern::difficulty)
        .def_readwrite("timestamp", &BioMining::Bio::BitcoinLearningPattern::timestamp)
        .def_readwrite("successRate", &BioMining::Bio::BitcoinLearningPattern::successRate);
    
    // Neural Bitcoin Response
    py::class_<BioMining::Bio::NeuralBitcoinResponse>(m_bio, "NeuralBitcoinResponse")
        .def(py::init<>())
        .def_readwrite("inputPattern", &BioMining::Bio::NeuralBitcoinResponse::inputPattern)
        .def_readwrite("predictedNonce", &BioMining::Bio::NeuralBitcoinResponse::predictedNonce)
        .def_readwrite("confidence", &BioMining::Bio::NeuralBitcoinResponse::confidence)
        .def_readwrite("rewardSignal", &BioMining::Bio::NeuralBitcoinResponse::rewardSignal)
        .def_readwrite("responseTime", &BioMining::Bio::NeuralBitcoinResponse::responseTime);
    
    // Bitcoin Learning Configuration
    py::class_<BioMining::Bio::BitcoinLearningConfig>(m_bio, "BitcoinLearningConfig")
        .def(py::init<>())
        .def_readwrite("learningRate", &BioMining::Bio::BitcoinLearningConfig::learningRate)
        .def_readwrite("decayRate", &BioMining::Bio::BitcoinLearningConfig::decayRate)
        .def_readwrite("maxTrainingEpochs", &BioMining::Bio::BitcoinLearningConfig::maxTrainingEpochs)
        .def_readwrite("targetAccuracy", &BioMining::Bio::BitcoinLearningConfig::targetAccuracy)
        .def_readwrite("stimulationAmplitude", &BioMining::Bio::BitcoinLearningConfig::stimulationAmplitude)
        .def_readwrite("stimulationDuration", &BioMining::Bio::BitcoinLearningConfig::stimulationDuration)
        .def_readwrite("reinforcementDelay", &BioMining::Bio::BitcoinLearningConfig::reinforcementDelay)
        .def_readwrite("punishmentAmplitude", &BioMining::Bio::BitcoinLearningConfig::punishmentAmplitude)
        .def_readwrite("enableRealtimeLearning", &BioMining::Bio::BitcoinLearningConfig::enableRealtimeLearning)
        .def_readwrite("enableBackpropagation", &BioMining::Bio::BitcoinLearningConfig::enableBackpropagation);
    
    // Bitcoin Learning Statistics
    py::class_<BioMining::Bio::BitcoinLearningStats>(m_bio, "BitcoinLearningStats")
        .def(py::init<>())
        .def_readwrite("totalPatternsLearned", &BioMining::Bio::BitcoinLearningStats::totalPatternsLearned)
        .def_readwrite("successfulPredictions", &BioMining::Bio::BitcoinLearningStats::successfulPredictions)
        .def_readwrite("totalPredictions", &BioMining::Bio::BitcoinLearningStats::totalPredictions)
        .def_readwrite("currentAccuracy", &BioMining::Bio::BitcoinLearningStats::currentAccuracy)
        .def_readwrite("bestAccuracy", &BioMining::Bio::BitcoinLearningStats::bestAccuracy)
        .def_readwrite("totalTrainingTime", &BioMining::Bio::BitcoinLearningStats::totalTrainingTime);
    
    // Main RealMEAInterface class
    py::class_<BioMining::Bio::RealMEAInterface>(m_bio, "RealMEAInterface")
        .def(py::init<>())
        .def("initialize", &BioMining::Bio::RealMEAInterface::initialize,
             "Initialize MEA interface with configuration")
        .def("disconnect", &BioMining::Bio::RealMEAInterface::disconnect,
             "Disconnect from MEA device")
        .def("getStatus", &BioMining::Bio::RealMEAInterface::getStatus,
             "Get current connection status")
        .def("getDeviceInfo", &BioMining::Bio::RealMEAInterface::getDeviceInfo,
             "Get device information")
        .def("getActiveElectrodes", &BioMining::Bio::RealMEAInterface::getActiveElectrodes,
             "Get list of active electrodes")
        .def("getSamplingRate", &BioMining::Bio::RealMEAInterface::getSamplingRate,
             "Get current sampling rate")
        .def("readElectrodeData", &BioMining::Bio::RealMEAInterface::readElectrodeData,
             "Read current electrode data")
        .def("readRawSignals", &BioMining::Bio::RealMEAInterface::readRawSignals,
             "Read raw signal data")
        .def("startContinuousAcquisition", &BioMining::Bio::RealMEAInterface::startContinuousAcquisition,
             "Start continuous data acquisition")
        .def("stopContinuousAcquisition", &BioMining::Bio::RealMEAInterface::stopContinuousAcquisition,
             "Stop continuous data acquisition")
        .def("stimulateElectrode", &BioMining::Bio::RealMEAInterface::stimulateElectrode,
             "Stimulate specific electrode")
        .def("stimulatePattern", &BioMining::Bio::RealMEAInterface::stimulatePattern,
             "Stimulate with pattern")
        .def("setStimulationParameters", &BioMining::Bio::RealMEAInterface::setStimulationParameters,
             "Set stimulation parameters")
        .def("performElectrodeImpedanceTest", &BioMining::Bio::RealMEAInterface::performElectrodeImpedanceTest,
             "Test electrode impedances")
        .def("calibrateElectrode", &BioMining::Bio::RealMEAInterface::calibrateElectrode,
             "Calibrate specific electrode")
        .def("performFullCalibration", &BioMining::Bio::RealMEAInterface::performFullCalibration,
             "Perform full system calibration")
        .def("getElectrodeImpedances", &BioMining::Bio::RealMEAInterface::getElectrodeImpedances,
             "Get electrode impedance values")
        .def("enableSpikeDetection", &BioMining::Bio::RealMEAInterface::enableSpikeDetection,
             "Enable or disable spike detection")
        .def("setSpikeThreshold", &BioMining::Bio::RealMEAInterface::setSpikeThreshold,
             "Set spike detection threshold")
        .def("getRecentSpikes", &BioMining::Bio::RealMEAInterface::getRecentSpikes,
             "Get recent spike events")
        .def("getSignalQuality", &BioMining::Bio::RealMEAInterface::getSignalQuality,
             "Get overall signal quality")
        .def("getLastError", &BioMining::Bio::RealMEAInterface::getLastError,
             "Get last error message")
        .def("setElectrodeActive", &BioMining::Bio::RealMEAInterface::setElectrodeActive,
             "Set electrode active state")
        .def("setAcquisitionFilter", &BioMining::Bio::RealMEAInterface::setAcquisitionFilter,
             "Set acquisition filter parameters")
        // Bitcoin Learning methods
        .def("initializeBitcoinLearning", &BioMining::Bio::RealMEAInterface::initializeBitcoinLearning,
             "Initialize Bitcoin learning system")
        .def("trainBitcoinPattern", &BioMining::Bio::RealMEAInterface::trainBitcoinPattern,
             "Train with Bitcoin pattern")
        .def("predictNonce", &BioMining::Bio::RealMEAInterface::predictNonce,
             "Predict nonce from block hash")
        .def("applyReinforcementLearning", &BioMining::Bio::RealMEAInterface::applyReinforcementLearning,
             "Apply reinforcement learning")
        .def("saveNeuralModel", &BioMining::Bio::RealMEAInterface::saveNeuralModel,
             "Save neural model to file")
        .def("loadNeuralModel", &BioMining::Bio::RealMEAInterface::loadNeuralModel,
             "Load neural model from file")
        .def("getLearningStats", &BioMining::Bio::RealMEAInterface::getLearningStats,
             "Get learning statistics")
        .def("resetNeuralModel", &BioMining::Bio::RealMEAInterface::resetNeuralModel,
             "Reset neural model")
        .def("enableRealtimeLearning", &BioMining::Bio::RealMEAInterface::enableRealtimeLearning,
             "Enable real-time learning")
        .def("optimizeLearningParameters", &BioMining::Bio::RealMEAInterface::optimizeLearningParameters,
             "Optimize learning parameters")
        .def("generateStimulationPattern", &BioMining::Bio::RealMEAInterface::generateStimulationPattern,
             "Generate stimulation pattern from hash")
        .def("extractNonceFromNeuralResponse", &BioMining::Bio::RealMEAInterface::extractNonceFromNeuralResponse,
             "Extract nonce from neural response")
        .def("stimulateWithReinforcement", &BioMining::Bio::RealMEAInterface::stimulateWithReinforcement,
             "Stimulate with reinforcement signal");
}