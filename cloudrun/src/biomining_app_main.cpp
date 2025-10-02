#include "biomining_app.hpp"
#include <iostream>
#include <fstream>
#include <signal.h>

namespace biomining {

// Global app instance for signal handling
BioMiningApp* g_app_instance = nullptr;

void signal_handler(int signal) {
    if (g_app_instance) {
        std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
        g_app_instance->shutdown();
    }
    exit(signal);
}

// ====================================================================
// BIOMINING APP MAIN IMPLEMENTATION
// ====================================================================

BioMiningApp::BioMiningApp(const std::string& config_file) {
    load_configuration(config_file);
    g_app_instance = this;
    
    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

BioMiningApp::~BioMiningApp() {
    shutdown();
}

bool BioMiningApp::initialize() {
    log("info", "Initializing BioMining Platform");
    
    // Initialize neural network
    neural_network_ = NetworkFactory::create_network(
        config_.network.type, config_.network);
    
    if (!neural_network_ || !neural_network_->initialize(config_.network)) {
        log("error", "Failed to initialize neural network");
        return false;
    }
    
    // Initialize Bitcoin miner
    bitcoin_miner_ = std::make_unique<BitcoinMiner>(config_.bitcoin);
    if (!bitcoin_miner_->initialize()) {
        log("error", "Failed to initialize Bitcoin miner");
        return false;
    }
    
    // Initialize HTTP server
    http_server_ = std::make_unique<httplib::Server>();
    setup_http_routes();
    
    // Start performance monitoring
    monitoring_active_.store(true);
    metrics_thread_ = std::thread(&BioMiningApp::performance_monitoring_thread, this);
    
    log("info", "BioMining Platform initialized successfully");
    return true;
}

void BioMiningApp::run() {
    log("info", "Starting BioMining Platform on port " + std::to_string(config_.http_port));
    
    if (!http_server_->listen("0.0.0.0", config_.http_port)) {
        log("error", "Failed to start HTTP server on port " + std::to_string(config_.http_port));
        return;
    }
}

void BioMiningApp::shutdown() {
    log("info", "Shutting down BioMining Platform");
    
    // Stop all running operations
    stop_training();
    stop_bitcoin_mining();
    stop_mea_acquisition();
    
    // Stop monitoring
    monitoring_active_.store(false);
    if (metrics_thread_.joinable()) {
        metrics_thread_.join();
    }
    
    // Stop HTTP server
    if (http_server_) {
        http_server_->stop();
    }
    
    log("info", "BioMining Platform shutdown complete");
}

bool BioMiningApp::load_configuration(const std::string& config_file) {
    try {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            log("warning", "Config file not found, using defaults: " + config_file);
            return true; // Use default config
        }
        
        json config_json;
        file >> config_json;
        
        // Parse MEA configuration
        if (config_json.contains("mea")) {
            auto& mea_json = config_json["mea"];
            config_.mea.device_type = mea_json.value("device_type", config_.mea.device_type);
            config_.mea.host = mea_json.value("host", config_.mea.host);
            config_.mea.port = mea_json.value("port", config_.mea.port);
            config_.mea.num_electrodes = mea_json.value("num_electrodes", config_.mea.num_electrodes);
            config_.mea.sampling_rate = mea_json.value("sampling_rate", config_.mea.sampling_rate);
        }
        
        // Parse network configuration
        if (config_json.contains("network")) {
            auto& net_json = config_json["network"];
            std::string type_str = net_json.value("type", "hybrid");
            if (type_str == "biological") {
                config_.network.type = NetworkConfiguration::BIOLOGICAL_ONLY;
            } else if (type_str == "hybrid") {
                config_.network.type = NetworkConfiguration::HYBRID_BIO_ARTIFICIAL;
            } else {
                config_.network.type = NetworkConfiguration::ARTIFICIAL_ASSISTED;
            }
            
            config_.network.hidden_layers = net_json.value("hidden_layers", config_.network.hidden_layers);
            config_.network.neurons_per_layer = net_json.value("neurons_per_layer", config_.network.neurons_per_layer);
            config_.network.learning_rate = net_json.value("learning_rate", config_.network.learning_rate);
        }
        
        // Parse Bitcoin configuration
        if (config_json.contains("bitcoin")) {
            auto& btc_json = config_json["bitcoin"];
            config_.bitcoin.difficulty_target = btc_json.value("difficulty_target", config_.bitcoin.difficulty_target);
            config_.bitcoin.use_testnet = btc_json.value("use_testnet", config_.bitcoin.use_testnet);
            config_.bitcoin.max_threads = btc_json.value("max_threads", config_.bitcoin.max_threads);
        }
        
        // Parse general configuration
        config_.http_port = config_json.value("http_port", config_.http_port);
        config_.log_level = config_json.value("log_level", config_.log_level);
        
        log("info", "Configuration loaded from " + config_file);
        return true;
        
    } catch (const std::exception& e) {
        log("error", "Failed to load configuration: " + std::string(e.what()));
        return false;
    }
}

bool BioMiningApp::save_configuration(const std::string& config_file) const {
    try {
        json config_json;
        
        // MEA configuration
        config_json["mea"] = {
            {"device_type", config_.mea.device_type},
            {"host", config_.mea.host},
            {"port", config_.mea.port},
            {"num_electrodes", config_.mea.num_electrodes},
            {"sampling_rate", config_.mea.sampling_rate},
            {"max_voltage", config_.mea.max_voltage},
            {"impedance_threshold", config_.mea.impedance_threshold}
        };
        
        // Network configuration
        std::string type_str = "hybrid";
        switch (config_.network.type) {
            case NetworkConfiguration::BIOLOGICAL_ONLY: type_str = "biological"; break;
            case NetworkConfiguration::HYBRID_BIO_ARTIFICIAL: type_str = "hybrid"; break;
            case NetworkConfiguration::ARTIFICIAL_ASSISTED: type_str = "artificial"; break;
        }
        
        config_json["network"] = {
            {"type", type_str},
            {"hidden_layers", config_.network.hidden_layers},
            {"neurons_per_layer", config_.network.neurons_per_layer},
            {"learning_rate", config_.network.learning_rate},
            {"dropout_rate", config_.network.dropout_rate},
            {"batch_size", config_.network.batch_size}
        };
        
        // Bitcoin configuration
        config_json["bitcoin"] = {
            {"difficulty_target", config_.bitcoin.difficulty_target},
            {"use_testnet", config_.bitcoin.use_testnet},
            {"max_threads", config_.bitcoin.max_threads},
            {"enable_real_mining", config_.bitcoin.enable_real_mining}
        };
        
        // General configuration
        config_json["http_port"] = config_.http_port;
        config_json["log_level"] = config_.log_level;
        config_json["data_directory"] = config_.data_directory;
        config_json["enable_metrics"] = config_.enable_metrics;
        
        std::ofstream file(config_file);
        file << config_json.dump(2);
        
        log("info", "Configuration saved to " + config_file);
        return true;
        
    } catch (const std::exception& e) {
        log("error", "Failed to save configuration: " + std::string(e.what()));
        return false;
    }
}

bool BioMiningApp::import_training_data(const std::string& data_path) {
    log("info", "Importing training data from " + data_path);
    
    try {
        std::ifstream file(data_path);
        if (!file.is_open()) {
            log("error", "Cannot open training data file: " + data_path);
            return false;
        }
        
        json data_json;
        file >> data_json;
        
        MEADataset dataset;
        dataset.session_id = data_json.value("session_id", "imported_" + get_timestamp());
        dataset.sampling_rate = data_json.value("sampling_rate", config_.mea.sampling_rate);
        dataset.start_timestamp = data_json.value("start_timestamp", 0);
        dataset.end_timestamp = data_json.value("end_timestamp", 0);
        
        if (data_json.contains("electrodes")) {
            for (const auto& electrode_json : data_json["electrodes"]) {
                ElectrodeData electrode;
                electrode.electrode_id = electrode_json.value("electrode_id", 0);
                electrode.voltages = electrode_json.value("voltages", std::vector<double>());
                electrode.impedance = electrode_json.value("impedance", 1.0);
                electrode.noise_level = electrode_json.value("noise_level", 0.1);
                electrode.is_active = electrode_json.value("is_active", true);
                
                // Generate timestamps if not provided
                electrode.timestamps.clear();
                uint64_t base_time = dataset.start_timestamp;
                for (size_t i = 0; i < electrode.voltages.size(); ++i) {
                    electrode.timestamps.push_back(base_time + i * (1000000 / dataset.sampling_rate));
                }
                
                dataset.add_electrode_data(electrode);
            }
        }
        
        training_datasets_.push_back(dataset);
        log("info", "Imported " + std::to_string(dataset.electrodes.size()) + 
            " electrodes from " + data_path);
        
        return true;
        
    } catch (const std::exception& e) {
        log("error", "Failed to import training data: " + std::string(e.what()));
        return false;
    }
}

bool BioMiningApp::start_training() {
    if (training_active_.load()) {
        log("warning", "Training already in progress");
        return false;
    }
    
    if (training_datasets_.empty()) {
        log("error", "No training data available");
        return false;
    }
    
    log("info", "Starting neural network training");
    training_active_.store(true);
    training_progress_.store(0.0);
    
    // Start training in separate thread
    std::thread([this]() {
        for (size_t i = 0; i < training_datasets_.size() && training_active_.load(); ++i) {
            const auto& dataset = training_datasets_[i];
            log("info", "Training on dataset " + dataset.session_id);
            
            if (!neural_network_->train(dataset)) {
                log("error", "Training failed on dataset " + dataset.session_id);
                break;
            }
            
            training_progress_.store(static_cast<double>(i + 1) / training_datasets_.size());
            
            // Simulate training time
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        training_active_.store(false);
        log("info", "Neural network training completed");
    }).detach();
    
    return true;
}

bool BioMiningApp::start_bitcoin_mining() {
    log("info", "Starting Bitcoin mining");
    bitcoin_miner_->start_mining();
    return true;
}

bool BioMiningApp::stop_bitcoin_mining() {
    log("info", "Stopping Bitcoin mining");
    bitcoin_miner_->stop_mining();
    return true;
}

json BioMiningApp::get_system_status() const {
    return json{
        {"timestamp", get_timestamp()},
        {"mea_acquisition", false}, // Placeholder
        {"training_active", training_active_.load()},
        {"training_progress", training_progress_.load()},
        {"mining_active", bitcoin_miner_ ? true : false}, // Simplified check
        {"network_type", 
            config_.network.type == NetworkConfiguration::BIOLOGICAL_ONLY ? "biological" :
            config_.network.type == NetworkConfiguration::HYBRID_BIO_ARTIFICIAL ? "hybrid" : "artificial"},
        {"neural_network", neural_network_ ? neural_network_->get_metrics() : json::object()},
        {"mining_stats", bitcoin_miner_ ? bitcoin_miner_->get_mining_stats() : json::object()},
        {"configuration", {
            {"mea_electrodes", config_.mea.num_electrodes},
            {"sampling_rate", config_.mea.sampling_rate},
            {"network_layers", config_.network.hidden_layers},
            {"mining_threads", config_.bitcoin.max_threads}
        }}
    };
}

json BioMiningApp::get_performance_metrics() const {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    return performance_metrics_;
}

json BioMiningApp::export_results() const {
    json results;
    
    // System status
    results["system_status"] = get_system_status();
    
    // Performance metrics
    results["performance"] = get_performance_metrics();
    
    // Training datasets summary
    results["datasets"] = json::array();
    for (const auto& dataset : training_datasets_) {
        results["datasets"].push_back({
            {"session_id", dataset.session_id},
            {"electrodes_count", dataset.electrodes.size()},
            {"sampling_rate", dataset.sampling_rate},
            {"duration_ms", dataset.end_timestamp - dataset.start_timestamp}
        });
    }
    
    // Current session data
    if (!current_session_data_.electrodes.empty()) {
        results["current_session"] = current_session_data_.to_json();
    }
    
    results["export_timestamp"] = get_timestamp();
    
    return results;
}

void BioMiningApp::performance_monitoring_thread() {
    while (monitoring_active_.load()) {
        update_performance_metrics();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void BioMiningApp::update_performance_metrics() {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    
    // System metrics
    performance_metrics_["timestamp"] = get_timestamp();
    performance_metrics_["system"] = collect_system_metrics();
    
    // Mining metrics
    if (bitcoin_miner_) {
        performance_metrics_["mining"] = bitcoin_miner_->get_mining_stats();
    }
    
    // Neural network metrics
    if (neural_network_) {
        performance_metrics_["neural_network"] = neural_network_->get_metrics();
    }
    
    // Training metrics
    performance_metrics_["training"] = json{
        {"active", training_active_.load()},
        {"progress", training_progress_.load()},
        {"datasets_loaded", training_datasets_.size()}
    };
}

json BioMiningApp::collect_system_metrics() const {
    json metrics;
    
    // CPU and memory usage
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        metrics["cpu_time_user"] = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
        metrics["cpu_time_system"] = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
        metrics["memory_max_rss"] = usage.ru_maxrss;
    }
    
    // System info
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        metrics["system_uptime"] = sys_info.uptime;
        metrics["system_load_1min"] = sys_info.loads[0] / 65536.0;
        metrics["memory_total"] = sys_info.totalram;
        metrics["memory_free"] = sys_info.freeram;
    }
    
    return metrics;
}

std::string BioMiningApp::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    
    return ss.str();
}

// ====================================================================
// HTTP API ROUTES
// ====================================================================

void BioMiningApp::setup_http_routes() {
    if (!http_server_) return;
    
    // Enable CORS
    http_server_->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    // Health check
    http_server_->Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json health = {
            {"status", "healthy"},
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"service", "BioMining Platform"},
            {"version", "1.0.0"}
        };
        res.set_content(health.dump(), "application/json");
    });
    
    // Status endpoint
    http_server_->Get("/api/status", 
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_get_status(req, res);
        });
    
    // Configuration endpoints
    http_server_->Post("/api/configure",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_post_configure(req, res);
        });
    
    // Metrics endpoint
    http_server_->Get("/api/metrics",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_get_metrics(req, res);
        });
    
    // Training endpoints
    http_server_->Post("/api/train/start",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_post_train(req, res);
        });
    
    // Mining endpoints
    http_server_->Post("/api/mine/start",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_post_mine(req, res);
        });
    
    http_server_->Post("/api/mine/stop",
        [this](const httplib::Request& req, httplib::Response& res) {
            stop_bitcoin_mining();
            json response = {{"success", true}, {"message", "Mining stopped"}};
            res.set_content(response.dump(), "application/json");
        });
    
    // Export results
    http_server_->Get("/api/export",
        [this](const httplib::Request& req, httplib::Response& res) {
            json results = export_results();
            res.set_content(results.dump(), "application/json");
        });
    
    // Static files (if needed)
    http_server_->set_mount_point("/", "./public");
}

void BioMiningApp::handle_get_status(const httplib::Request& req, httplib::Response& res) {
    json status = get_system_status();
    res.set_content(status.dump(), "application/json");
}

void BioMiningApp::handle_post_configure(const httplib::Request& req, httplib::Response& res) {
    try {
        json config_update = json::parse(req.body);
        
        // Update configuration based on request
        if (config_update.contains("mea") && config_update["mea"].contains("num_electrodes")) {
            config_.mea.num_electrodes = config_update["mea"]["num_electrodes"];
        }
        if (config_update.contains("network") && config_update["network"].contains("type")) {
            std::string type = config_update["network"]["type"];
            if (type == "biological") {
                config_.network.type = NetworkConfiguration::BIOLOGICAL_ONLY;
            } else if (type == "hybrid") {
                config_.network.type = NetworkConfiguration::HYBRID_BIO_ARTIFICIAL;
            }
        }
        
        json response = {{"success", true}, {"message", "Configuration updated"}};
        res.set_content(response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        json error = {{"success", false}, {"error", e.what()}};
        res.status = 400;
        res.set_content(error.dump(), "application/json");
    }
}

void BioMiningApp::handle_get_metrics(const httplib::Request& req, httplib::Response& res) {
    json metrics = get_performance_metrics();
    res.set_content(metrics.dump(), "application/json");
}

void BioMiningApp::handle_post_train(const httplib::Request& req, httplib::Response& res) {
    try {
        json request_data = json::parse(req.body);
        
        if (request_data.contains("data_path")) {
            std::string data_path = request_data["data_path"];
            if (import_training_data(data_path) && start_training()) {
                json response = {{"success", true}, {"message", "Training started"}};
                res.set_content(response.dump(), "application/json");
            } else {
                json error = {{"success", false}, {"error", "Failed to start training"}};
                res.status = 500;
                res.set_content(error.dump(), "application/json");
            }
        } else {
            if (start_training()) {
                json response = {{"success", true}, {"message", "Training started with existing data"}};
                res.set_content(response.dump(), "application/json");
            } else {
                json error = {{"success", false}, {"error", "No training data available"}};
                res.status = 400;
                res.set_content(error.dump(), "application/json");
            }
        }
        
    } catch (const std::exception& e) {
        json error = {{"success", false}, {"error", e.what()}};
        res.status = 400;
        res.set_content(error.dump(), "application/json");
    }
}

void BioMiningApp::handle_post_mine(const httplib::Request& req, httplib::Response& res) {
    if (start_bitcoin_mining()) {
        json response = {{"success", true}, {"message", "Bitcoin mining started"}};
        res.set_content(response.dump(), "application/json");
    } else {
        json error = {{"success", false}, {"error", "Failed to start mining"}};
        res.status = 500;
        res.set_content(error.dump(), "application/json");
    }
}

} // namespace biomining

// ====================================================================
// MAIN FUNCTION
// ====================================================================

int main(int argc, char* argv[]) {
    std::string config_file = "/config/biomining_config.json";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            config_file = argv[++i];
        } else if (arg == "--help") {
            std::cout << "BioMining Platform - High Performance C++ Application\n"
                      << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --config <file>  Configuration file path (default: /config/biomining_config.json)\n"
                      << "  --help           Show this help message\n";
            return 0;
        }
    }
    
    try {
        biomining::BioMiningApp app(config_file);
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize BioMining Platform" << std::endl;
            return 1;
        }
        
        std::cout << "🧬 BioMining Platform C++ - Starting..." << std::endl;
        std::cout << "Configuration: " << config_file << std::endl;
        std::cout << "Cloud Run Ready - High Performance Mode" << std::endl;
        
        app.run(); // This will block until shutdown
        
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}