#!/usr/bin/env python3
"""
================================================================
🔌 MINING POOL CONNECTOR - Advanced Pool Management
Automatic reconnection, failover, and pool switching
================================================================
"""

import asyncio
import logging
import time
from typing import Dict, List, Optional, Callable, Any
from dataclasses import dataclass, field
from enum import Enum

from stratum_client import StratumClient, StratumJob

logger = logging.getLogger(__name__)


class PoolStatus(Enum):
    """Pool connection status"""
    DISCONNECTED = "disconnected"
    CONNECTING = "connecting"
    CONNECTED = "connected"
    AUTHORIZED = "authorized"
    MINING = "mining"
    ERROR = "error"


@dataclass
class PoolConfig:
    """Configuration for a mining pool"""
    name: str
    host: str
    port: int
    worker_name: str
    password: str = "x"
    priority: int = 0  # Higher priority = preferred pool
    backup: bool = False
    
    def __repr__(self):
        return f"PoolConfig({self.name}, {self.host}:{self.port}, priority={self.priority})"


@dataclass
class PoolStats:
    """Statistics for a pool"""
    pool_name: str
    status: PoolStatus = PoolStatus.DISCONNECTED
    
    # Connection stats
    connect_attempts: int = 0
    successful_connections: int = 0
    last_connect_time: float = 0.0
    uptime: float = 0.0
    
    # Mining stats
    jobs_received: int = 0
    shares_submitted: int = 0
    shares_accepted: int = 0
    shares_rejected: int = 0
    stale_shares: int = 0
    
    # Performance
    latency_ms: float = 0.0
    last_job_time: float = 0.0
    
    def acceptance_rate(self) -> float:
        if self.shares_submitted == 0:
            return 0.0
        return (self.shares_accepted / self.shares_submitted) * 100
    
    def __repr__(self):
        return (f"PoolStats({self.pool_name}, {self.status.value}, "
                f"accepted={self.shares_accepted}/{self.shares_submitted})")


class MiningPoolConnector:
    """
    Advanced mining pool connector with:
    - Automatic reconnection
    - Multiple pool support (primary + backups)
    - Automatic failover
    - Load balancing
    - Statistics tracking
    """
    
    def __init__(self, pools: List[PoolConfig]):
        self.pools = sorted(pools, key=lambda p: p.priority, reverse=True)
        self.current_pool: Optional[PoolConfig] = None
        self.stratum: Optional[StratumClient] = None
        
        # Pool statistics
        self.pool_stats: Dict[str, PoolStats] = {}
        for pool in pools:
            self.pool_stats[pool.name] = PoolStats(pool_name=pool.name)
        
        # Connection management
        self.reconnect_delay = 5.0  # seconds
        self.max_reconnect_attempts = 5
        self.failover_enabled = True
        
        # Callbacks
        self.on_job: Optional[Callable[[StratumJob], None]] = None
        self.on_pool_change: Optional[Callable[[str], None]] = None
        self.on_status_change: Optional[Callable[[str, PoolStatus], None]] = None
        
        # Tasks
        self.connection_task: Optional[asyncio.Task] = None
        self.monitor_task: Optional[asyncio.Task] = None
        
        logger.info(f"🔌 MiningPoolConnector initialized with {len(pools)} pools")
        for pool in self.pools:
            logger.info(f"   📍 {pool}")
    
    async def start(self):
        """Start pool connection management"""
        logger.info("🚀 Starting pool connector...")
        
        # Start connection manager
        self.connection_task = asyncio.create_task(self._connection_manager())
        
        # Start pool monitor
        self.monitor_task = asyncio.create_task(self._pool_monitor())
        
        logger.info("✅ Pool connector started")
    
    async def stop(self):
        """Stop pool connector"""
        logger.info("🛑 Stopping pool connector...")
        
        # Cancel tasks
        if self.connection_task:
            self.connection_task.cancel()
        if self.monitor_task:
            self.monitor_task.cancel()
        
        # Disconnect from current pool
        if self.stratum:
            await self.stratum.disconnect()
        
        logger.info("✅ Pool connector stopped")
    
    async def _connection_manager(self):
        """Manage pool connections with automatic reconnection"""
        try:
            while True:
                # Select best pool
                pool = self._select_best_pool()
                
                if not pool:
                    logger.error("❌ No available pools!")
                    await asyncio.sleep(self.reconnect_delay)
                    continue
                
                # Try to connect
                if await self._connect_to_pool(pool):
                    logger.info(f"✅ Mining on {pool.name}")
                    
                    # Wait until disconnected
                    while self.stratum and self.stratum.conn.connected:
                        await asyncio.sleep(1)
                    
                    logger.warning(f"⚠️ Disconnected from {pool.name}")
                    self._update_pool_status(pool.name, PoolStatus.DISCONNECTED)
                
                # Wait before reconnecting
                logger.info(f"⏱️ Waiting {self.reconnect_delay}s before reconnecting...")
                await asyncio.sleep(self.reconnect_delay)
                
        except asyncio.CancelledError:
            logger.info("🛑 Connection manager stopped")
        except Exception as e:
            logger.error(f"❌ Connection manager error: {e}", exc_info=True)
    
    async def _connect_to_pool(self, pool: PoolConfig) -> bool:
        """Connect to a specific pool"""
        stats = self.pool_stats[pool.name]
        stats.connect_attempts += 1
        
        self._update_pool_status(pool.name, PoolStatus.CONNECTING)
        
        try:
            logger.info(f"🔌 Connecting to {pool.name} ({pool.host}:{pool.port})...")
            
            # Create Stratum client
            self.stratum = StratumClient(
                host=pool.host,
                port=pool.port,
                worker_name=pool.worker_name,
                password=pool.password
            )
            
            # Set up callbacks
            self.stratum.on_job = self._on_job_received
            self.stratum.on_difficulty = self._on_difficulty_change
            self.stratum.on_disconnect = lambda: self._on_pool_disconnect(pool.name)
            
            # Measure connection latency
            start_time = time.time()
            
            # Connect
            if await self.stratum.connect(timeout=10.0):
                latency = (time.time() - start_time) * 1000
                
                self.current_pool = pool
                stats.successful_connections += 1
                stats.last_connect_time = time.time()
                stats.latency_ms = latency
                
                self._update_pool_status(pool.name, PoolStatus.AUTHORIZED)
                
                logger.info(f"✅ Connected to {pool.name}")
                logger.info(f"   ⚡ Latency: {latency:.1f}ms")
                
                if self.on_pool_change:
                    self.on_pool_change(pool.name)
                
                return True
            else:
                logger.error(f"❌ Failed to connect to {pool.name}")
                self._update_pool_status(pool.name, PoolStatus.ERROR)
                return False
                
        except Exception as e:
            logger.error(f"❌ Connection error to {pool.name}: {e}")
            self._update_pool_status(pool.name, PoolStatus.ERROR)
            return False
    
    def _select_best_pool(self) -> Optional[PoolConfig]:
        """Select best pool based on priority and statistics"""
        available_pools = []
        
        for pool in self.pools:
            stats = self.pool_stats[pool.name]
            
            # Skip if too many failed attempts recently
            if stats.connect_attempts >= self.max_reconnect_attempts:
                if time.time() - stats.last_connect_time < 300:  # 5 minute cooldown
                    continue
                else:
                    # Reset after cooldown
                    stats.connect_attempts = 0
            
            available_pools.append(pool)
        
        if not available_pools:
            return None
        
        # Return highest priority pool
        return available_pools[0]
    
    def _on_job_received(self, job: StratumJob):
        """Handle new job from pool"""
        if self.current_pool:
            stats = self.pool_stats[self.current_pool.name]
            stats.jobs_received += 1
            stats.last_job_time = time.time()
            
            self._update_pool_status(self.current_pool.name, PoolStatus.MINING)
        
        # Forward to user callback
        if self.on_job:
            self.on_job(job)
    
    def _on_difficulty_change(self, difficulty: float):
        """Handle difficulty change"""
        logger.info(f"🎯 Pool difficulty changed to {difficulty}")
    
    def _on_pool_disconnect(self, pool_name: str):
        """Handle pool disconnection"""
        logger.warning(f"⚠️ Disconnected from {pool_name}")
        self._update_pool_status(pool_name, PoolStatus.DISCONNECTED)
    
    def _update_pool_status(self, pool_name: str, status: PoolStatus):
        """Update pool status"""
        if pool_name in self.pool_stats:
            self.pool_stats[pool_name].status = status
            
            if self.on_status_change:
                self.on_status_change(pool_name, status)
    
    async def _pool_monitor(self):
        """Monitor pool performance and switch if needed"""
        try:
            while True:
                await asyncio.sleep(60)  # Check every minute
                
                if not self.current_pool:
                    continue
                
                current_stats = self.pool_stats[self.current_pool.name]
                
                # Check if current pool is performing poorly
                if current_stats.shares_submitted > 10:
                    acceptance_rate = current_stats.acceptance_rate()
                    
                    if acceptance_rate < 90.0:
                        logger.warning(f"⚠️ Low acceptance rate on {self.current_pool.name}: {acceptance_rate:.1f}%")
                        
                        # Consider switching to backup pool
                        if self.failover_enabled:
                            logger.info("🔄 Considering pool failover...")
                            # Disconnect to trigger reconnection to different pool
                            if self.stratum:
                                await self.stratum.disconnect()
                
        except asyncio.CancelledError:
            logger.info("🛑 Pool monitor stopped")
        except Exception as e:
            logger.error(f"❌ Pool monitor error: {e}", exc_info=True)
    
    async def submit_share(self, job_id: str, extranonce2: str, ntime: str, nonce: str) -> bool:
        """Submit share to current pool"""
        if not self.stratum or not self.current_pool:
            logger.error("❌ No active pool connection")
            return False
        
        stats = self.pool_stats[self.current_pool.name]
        stats.shares_submitted += 1
        
        success = await self.stratum.submit_share(job_id, extranonce2, ntime, nonce)
        
        if success:
            stats.shares_accepted += 1
        else:
            stats.shares_rejected += 1
        
        return success
    
    def get_current_pool(self) -> Optional[PoolConfig]:
        """Get currently connected pool"""
        return self.current_pool
    
    def get_pool_stats(self, pool_name: Optional[str] = None) -> Dict[str, Any]:
        """Get statistics for a pool or all pools"""
        if pool_name:
            if pool_name in self.pool_stats:
                stats = self.pool_stats[pool_name]
                return {
                    "name": pool_name,
                    "status": stats.status.value,
                    "jobs_received": stats.jobs_received,
                    "shares_submitted": stats.shares_submitted,
                    "shares_accepted": stats.shares_accepted,
                    "shares_rejected": stats.shares_rejected,
                    "acceptance_rate": stats.acceptance_rate(),
                    "latency_ms": stats.latency_ms,
                    "uptime": time.time() - stats.last_connect_time if stats.last_connect_time > 0 else 0
                }
            else:
                return {}
        else:
            # Return all pools
            return {
                name: {
                    "name": name,
                    "status": stats.status.value,
                    "jobs_received": stats.jobs_received,
                    "shares_submitted": stats.shares_submitted,
                    "shares_accepted": stats.shares_accepted,
                    "acceptance_rate": stats.acceptance_rate(),
                    "latency_ms": stats.latency_ms
                }
                for name, stats in self.pool_stats.items()
            }
    
    def get_all_statistics(self) -> Dict[str, Any]:
        """Get complete statistics"""
        total_shares = sum(s.shares_submitted for s in self.pool_stats.values())
        total_accepted = sum(s.shares_accepted for s in self.pool_stats.values())
        total_rejected = sum(s.shares_rejected for s in self.pool_stats.values())
        
        return {
            "current_pool": self.current_pool.name if self.current_pool else None,
            "pools": self.get_pool_stats(),
            "totals": {
                "shares_submitted": total_shares,
                "shares_accepted": total_accepted,
                "shares_rejected": total_rejected,
                "acceptance_rate": (total_accepted / total_shares * 100) if total_shares > 0 else 0.0
            }
        }


# Example usage
if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s'
    )
    
    async def test_connector():
        # Configure multiple pools
        pools = [
            PoolConfig(
                name="Primary Testnet",
                host="testnet-pool.bitcoin.com",
                port=3333,
                worker_name="your_address.worker1",
                priority=10
            ),
            PoolConfig(
                name="Backup Testnet",
                host="solo.ckpool.org",
                port=13333,
                worker_name="your_address.worker2",
                priority=5,
                backup=True
            )
        ]
        
        connector = MiningPoolConnector(pools)
        
        def on_job(job):
            print(f"\n🎯 New Job: {job}")
        
        def on_pool_change(pool_name):
            print(f"\n🔄 Switched to pool: {pool_name}")
        
        connector.on_job = on_job
        connector.on_pool_change = on_pool_change
        
        await connector.start()
        
        # Keep running
        try:
            await asyncio.sleep(300)  # 5 minutes
        except KeyboardInterrupt:
            pass
        
        print(f"\n📊 Statistics: {connector.get_all_statistics()}")
        await connector.stop()
    
    asyncio.run(test_connector())
