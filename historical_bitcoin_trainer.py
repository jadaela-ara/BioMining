#!/usr/bin/env python3
"""
Historical Bitcoin Trainer
==========================

Trains Bio-Entropy Mining components (Neural Network + MEA) on real Bitcoin blocks.

Features:
- Train on historical Bitcoin blocks
- Validate predictions against real blocks
- Track improvement metrics
- Adaptive learning based on difficulty
- Performance analysis and reporting
"""

import time
import logging
import json
from typing import Dict, List, Any, Optional, Tuple
from dataclasses import dataclass, asdict
from datetime import datetime
import numpy as np

# Import Bitcoin validation module
from bitcoin_real_data_validator import BitcoinBlockchainFetcher, BitcoinBlock

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


@dataclass
class TrainingResult:
    """Result of training on a single block"""
    block_height: int
    neural_loss: float
    mea_success: bool
    training_time: float
    timestamp: str
    
    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


@dataclass
class ValidationResult:
    """Result of validation on a block"""
    block_height: int
    actual_nonce: int
    neural_prediction: int
    neural_distance: int
    neural_distance_percent: float
    mea_prediction: int
    mea_distance: int
    mea_distance_percent: float
    best_method: str
    success: bool
    timestamp: str
    
    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


@dataclass
class TrainingSession:
    """Complete training session results"""
    session_id: str
    start_time: str
    end_time: str
    blocks_trained: int
    blocks_validated: int
    avg_neural_loss: float
    avg_neural_distance_before: float
    avg_neural_distance_after: float
    improvement_percent: float
    success_rate_before: float
    success_rate_after: float
    training_results: List[TrainingResult]
    validation_results: List[ValidationResult]
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            **asdict(self),
            'training_results': [r.to_dict() for r in self.training_results],
            'validation_results': [r.to_dict() for r in self.validation_results]
        }


class HistoricalBitcoinTrainer:
    """Train Bio-Entropy Mining components on historical Bitcoin blocks"""
    
    def __init__(self, neural_network, mea_interface, bio_entropy_generator):
        """
        Initialize trainer
        
        Args:
            neural_network: Neural network instance (PurePythonBiologicalNetwork)
            mea_interface: MEA interface instance (PurePythonRealMEAInterface)
            bio_entropy_generator: Bio-entropy generator instance
        """
        self.neural_network = neural_network
        self.mea = mea_interface
        self.bio_entropy_generator = bio_entropy_generator
        self.blockchain_fetcher = BitcoinBlockchainFetcher()
        
        self.training_history: List[TrainingResult] = []
        self.validation_history: List[ValidationResult] = []
        
        logger.info("🎓 Historical Bitcoin Trainer initialized")
    
    def extract_features(self, block: BitcoinBlock) -> np.ndarray:
        """Extract features from Bitcoin block"""
        block_header = block.get_block_header()
        features_dict = self.bio_entropy_generator.extract_features(
            block_header,
            block.difficulty if block.difficulty > 0 else 1.0
        )
        
        # Convert to numpy array (60 dimensions)
        features = np.array([
            features_dict.get('version_norm', 0.0),
            features_dict.get('timestamp_norm', 0.0),
            features_dict.get('difficulty_level', 0.0),
            features_dict.get('prev_hash_entropy', 0.0),
            features_dict.get('merkle_entropy', 0.0),
            features_dict.get('bits_complexity', 0.0),
            features_dict.get('nonce_seed', 0.0),
            features_dict.get('block_position', 0.0),
            features_dict.get('time_variance', 0.0)
        ])
        
        # Expand to 60 dimensions by repeating and adding noise
        expanded = np.tile(features, 7)[:60]  # Repeat to fill 60
        expanded += np.random.randn(60) * 0.01  # Small noise for diversity
        
        return expanded
    
    def train_neural_network_on_block(self, block: BitcoinBlock) -> float:
        """
        Train neural network on a single block
        
        Returns:
            Training loss
        """
        features = self.extract_features(block)
        
        # Normalize nonce to [0, 1] for training
        target_nonce_normalized = block.nonce / 0xFFFFFFFF
        
        # Create target vector (32 dimensions for output layer)
        target = np.zeros(32)
        for i in range(32):
            bit = (block.nonce >> i) & 1
            target[i] = float(bit)
        
        # Forward pass
        prediction = self.neural_network.forward_propagation(features)
        
        # Calculate loss (MSE)
        loss = np.mean((prediction - target) ** 2)
        
        # Backward pass (gradient descent)
        self.neural_network.backward_propagation(target, learning_rate=0.001)
        
        return float(loss)
    
    def train_mea_on_block(self, block: BitcoinBlock) -> bool:
        """
        Train MEA on a single block
        
        Returns:
            Success status
        """
        pattern_data = {
            'block_header': block.get_block_header().encode(),
            'target_nonce': block.nonce,
            'difficulty': block.difficulty if block.difficulty > 0 else 1.0,
            'timestamp': block.timestamp
        }
        
        success = self.mea.train_bitcoin_pattern(pattern_data)
        return success
    
    def predict_with_neural_network(self, block: BitcoinBlock) -> Tuple[int, float]:
        """
        Predict nonce using neural network
        
        Returns:
            (predicted_nonce, confidence)
        """
        block_data = block.get_block_header().encode()
        
        try:
            prediction = self.neural_network.predict_optimal_nonce(block_data)
            
            if isinstance(prediction, dict):
                predicted_nonce = prediction.get('predicted_nonce', 0)
                confidence = prediction.get('confidence', 0.0)
            else:
                predicted_nonce = prediction
                confidence = 0.5
            
            return predicted_nonce, confidence
        except Exception as e:
            logger.warning(f"Neural prediction failed: {e}")
            return 0, 0.0
    
    def predict_with_mea(self, block: BitcoinBlock) -> Optional[int]:
        """
        Predict nonce using MEA
        
        Returns:
            Predicted nonce or None
        """
        try:
            block_hash = block.hash
            
            if hasattr(self.mea, 'generate_stimulation_pattern'):
                stim_pattern = self.mea.generate_stimulation_pattern(block_hash)
                
                if hasattr(self.mea, 'stimulate_electrodes'):
                    spikes = self.mea.stimulate_electrodes(stim_pattern, duration=50.0)
                    
                    if hasattr(self.mea, 'extract_nonce_from_spikes') and len(spikes) > 0:
                        mea_nonce = self.mea.extract_nonce_from_spikes(spikes)
                        return mea_nonce
            
            return None
        except Exception as e:
            logger.warning(f"MEA prediction failed: {e}")
            return None
    
    def validate_on_block(self, block: BitcoinBlock) -> ValidationResult:
        """
        Validate predictions on a single block
        
        Returns:
            ValidationResult
        """
        # Neural network prediction
        neural_prediction, neural_confidence = self.predict_with_neural_network(block)
        neural_distance = abs(neural_prediction - block.nonce)
        neural_distance_percent = (neural_distance / 0xFFFFFFFF) * 100
        
        # MEA prediction
        mea_prediction = self.predict_with_mea(block)
        if mea_prediction is not None:
            mea_distance = abs(mea_prediction - block.nonce)
            mea_distance_percent = (mea_distance / 0xFFFFFFFF) * 100
        else:
            mea_prediction = 0
            mea_distance = 0xFFFFFFFF
            mea_distance_percent = 100.0
        
        # Determine best method
        if neural_distance < mea_distance:
            best_method = "neural_network"
            best_distance_percent = neural_distance_percent
        else:
            best_method = "mea_interface"
            best_distance_percent = mea_distance_percent
        
        # Success if within 10% of nonce space
        success = best_distance_percent < 10.0
        
        result = ValidationResult(
            block_height=block.height,
            actual_nonce=block.nonce,
            neural_prediction=neural_prediction,
            neural_distance=neural_distance,
            neural_distance_percent=neural_distance_percent,
            mea_prediction=mea_prediction,
            mea_distance=mea_distance,
            mea_distance_percent=mea_distance_percent,
            best_method=best_method,
            success=success,
            timestamp=datetime.now().isoformat()
        )
        
        self.validation_history.append(result)
        return result
    
    def train_on_historical_blocks(
        self,
        start_height: int,
        count: int = 100,
        validate_every: int = 10,
        validation_count: int = 5
    ) -> TrainingSession:
        """
        Train on historical Bitcoin blocks
        
        Args:
            start_height: Starting block height
            count: Number of blocks to train on
            validate_every: Validate every N blocks
            validation_count: Number of blocks to validate each time
            
        Returns:
            TrainingSession with results
        """
        session_id = f"training_{start_height}_{count}_{int(time.time())}"
        start_time = datetime.now().isoformat()
        
        logger.info(f"🎓 Starting training session: {session_id}")
        logger.info(f"   Training blocks: {start_height} to {start_height + count - 1}")
        logger.info(f"   Validation every: {validate_every} blocks")
        
        # Baseline validation (before training)
        logger.info(f"\n📊 Baseline validation (before training)...")
        baseline_results = []
        for i in range(validation_count):
            block = self.blockchain_fetcher.fetch_block_by_height(start_height + count + i)
            if block:
                result = self.validate_on_block(block)
                baseline_results.append(result)
                time.sleep(1.5)  # Rate limiting
        
        avg_baseline_distance = sum(r.neural_distance_percent for r in baseline_results) / len(baseline_results)
        baseline_success_rate = sum(1 for r in baseline_results if r.success) / len(baseline_results)
        
        logger.info(f"   Baseline neural distance: {avg_baseline_distance:.2f}%")
        logger.info(f"   Baseline success rate: {baseline_success_rate:.2%}")
        
        # Training loop
        training_results = []
        total_neural_loss = 0.0
        
        for i in range(count):
            block_height = start_height + i
            
            logger.info(f"\n🔄 Training on block {i+1}/{count}: height {block_height}")
            
            # Fetch block
            block = self.blockchain_fetcher.fetch_block_by_height(block_height)
            if not block:
                logger.warning(f"   ⚠️  Failed to fetch block {block_height}, skipping")
                continue
            
            logger.info(f"   Block hash: {block.hash[:16]}...")
            logger.info(f"   Actual nonce: {block.nonce:#010x}")
            
            # Train neural network
            training_start = time.time()
            neural_loss = self.train_neural_network_on_block(block)
            total_neural_loss += neural_loss
            
            # Train MEA
            mea_success = self.train_mea_on_block(block)
            
            training_time = time.time() - training_start
            
            result = TrainingResult(
                block_height=block_height,
                neural_loss=neural_loss,
                mea_success=mea_success,
                training_time=training_time,
                timestamp=datetime.now().isoformat()
            )
            training_results.append(result)
            self.training_history.append(result)
            
            logger.info(f"   ✅ Neural loss: {neural_loss:.6f}")
            logger.info(f"   ✅ MEA training: {'success' if mea_success else 'failed'}")
            logger.info(f"   ⏱️  Training time: {training_time:.2f}s")
            
            # Periodic validation
            if (i + 1) % validate_every == 0:
                logger.info(f"\n📊 Validation after {i+1} blocks...")
                validation_results = []
                for j in range(validation_count):
                    val_block = self.blockchain_fetcher.fetch_block_by_height(start_height + count + j)
                    if val_block:
                        val_result = self.validate_on_block(val_block)
                        validation_results.append(val_result)
                        logger.info(f"   Block {val_block.height}: neural={val_result.neural_distance_percent:.2f}%, success={'✅' if val_result.success else '❌'}")
                        time.sleep(1.5)
                
                if validation_results:
                    avg_distance = sum(r.neural_distance_percent for r in validation_results) / len(validation_results)
                    success_rate = sum(1 for r in validation_results if r.success) / len(validation_results)
                    improvement = ((avg_baseline_distance - avg_distance) / avg_baseline_distance) * 100
                    
                    logger.info(f"   📈 Current neural distance: {avg_distance:.2f}%")
                    logger.info(f"   📈 Current success rate: {success_rate:.2%}")
                    logger.info(f"   📈 Improvement vs baseline: {improvement:+.1f}%")
            
            # Rate limiting
            time.sleep(1.5)
        
        # Final validation
        logger.info(f"\n📊 Final validation after training...")
        final_results = []
        for i in range(validation_count):
            block = self.blockchain_fetcher.fetch_block_by_height(start_height + count + i)
            if block:
                result = self.validate_on_block(block)
                final_results.append(result)
                time.sleep(1.5)
        
        avg_final_distance = sum(r.neural_distance_percent for r in final_results) / len(final_results)
        final_success_rate = sum(1 for r in final_results if r.success) / len(final_results)
        
        improvement_percent = ((avg_baseline_distance - avg_final_distance) / avg_baseline_distance) * 100
        
        end_time = datetime.now().isoformat()
        
        # Create session summary
        session = TrainingSession(
            session_id=session_id,
            start_time=start_time,
            end_time=end_time,
            blocks_trained=len(training_results),
            blocks_validated=len(final_results),
            avg_neural_loss=total_neural_loss / len(training_results) if training_results else 0.0,
            avg_neural_distance_before=avg_baseline_distance,
            avg_neural_distance_after=avg_final_distance,
            improvement_percent=improvement_percent,
            success_rate_before=baseline_success_rate,
            success_rate_after=final_success_rate,
            training_results=training_results,
            validation_results=final_results
        )
        
        # Log final summary
        logger.info(f"\n{'='*60}")
        logger.info(f"🎓 TRAINING SESSION COMPLETE")
        logger.info(f"{'='*60}")
        logger.info(f"Session ID: {session_id}")
        logger.info(f"Blocks trained: {session.blocks_trained}")
        logger.info(f"Avg neural loss: {session.avg_neural_loss:.6f}")
        logger.info(f"\n📊 Performance Comparison:")
        logger.info(f"   Before training:")
        logger.info(f"      Neural distance: {session.avg_neural_distance_before:.2f}%")
        logger.info(f"      Success rate: {session.success_rate_before:.2%}")
        logger.info(f"   After training:")
        logger.info(f"      Neural distance: {session.avg_neural_distance_after:.2f}%")
        logger.info(f"      Success rate: {session.success_rate_after:.2%}")
        logger.info(f"   Improvement: {session.improvement_percent:+.1f}%")
        logger.info(f"{'='*60}\n")
        
        return session
    
    def save_session(self, session: TrainingSession, filename: str):
        """Save training session to file"""
        with open(filename, 'w') as f:
            json.dump(session.to_dict(), f, indent=2)
        logger.info(f"✅ Session saved to {filename}")
    
    def load_session(self, filename: str) -> Dict[str, Any]:
        """Load training session from file"""
        with open(filename, 'r') as f:
            data = json.load(f)
        logger.info(f"✅ Session loaded from {filename}")
        return data


def main():
    """Test the trainer"""
    logger.info("🧪 Testing Historical Bitcoin Trainer")
    
    # This is a test - in real usage, these would come from the platform
    logger.info("\n⚠️  Note: This is a test script.")
    logger.info("   Real training should be launched from the web platform.")
    logger.info("   See train_on_historical_blocks.py for standalone training.")


if __name__ == "__main__":
    main()
