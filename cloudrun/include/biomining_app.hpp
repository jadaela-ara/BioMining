#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <fstream>
#include <sstream>

// HTTP Server
#include <httplib.h>

// JSON handling
#include <nlohmann/json.hpp>

// Bitcoin mining
#include <openssl/sha.h>

// Performance monitoring
#include <sys/resource.h>
#include <sys/sysinfo.h>

namespace biomining {

using json = nlohmann::json;

// ====================================================================
// 1. CONFIGURATION SYSTÈME
// ====================================================================

struct MEAConfiguration {
    std::string device_type = "MultiChannelSystems_MEA2100";
    std::string connection_protocol = "tcp";
    std::string host = "localhost";
    int port = 6340;
    int num_electrodes = 60;
    int sampling_rate = 25000;
    double max_voltage = 200.0; // μV
    double impedance_threshold = 5.0; // MΩ
    bool safety_enabled = true;
};

struct NetworkConfiguration {
    enum NetworkType {
        BIOLOGICAL_ONLY,
        HYBRID_BIO_ARTIFICIAL,
        ARTIFICIAL_ASSISTED
    };
    
    NetworkType type = HYBRID_BIO_ARTIFICIAL;
    int hidden_layers = 3;
    int neurons_per_layer = 128;
    double learning_rate = 0.001;
    double dropout_rate = 0.2;
    int batch_size = 32;
    int max_epochs = 1000;
    bool use_gpu_acceleration = true;
};

struct BitcoinConfiguration {
    std::string pool_url = "";
    std::string wallet_address = "";
    int difficulty_target = 4; // Starting difficulty
    bool use_testnet = true;
    int max_threads = std::thread::hardware_concurrency();
    bool enable_real_mining = false;
};

struct PlatformConfiguration {
    MEAConfiguration mea;
    NetworkConfiguration network;
    BitcoinConfiguration bitcoin;
    std::string log_level = "info";
    std::string data_directory = "/data";
    bool enable_metrics = true;
    int http_port = 8080;
};

// ====================================================================
// 2. GESTION DONNÉES MEA
// ====================================================================

struct ElectrodeData {
    int electrode_id;
    std::vector<double> voltages;
    std::vector<uint64_t> timestamps;
    double impedance;
    double noise_level;
    int spike_count;
    bool is_active;
    
    json to_json() const {
        return json{
            {"electrode_id", electrode_id},
            {"voltages", voltages},
            {"timestamps", timestamps}, 
            {"impedance", impedance},
            {"noise_level", noise_level},
            {"spike_count", spike_count},
            {"is_active", is_active}
        };
    }
};

struct MEADataset {
    std::vector<ElectrodeData> electrodes;
    uint64_t start_timestamp;
    uint64_t end_timestamp;
    int sampling_rate;
    std::string session_id;
    
    void add_electrode_data(const ElectrodeData& data) {
        electrodes.push_back(data);
    }
    
    json to_json() const {
        json result;
        result["electrodes"] = json::array();
        for (const auto& electrode : electrodes) {
            result["electrodes"].push_back(electrode.to_json());
        }
        result["start_timestamp"] = start_timestamp;
        result["end_timestamp"] = end_timestamp;
        result["sampling_rate"] = sampling_rate;
        result["session_id"] = session_id;
        return result;
    }
};

// ====================================================================
// 3. RÉSEAUX DE NEURONES (BIO + HYBRIDE)
// ====================================================================

class NeuralNetwork {
public:
    virtual ~NeuralNetwork() = default;
    virtual bool initialize(const NetworkConfiguration& config) = 0;
    virtual bool train(const MEADataset& training_data) = 0;
    virtual std::vector<double> predict(const std::vector<double>& input) = 0;
    virtual double get_training_accuracy() const = 0;
    virtual json get_metrics() const = 0;
};

class BiologicalNetwork : public NeuralNetwork {
private:
    MEAConfiguration mea_config_;
    std::vector<std::vector<double>> connection_weights_;
    std::vector<double> neuron_states_;
    double plasticity_rate_ = 0.01;
    
public:
    bool initialize(const NetworkConfiguration& config) override;
    bool train(const MEADataset& training_data) override;
    std::vector<double> predict(const std::vector<double>& input) override;
    double get_training_accuracy() const override;
    json get_metrics() const override;
    
    // Biological-specific methods
    void stimulate_electrodes(const std::vector<int>& electrode_ids, double amplitude);
    std::vector<double> read_electrode_responses(const std::vector<int>& electrode_ids);
    void update_synaptic_weights(const std::vector<double>& hebbian_updates);
};

class HybridNetwork : public NeuralNetwork {
private:
    std::unique_ptr<BiologicalNetwork> bio_component_;
    std::vector<std::vector<double>> artificial_weights_;
    double bio_artificial_ratio_ = 0.7; // 70% biological, 30% artificial
    
public:
    bool initialize(const NetworkConfiguration& config) override;
    bool train(const MEADataset& training_data) override;
    std::vector<double> predict(const std::vector<double>& input) override;
    double get_training_accuracy() const override;
    json get_metrics() const override;
    
    // Hybrid-specific methods
    void set_bio_artificial_ratio(double ratio) { bio_artificial_ratio_ = ratio; }
    double get_bio_artificial_ratio() const { return bio_artificial_ratio_; }
};

// ====================================================================
// 4. MINING BITCOIN PERFORMANT
// ====================================================================

struct BitcoinBlock {
    uint32_t version;
    std::string previous_hash;
    std::string merkle_root;
    uint32_t timestamp;
    uint32_t bits;
    uint32_t nonce;
    
    std::string calculate_hash() const;
    std::vector<uint8_t> serialize() const;
    bool is_valid_hash(const std::string& hash, uint32_t difficulty_target) const;
};

class BitcoinMiner {
private:
    BitcoinConfiguration config_;
    std::atomic<bool> mining_active_{false};
    std::atomic<uint64_t> hash_count_{0};
    std::atomic<uint32_t> blocks_found_{0};
    std::vector<std::thread> mining_threads_;
    std::unique_ptr<NeuralNetwork> neural_predictor_;
    
    // Performance metrics
    std::chrono::steady_clock::time_point start_time_;
    std::atomic<double> current_hashrate_{0.0};
    
public:
    explicit BitcoinMiner(const BitcoinConfiguration& config);
    ~BitcoinMiner();
    
    bool initialize();
    void start_mining();
    void stop_mining();
    
    // Mining with neural network assistance
    bool mine_block_with_neural_assistance(BitcoinBlock& block);
    std::vector<uint32_t> predict_nonce_candidates(const BitcoinBlock& block);
    
    // Metrics
    double get_hashrate() const { return current_hashrate_.load(); }
    uint64_t get_total_hashes() const { return hash_count_.load(); }
    uint32_t get_blocks_found() const { return blocks_found_.load(); }
    
    json get_mining_stats() const;
    
private:
    void mining_thread_worker(int thread_id);
    std::string double_sha256(const std::vector<uint8_t>& data) const;
};

// ====================================================================
// 5. GESTIONNAIRE PRINCIPAL APPLICATION
// ====================================================================

class BioMiningApp {
private:
    PlatformConfiguration config_;
    std::unique_ptr<NeuralNetwork> neural_network_;
    std::unique_ptr<BitcoinMiner> bitcoin_miner_;
    std::unique_ptr<httplib::Server> http_server_;
    
    // Data management
    std::vector<MEADataset> training_datasets_;
    MEADataset current_session_data_;
    
    // Performance monitoring
    std::mutex performance_mutex_;
    json performance_metrics_;
    std::thread metrics_thread_;
    std::atomic<bool> monitoring_active_{false};
    
    // Training state
    std::atomic<bool> training_active_{false};
    std::atomic<double> training_progress_{0.0};
    
public:
    explicit BioMiningApp(const std::string& config_file);
    ~BioMiningApp();
    
    // Lifecycle
    bool initialize();
    void run();
    void shutdown();
    
    // Configuration
    bool load_configuration(const std::string& config_file);
    bool save_configuration(const std::string& config_file) const;
    void update_configuration(const PlatformConfiguration& new_config);
    
    // MEA Operations
    bool configure_mea_device(const MEAConfiguration& config);
    bool start_mea_acquisition();
    bool stop_mea_acquisition();
    MEADataset get_current_mea_data() const;
    
    // Network Management
    bool create_network(NetworkConfiguration::NetworkType type);
    bool import_training_data(const std::string& data_path);
    bool start_training();
    bool stop_training();
    double get_training_progress() const { return training_progress_.load(); }
    
    // Bitcoin Mining
    bool start_bitcoin_mining();
    bool stop_bitcoin_mining();
    json get_mining_performance() const;
    
    // Results & Analytics
    json get_system_status() const;
    json get_performance_metrics() const;
    json export_results() const;
    
    // HTTP API
    void setup_http_routes();
    void handle_get_status(const httplib::Request& req, httplib::Response& res);
    void handle_post_configure(const httplib::Request& req, httplib::Response& res);
    void handle_get_metrics(const httplib::Request& req, httplib::Response& res);
    void handle_post_train(const httplib::Request& req, httplib::Response& res);
    void handle_post_mine(const httplib::Request& req, httplib::Response& res);
    
private:
    void performance_monitoring_thread();
    void update_performance_metrics();
    json collect_system_metrics() const;
    
    // Utility methods
    std::string get_timestamp() const;
    void log(const std::string& level, const std::string& message) const;
};

// ====================================================================
// 6. FACTORY PATTERNS
// ====================================================================

class NetworkFactory {
public:
    static std::unique_ptr<NeuralNetwork> create_network(
        NetworkConfiguration::NetworkType type,
        const NetworkConfiguration& config
    );
};

} // namespace biomining