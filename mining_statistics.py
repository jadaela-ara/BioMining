#!/usr/bin/env python3
"""
================================================================
📊 MINING STATISTICS TRACKER - Performance Monitoring
Real-time statistics, historical tracking, and performance analysis
================================================================
"""

import time
import json
import logging
from pathlib import Path
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, field, asdict
from datetime import datetime, timedelta
from collections import deque

logger = logging.getLogger(__name__)


@dataclass
class MiningSession:
    """Represents a single mining session"""
    session_id: str
    start_time: float
    end_time: Optional[float] = None
    
    # Mining stats
    total_hashes: int = 0
    shares_found: int = 0
    shares_accepted: int = 0
    shares_rejected: int = 0
    blocks_found: int = 0
    jobs_processed: int = 0
    
    # Bio-entropy stats
    bio_entropy_nonces_used: int = 0
    neural_predictions_made: int = 0
    
    # Pool stats
    pool_name: str = ""
    pool_difficulty: float = 1.0
    
    # Performance
    average_hashrate: float = 0.0
    peak_hashrate: float = 0.0
    
    def duration_seconds(self) -> float:
        """Get session duration in seconds"""
        end = self.end_time or time.time()
        return end - self.start_time
    
    def acceptance_rate(self) -> float:
        """Calculate share acceptance rate"""
        total = self.shares_accepted + self.shares_rejected
        if total == 0:
            return 0.0
        return (self.shares_accepted / total) * 100
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            **asdict(self),
            "duration_seconds": self.duration_seconds(),
            "acceptance_rate": self.acceptance_rate(),
            "start_time_iso": datetime.fromtimestamp(self.start_time).isoformat(),
            "end_time_iso": datetime.fromtimestamp(self.end_time).isoformat() if self.end_time else None
        }


@dataclass
class HashRateSnapshot:
    """Single hashrate measurement"""
    timestamp: float
    hashrate: float  # H/s
    hashes_in_period: int
    period_seconds: float


class MiningStatistics:
    """
    Comprehensive mining statistics tracker
    
    Features:
    - Real-time hashrate calculation
    - Session management
    - Historical tracking
    - Performance analysis
    - JSON export/import
    """
    
    def __init__(self, save_path: Optional[str] = None):
        self.save_path = Path(save_path) if save_path else Path("mining_stats.json")
        
        # Current session
        self.current_session: Optional[MiningSession] = None
        
        # Session history
        self.session_history: List[MiningSession] = []
        self.max_history = 100  # Keep last 100 sessions
        
        # Real-time tracking
        self.hashrate_window = deque(maxlen=60)  # Last 60 measurements (1 minute)
        self.last_hash_count = 0
        self.last_hash_time = time.time()
        
        # Cumulative statistics
        self.total_hashes_all_time = 0
        self.total_shares_all_time = 0
        self.total_blocks_all_time = 0
        
        # Performance tracking
        self.best_hashrate_ever = 0.0
        self.longest_session_seconds = 0.0
        
        logger.info(f"📊 MiningStatistics initialized (save path: {self.save_path})")
        
        # Try to load previous history
        self._load_history()
    
    def start_session(self, session_id: Optional[str] = None, pool_name: str = "") -> str:
        """Start a new mining session"""
        if self.current_session:
            logger.warning("⚠️ Starting new session without ending previous one")
            self.end_session()
        
        if not session_id:
            session_id = f"session_{int(time.time())}"
        
        self.current_session = MiningSession(
            session_id=session_id,
            start_time=time.time(),
            pool_name=pool_name
        )
        
        self.last_hash_count = 0
        self.last_hash_time = time.time()
        self.hashrate_window.clear()
        
        logger.info(f"🚀 Started mining session: {session_id}")
        return session_id
    
    def end_session(self):
        """End current mining session"""
        if not self.current_session:
            logger.warning("⚠️ No active session to end")
            return
        
        self.current_session.end_time = time.time()
        
        # Add to history
        self.session_history.append(self.current_session)
        
        # Update all-time stats
        self.total_hashes_all_time += self.current_session.total_hashes
        self.total_shares_all_time += self.current_session.shares_found
        self.total_blocks_all_time += self.current_session.blocks_found
        
        # Update records
        if self.current_session.peak_hashrate > self.best_hashrate_ever:
            self.best_hashrate_ever = self.current_session.peak_hashrate
        
        duration = self.current_session.duration_seconds()
        if duration > self.longest_session_seconds:
            self.longest_session_seconds = duration
        
        # Trim history
        if len(self.session_history) > self.max_history:
            self.session_history = self.session_history[-self.max_history:]
        
        logger.info(f"🛑 Ended mining session: {self.current_session.session_id}")
        logger.info(f"   Duration: {duration/3600:.2f} hours")
        logger.info(f"   Total hashes: {self.current_session.total_hashes:,}")
        logger.info(f"   Shares found: {self.current_session.shares_found}")
        logger.info(f"   Acceptance rate: {self.current_session.acceptance_rate():.1f}%")
        
        # Save to disk
        self._save_history()
        
        self.current_session = None
    
    def update_hashes(self, hash_count: int):
        """Update hash count"""
        if not self.current_session:
            return
        
        self.current_session.total_hashes += hash_count
        
        # Calculate instantaneous hashrate
        now = time.time()
        elapsed = now - self.last_hash_time
        
        if elapsed >= 1.0:  # Update every second
            hashrate = hash_count / elapsed
            
            # Add to window
            self.hashrate_window.append(HashRateSnapshot(
                timestamp=now,
                hashrate=hashrate,
                hashes_in_period=hash_count,
                period_seconds=elapsed
            ))
            
            # Update session stats
            self.current_session.average_hashrate = self.get_average_hashrate()
            
            if hashrate > self.current_session.peak_hashrate:
                self.current_session.peak_hashrate = hashrate
            
            # Reset counters
            self.last_hash_count = 0
            self.last_hash_time = now
    
    def add_share_found(self, accepted: bool = True):
        """Record a share found"""
        if not self.current_session:
            return
        
        self.current_session.shares_found += 1
        
        if accepted:
            self.current_session.shares_accepted += 1
        else:
            self.current_session.shares_rejected += 1
    
    def add_block_found(self):
        """Record a block found"""
        if not self.current_session:
            return
        
        self.current_session.blocks_found += 1
        logger.info(f"🎉 Block found! Total blocks in session: {self.current_session.blocks_found}")
    
    def add_job_processed(self):
        """Record a job processed"""
        if not self.current_session:
            return
        
        self.current_session.jobs_processed += 1
    
    def add_bio_entropy_usage(self, nonces_used: int = 1, predictions_made: int = 0):
        """Record bio-entropy usage"""
        if not self.current_session:
            return
        
        self.current_session.bio_entropy_nonces_used += nonces_used
        self.current_session.neural_predictions_made += predictions_made
    
    def update_pool_info(self, pool_name: str, difficulty: float):
        """Update current pool information"""
        if not self.current_session:
            return
        
        self.current_session.pool_name = pool_name
        self.current_session.pool_difficulty = difficulty
    
    def get_current_hashrate(self) -> float:
        """Get current hashrate (last measurement)"""
        if not self.hashrate_window:
            return 0.0
        return self.hashrate_window[-1].hashrate
    
    def get_average_hashrate(self, seconds: int = 60) -> float:
        """Get average hashrate over time window"""
        if not self.hashrate_window:
            return 0.0
        
        cutoff_time = time.time() - seconds
        recent_snapshots = [s for s in self.hashrate_window if s.timestamp >= cutoff_time]
        
        if not recent_snapshots:
            return 0.0
        
        total_hashes = sum(s.hashes_in_period for s in recent_snapshots)
        total_time = sum(s.period_seconds for s in recent_snapshots)
        
        if total_time == 0:
            return 0.0
        
        return total_hashes / total_time
    
    def get_current_session_stats(self) -> Dict[str, Any]:
        """Get current session statistics"""
        if not self.current_session:
            return {"error": "No active session"}
        
        return {
            "session_id": self.current_session.session_id,
            "duration_seconds": self.current_session.duration_seconds(),
            "duration_hours": self.current_session.duration_seconds() / 3600,
            "total_hashes": self.current_session.total_hashes,
            "shares_found": self.current_session.shares_found,
            "shares_accepted": self.current_session.shares_accepted,
            "shares_rejected": self.current_session.shares_rejected,
            "blocks_found": self.current_session.blocks_found,
            "jobs_processed": self.current_session.jobs_processed,
            "acceptance_rate": self.current_session.acceptance_rate(),
            "current_hashrate": self.get_current_hashrate(),
            "average_hashrate_1min": self.get_average_hashrate(60),
            "average_hashrate_session": self.current_session.average_hashrate,
            "peak_hashrate": self.current_session.peak_hashrate,
            "bio_entropy_nonces": self.current_session.bio_entropy_nonces_used,
            "neural_predictions": self.current_session.neural_predictions_made,
            "pool_name": self.current_session.pool_name,
            "pool_difficulty": self.current_session.pool_difficulty
        }
    
    def get_all_time_stats(self) -> Dict[str, Any]:
        """Get all-time statistics"""
        return {
            "total_sessions": len(self.session_history),
            "total_hashes": self.total_hashes_all_time,
            "total_shares": self.total_shares_all_time,
            "total_blocks": self.total_blocks_all_time,
            "best_hashrate_ever": self.best_hashrate_ever,
            "longest_session_hours": self.longest_session_seconds / 3600,
            "total_mining_time_hours": sum(s.duration_seconds() for s in self.session_history) / 3600
        }
    
    def get_recent_sessions(self, count: int = 10) -> List[Dict[str, Any]]:
        """Get recent session summaries"""
        recent = self.session_history[-count:]
        return [session.to_dict() for session in reversed(recent)]
    
    def get_performance_summary(self) -> Dict[str, Any]:
        """Get comprehensive performance summary"""
        if not self.session_history:
            return {"error": "No session history"}
        
        # Calculate averages across all sessions
        total_sessions = len(self.session_history)
        avg_duration = sum(s.duration_seconds() for s in self.session_history) / total_sessions
        avg_hashrate = sum(s.average_hashrate for s in self.session_history) / total_sessions
        avg_acceptance = sum(s.acceptance_rate() for s in self.session_history) / total_sessions
        
        return {
            "average_session_duration_hours": avg_duration / 3600,
            "average_hashrate": avg_hashrate,
            "average_acceptance_rate": avg_acceptance,
            "best_hashrate": self.best_hashrate_ever,
            "total_shares_found": sum(s.shares_found for s in self.session_history),
            "total_blocks_found": sum(s.blocks_found for s in self.session_history)
        }
    
    def _save_history(self):
        """Save session history to disk"""
        try:
            data = {
                "sessions": [s.to_dict() for s in self.session_history],
                "all_time_stats": self.get_all_time_stats(),
                "saved_at": datetime.now().isoformat()
            }
            
            with open(self.save_path, 'w') as f:
                json.dump(data, f, indent=2)
            
            logger.info(f"💾 Saved mining statistics to {self.save_path}")
            
        except Exception as e:
            logger.error(f"❌ Failed to save statistics: {e}")
    
    def _load_history(self):
        """Load session history from disk"""
        try:
            if not self.save_path.exists():
                logger.info("ℹ️ No previous statistics file found")
                return
            
            with open(self.save_path, 'r') as f:
                data = json.load(f)
            
            # Load sessions
            for session_data in data.get("sessions", []):
                session = MiningSession(**session_data)
                self.session_history.append(session)
            
            # Load all-time stats
            all_time = data.get("all_time_stats", {})
            self.total_hashes_all_time = all_time.get("total_hashes", 0)
            self.total_shares_all_time = all_time.get("total_shares", 0)
            self.total_blocks_all_time = all_time.get("total_blocks", 0)
            self.best_hashrate_ever = all_time.get("best_hashrate_ever", 0.0)
            self.longest_session_seconds = all_time.get("longest_session_hours", 0.0) * 3600
            
            logger.info(f"✅ Loaded {len(self.session_history)} previous sessions from {self.save_path}")
            
        except Exception as e:
            logger.error(f"❌ Failed to load statistics: {e}")
    
    def print_summary(self):
        """Print formatted summary"""
        if self.current_session:
            stats = self.get_current_session_stats()
            
            print("\n" + "="*60)
            print("📊 CURRENT SESSION STATISTICS")
            print("="*60)
            print(f"⏱️  Duration: {stats['duration_hours']:.2f} hours")
            print(f"🔢 Total hashes: {stats['total_hashes']:,}")
            print(f"⚡ Current hashrate: {stats['current_hashrate']:.2f} H/s")
            print(f"📈 Average hashrate: {stats['average_hashrate_session']:.2f} H/s")
            print(f"🚀 Peak hashrate: {stats['peak_hashrate']:.2f} H/s")
            print(f"✨ Shares found: {stats['shares_found']}")
            print(f"✅ Shares accepted: {stats['shares_accepted']}")
            print(f"❌ Shares rejected: {stats['shares_rejected']}")
            print(f"📈 Acceptance rate: {stats['acceptance_rate']:.1f}%")
            print(f"🎉 Blocks found: {stats['blocks_found']}")
            print(f"📦 Jobs processed: {stats['jobs_processed']}")
            
            if stats['bio_entropy_nonces'] > 0:
                print(f"🧬 Bio-entropy nonces: {stats['bio_entropy_nonces']}")
                print(f"🧠 Neural predictions: {stats['neural_predictions']}")
            
            print("="*60 + "\n")
        
        if self.session_history:
            all_time = self.get_all_time_stats()
            
            print("📊 ALL-TIME STATISTICS")
            print("="*60)
            print(f"Total sessions: {all_time['total_sessions']}")
            print(f"Total hashes: {all_time['total_hashes']:,}")
            print(f"Total shares: {all_time['total_shares']}")
            print(f"Total blocks: {all_time['total_blocks']}")
            print(f"Best hashrate: {all_time['best_hashrate_ever']:.2f} H/s")
            print(f"Longest session: {all_time['longest_session_hours']:.2f} hours")
            print(f"Total mining time: {all_time['total_mining_time_hours']:.2f} hours")
            print("="*60 + "\n")


# Example usage
if __name__ == "__main__":
    import random
    
    logging.basicConfig(level=logging.INFO)
    
    # Create statistics tracker
    stats = MiningStatistics("test_mining_stats.json")
    
    # Start session
    stats.start_session(pool_name="Test Pool")
    stats.update_pool_info("Test Pool", 1.0)
    
    # Simulate mining
    for i in range(60):
        # Simulate hashing
        hashes = random.randint(80, 120)
        stats.update_hashes(hashes)
        
        # Occasionally find shares
        if random.random() < 0.1:
            stats.add_share_found(accepted=random.random() < 0.95)
        
        # Record bio-entropy usage
        if random.random() < 0.3:
            stats.add_bio_entropy_usage(nonces_used=5, predictions_made=1)
        
        time.sleep(1)
    
    # Print summary
    stats.print_summary()
    
    # End session
    stats.end_session()
    
    print(f"\n📁 Statistics saved to: {stats.save_path}")
