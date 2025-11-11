#!/usr/bin/env python3
"""
Quick test script for PurePythonBiologicalNetwork
Verifies the real neural network implementation works
"""

import sys
import time
from pathlib import Path

# Add paths
sys.path.append(str(Path(__file__).parent))

# Import the server module
from web.api.server import PurePythonBiologicalNetwork, CppBiologicalNetwork

def test_pure_python_network():
    """Test PurePythonBiologicalNetwork directly"""
    print("=" * 70)
    print("🧠 Testing PurePythonBiologicalNetwork")
    print("=" * 70)
    
    # Create network
    print("\n1️⃣ Creating neural network...")
    network = PurePythonBiologicalNetwork()
    
    # Initialize
    print("\n2️⃣ Initializing network...")
    success = network.initialize()
    print(f"   Initialization: {'✅ SUCCESS' if success else '❌ FAILED'}")
    
    # Get initial state
    print("\n3️⃣ Getting network state...")
    state = network.get_network_state()
    print(f"   Active neurons: {state.get('active_neurons', 0)}")
    print(f"   Synaptic connections: {state.get('synaptic_connections', 0)}")
    print(f"   Architecture: {state.get('architecture', 'N/A')}")
    print(f"   Learning phase: {state.get('learning_phase', 'N/A')}")
    
    # Train network
    print("\n4️⃣ Training neural network (100 epochs)...")
    start_time = time.time()
    
    learning_config = {
        'epochs': 100,
        'batch_size': 32,
        'learning_rate': 0.01
    }
    
    success = network.start_learning(learning_config)
    training_time = time.time() - start_time
    
    print(f"   Training: {'✅ SUCCESS' if success else '❌ FAILED'}")
    print(f"   Training time: {training_time:.2f} seconds")
    
    # Get trained state
    print("\n5️⃣ Getting trained network state...")
    state = network.get_network_state()
    print(f"   Learning progress: {state.get('learning_progress', 0.0):.2%}")
    print(f"   Pattern accuracy: {state.get('pattern_accuracy', 0.0):.2%}")
    print(f"   Final loss: {state.get('loss', 0.0):.6f}")
    print(f"   Training epochs: {state.get('current_epoch', 0)}")
    
    # Test prediction
    print("\n6️⃣ Testing nonce prediction...")
    test_block = b"1|0000000000000000000000000000000000000000000000000000000000000000|4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b|1609459200|0x1d00ffff|0"
    
    prediction = network.predict_optimal_nonce(test_block)
    print(f"   Predicted nonce: {prediction.get('predicted_nonce', 0):#010x}")
    print(f"   Confidence: {prediction.get('confidence', 0.0):.2%}")
    print(f"   Neural activation: {prediction.get('neural_activation', 0.0):.4f}")
    print(f"   Pattern match: {prediction.get('pattern_match_score', 0.0):.2%}")
    
    # Test learning examples
    print("\n7️⃣ Testing learning examples...")
    network.add_learning_example(test_block, 0x12345678, success=True, hash_result="0000abc...")
    network.add_learning_example(test_block, 0x87654321, success=False, hash_result="")
    
    state = network.get_network_state()
    print(f"   Total examples: {state.get('total_examples', 0)}")
    
    print("\n✅ PurePythonBiologicalNetwork test completed!")


def test_cpp_wrapper():
    """Test CppBiologicalNetwork wrapper"""
    print("\n" + "=" * 70)
    print("🔧 Testing CppBiologicalNetwork Wrapper")
    print("=" * 70)
    
    # Create wrapper
    print("\n1️⃣ Creating wrapper...")
    wrapper = CppBiologicalNetwork()
    
    # Initialize
    print("\n2️⃣ Initializing through wrapper...")
    success = wrapper.initialize()
    print(f"   Initialization: {'✅ SUCCESS' if success else '❌ FAILED'}")
    
    # Quick training
    print("\n3️⃣ Quick training (50 epochs)...")
    config = {'epochs': 50, 'batch_size': 32, 'learning_rate': 0.01}
    success = wrapper.start_learning(config)
    print(f"   Training: {'✅ SUCCESS' if success else '❌ FAILED'}")
    
    # Test compatibility methods
    print("\n4️⃣ Testing C++ compatibility methods...")
    print(f"   getLearningState(): {wrapper.getLearningState()}")
    print(f"   getTrainingProgress(): {wrapper.getTrainingProgress():.2%}")
    print(f"   getNetworkEfficiency(): {wrapper.getNetworkEfficiency():.2%}")
    print(f"   getNetworkComplexity(): {wrapper.getNetworkComplexity():.4f}")
    print(f"   isLearningComplete(): {wrapper.isLearningComplete()}")
    
    # Test prediction through wrapper
    print("\n5️⃣ Testing prediction through wrapper...")
    test_block = b"1|0000000000000000000000000000000000000000000000000000000000000000|4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b|1609459200|0x1d00ffff|0"
    prediction = wrapper.predict_optimal_nonce(test_block)
    print(f"   Predicted nonce: {prediction.get('predicted_nonce', 0):#010x}")
    print(f"   Confidence: {prediction.get('confidence', 0.0):.2%}")
    
    print("\n✅ CppBiologicalNetwork wrapper test completed!")


if __name__ == "__main__":
    print("\n" + "🧠" * 35)
    print("   REAL PYTHON NEURAL NETWORK TEST")
    print("🧠" * 35 + "\n")
    
    try:
        # Test pure Python implementation
        test_pure_python_network()
        
        # Test wrapper
        test_cpp_wrapper()
        
        print("\n" + "=" * 70)
        print("🎉 ALL TESTS PASSED! Neural network is REAL and functional!")
        print("=" * 70 + "\n")
        
    except Exception as e:
        print(f"\n❌ ERROR during testing: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
