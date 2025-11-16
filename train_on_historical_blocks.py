#!/usr/bin/env python3
"""
Train on Historical Bitcoin Blocks
===================================

Standalone script to train Bio-Entropy Mining components on historical blocks.

Usage:
    python train_on_historical_blocks.py --start 869900 --count 100
"""

import sys
import argparse
import logging
from datetime import datetime

# Import platform components
sys.path.append('.')
from web.api.server import BioMiningPlatform
from historical_bitcoin_trainer import HistoricalBitcoinTrainer

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


def main():
    parser = argparse.ArgumentParser(description='Train on historical Bitcoin blocks')
    parser.add_argument('--start', type=int, default=869900, help='Starting block height')
    parser.add_argument('--count', type=int, default=100, help='Number of blocks to train on')
    parser.add_argument('--validate-every', type=int, default=10, help='Validate every N blocks')
    parser.add_argument('--validation-count', type=int, default=5, help='Number of blocks to validate')
    parser.add_argument('--output', type=str, default=None, help='Output file for results (JSON)')
    
    args = parser.parse_args()
    
    logger.info("="*60)
    logger.info("🎓 HISTORICAL BITCOIN TRAINING")
    logger.info("="*60)
    logger.info(f"Start block: {args.start}")
    logger.info(f"Block count: {args.count}")
    logger.info(f"Validate every: {args.validate_every} blocks")
    logger.info(f"Validation count: {args.validation_count} blocks")
    logger.info("="*60)
    
    # Initialize Bio-Entropy platform
    logger.info("\n🚀 Initializing Bio-Entropy Mining Platform...")
    platform = BioMiningPlatform()
    
    # Get neural network and MEA
    neural_network = platform.biological_network
    mea_interface = platform.mea_interface
    bio_entropy_generator = platform.bio_entropy_generator
    
    logger.info("✅ Platform initialized")
    logger.info(f"   Neural Network: {type(neural_network).__name__}")
    logger.info(f"   MEA Interface: {type(mea_interface).__name__}")
    
    # Create trainer
    trainer = HistoricalBitcoinTrainer(
        neural_network=neural_network,
        mea_interface=mea_interface,
        bio_entropy_generator=bio_entropy_generator
    )
    
    # Train
    logger.info(f"\n🎓 Starting training on {args.count} blocks...")
    session = trainer.train_on_historical_blocks(
        start_height=args.start,
        count=args.count,
        validate_every=args.validate_every,
        validation_count=args.validation_count
    )
    
    # Save results
    if args.output:
        output_file = args.output
    else:
        output_file = f"training_session_{args.start}_{args.count}_{int(datetime.now().timestamp())}.json"
    
    trainer.save_session(session, output_file)
    
    logger.info(f"\n✅ Training complete! Results saved to: {output_file}")
    
    # Print summary
    print("\n" + "="*60)
    print("📊 TRAINING SUMMARY")
    print("="*60)
    print(f"Blocks trained: {session.blocks_trained}")
    print(f"Average loss: {session.avg_neural_loss:.6f}")
    print(f"\nPerformance:")
    print(f"  Before: {session.avg_neural_distance_before:.2f}% distance, {session.success_rate_before:.2%} success")
    print(f"  After:  {session.avg_neural_distance_after:.2f}% distance, {session.success_rate_after:.2%} success")
    print(f"  Improvement: {session.improvement_percent:+.1f}%")
    print("="*60)
    
    # Recommendation
    if session.improvement_percent > 20:
        print("\n✅ EXCELLENT! Improvement > 20%")
        print("   Recommendation: Continue with extended training (1000+ blocks)")
    elif session.improvement_percent > 10:
        print("\n✅ GOOD! Improvement > 10%")
        print("   Recommendation: Continue training with more blocks")
    elif session.improvement_percent > 0:
        print("\n⚠️  MODEST improvement")
        print("   Recommendation: Adjust hyperparameters or try more blocks")
    else:
        print("\n❌ NO improvement or regression")
        print("   Recommendation: Review training strategy")


if __name__ == "__main__":
    main()
