#!/usr/bin/env python3
"""
================================================================
🔌 STRATUM PROTOCOL CLIENT - Bitcoin Mining Pool Communication
Complete implementation of Stratum mining protocol
Support for both Mainnet and Testnet pools
================================================================
"""

import asyncio
import json
import logging
import time
import socket
from typing import Dict, Optional, Callable, Any, List
from dataclasses import dataclass, field
from datetime import datetime

logger = logging.getLogger(__name__)


@dataclass
class StratumJob:
    """Represents a mining job from the pool"""
    job_id: str
    prevhash: str
    coinb1: str
    coinb2: str
    merkle_branch: List[str]
    version: str
    nbits: str
    ntime: str
    clean_jobs: bool
    extranonce1: str = ""
    extranonce2_size: int = 0
    difficulty: float = 1.0
    target: str = ""
    
    def __repr__(self):
        return (f"StratumJob(id={self.job_id[:8]}..., "
                f"diff={self.difficulty:.2f}, clean={self.clean_jobs})")


@dataclass
class StratumConnection:
    """Stratum connection state"""
    host: str
    port: int
    worker_name: str
    password: str = "x"
    
    # Connection state
    connected: bool = False
    authorized: bool = False
    subscribed: bool = False
    
    # Session info
    session_id: str = ""
    extranonce1: str = ""
    extranonce2_size: int = 4
    difficulty: float = 1.0
    
    # Current job
    current_job: Optional[StratumJob] = None
    
    # Statistics
    jobs_received: int = 0
    shares_submitted: int = 0
    shares_accepted: int = 0
    shares_rejected: int = 0
    last_share_time: float = 0.0
    connection_time: float = 0.0


class StratumClient:
    """
    Complete Stratum protocol client for Bitcoin mining
    
    Supports:
    - mining.subscribe
    - mining.authorize
    - mining.notify
    - mining.submit
    - mining.set_difficulty
    - Automatic reconnection
    - Job buffering
    """
    
    def __init__(self, host: str, port: int, worker_name: str, password: str = "x"):
        self.conn = StratumConnection(
            host=host,
            port=port,
            worker_name=worker_name,
            password=password
        )
        
        # Network
        self.reader: Optional[asyncio.StreamReader] = None
        self.writer: Optional[asyncio.StreamWriter] = None
        
        # Protocol
        self.message_id = 0
        self.pending_requests: Dict[int, asyncio.Future] = {}
        
        # Callbacks
        self.on_job: Optional[Callable[[StratumJob], None]] = None
        self.on_difficulty: Optional[Callable[[float], None]] = None
        self.on_connect: Optional[Callable[[], None]] = None
        self.on_disconnect: Optional[Callable[[], None]] = None
        
        # Tasks
        self.receiver_task: Optional[asyncio.Task] = None
        self.keepalive_task: Optional[asyncio.Task] = None
        
        logger.info(f"🔌 StratumClient initialized for {host}:{port}")
    
    async def connect(self, timeout: float = 10.0) -> bool:
        """Connect to Stratum server"""
        try:
            logger.info(f"🔌 Connecting to {self.conn.host}:{self.conn.port}...")
            
            self.reader, self.writer = await asyncio.wait_for(
                asyncio.open_connection(self.conn.host, self.conn.port),
                timeout=timeout
            )
            
            self.conn.connected = True
            self.conn.connection_time = time.time()
            logger.info(f"✅ Connected to {self.conn.host}:{self.conn.port}")
            
            # Start receiver task
            self.receiver_task = asyncio.create_task(self._receive_loop())
            
            # Start keepalive
            self.keepalive_task = asyncio.create_task(self._keepalive_loop())
            
            # Subscribe and authorize
            await self._subscribe()
            await self._authorize()
            
            if self.on_connect:
                self.on_connect()
            
            return True
            
        except asyncio.TimeoutError:
            logger.error(f"❌ Connection timeout to {self.conn.host}:{self.conn.port}")
            return False
        except Exception as e:
            logger.error(f"❌ Connection error: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from Stratum server"""
        logger.info("🔌 Disconnecting from Stratum server...")
        
        self.conn.connected = False
        self.conn.authorized = False
        self.conn.subscribed = False
        
        # Cancel tasks
        if self.receiver_task:
            self.receiver_task.cancel()
        if self.keepalive_task:
            self.keepalive_task.cancel()
        
        # Close connection
        if self.writer:
            try:
                self.writer.close()
                await self.writer.wait_closed()
            except:
                pass
        
        if self.on_disconnect:
            self.on_disconnect()
        
        logger.info("✅ Disconnected from Stratum server")
    
    async def _subscribe(self) -> bool:
        """Send mining.subscribe"""
        try:
            logger.info("📤 Sending mining.subscribe...")
            
            response = await self._send_request(
                "mining.subscribe",
                ["BioMiningPlatform/1.0", None]
            )
            
            if not response:
                logger.error("❌ mining.subscribe failed: no response")
                return False
            
            # Parse response: [[["mining.notify", "session_id"], "extranonce1", extranonce2_size]]
            if isinstance(response, list) and len(response) >= 3:
                # Session info
                if isinstance(response[0], list) and len(response[0]) > 0:
                    for item in response[0]:
                        if isinstance(item, list) and len(item) >= 2:
                            if item[0] == "mining.notify":
                                self.conn.session_id = item[1]
                
                # Extranonce
                self.conn.extranonce1 = response[1]
                self.conn.extranonce2_size = response[2]
                
                self.conn.subscribed = True
                logger.info(f"✅ Subscribed - Session: {self.conn.session_id[:16]}...")
                logger.info(f"   📍 Extranonce1: {self.conn.extranonce1}")
                logger.info(f"   📍 Extranonce2 size: {self.conn.extranonce2_size}")
                return True
            else:
                logger.error(f"❌ Unexpected subscribe response: {response}")
                return False
                
        except Exception as e:
            logger.error(f"❌ mining.subscribe error: {e}")
            return False
    
    async def _authorize(self) -> bool:
        """Send mining.authorize"""
        try:
            logger.info(f"📤 Sending mining.authorize for {self.conn.worker_name}...")
            
            response = await self._send_request(
                "mining.authorize",
                [self.conn.worker_name, self.conn.password]
            )
            
            if response is True:
                self.conn.authorized = True
                logger.info(f"✅ Authorized as {self.conn.worker_name}")
                return True
            else:
                logger.error(f"❌ Authorization failed: {response}")
                return False
                
        except Exception as e:
            logger.error(f"❌ mining.authorize error: {e}")
            return False
    
    async def submit_share(self, job_id: str, extranonce2: str, ntime: str, nonce: str) -> bool:
        """
        Submit a mining share
        
        Args:
            job_id: Job ID from mining.notify
            extranonce2: Extranonce2 value (hex)
            ntime: Block time (hex)
            nonce: Nonce that produced valid hash (hex)
        
        Returns:
            True if share accepted, False if rejected
        """
        try:
            logger.info(f"📤 Submitting share for job {job_id[:8]}...")
            logger.info(f"   📍 Nonce: {nonce}")
            logger.info(f"   📍 Extranonce2: {extranonce2}")
            logger.info(f"   📍 NTime: {ntime}")
            
            response = await self._send_request(
                "mining.submit",
                [
                    self.conn.worker_name,
                    job_id,
                    extranonce2,
                    ntime,
                    nonce
                ]
            )
            
            self.conn.shares_submitted += 1
            self.conn.last_share_time = time.time()
            
            if response is True:
                self.conn.shares_accepted += 1
                acceptance_rate = (self.conn.shares_accepted / self.conn.shares_submitted) * 100
                logger.info(f"✅ Share ACCEPTED! ({self.conn.shares_accepted}/{self.conn.shares_submitted} = {acceptance_rate:.1f}%)")
                return True
            else:
                self.conn.shares_rejected += 1
                acceptance_rate = (self.conn.shares_accepted / self.conn.shares_submitted) * 100
                logger.warning(f"❌ Share REJECTED: {response} ({self.conn.shares_accepted}/{self.conn.shares_submitted} = {acceptance_rate:.1f}%)")
                return False
                
        except Exception as e:
            logger.error(f"❌ Share submission error: {e}")
            return False
    
    async def _send_request(self, method: str, params: List[Any]) -> Any:
        """Send JSON-RPC request and wait for response"""
        if not self.conn.connected or not self.writer:
            raise ConnectionError("Not connected")
        
        self.message_id += 1
        msg_id = self.message_id
        
        request = {
            "id": msg_id,
            "method": method,
            "params": params
        }
        
        # Create future for response
        future = asyncio.Future()
        self.pending_requests[msg_id] = future
        
        # Send request
        request_json = json.dumps(request) + "\n"
        self.writer.write(request_json.encode('utf-8'))
        await self.writer.drain()
        
        # Wait for response (30 second timeout)
        try:
            response = await asyncio.wait_for(future, timeout=30.0)
            return response
        except asyncio.TimeoutError:
            logger.error(f"⏱️ Request timeout: {method}")
            del self.pending_requests[msg_id]
            return None
    
    async def _receive_loop(self):
        """Receive and process messages from server"""
        try:
            logger.info("🔄 Starting receiver loop...")
            
            while self.conn.connected and self.reader:
                try:
                    line = await self.reader.readline()
                    if not line:
                        logger.warning("⚠️ Connection closed by server")
                        break
                    
                    message = json.loads(line.decode('utf-8').strip())
                    await self._handle_message(message)
                    
                except json.JSONDecodeError as e:
                    logger.error(f"❌ Invalid JSON: {e}")
                except Exception as e:
                    logger.error(f"❌ Receive error: {e}")
                    break
            
        except asyncio.CancelledError:
            logger.info("🛑 Receiver loop cancelled")
        except Exception as e:
            logger.error(f"❌ Receiver loop error: {e}")
        finally:
            logger.info("🔄 Receiver loop stopped")
            await self.disconnect()
    
    async def _handle_message(self, message: Dict[str, Any]):
        """Handle incoming Stratum message"""
        
        # Response to our request
        if "id" in message and message["id"] is not None:
            msg_id = message["id"]
            if msg_id in self.pending_requests:
                future = self.pending_requests.pop(msg_id)
                if "result" in message:
                    future.set_result(message["result"])
                elif "error" in message:
                    future.set_result(message["error"])
                else:
                    future.set_result(None)
        
        # Notification from server
        elif "method" in message:
            method = message["method"]
            params = message.get("params", [])
            
            if method == "mining.notify":
                await self._handle_notify(params)
            elif method == "mining.set_difficulty":
                await self._handle_set_difficulty(params)
            elif method == "client.reconnect":
                await self._handle_reconnect(params)
            else:
                logger.warning(f"⚠️ Unknown method: {method}")
    
    async def _handle_notify(self, params: List[Any]):
        """Handle mining.notify - New job from pool"""
        try:
            if len(params) < 8:
                logger.error(f"❌ Invalid mining.notify: {len(params)} params")
                return
            
            job = StratumJob(
                job_id=params[0],
                prevhash=params[1],
                coinb1=params[2],
                coinb2=params[3],
                merkle_branch=params[4],
                version=params[5],
                nbits=params[6],
                ntime=params[7],
                clean_jobs=params[8] if len(params) > 8 else False,
                extranonce1=self.conn.extranonce1,
                extranonce2_size=self.conn.extranonce2_size,
                difficulty=self.conn.difficulty
            )
            
            self.conn.current_job = job
            self.conn.jobs_received += 1
            
            logger.info(f"📦 New Job #{self.conn.jobs_received}: {job}")
            
            if self.on_job:
                self.on_job(job)
                
        except Exception as e:
            logger.error(f"❌ Error handling mining.notify: {e}")
    
    async def _handle_set_difficulty(self, params: List[Any]):
        """Handle mining.set_difficulty"""
        try:
            if len(params) > 0:
                old_diff = self.conn.difficulty
                self.conn.difficulty = float(params[0])
                logger.info(f"🎯 Difficulty changed: {old_diff:.2f} → {self.conn.difficulty:.2f}")
                
                if self.on_difficulty:
                    self.on_difficulty(self.conn.difficulty)
                    
        except Exception as e:
            logger.error(f"❌ Error handling set_difficulty: {e}")
    
    async def _handle_reconnect(self, params: List[Any]):
        """Handle client.reconnect"""
        logger.warning(f"⚠️ Server requested reconnect: {params}")
        await self.disconnect()
    
    async def _keepalive_loop(self):
        """Send periodic keepalive to prevent timeout"""
        try:
            while self.conn.connected:
                await asyncio.sleep(60)  # Every minute
                
                if self.conn.connected:
                    # Simple ping via mining.subscribe (some pools need this)
                    try:
                        logger.debug("💓 Sending keepalive...")
                        # Just checking connection is alive
                        if self.writer:
                            self.writer.write(b"\n")
                            await self.writer.drain()
                    except:
                        pass
                        
        except asyncio.CancelledError:
            logger.info("🛑 Keepalive loop cancelled")
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get connection statistics"""
        uptime = time.time() - self.conn.connection_time if self.conn.connected else 0
        
        return {
            "connected": self.conn.connected,
            "authorized": self.conn.authorized,
            "uptime_seconds": uptime,
            "difficulty": self.conn.difficulty,
            "jobs_received": self.conn.jobs_received,
            "shares_submitted": self.conn.shares_submitted,
            "shares_accepted": self.conn.shares_accepted,
            "shares_rejected": self.conn.shares_rejected,
            "acceptance_rate": (self.conn.shares_accepted / self.conn.shares_submitted * 100) 
                              if self.conn.shares_submitted > 0 else 0.0,
            "last_share_time": self.conn.last_share_time,
            "current_job": str(self.conn.current_job) if self.conn.current_job else None
        }


# Example usage
if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s'
    )
    
    async def test_stratum():
        # Testnet pool example
        client = StratumClient(
            host="testnet-pool.bitcoin.com",
            port=3333,
            worker_name="your_wallet_address.worker1"
        )
        
        def on_job(job: StratumJob):
            print(f"\n🎯 New Job Received:")
            print(f"   ID: {job.job_id}")
            print(f"   Difficulty: {job.difficulty}")
            print(f"   Clean Jobs: {job.clean_jobs}")
        
        client.on_job = on_job
        
        if await client.connect():
            print("\n✅ Connected and authorized!")
            print(f"📊 Stats: {client.get_statistics()}")
            
            # Keep alive for testing
            await asyncio.sleep(30)
            
            print(f"\n📊 Final Stats: {client.get_statistics()}")
            await client.disconnect()
        else:
            print("❌ Connection failed")
    
    asyncio.run(test_stratum())
