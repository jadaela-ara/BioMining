#!/usr/bin/env python3
"""
================================================================
⛏️🧬 REAL BITCOIN MINER - Bio-Entropy Enhanced Mining
Complete Bitcoin miner with Testnet support
Integrates biological entropy and neural network predictions
================================================================
"""

import asyncio
import hashlib
import struct
import time
import logging
import argparse
import json
import sys
from pathlib import Path
from typing import Optional, Dict, Any, List
from dataclasses import dataclass

# Add parent directory to path
sys.path.append(str(Path(__file__).parent))

from stratum_client import StratumClient, StratumJob

# Bio-entropy will be imported lazily to avoid circular imports
BIO_ENTROPY_AVAILABLE = False
PurePythonBioEntropyGenerator = None

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


@dataclass
class MiningConfig:
    """Mining configuration"""
    # Network
    network: str = "testnet"  # "testnet" or "mainnet"
    pool_host: str = "testnet-pool.bitcoin.com"
    pool_port: int = 3333
    worker_name: str = ""
    password: str = "x"
    
    # Mining parameters
    threads: int = 1
    scan_depth: int = 1000000  # Nonces to try per job
    use_bio_entropy: bool = True
    
    # Bio-entropy parameters
    bio_entropy_weight: float = 0.3  # 30% bio influence
    neural_predictions: int = 5  # Number of neural predictions per job


class BitcoinMiner:
    """
    Real Bitcoin miner with bio-entropy enhancement
    
    Features:
    - Full SHA-256d Bitcoin block hashing
    - Stratum protocol support
    - Testnet and Mainnet compatible
    - Bio-entropy seed generation
    - Neural network nonce prediction
    - Multi-threaded mining (future)
    """
    
    def __init__(self, config: MiningConfig):
        self.config = config
        self.running = False
        
        # Stratum client
        self.stratum: Optional[StratumClient] = None
        
        # Bio-entropy generator (lazy import to avoid circular dependency)
        self.bio_entropy = None
        if config.use_bio_entropy:
            try:
                # Lazy import to avoid circular dependency with server.py
                from web.api.server import PurePythonBioEntropyGenerator
                self.bio_entropy = PurePythonBioEntropyGenerator()
                logger.info("✅ Bio-entropy generator initialized")
            except ImportError as e:
                logger.warning(f"⚠️ Bio-entropy module not available: {e}")
            except Exception as e:
                logger.warning(f"⚠️ Bio-entropy init failed: {e}")
        
        # Statistics
        self.stats = {
            "start_time": 0,
            "hashes_computed": 0,
            "shares_found": 0,
            "blocks_found": 0,
            "jobs_processed": 0,
            "current_hashrate": 0.0,
            "bio_entropy_used": 0,
            "neural_predictions_used": 0
        }
        
        logger.info(f"⛏️ BitcoinMiner initialized")
        logger.info(f"   Network: {config.network}")
        logger.info(f"   Pool: {config.pool_host}:{config.pool_port}")
        logger.info(f"   Bio-Entropy: {config.use_bio_entropy and self.bio_entropy is not None}")
    
    async def start(self):
        """Start mining"""
        try:
            logger.info("🚀 Starting Bitcoin mining...")
            self.running = True
            self.stats["start_time"] = time.time()
            
            # Initialize Stratum client
            self.stratum = StratumClient(
                host=self.config.pool_host,
                port=self.config.pool_port,
                worker_name=self.config.worker_name,
                password=self.config.password
            )
            
            # Set up callbacks
            self.stratum.on_job = self._on_new_job
            self.stratum.on_difficulty = self._on_difficulty_change
            
            # Connect to pool
            if not await self.stratum.connect():
                logger.error("❌ Failed to connect to pool")
                return
            
            logger.info("✅ Connected to pool, waiting for jobs...")
            
            # Keep running
            while self.running:
                await asyncio.sleep(1)
                self._update_hashrate()
            
        except KeyboardInterrupt:
            logger.info("\n⚠️ Mining interrupted by user")
        except Exception as e:
            logger.error(f"❌ Mining error: {e}", exc_info=True)
        finally:
            await self.stop()
    
    async def stop(self):
        """Stop mining"""
        logger.info("🛑 Stopping mining...")
        self.running = False
        
        if self.stratum:
            await self.stratum.disconnect()
        
        # Print final statistics
        self._print_statistics()
        logger.info("✅ Mining stopped")
    
    def _on_new_job(self, job: StratumJob):
        """Handle new mining job from pool"""
        logger.info(f"📦 New Job Received: {job}")
        self.stats["jobs_processed"] += 1
        
        # Start mining this job
        asyncio.create_task(self._mine_job(job))
    
    def _on_difficulty_change(self, difficulty: float):
        """Handle difficulty change"""
        logger.info(f"🎯 Difficulty changed to: {difficulty}")
    
    async def _mine_job(self, job: StratumJob):
        """
        Mine a job from the pool
        
        Process:
        1. Build block header from job parameters
        2. Generate starting nonces using bio-entropy (if enabled)
        3. Hash block header with different nonces
        4. Check if hash meets difficulty target
        5. Submit share if valid
        """
        try:
            logger.info(f"⛏️ Mining job {job.job_id[:8]}...")
            
            # Calculate target from difficulty
            target = self._difficulty_to_target(job.difficulty)
            target_int = int(target, 16)
            
            logger.info(f"   🎯 Target difficulty: {job.difficulty}")
            logger.info(f"   🎯 Target hash: {target[:16]}...")
            
            # Generate starting nonces
            starting_nonces = self._generate_starting_nonces(job)
            
            # Try each starting point
            for start_nonce in starting_nonces:
                if not self.running:
                    break
                
                # Mine from this starting point
                result = await self._mine_nonce_range(
                    job, start_nonce, self.config.scan_depth, target_int
                )
                
                if result:
                    nonce, hash_result = result
                    logger.info(f"✨ Valid share found!")
                    logger.info(f"   Nonce: {nonce:#010x}")
                    logger.info(f"   Hash: {hash_result}")
                    
                    # Submit share
                    await self._submit_share(job, nonce)
                    
                    # Check if block (very rare!)
                    if self._is_block(hash_result, job.nbits):
                        logger.info("🎉🎉🎉 BLOCK FOUND! 🎉🎉🎉")
                        self.stats["blocks_found"] += 1
        
        except asyncio.CancelledError:
            logger.debug(f"Mining job {job.job_id[:8]} cancelled")
        except Exception as e:
            logger.error(f"❌ Error mining job: {e}", exc_info=True)
    
    def _generate_starting_nonces(self, job: StratumJob) -> List[int]:
        """
        Generate starting nonces for mining
        
        Strategy:
        1. Random baseline nonces
        2. Bio-entropy enhanced nonces (if enabled)
        3. Neural network predictions (if available)
        """
        nonces = []
        
        # Baseline: Random starting points
        import random
        for _ in range(10):
            nonces.append(random.randint(0, 0xFFFFFFFF))
        
        # Bio-entropy enhanced nonces
        if self.bio_entropy and self.config.use_bio_entropy:
            try:
                # Create block data for prediction
                block_data = f"{job.version}|{job.prevhash}|{job.coinb1[:32]}|{job.ntime}|{job.nbits}|00000000"
                
                # Get neural prediction
                prediction = self.bio_entropy.extract_features(block_data, int(job.nbits, 16))
                
                # Generate bio-entropy nonces
                for _ in range(self.config.neural_predictions):
                    # Use prediction features to generate nonce
                    bio_nonce = self._prediction_to_nonce(prediction)
                    nonces.append(bio_nonce)
                    self.stats["bio_entropy_used"] += 1
                
                logger.debug(f"🧬 Generated {self.config.neural_predictions} bio-entropy nonces")
                
            except Exception as e:
                logger.debug(f"⚠️ Bio-entropy generation failed: {e}")
        
        return nonces
    
    def _prediction_to_nonce(self, prediction: Dict[str, Any]) -> int:
        """Convert neural prediction to nonce value"""
        try:
            # Use prediction features to generate nonce
            features = prediction.get("features", [0.0] * 60)
            
            # Combine first few features to create nonce
            nonce = 0
            for i in range(min(4, len(features))):
                byte_val = int(features[i] * 255) & 0xFF
                nonce |= (byte_val << (i * 8))
            
            return nonce & 0xFFFFFFFF
            
        except:
            import random
            return random.randint(0, 0xFFFFFFFF)
    
    async def _mine_nonce_range(self, job: StratumJob, start_nonce: int, 
                                 count: int, target: int) -> Optional[tuple]:
        """
        Mine a range of nonces
        
        Returns: (nonce, hash_hex) if valid share found, None otherwise
        """
        # Build coinbase transaction
        extranonce2 = "00" * job.extranonce2_size
        coinbase = bytes.fromhex(job.coinb1 + job.extranonce1 + extranonce2 + job.coinb2)
        coinbase_hash = self._double_sha256(coinbase)
        
        # Build merkle root
        merkle_root = coinbase_hash
        for branch in job.merkle_branch:
            merkle_root = self._double_sha256(merkle_root + bytes.fromhex(branch))
        
        # Build block header (without nonce)
        header_prefix = (
            struct.pack("<I", int(job.version, 16)) +
            bytes.fromhex(job.prevhash)[::-1] +
            merkle_root[::-1] +
            struct.pack("<I", int(job.ntime, 16)) +
            struct.pack("<I", int(job.nbits, 16))
        )
        
        # Try nonces
        end_nonce = min(start_nonce + count, 0xFFFFFFFF)
        
        for nonce in range(start_nonce, end_nonce):
            if not self.running:
                break
            
            # Build complete header
            header = header_prefix + struct.pack("<I", nonce)
            
            # Double SHA-256
            hash_result = self._double_sha256(header)
            hash_int = int.from_bytes(hash_result[::-1], 'big')
            
            self.stats["hashes_computed"] += 1
            
            # Check if valid share
            if hash_int < target:
                hash_hex = hash_result[::-1].hex()
                return (nonce, hash_hex)
            
            # Yield control periodically
            if nonce % 10000 == 0:
                await asyncio.sleep(0)
        
        return None
    
    async def _submit_share(self, job: StratumJob, nonce: int):
        """Submit found share to pool"""
        try:
            extranonce2 = "00" * job.extranonce2_size
            nonce_hex = f"{nonce:08x}"
            
            success = await self.stratum.submit_share(
                job_id=job.job_id,
                extranonce2=extranonce2,
                ntime=job.ntime,
                nonce=nonce_hex
            )
            
            if success:
                self.stats["shares_found"] += 1
                logger.info(f"✅ Share #{self.stats['shares_found']} accepted!")
            
        except Exception as e:
            logger.error(f"❌ Error submitting share: {e}")
    
    def _double_sha256(self, data: bytes) -> bytes:
        """Compute double SHA-256 (Bitcoin standard)"""
        return hashlib.sha256(hashlib.sha256(data).digest()).digest()
    
    def _difficulty_to_target(self, difficulty: float) -> str:
        """Convert difficulty to target hash"""
        # Max target (difficulty 1)
        max_target = 0x00000000FFFF0000000000000000000000000000000000000000000000000000
        
        # Calculate target for this difficulty
        target = int(max_target / difficulty)
        
        # Convert to 64-char hex string
        return f"{target:064x}"
    
    def _is_block(self, hash_hex: str, nbits: str) -> bool:
        """Check if hash is valid block (not just share)"""
        try:
            # Get target from nbits (compact format)
            nbits_int = int(nbits, 16)
            exponent = nbits_int >> 24
            coefficient = nbits_int & 0xFFFFFF
            
            target = coefficient * (2 ** (8 * (exponent - 3)))
            hash_int = int(hash_hex, 16)
            
            return hash_int < target
        except:
            return False
    
    def _update_hashrate(self):
        """Update current hashrate"""
        elapsed = time.time() - self.stats["start_time"]
        if elapsed > 0:
            self.stats["current_hashrate"] = self.stats["hashes_computed"] / elapsed
    
    def _print_statistics(self):
        """Print mining statistics"""
        elapsed = time.time() - self.stats["start_time"]
        hours = elapsed / 3600
        
        logger.info("\n" + "="*60)
        logger.info("📊 MINING STATISTICS")
        logger.info("="*60)
        logger.info(f"⏱️  Mining time: {hours:.2f} hours")
        logger.info(f"🔢 Total hashes: {self.stats['hashes_computed']:,}")
        logger.info(f"⚡ Average hashrate: {self.stats['current_hashrate']:.2f} H/s")
        logger.info(f"📦 Jobs processed: {self.stats['jobs_processed']}")
        logger.info(f"✨ Shares found: {self.stats['shares_found']}")
        logger.info(f"🎉 Blocks found: {self.stats['blocks_found']}")
        
        if self.bio_entropy:
            logger.info(f"🧬 Bio-entropy nonces: {self.stats['bio_entropy_used']}")
        
        if self.stratum:
            stratum_stats = self.stratum.get_statistics()
            logger.info(f"📤 Shares submitted: {stratum_stats['shares_submitted']}")
            logger.info(f"✅ Shares accepted: {stratum_stats['shares_accepted']}")
            logger.info(f"❌ Shares rejected: {stratum_stats['shares_rejected']}")
            logger.info(f"📈 Acceptance rate: {stratum_stats['acceptance_rate']:.1f}%")
        
        logger.info("="*60 + "\n")


# Predefined pool configurations
POOL_CONFIGS = {
    "testnet": {
        "bitcoin.com": {
            "host": "testnet-pool.bitcoin.com",
            "port": 3333,
            "name": "Bitcoin.com Testnet"
        },
        "solo": {
            "host": "solo.ckpool.org",
            "port": 13333,
            "name": "CKPool Solo Testnet"
        }
    },
    "mainnet": {
        "slushpool": {
            "host": "stratum.slushpool.com",
            "port": 3333,
            "name": "Slush Pool"
        },
        "f2pool": {
            "host": "stratum.f2pool.com",
            "port": 3333,
            "name": "F2Pool"
        },
        "antpool": {
            "host": "stratum.antpool.com",
            "port": 3333,
            "name": "AntPool"
        }
    }
}


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="🧬⛏️ Real Bitcoin Miner with Bio-Entropy Enhancement",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Mine on testnet with Bitcoin.com pool
  python real_bitcoin_miner.py --network testnet --pool bitcoin.com --worker youraddress.worker1
  
  # Mine on mainnet with Slush Pool
  python real_bitcoin_miner.py --network mainnet --pool slushpool --worker username.worker1
  
  # Custom pool configuration
  python real_bitcoin_miner.py --host custom-pool.com --port 3333 --worker myworker
  
  # Disable bio-entropy (standard mining)
  python real_bitcoin_miner.py --network testnet --worker myworker --no-bio-entropy
        """
    )
    
    # Network selection
    parser.add_argument("--network", choices=["testnet", "mainnet"], 
                       default="testnet", help="Bitcoin network (default: testnet)")
    
    # Pool selection
    parser.add_argument("--pool", help="Pool name (bitcoin.com, solo, slushpool, f2pool, etc.)")
    parser.add_argument("--host", help="Custom pool host")
    parser.add_argument("--port", type=int, help="Custom pool port")
    
    # Worker credentials
    parser.add_argument("--worker", required=True, 
                       help="Worker name (format: username.workername or wallet_address.workername)")
    parser.add_argument("--password", default="x", help="Worker password (default: x)")
    
    # Mining parameters
    parser.add_argument("--threads", type=int, default=1, help="Number of mining threads (default: 1)")
    parser.add_argument("--scan-depth", type=int, default=1000000, 
                       help="Nonces to scan per job (default: 1,000,000)")
    
    # Bio-entropy options
    parser.add_argument("--no-bio-entropy", action="store_true", 
                       help="Disable bio-entropy enhancement")
    parser.add_argument("--bio-weight", type=float, default=0.3, 
                       help="Bio-entropy weight 0-1 (default: 0.3)")
    parser.add_argument("--neural-predictions", type=int, default=5,
                       help="Neural predictions per job (default: 5)")
    
    # Configuration file
    parser.add_argument("--config", help="Load configuration from JSON file")
    
    args = parser.parse_args()
    
    # Load from config file if specified
    if args.config:
        try:
            with open(args.config) as f:
                config_data = json.load(f)
            logger.info(f"✅ Loaded configuration from {args.config}")
        except Exception as e:
            logger.error(f"❌ Failed to load config file: {e}")
            return
    else:
        config_data = {}
    
    # Build configuration
    config = MiningConfig()
    config.network = args.network
    config.worker_name = args.worker
    config.password = args.password
    config.threads = args.threads
    config.scan_depth = args.scan_depth
    config.use_bio_entropy = not args.no_bio_entropy
    config.bio_entropy_weight = args.bio_weight
    config.neural_predictions = args.neural_predictions
    
    # Determine pool settings
    if args.host and args.port:
        # Custom pool
        config.pool_host = args.host
        config.pool_port = args.port
        logger.info(f"Using custom pool: {args.host}:{args.port}")
    elif args.pool:
        # Predefined pool
        if args.pool in POOL_CONFIGS.get(args.network, {}):
            pool_config = POOL_CONFIGS[args.network][args.pool]
            config.pool_host = pool_config["host"]
            config.pool_port = pool_config["port"]
            logger.info(f"Using {pool_config['name']}")
        else:
            logger.error(f"❌ Unknown pool: {args.pool}")
            logger.info(f"Available pools for {args.network}: {list(POOL_CONFIGS.get(args.network, {}).keys())}")
            return
    else:
        # Default to first testnet pool
        if args.network == "testnet":
            default_pool = POOL_CONFIGS["testnet"]["bitcoin.com"]
            config.pool_host = default_pool["host"]
            config.pool_port = default_pool["port"]
            logger.info(f"Using default pool: {default_pool['name']}")
        else:
            logger.error("❌ For mainnet, you must specify --pool or --host/--port")
            return
    
    # Print banner
    print("\n" + "="*60)
    print("⛏️🧬 BIOMINING - Real Bitcoin Miner with Bio-Entropy")
    print("="*60)
    print(f"Network: {config.network.upper()}")
    print(f"Pool: {config.pool_host}:{config.pool_port}")
    print(f"Worker: {config.worker_name}")
    print(f"Bio-Entropy: {'ENABLED' if config.use_bio_entropy else 'DISABLED'}")
    print(f"Threads: {config.threads}")
    print("="*60 + "\n")
    
    # Create and start miner
    miner = BitcoinMiner(config)
    
    try:
        asyncio.run(miner.start())
    except KeyboardInterrupt:
        print("\n\n⚠️ Mining stopped by user")


if __name__ == "__main__":
    main()
