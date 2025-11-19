#!/usr/bin/env python3
"""
Quick test script to verify BiologicalNetwork Python bindings
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

def test_biological_network_bindings():
    """Test the new BiologicalNetwork bindings"""
    
    print("🧬 Testing BiologicalNetwork Python Bindings")
    print("=" * 50)
    
    try:
        # Try to import the bindings
        import biomining_cpp
        print("✅ C++ bindings import: SUCCESS")
        
        # Test BiologicalNetwork access
        try:
            network = biomining_cpp.bio.BiologicalNetwork()
            print("✅ BiologicalNetwork class: SUCCESS")
        except AttributeError as e:
            print(f"❌ BiologicalNetwork class: FAILED - {e}")
            return False
        except Exception as e:
            print(f"❌ BiologicalNetwork creation: FAILED - {e}")
            return False
        
        # Test NetworkConfig
        try:
            config = biomining_cpp.bio.NetworkConfig()
            print("✅ NetworkConfig structure: SUCCESS")
            
            # Test setting properties
            config.neuronCount = 60
            config.learningRate = 0.01
            config.memoryDepth = 1000
            print("✅ NetworkConfig properties: SUCCESS")
        except Exception as e:
            print(f"❌ NetworkConfig: FAILED - {e}")
            return False
        
        # Test LearningState enum
        try:
            state = biomining_cpp.bio.LearningState.Untrained
            print(f"✅ LearningState enum: SUCCESS - {state}")
        except Exception as e:
            print(f"❌ LearningState enum: FAILED - {e}")
            return False
        
        # Test essential methods
        try:
            # Test initialize
            success = network.initialize()
            print(f"✅ initialize() method: SUCCESS - returned {success}")
            
            # Test setNetworkConfig
            config_success = network.setNetworkConfig(config)
            print(f"✅ setNetworkConfig() method: SUCCESS - returned {config_success}")
            
            # Test isLearningComplete
            is_complete = network.isLearningComplete()
            print(f"✅ isLearningComplete() method: SUCCESS - returned {is_complete}")
            
            # Test getLearningState
            learning_state = network.getLearningState()
            print(f"✅ getLearningState() method: SUCCESS - returned {learning_state}")
            
        except Exception as e:
            print(f"❌ Method testing: FAILED - {e}")
            return False
        
        # Test NoncePredicition and LearningData
        try:
            prediction = biomining_cpp.bio.NoncePredicition()
            learning_data = biomining_cpp.bio.LearningData()
            print("✅ Data structures (NoncePredicition, LearningData): SUCCESS")
        except Exception as e:
            print(f"❌ Data structures: FAILED - {e}")
            return False
        
        print("\n🎉 ALL TESTS PASSED!")
        print("🔥 BiologicalNetwork Python bindings are working correctly!")
        return True
        
    except ImportError as e:
        print(f"❌ C++ bindings import: FAILED - {e}")
        print("💡 This is expected if C++ bindings are not compiled yet")
        return False
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return False

if __name__ == "__main__":
    success = test_biological_network_bindings()
    exit(0 if success else 1)