#include <gtest/gtest.h>
#include "../include/bio/biological_network.h"
#include <QCoreApplication>

using namespace BioMining::Bio;

class BiologicalNetworkTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test fixture
        int argc = 0;
        char* argv[] = {nullptr};
        if (!QCoreApplication::instance()) {
            app = new QCoreApplication(argc, argv);
        }
        network = new BioMining::Bio::BiologicalNetwork();
    }

    void TearDown() override {
        delete network;
        // Note: QCoreApplication cleanup is handled by Qt
    }

    BioMining::Bio::BiologicalNetwork* network;
    QCoreApplication* app = nullptr;
};

TEST_F(BiologicalNetworkTest, Initialization) {
    EXPECT_TRUE(network->initialize());
}

TEST_F(BiologicalNetworkTest, NetworkConfiguration) {
    BioMining::Bio::BiologicalNetwork::NetworkConfig config;
    config.neuronCount = 64;
    config.learningRate = 0.02;
    
    network->setNetworkConfig(config);
    auto retrievedConfig = network->getNetworkConfig();
    
    EXPECT_EQ(retrievedConfig.neuronCount, 64);
    EXPECT_DOUBLE_EQ(retrievedConfig.learningRate, 0.02);
}

TEST_F(BiologicalNetworkTest, LearningStateTransition) {
    EXPECT_EQ(network->getLearningState(), BioMining::Bio::BiologicalNetwork::LearningState::Untrained);
    
    // Start learning
    EXPECT_TRUE(network->startInitialLearning(10));
    
    // Stop learning
    network->stopLearning();
}

TEST_F(BiologicalNetworkTest, NoncePredicition) {
    // Initialize and configure network
    EXPECT_TRUE(network->initialize());
    
    QString blockHeader = "0000000000000000000000000000000000000000000000000000000000000000";
    uint64_t difficulty = 1000;
    QVector<double> signalData = {0.1, 0.2, 0.3, 0.4, 0.5};
    
    auto prediction = network->predictOptimalNonce(blockHeader, difficulty, signalData);
    
    EXPECT_GT(prediction.confidence, 0.0);
    EXPECT_LE(prediction.confidence, 1.0);
    EXPECT_GT(prediction.suggestedNonce, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}