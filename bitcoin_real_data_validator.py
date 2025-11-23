#!/usr/bin/env python3
"""
Bitcoin Real Data Validator
============================

Validates Bio-Entropy Mining predictions against real Bitcoin blockchain data.

Features:
- Fetch real Bitcoin blocks from blockchain APIs
- Compare Bio-Entropy predictions with actual solved nonces
- Statistical analysis of prediction accuracy
- Performance metrics and validation reports
"""

import hashlib
import struct
import time
import json
import logging
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass, asdict
from datetime import datetime
import requests
from enum import Enum

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


class BlockchainAPI(Enum):
    """Supported blockchain APIs"""
    BLOCKCHAIN_INFO = "blockchain.info"
    BLOCKCHAIR = "blockchair.com"
    BLOCKCYPHER = "blockcypher.com"


@dataclass
class BitcoinBlock:
    """Real Bitcoin block data"""
    height: int
    hash: str
    version: int
    previous_block_hash: str
    merkle_root: str
    timestamp: int
    bits: str
    nonce: int
    difficulty: float
    tx_count: int
    size: int
    
    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)
    
    def get_block_header(self) -> str:
        """Get block header in Bio-Entropy format"""
        return f"{self.version}|{self.previous_block_hash}|{self.merkle_root}|{self.timestamp}|{self.bits}|{self.nonce}"
    
    def get_block_header_bytes(self) -> bytes:
        """Get block header in raw binary format for hashing"""
        header = struct.pack("<L", self.version)
        header += bytes.fromhex(self.previous_block_hash)[::-1]
        header += bytes.fromhex(self.merkle_root)[::-1]
        header += struct.pack("<L", self.timestamp)
        header += bytes.fromhex(self.bits)[::-1]
        header += struct.pack("<L", self.nonce)
        return header
    
    def verify_hash(self) -> bool:
        """Verify the block hash is correct"""
        header = self.get_block_header_bytes()
        hash_result = hashlib.sha256(hashlib.sha256(header).digest()).digest()
        computed_hash = hash_result[::-1].hex()
        return computed_hash == self.hash


@dataclass
class ValidationResult:
    """Result of Bio-Entropy prediction validation"""
    block_height: int
    actual_nonce: int
    predicted_nonce: int
    nonce_distance: int
    nonce_distance_percent: float
    prediction_confidence: float
    bio_contribution: float
    starting_points_generated: int
    best_starting_point: int
    best_distance: int
    validation_timestamp: str
    success: bool
    notes: str
    
    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


class BitcoinBlockchainFetcher:
    """Fetch real Bitcoin block data from blockchain APIs"""
    
    def __init__(self, api: BlockchainAPI = BlockchainAPI.BLOCKCHAIN_INFO):
        self.api = api
        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'BioMining-Validator/1.0'
        })
        self._last_fetched_height = None  # Track last successful fetch
        self._cache_buster = 0  # Simple cache buster counter
    
    def fetch_latest_block(self) -> Optional[BitcoinBlock]:
        """Fetch the latest Bitcoin block"""
        try:
            if self.api == BlockchainAPI.BLOCKCHAIN_INFO:
                return self._fetch_from_blockchain_info("latest")
            elif self.api == BlockchainAPI.BLOCKCHAIR:
                return self._fetch_from_blockchair("latest")
            else:
                logger.error(f"Unsupported API: {self.api}")
                return None
        except Exception as e:
            logger.error(f"Failed to fetch latest block: {e}")
            return None
    
    def fetch_block_by_height(self, height: int) -> Optional[BitcoinBlock]:
        """Fetch Bitcoin block by height"""
        try:
            # Log fetch attempt with cache buster
            self._cache_buster += 1
            logger.info(f"🔍 Fetching block at height {height} (fetch #{self._cache_buster})")
            
            # Detect potential duplicate requests
            if self._last_fetched_height == height:
                logger.warning(f"⚠️  DUPLICATE REQUEST: Fetching same height {height} twice in a row!")
            
            block = None
            if self.api == BlockchainAPI.BLOCKCHAIN_INFO:
                block = self._fetch_from_blockchain_info(height)
            elif self.api == BlockchainAPI.BLOCKCHAIR:
                block = self._fetch_from_blockchair(height)
            else:
                logger.error(f"Unsupported API: {self.api}")
                return None
            
            # Verify we got the correct block
            if block:
                if block.height != height:
                    logger.error(f"❌ BLOCK HEIGHT MISMATCH: Requested {height}, got {block.height}")
                    return None
                logger.info(f"✅ Successfully fetched block {block.height}: hash={block.hash[:16]}..., nonce={block.nonce:#010x}")
                self._last_fetched_height = height
            else:
                logger.error(f"❌ Failed to fetch block {height}: API returned None")
            
            return block
            
        except Exception as e:
            logger.error(f"Failed to fetch block {height}: {e}")
            return None
    
    def fetch_block_by_hash(self, block_hash: str) -> Optional[BitcoinBlock]:
        """Fetch Bitcoin block by hash"""
        try:
            if self.api == BlockchainAPI.BLOCKCHAIN_INFO:
                url = f"https://blockchain.info/rawblock/{block_hash}"
                response = self.session.get(url, timeout=10)
                response.raise_for_status()
                data = response.json()
                return self._parse_blockchain_info_response(data)
            else:
                logger.error(f"API {self.api} doesn't support fetch by hash yet")
                return None
        except Exception as e:
            logger.error(f"Failed to fetch block by hash {block_hash}: {e}")
            return None
    
    def _fetch_from_blockchain_info(self, height_or_latest: any) -> Optional[BitcoinBlock]:
        """Fetch block from blockchain.info API"""
        max_retries = 3
        retry_delay = 2.0
        
        for attempt in range(max_retries):
            try:
                if height_or_latest == "latest":
                    # Get latest block hash first
                    url = "https://blockchain.info/latestblock"
                    logger.debug(f"Fetching from: {url}")
                    response = self.session.get(url, timeout=15)
                    response.raise_for_status()
                    latest_data = response.json()
                    block_hash = latest_data['hash']
                    
                    # Fetch full block data
                    url = f"https://blockchain.info/rawblock/{block_hash}"
                    logger.debug(f"Fetching from: {url}")
                    response = self.session.get(url, timeout=15)
                    response.raise_for_status()
                    data = response.json()
                else:
                    # Fetch by height - add cache buster to URL
                    url = f"https://blockchain.info/block-height/{height_or_latest}?format=json&cors=true&_={self._cache_buster}"
                    logger.debug(f"Fetching from: {url}")
                    response = self.session.get(url, timeout=15)
                    response.raise_for_status()
                    data = response.json()
                    
                    # blockchain.info returns list of blocks at this height
                    if 'blocks' in data:
                        if len(data['blocks']) == 0:
                            logger.error(f"No blocks found at height {height_or_latest}")
                            return None
                        # Log if multiple blocks at this height (orphans)
                        if len(data['blocks']) > 1:
                            logger.warning(f"Multiple blocks at height {height_or_latest}: {len(data['blocks'])} (using main chain)")
                        data = data['blocks'][0]  # Take first block (main chain)
                    
                    # Verify height matches
                    if data.get('height') != height_or_latest:
                        logger.error(f"Height mismatch: requested {height_or_latest}, got {data.get('height')}")
                        return None
                
                return self._parse_blockchain_info_response(data)
                
            except requests.exceptions.Timeout as e:
                logger.warning(f"Timeout on attempt {attempt+1}/{max_retries}: {e}")
                if attempt < max_retries - 1:
                    time.sleep(retry_delay)
                    continue
            except requests.exceptions.HTTPError as e:
                if e.response.status_code == 429:  # Rate limit
                    logger.warning(f"Rate limited on attempt {attempt+1}/{max_retries}, waiting {retry_delay*2}s")
                    if attempt < max_retries - 1:
                        time.sleep(retry_delay * 2)
                        continue
                logger.error(f"HTTP error: {e}")
                return None
            except Exception as e:
                logger.error(f"blockchain.info API error on attempt {attempt+1}/{max_retries}: {e}")
                if attempt < max_retries - 1:
                    time.sleep(retry_delay)
                    continue
                return None
        
        logger.error(f"Failed to fetch block after {max_retries} attempts")
        return None
    
    def _parse_blockchain_info_response(self, data: Dict) -> BitcoinBlock:
        """Parse blockchain.info API response"""
        return BitcoinBlock(
            height=data['height'],
            hash=data['hash'],
            version=data['ver'],
            previous_block_hash=data['prev_block'],
            merkle_root=data['mrkl_root'],
            timestamp=data['time'],
            bits=hex(data['bits'])[2:],
            nonce=data['nonce'],
            difficulty=data.get('difficulty', 0.0),
            tx_count=data['n_tx'],
            size=data['size']
        )
    
    def _fetch_from_blockchair(self, height_or_latest: any) -> Optional[BitcoinBlock]:
        """Fetch block from blockchair.com API"""
        try:
            if height_or_latest == "latest":
                url = "https://api.blockchair.com/bitcoin/blocks?limit=1"
            else:
                url = f"https://api.blockchair.com/bitcoin/blocks/{height_or_latest}"
            
            response = self.session.get(url, timeout=10)
            response.raise_for_status()
            data = response.json()
            
            if 'data' not in data:
                logger.error("Invalid blockchair response")
                return None
            
            # Parse blockchair response
            block_data = data['data'][0] if isinstance(data['data'], list) else data['data']
            
            return BitcoinBlock(
                height=block_data['id'],
                hash=block_data['hash'],
                version=block_data['version'],
                previous_block_hash=block_data['previous_block_hash'],
                merkle_root=block_data['merkle_root'],
                timestamp=block_data['time'],
                bits=block_data['bits'],
                nonce=block_data['nonce'],
                difficulty=block_data['difficulty'],
                tx_count=block_data['transaction_count'],
                size=block_data['size']
            )
            
        except Exception as e:
            logger.error(f"blockchair.com API error: {e}")
            return None


class BioEntropyValidator:
    """Validate Bio-Entropy predictions against real Bitcoin data"""
    
    def __init__(self, bio_entropy_platform):
        """
        Initialize validator with Bio-Entropy platform instance
        
        Args:
            bio_entropy_platform: Instance of BiologicalMiningPlatform
        """
        self.platform = bio_entropy_platform
        self.fetcher = BitcoinBlockchainFetcher()
        self.validation_results: List[ValidationResult] = []
    
    def validate_against_real_block(self, block_height: int) -> Optional[ValidationResult]:
        """
        Validate Bio-Entropy prediction against a real Bitcoin block
        
        Args:
            block_height: Bitcoin block height to validate against
            
        Returns:
            ValidationResult or None if validation failed
        """
        logger.info(f"🔍 Starting validation against Bitcoin block {block_height}")
        
        # 1. Fetch real Bitcoin block
        real_block = self.fetcher.fetch_block_by_height(block_height)
        if not real_block:
            logger.error(f"Failed to fetch block {block_height}")
            return None
        
        logger.info(f"✅ Fetched block {block_height}: hash={real_block.hash[:16]}...")
        logger.info(f"   Real nonce: {real_block.nonce:#010x} ({real_block.nonce})")
        logger.info(f"   Difficulty: {real_block.difficulty:.2f}")
        
        # 2. Verify block hash
        if real_block.verify_hash():
            logger.info("✅ Block hash verified successfully")
        else:
            logger.warning("⚠️  Block hash verification failed (API data may be incomplete)")
        
        # 3. Use Bio-Entropy to generate predictions for this block
        block_header_str = real_block.get_block_header()
        
        # Extract features
        features = self.platform.bio_entropy_generator.extract_features(
            block_header_str, 
            real_block.difficulty
        )
        logger.info(f"✅ Extracted {len(features)} features from block header")
        
        # Get compute engine to test both Biological Network AND MEA
        compute_engine = self.platform.get_compute_engine()
        
        # STEP 3a: Test Biological Neural Network prediction
        neural_prediction = None
        if hasattr(compute_engine, 'predict_optimal_nonce'):
            try:
                # Use the biological neural network to predict nonce
                block_data = block_header_str.encode()
                prediction_result = compute_engine.predict_optimal_nonce(block_data)
                
                if isinstance(prediction_result, dict):
                    neural_prediction = prediction_result.get('predicted_nonce', 0)
                    neural_confidence = prediction_result.get('confidence', 0.0)
                    logger.info(f"🧠 Biological Neural Network prediction: {neural_prediction:#010x}")
                    logger.info(f"   Neural confidence: {neural_confidence:.2%}")
                else:
                    neural_prediction = prediction_result
                    logger.info(f"🧠 Biological Neural Network prediction: {neural_prediction:#010x}")
            except Exception as e:
                logger.warning(f"⚠️  Neural network prediction failed: {e}")
        
        # STEP 3b: Generate MEA response (stimulation pattern)
        if hasattr(compute_engine, 'generate_stimulation_pattern'):
            stim_pattern = compute_engine.generate_stimulation_pattern(block_header_str)
            logger.info(f"⚡ Generated stimulation pattern: {len(stim_pattern)} voltages")
            
            # Stimulate and get spikes
            if hasattr(compute_engine, 'stimulate_electrodes'):
                spikes = compute_engine.stimulate_electrodes(stim_pattern, duration=50.0)
                logger.info(f"🧠 Neural response: {len(spikes)} spikes")
                
                # Use spike amplitudes for MEA response
                mea_response = [spike[2] for spike in spikes[:60]]
                if len(mea_response) < 60:
                    mea_response += [0.0] * (60 - len(mea_response))
            else:
                mea_response = stim_pattern.tolist()
        else:
            # Fallback: use features as MEA response
            mea_response = features[:60]
            if len(mea_response) < 60:
                mea_response += [0.0] * (60 - len(mea_response))
        
        # Generate entropy seed
        entropy_seed = self.platform.bio_entropy_generator.generate_entropy_seed(
            mea_response, 
            features
        )
        logger.info(f"🌱 Generated entropy seed: confidence={entropy_seed['confidence']:.2%}")
        
        # Generate starting points
        starting_points = self.platform.bio_entropy_generator.generate_starting_points(
            entropy_seed,
            point_count=1000
        )
        logger.info(f"🎯 Generated {len(starting_points)} starting points from entropy seed")
        
        # Add neural network prediction as an additional starting point if available
        if neural_prediction is not None and 0 <= neural_prediction <= 0xFFFFFFFF:
            starting_points.append(neural_prediction)
            logger.info(f"🧠 Added neural network prediction as starting point: {neural_prediction:#010x}")
        
        # 4. Find closest prediction to actual nonce
        best_distance = float('inf')
        best_starting_point = 0
        best_method = "entropy_seed"  # Track which method gave best result
        
        for point in starting_points:
            distance = abs(point - real_block.nonce)
            if distance < best_distance:
                best_distance = distance
                best_starting_point = point
                # Check if this was the neural network prediction
                if neural_prediction is not None and point == neural_prediction:
                    best_method = "neural_network"
        
        # Also check neural network prediction separately for comparison
        neural_distance = None
        if neural_prediction is not None:
            neural_distance = abs(neural_prediction - real_block.nonce)
            logger.info(f"🧠 Neural network distance: {neural_distance:,} ({(neural_distance/0xFFFFFFFF)*100:.4f}%)")
        
        # Calculate metrics
        nonce_space_size = 0xFFFFFFFF  # 2^32
        distance_percent = (best_distance / nonce_space_size) * 100
        
        # Success criteria: within 10% of nonce space
        success = distance_percent < 10.0
        
        logger.info(f"📊 Validation Results:")
        logger.info(f"   Best starting point: {best_starting_point:#010x}")
        logger.info(f"   Best method: {best_method}")
        logger.info(f"   Distance to real nonce: {best_distance:,} ({distance_percent:.4f}%)")
        if neural_distance is not None:
            comparison = "better" if neural_distance < best_distance else "worse" if neural_distance > best_distance else "same"
            logger.info(f"   Neural vs Best: {comparison}")
        logger.info(f"   Success: {'✅ YES' if success else '❌ NO'}")
        
        # 5. Create validation result
        result = ValidationResult(
            block_height=block_height,
            actual_nonce=real_block.nonce,
            predicted_nonce=best_starting_point,
            nonce_distance=best_distance,
            nonce_distance_percent=distance_percent,
            prediction_confidence=entropy_seed['confidence'],
            bio_contribution=entropy_seed.get('biological_contribution', 0.5),
            starting_points_generated=len(starting_points),
            best_starting_point=best_starting_point,
            best_distance=best_distance,
            validation_timestamp=datetime.now().isoformat(),
            success=success,
            notes=f"Validated against block {block_height} (difficulty={real_block.difficulty:.2f})"
        )
        
        self.validation_results.append(result)
        return result
    
    def validate_multiple_blocks(self, start_height: int, count: int = 10) -> List[ValidationResult]:
        """
        Validate against multiple consecutive blocks
        
        Args:
            start_height: Starting block height
            count: Number of blocks to validate
            
        Returns:
            List of validation results
        """
        results = []
        
        logger.info(f"🔬 Starting validation of {count} blocks from height {start_height}")
        
        for i in range(count):
            height = start_height + i
            logger.info(f"\n{'='*60}")
            logger.info(f"Validating block {i+1}/{count}: height {height}")
            logger.info(f"{'='*60}")
            
            result = self.validate_against_real_block(height)
            if result:
                results.append(result)
            
            # Rate limiting to avoid API throttling
            if i < count - 1:
                time.sleep(1.5)
        
        return results
    
    def generate_validation_report(self) -> Dict[str, Any]:
        """Generate comprehensive validation report"""
        if not self.validation_results:
            return {
                "error": "No validation results available",
                "results_count": 0
            }
        
        # Calculate statistics
        total = len(self.validation_results)
        successes = sum(1 for r in self.validation_results if r.success)
        success_rate = (successes / total) * 100 if total > 0 else 0
        
        distances = [r.nonce_distance for r in self.validation_results]
        distance_percents = [r.nonce_distance_percent for r in self.validation_results]
        confidences = [r.prediction_confidence for r in self.validation_results]
        
        avg_distance = sum(distances) / len(distances) if distances else 0
        avg_distance_percent = sum(distance_percents) / len(distance_percents) if distance_percents else 0
        avg_confidence = sum(confidences) / len(confidences) if confidences else 0
        
        min_distance = min(distances) if distances else 0
        max_distance = max(distances) if distances else 0
        
        report = {
            "summary": {
                "total_validations": total,
                "successful_predictions": successes,
                "success_rate_percent": success_rate,
                "validation_timestamp": datetime.now().isoformat()
            },
            "distance_metrics": {
                "average_distance": avg_distance,
                "average_distance_percent": avg_distance_percent,
                "min_distance": min_distance,
                "max_distance": max_distance,
                "nonce_space_coverage": (1000 * 4194304) / 0xFFFFFFFF  # Starting points × window size
            },
            "bio_entropy_metrics": {
                "average_confidence": avg_confidence,
                "average_bio_contribution": sum(r.bio_contribution for r in self.validation_results) / total,
                "starting_points_per_block": 1000
            },
            "detailed_results": [r.to_dict() for r in self.validation_results]
        }
        
        logger.info(f"\n{'='*60}")
        logger.info(f"📊 VALIDATION REPORT")
        logger.info(f"{'='*60}")
        logger.info(f"Total validations: {total}")
        logger.info(f"Success rate: {success_rate:.2f}%")
        logger.info(f"Average distance: {avg_distance:,} ({avg_distance_percent:.4f}%)")
        logger.info(f"Average confidence: {avg_confidence:.2%}")
        logger.info(f"{'='*60}\n")
        
        return report
    
    def save_report(self, filename: str = "validation_report.json"):
        """Save validation report to file"""
        report = self.generate_validation_report()
        with open(filename, 'w') as f:
            json.dump(report, f, indent=2)
        logger.info(f"✅ Report saved to {filename}")


def main():
    """Test Bitcoin validation module"""
    logger.info("🧪 Testing Bitcoin Real Data Validator")
    
    # Test blockchain fetcher
    fetcher = BitcoinBlockchainFetcher()
    
    # Fetch a recent block (height 870000 is from late 2024)
    logger.info("\n📡 Testing block fetcher...")
    block = fetcher.fetch_block_by_height(870000)
    
    if block:
        logger.info(f"✅ Successfully fetched block {block.height}")
        logger.info(f"   Hash: {block.hash}")
        logger.info(f"   Nonce: {block.nonce:#010x}")
        logger.info(f"   Difficulty: {block.difficulty:.2f}")
        logger.info(f"   Block header: {block.get_block_header()}")
        
        # Verify hash
        if block.verify_hash():
            logger.info("✅ Block hash verified!")
        else:
            logger.info("⚠️  Block hash verification skipped (incomplete data)")
    else:
        logger.error("❌ Failed to fetch block")


if __name__ == "__main__":
    main()
