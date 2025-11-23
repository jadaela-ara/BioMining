#!/usr/bin/env python3
"""
Test script to verify blockchain fetcher behavior during training simulation
"""

import time
import logging
from bitcoin_real_data_validator import BitcoinBlockchainFetcher

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def simulate_training_loop():
    """Simulate the training loop to check for duplicate blocks"""
    fetcher = BitcoinBlockchainFetcher()
    
    start_height = 870000
    count = 10
    
    logger.info(f"🧪 Simulating training loop: {start_height} to {start_height + count - 1}")
    logger.info("="*60)
    
    fetched_blocks = []
    
    for i in range(count):
        block_height = start_height + i
        
        logger.info(f"\n🔄 Training iteration {i+1}/{count}: Fetching block {block_height}")
        
        block = fetcher.fetch_block_by_height(block_height)
        
        if block:
            logger.info(f"   ✅ Got block {block.height}: hash={block.hash[:16]}..., nonce={block.nonce:#010x}")
            fetched_blocks.append({
                'iteration': i+1,
                'requested_height': block_height,
                'received_height': block.height,
                'hash': block.hash,
                'nonce': block.nonce
            })
        else:
            logger.error(f"   ❌ Failed to fetch block {block_height}")
        
        # Rate limiting (like in real training)
        time.sleep(1.5)
    
    # Analysis
    logger.info("\n" + "="*60)
    logger.info("📊 ANALYSIS OF FETCHED BLOCKS")
    logger.info("="*60)
    
    # Check for duplicates
    unique_heights = set(b['received_height'] for b in fetched_blocks)
    unique_hashes = set(b['hash'] for b in fetched_blocks)
    unique_nonces = set(b['nonce'] for b in fetched_blocks)
    
    logger.info(f"Total blocks fetched: {len(fetched_blocks)}")
    logger.info(f"Unique heights: {len(unique_heights)}")
    logger.info(f"Unique hashes: {len(unique_hashes)}")
    logger.info(f"Unique nonces: {len(unique_nonces)}")
    
    if len(fetched_blocks) == len(unique_heights) == len(unique_hashes) == len(unique_nonces):
        logger.info("✅ SUCCESS: All blocks are unique!")
    else:
        logger.error("❌ FAILURE: Duplicate blocks detected!")
        
        # Find duplicates
        nonce_counts = {}
        for b in fetched_blocks:
            nonce = b['nonce']
            if nonce not in nonce_counts:
                nonce_counts[nonce] = []
            nonce_counts[nonce].append(b['iteration'])
        
        duplicates = {k: v for k, v in nonce_counts.items() if len(v) > 1}
        if duplicates:
            logger.error(f"\n🔍 DUPLICATE NONCES FOUND:")
            for nonce, iterations in duplicates.items():
                logger.error(f"   Nonce {nonce:#010x} appeared in iterations: {iterations}")
    
    logger.info("\n" + "="*60)
    logger.info("📋 DETAILED BLOCK LIST:")
    logger.info("="*60)
    for b in fetched_blocks:
        match = "✅" if b['requested_height'] == b['received_height'] else "❌"
        logger.info(f"{match} Iter {b['iteration']:2d}: Requested {b['requested_height']}, Got {b['received_height']} - Hash {b['hash'][:16]}... Nonce {b['nonce']:#010x}")

if __name__ == "__main__":
    simulate_training_loop()
