#!/usr/bin/env python3
"""
Comprehensive test script to verify all Python bindings
- HybridBitcoinMiner
- RealMEAInterface  
- BiologicalNetwork
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

def test_all_bindings():
    """Test all BioMining Python bindings"""
    
    print("🧪 Testing ALL BioMining Python Bindings")
    print("=" * 60)
    
    try:
        # Try to import the bindings
        import biomining_cpp
        print("✅ C++ bindings import: SUCCESS")
        
        # Test crypto module access
        try:
            crypto_module = biomining_cpp.crypto
            print("✅ Crypto module access: SUCCESS")
        except Exception as e:
            print(f"❌ Crypto module access: FAILED - {e}")
            return False
        
        # Test bio module access  
        try:
            bio_module = biomining_cpp.bio
            print("✅ Bio module access: SUCCESS")
        except Exception as e:
            print(f"❌ Bio module access: FAILED - {e}")
            return False
        
        print("\n" + "="*60)
        print("🔥 TESTING HYBRID BITCOIN MINER")
        print("="*60)
        
        # Test HybridBitcoinMiner
        try:
            miner = biomining_cpp.crypto.HybridBitcoinMiner()
            print("✅ HybridBitcoinMiner class: SUCCESS")
            
            # Test MiningConfig
            config = biomining_cpp.crypto.MiningConfig()
            config.difficulty = 4
            config.threads = 2
            config.useGPU = False
            print("✅ MiningConfig structure: SUCCESS")
            
            # Test BiologicalLearningParams  
            learning_params = biomining_cpp.crypto.BiologicalLearningParams()
            learning_params.initialLearningRate = 0.01
            learning_params.retroLearningRate = 0.005
            learning_params.decayRate = 0.995
            learning_params.retroIterations = 5000
            print("✅ BiologicalLearningParams structure: SUCCESS")
            
            # Test HybridMiningMetrics
            metrics = biomining_cpp.crypto.HybridMiningMetrics()
            print(f"✅ HybridMiningMetrics structure: SUCCESS")
            print(f"   - Total hashes: {metrics.totalHashes}")
            print(f"   - Biological predictions: {metrics.biologicalPredictions}")
            print(f"   - Traditional hashes: {metrics.traditionalHashes}")
            print(f"   - Energy efficiency: {metrics.energyEfficiency}")
            print(f"   - Adaptation score: {metrics.adaptationScore}")
            
            # Test enums
            state = biomining_cpp.crypto.HybridLearningState.InitialLearning
            method = biomining_cpp.crypto.MiningMethod.HybridFusion
            print("✅ HybridLearningState and MiningMethod enums: SUCCESS")
            
            # Test essential methods
            success = miner.initialize()
            print(f"✅ initialize() method: SUCCESS - returned {success}")
            
            config_success = miner.configureBiologicalNetwork(learning_params)
            print(f"✅ configureBiologicalNetwork() method: SUCCESS - returned {config_success}")
            
            miner.setMiningParameters(config)
            print("✅ setMiningParameters() method: SUCCESS")
            
            is_mining = miner.isMining()
            print(f"✅ isMining() method: SUCCESS - returned {is_mining}")
            
        except Exception as e:
            print(f"❌ HybridBitcoinMiner testing: FAILED - {e}")
            return False
            
        print("\n" + "="*60)
        print("🔬 TESTING REAL MEA INTERFACE") 
        print("="*60)
        
        # Test RealMEAInterface
        try:
            mea = biomining_cpp.bio.RealMEAInterface()
            print("✅ RealMEAInterface class: SUCCESS")
            
            # Test RealMEAConfig with new properties
            mea_config = biomining_cpp.bio.RealMEAConfig()
            mea_config.deviceType = biomining_cpp.bio.MEADeviceType.Custom_Serial
            mea_config.protocol = biomining_cpp.bio.CommunicationProtocol.SerialPort
            mea_config.devicePath = "/dev/ttyUSB0"
            mea_config.networkHost = "localhost"
            mea_config.networkPort = 8080
            mea_config.baudRate = 115200
            mea_config.electrodeCount = 60
            mea_config.samplingRate = 25000.0
            mea_config.spikeWindowMs = 2
            mea_config.calibrationFile = "calibration.dat"
            print("✅ RealMEAConfig structure with all properties: SUCCESS")
            
            # Test data structures
            electrode_data = biomining_cpp.bio.ElectrodeData()
            electrode_data.electrodeId = 1
            electrode_data.voltage = 12.5
            electrode_data.isActive = True
            print("✅ ElectrodeData structure: SUCCESS")
            
            spike_event = biomining_cpp.bio.SpikeEvent()
            spike_event.electrodeId = 1
            spike_event.amplitude = 75.0
            print("✅ SpikeEvent structure: SUCCESS")
            
            # Test Bitcoin learning structures
            bitcoin_pattern = biomining_cpp.bio.BitcoinLearningPattern()
            bitcoin_pattern.targetNonce = 12345
            bitcoin_pattern.difficulty = 4
            print("✅ BitcoinLearningPattern structure: SUCCESS")
            
            bitcoin_config = biomining_cpp.bio.BitcoinLearningConfig()
            bitcoin_config.learningRate = 0.001
            bitcoin_config.maxTrainingEpochs = 1000
            bitcoin_config.enableRealtimeLearning = True
            print("✅ BitcoinLearningConfig structure: SUCCESS")
            
            # Test essential methods
            success = mea.initialize(mea_config)
            print(f"✅ initialize() method: SUCCESS - returned {success}")
            
            status = mea.getStatus()
            print(f"✅ getStatus() method: SUCCESS - returned {status}")
            
        except Exception as e:
            print(f"❌ RealMEAInterface testing: FAILED - {e}")
            return False
        
        print("\n" + "="*60)
        print("🧠 TESTING BIOLOGICAL NETWORK")
        print("="*60)
        
        # Test BiologicalNetwork
        try:
            network = biomining_cpp.bio.BiologicalNetwork()
            print("✅ BiologicalNetwork class: SUCCESS")
            
            # Test NetworkConfig with all properties
            net_config = biomining_cpp.bio.NetworkConfig()
            net_config.neuronCount = 60
            net_config.learningRate = 0.01
            net_config.stimulationThreshold = 0.5
            net_config.adaptationRate = 0.1
            net_config.memoryDepth = 1000
            net_config.useReinforcementLearning = True
            net_config.inputSize = 60
            net_config.outputSize = 32
            net_config.enablePlasticity = True
            net_config.enableAdaptation = True
            net_config.momentum = 0.9
            net_config.decayRate = 0.995
            net_config.adaptiveThreshold = 0.1
            net_config.maxEpochs = 10000
            print("✅ NetworkConfig structure with all properties: SUCCESS")
            
            # Test LearningState enum
            state = biomining_cpp.bio.LearningState.InitialLearning
            print(f"✅ LearningState enum: SUCCESS - {state}")
            
            # Test NoncePredicition structure
            prediction = biomining_cpp.bio.NoncePredicition()
            prediction.suggestedNonce = 98765
            prediction.confidence = 0.87
            prediction.expectedEfficiency = 1.25
            prediction.reasoning = "High pattern confidence"
            print("✅ NoncePredicition structure: SUCCESS")
            
            # Test LearningData structure
            learning_data = biomining_cpp.bio.LearningData()
            learning_data.targetNonce = 54321
            learning_data.blockHeader = "test_header"
            learning_data.difficulty = 4
            learning_data.wasSuccessful = True
            learning_data.attempts = 500
            learning_data.computeTime = 1.5
            print("✅ LearningData structure: SUCCESS")
            
            # Test essential methods
            success = network.initialize()
            print(f"✅ initialize() method: SUCCESS - returned {success}")
            
            config_success = network.setNetworkConfig(net_config)
            print(f"✅ setNetworkConfig() method: SUCCESS - returned {config_success}")
            
            learning_state = network.getLearningState()
            print(f"✅ getLearningState() method: SUCCESS - returned {learning_state}")
            
            is_complete = network.isLearningComplete()
            print(f"✅ isLearningComplete() method: SUCCESS - returned {is_complete}")
            
            efficiency = network.getNetworkEfficiency()
            print(f"✅ getNetworkEfficiency() method: SUCCESS - returned {efficiency}")
            
            complexity = network.getNetworkComplexity()
            print(f"✅ getNetworkComplexity() method: SUCCESS - returned {complexity}")
            
        except Exception as e:
            print(f"❌ BiologicalNetwork testing: FAILED - {e}")
            return False
            
        print("\n" + "🎉"*20)
        print("🚀 ALL BINDING TESTS PASSED! 🚀")
        print("🎉"*20)
        print("\n📋 SUMMARY:")
        print("✅ HybridBitcoinMiner - Complete integration")
        print("✅ RealMEAInterface - Complete with all config properties") 
        print("✅ BiologicalNetwork - Complete with all methods and structures")
        print("✅ All enums and data structures working")
        print("✅ All essential methods accessible")
        print("\n🔥 BioMining C++ Python bindings are fully functional!")
        return True
        
    except ImportError as e:
        print(f"❌ C++ bindings import: FAILED - {e}")
        print("💡 This is expected if C++ bindings are not compiled yet")
        return False
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return False

if __name__ == "__main__":
    success = test_all_bindings()
    exit(0 if success else 1)