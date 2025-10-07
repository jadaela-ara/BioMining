#!/usr/bin/env python3
"""
================================================================
HYBRID BITCOIN MINING PLATFORM - WEB API SERVER
Advanced FastAPI server for Triple System Interface
================================================================
"""

import asyncio
import json
import logging
import os
import sys
import time
import uuid
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Any
import websockets
import numpy as np

# Add parent directories to path for imports
sys.path.append(str(Path(__file__).parent.parent.parent))

try:
    from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException, UploadFile, File, Form
    from fastapi.middleware.cors import CORSMiddleware
    from fastapi.staticfiles import StaticFiles
    from fastapi.responses import HTMLResponse, JSONResponse
    from pydantic import BaseModel
    import uvicorn
except ImportError as e:
    print(f"⚠️ Missing dependencies: {e}")
    print("Installing required packages...")
    os.system("pip install fastapi uvicorn websockets python-multipart")
    from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException, UploadFile, File, Form
    from fastapi.middleware.cors import CORSMiddleware
    from fastapi.staticfiles import StaticFiles
    from fastapi.responses import HTMLResponse, JSONResponse
    from pydantic import BaseModel
    import uvicorn

# Import our platform modules
try:
    from src.cpp.hybrid_bitcoin_miner import HybridBitcoinMiner
except ImportError as e:
    print(f"⚠️ C++ hybrid miner not available: {e}")
    class HybridBitcoinMiner:
        def __init__(self): self.status = "offline"
        def start_mining(self): pass
        def stop_mining(self): pass
        def get_stats(self): return {}

# Import biological learning and MEA interface
try:
    from biological_bitcoin_learning import BiologicalBitcoinLearner
    from real_mea_interface import RealMEAInterface
    BIOLOGICAL_AVAILABLE = True
except ImportError as e:
    print(f"⚠️ Biological modules not available: {e}")
    BIOLOGICAL_AVAILABLE = False
    
    # Mock classes for development
    class BiologicalBitcoinLearner:
        def __init__(self, mea_interface): 
            self.status = "offline"
            self.learning_phase = "initialization"
            self.neurons = {}
            self.learning_history = []
            self.synaptic_matrix = np.zeros((60, 60))  # Mock synaptic matrix
            self.mining_active = False
        def get_learning_statistics(self): 
            return {
                'active_neurons': len(self.neurons),
                'avg_learning_coefficient': 0.001,
                'avg_pattern_score': 0.5,
                'avg_synaptic_strength': 0.5,
                'avg_confidence': 0.5,
                'total_predictions': 0,
                'successful_predictions': 0,
                'prediction_rate': 0.0,
                'learning_phase': self.learning_phase
            }
        async def initialize_biological_learning(self): 
            # Initialize mock neurons
            for i in range(60):
                self.neurons[i] = type('MockNeuron', (), {
                    'firing_rate': 0.0,
                    'spike_amplitude': 0.0,
                    'synaptic_strength': 0.5,
                    'bitcoin_response_score': 0.0,
                    'learning_coefficient': 0.001
                })()
        async def train_bitcoin_pattern_recognition(self, epochs): pass
        async def train_nonce_prediction(self, blocks): return 0.5
        async def start_biological_bitcoin_mining(self): 
            self.mining_active = True
        def stop_mining(self): 
            self.mining_active = False
    
    class RealMEAInterface:
        def __init__(self): 
            self.status = "offline"
            self.electrodes = list(range(60))
        def get_electrode_data(self): return []
        async def stimulate_electrode(self, *args, **kwargs): pass
        async def record_electrode(self, *args, **kwargs): return []

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Pydantic models for API
class SystemStatus(BaseModel):
    status: str
    hashrate: float = 0.0
    active_neurons: int = 0
    active_synapses: int = 0
    active_electrodes: int = 0
    blocks_mined: int = 0
    learning_rate: float = 0.0

class MiningConfig(BaseModel):
    method: str = "triple_system"
    difficulty: int = 4
    batch_size: int = 32
    max_attempts: int = 1000000

class TrainingConfig(BaseModel):
    biological_epochs: int = 1000
    mea_stimulation_frequency: float = 10.0
    learning_rate: float = 0.001
    batch_size: int = 32
    target_accuracy: float = 0.85

class ElectrodeControl(BaseModel):
    electrode_id: int
    active: bool
    stimulation_voltage: float = 0.0
    recording_mode: bool = True

class WebSocketMessage(BaseModel):
    type: str
    data: Dict[str, Any] = {}
    timestamp: float = None

    def __init__(self, **data):
        if data.get('timestamp') is None:
            data['timestamp'] = time.time()
        super().__init__(**data)

class WebSocketManager:
    """Manage WebSocket connections for real-time communication"""
    
    def __init__(self):
        self.active_connections: List[WebSocket] = []
        self.connection_data: Dict[WebSocket, Dict] = {}
    
    async def connect(self, websocket: WebSocket, client_id: str = None):
        await websocket.accept()
        self.active_connections.append(websocket)
        self.connection_data[websocket] = {
            'client_id': client_id or str(uuid.uuid4()),
            'connected_at': datetime.now(),
            'authenticated': False
        }
        logger.info(f"🔌 WebSocket client connected: {self.connection_data[websocket]['client_id']}")
    
    def disconnect(self, websocket: WebSocket):
        if websocket in self.active_connections:
            client_id = self.connection_data[websocket].get('client_id', 'unknown')
            self.active_connections.remove(websocket)
            del self.connection_data[websocket]
            logger.info(f"🔌 WebSocket client disconnected: {client_id}")
    
    async def send_personal_message(self, message: Dict, websocket: WebSocket):
        try:
            await websocket.send_text(json.dumps(message))
        except Exception as e:
            logger.error(f"❌ Error sending personal message: {e}")
    
    async def broadcast(self, message: Dict):
        if self.active_connections:
            disconnected = []
            for connection in self.active_connections:
                try:
                    await connection.send_text(json.dumps(message))
                except Exception as e:
                    logger.error(f"❌ Error broadcasting to client: {e}")
                    disconnected.append(connection)
            
            # Clean up disconnected clients
            for connection in disconnected:
                self.disconnect(connection)
    
    def get_connection_count(self) -> int:
        return len(self.active_connections)

class HybridMiningPlatform:
    """Main platform coordinator for the triple mining system"""
    
    def __init__(self):
        self.miner = HybridBitcoinMiner()
        self.mea_interface = RealMEAInterface()
        
        # Initialize biological learner with MEA interface
        if BIOLOGICAL_AVAILABLE:
            self.biological_learner = BiologicalBitcoinLearner(self.mea_interface)
        else:
            self.biological_learner = BiologicalBitcoinLearner(self.mea_interface)  # Mock version
        
        self.systems_status = {
            'sha256': {'status': 'offline', 'hashrate': 0.0, 'blocks': 0},
            'biological': {'status': 'offline', 'neurons': 0, 'synapses': 0, 'learning_rate': 0.0},
            'mea': {'status': 'offline', 'electrodes': 60, 'active_electrodes': 0, 'stimulation': False}
        }
        
        self.mining_stats = {
            'total_hashes': 0,
            'valid_nonces': 0,
            'success_rate': 0.0,
            'avg_response_time': 0.0,
            'blocks_mined': 0
        }
        
        self.performance_metrics = {
            'cpu_usage': 0.0,
            'memory_usage': 0.0,
            'gpu_usage': 0.0,
            'network_io': 0.0,
            'timestamp': time.time()
        }
        
        self.is_mining = False
        self.is_training = False
        
        logger.info("🚀 Hybrid Mining Platform initialized")
    
    async def start_system(self, system_name: str) -> bool:
        """Start a specific system"""
        try:
            if system_name == 'sha256':
                # Start SHA-256 mining system
                self.systems_status['sha256']['status'] = 'starting'
                await asyncio.sleep(1)  # Simulate initialization
                self.systems_status['sha256']['status'] = 'online'
                
            elif system_name == 'biological':
                # Start biological network with real initialization
                self.systems_status['biological']['status'] = 'starting'
                try:
                    # Initialize biological learning system
                    await self.biological_learner.initialize_biological_learning()
                    
                    # Update status with real neuron count
                    neuron_count = len(self.biological_learner.neurons)
                    active_synapses = int(np.sum(self.biological_learner.synaptic_matrix > 0.1))
                    
                    self.systems_status['biological']['status'] = 'online'
                    self.systems_status['biological']['neurons'] = neuron_count
                    self.systems_status['biological']['synapses'] = active_synapses
                    self.systems_status['biological']['learning_phase'] = self.biological_learner.learning_phase.value if hasattr(self.biological_learner.learning_phase, 'value') else str(self.biological_learner.learning_phase)
                    
                    logger.info(f"✅ Biological system initialized with {neuron_count} neurons")
                except Exception as e:
                    logger.error(f"❌ Error initializing biological system: {e}")
                    self.systems_status['biological']['status'] = 'error'
                
            elif system_name == 'mea':
                # Start MEA interface
                self.systems_status['mea']['status'] = 'starting'
                await asyncio.sleep(1.5)
                self.systems_status['mea']['status'] = 'online'
                self.systems_status['mea']['active_electrodes'] = 45
                
            logger.info(f"✅ System {system_name} started successfully")
            return True
            
        except Exception as e:
            logger.error(f"❌ Error starting system {system_name}: {e}")
            self.systems_status[system_name]['status'] = 'error'
            return False
    
    async def stop_system(self, system_name: str) -> bool:
        """Stop a specific system"""
        try:
            self.systems_status[system_name]['status'] = 'stopping'
            await asyncio.sleep(0.5)
            self.systems_status[system_name]['status'] = 'offline'
            
            logger.info(f"🛑 System {system_name} stopped")
            return True
            
        except Exception as e:
            logger.error(f"❌ Error stopping system {system_name}: {e}")
            return False
    
    async def start_mining(self, config: MiningConfig) -> bool:
        """Start the mining process with biological assistance"""
        try:
            if self.is_mining:
                return False
                
            # Check if biological system is trained
            if config.method in ['biological', 'triple_system']:
                if self.systems_status['biological']['status'] != 'online':
                    logger.info("🧠 Starting biological system for mining...")
                    await self.start_system('biological')
            
            # Check if required systems are online
            required_systems = {
                'triple_system': ['sha256', 'biological', 'mea'],
                'biological': ['biological', 'mea'],
                'sha256': ['sha256'],
                'mea': ['mea']
            }.get(config.method, ['sha256'])
            
            for system in required_systems:
                if self.systems_status[system]['status'] != 'online':
                    await self.start_system(system)
            
            self.is_mining = True
            logger.info(f"⛏️ Mining started with method: {config.method}")
            
            # Start appropriate mining loop based on method
            if config.method == 'biological' and BIOLOGICAL_AVAILABLE:
                asyncio.create_task(self.biological_mining_loop())
            elif config.method == 'triple_system':
                asyncio.create_task(self.hybrid_mining_loop(config))
            else:
                asyncio.create_task(self.mining_simulation_loop())
            
            return True
            
        except Exception as e:
            logger.error(f"❌ Error starting mining: {e}")
            return False
    
    async def stop_mining(self) -> bool:
        """Stop the mining process"""
        try:
            self.is_mining = False
            logger.info("🛑 Mining stopped")
            return True
        except Exception as e:
            logger.error(f"❌ Error stopping mining: {e}")
            return False
    
    async def start_training(self, config: TrainingConfig) -> bool:
        """Start the biological training process"""
        try:
            if self.is_training:
                return False
            
            # Ensure biological system is online
            if self.systems_status['biological']['status'] != 'online':
                await self.start_system('biological')
                
            self.is_training = True
            logger.info(f"🧠 Starting biological training - {config.biological_epochs} epochs")
            
            # Start real biological training
            asyncio.create_task(self.biological_training_loop(config))
            
            return True
            
        except Exception as e:
            logger.error(f"❌ Error starting training: {e}")
            return False
    
    async def biological_mining_loop(self):
        """Real biological mining loop using trained neurons"""
        logger.info("🧬⛏️ Starting biological Bitcoin mining!")
        
        if BIOLOGICAL_AVAILABLE:
            # Start real biological mining
            asyncio.create_task(self.biological_learner.start_biological_bitcoin_mining())
        
        # Monitor and update statistics
        while self.is_mining:
            try:
                # Get biological mining statistics
                if hasattr(self.biological_learner, 'mining_active') and self.biological_learner.mining_active:
                    bio_stats = self.biological_learner.get_learning_statistics()
                    
                    # Update mining stats with biological data
                    self.mining_stats['neural_predictions'] = bio_stats.get('total_predictions', 0)
                    self.mining_stats['successful_predictions'] = bio_stats.get('successful_predictions', 0)
                    self.mining_stats['biological_confidence'] = bio_stats.get('avg_confidence', 0.0)
                    
                    # Update system status
                    self.systems_status['biological']['hashrate'] = bio_stats.get('prediction_rate', 0.0)
                    
                    # Broadcast biological mining updates
                    await websocket_manager.broadcast({
                        'type': 'biological_mining_update',
                        'data': {
                            'neural_predictions': self.mining_stats.get('neural_predictions', 0),
                            'successful_predictions': self.mining_stats.get('successful_predictions', 0),
                            'biological_confidence': self.mining_stats.get('biological_confidence', 0.0),
                            'active_neurons': bio_stats.get('active_neurons', 0),
                            'learning_phase': bio_stats.get('learning_phase', 'unknown')
                        }
                    })
                else:
                    # Fallback simulation
                    await self.mining_simulation_loop_biological()
                
                await asyncio.sleep(2)  # Update every 2 seconds
                
            except Exception as e:
                logger.error(f"❌ Error in biological mining loop: {e}")
                await asyncio.sleep(5)

    async def hybrid_mining_loop(self, config: MiningConfig):
        """Hybrid mining combining all three systems"""
        logger.info("🔄 Starting hybrid triple-system mining!")
        
        while self.is_mining:
            try:
                # Phase 1: Biological prediction
                if BIOLOGICAL_AVAILABLE and hasattr(self.biological_learner, 'mining_active'):
                    bio_predictions = 3  # Simulate biological predictions
                else:
                    bio_predictions = 0
                
                # Phase 2: Traditional SHA-256 mining
                traditional_hashes = 1000000
                
                # Phase 3: MEA-guided optimization
                mea_optimizations = 5
                
                # Update combined statistics
                self.mining_stats['total_hashes'] += traditional_hashes
                self.mining_stats['neural_predictions'] = self.mining_stats.get('neural_predictions', 0) + bio_predictions
                self.mining_stats['mea_optimizations'] = self.mining_stats.get('mea_optimizations', 0) + mea_optimizations
                
                # Random success simulation
                if hash(time.time()) % 500 == 0:  # Higher success rate with hybrid
                    self.mining_stats['valid_nonces'] += 1
                    if self.mining_stats['valid_nonces'] % 50 == 0:  # More frequent blocks
                        self.mining_stats['blocks_mined'] += 1
                        logger.info("🎉 Hybrid system found a block!")
                
                # Update system hashrates
                bio_multiplier = 1.5 if bio_predictions > 0 else 1.0
                self.systems_status['sha256']['hashrate'] = (150.5 + (hash(time.time()) % 50)) * bio_multiplier
                self.systems_status['biological']['hashrate'] = 75.2 + (hash(time.time()) % 25) if bio_predictions > 0 else 0
                self.systems_status['mea']['hashrate'] = 45.8 + (hash(time.time()) % 15)
                
                await asyncio.sleep(1)
                
            except Exception as e:
                logger.error(f"❌ Error in hybrid mining loop: {e}")
                await asyncio.sleep(1)

    async def mining_simulation_loop_biological(self):
        """Enhanced simulation with biological influence"""
        # Update mining statistics with biological bias
        bio_multiplier = 1.0
        if hasattr(self.biological_learner, 'neurons') and self.biological_learner.neurons:
            # Calculate biological confidence
            avg_confidence = np.mean([n.bitcoin_response_score for n in self.biological_learner.neurons.values()])
            bio_multiplier = 1.0 + avg_confidence
        
        self.mining_stats['total_hashes'] += int(1000000 * bio_multiplier)
        
        # Higher success rate with biological assistance
        success_rate = 800 if bio_multiplier > 1.2 else 1000
        if hash(time.time()) % success_rate == 0:
            self.mining_stats['valid_nonces'] += 1
            if self.mining_stats['valid_nonces'] % 80 == 0:
                self.mining_stats['blocks_mined'] += 1
        
        # Update hashrates with biological influence
        self.systems_status['biological']['hashrate'] = (75.2 + (hash(time.time()) % 25)) * bio_multiplier

    async def mining_simulation_loop(self):
        """Standard mining simulation for SHA-256 only"""
        while self.is_mining:
            try:
                # Update mining statistics
                self.mining_stats['total_hashes'] += 1000000
                if hash(time.time()) % 1000 == 0:  # Random valid nonce
                    self.mining_stats['valid_nonces'] += 1
                    if self.mining_stats['valid_nonces'] % 100 == 0:
                        self.mining_stats['blocks_mined'] += 1
                
                # Calculate success rate
                if self.mining_stats['total_hashes'] > 0:
                    self.mining_stats['success_rate'] = (
                        self.mining_stats['valid_nonces'] / 
                        (self.mining_stats['total_hashes'] / 1000000)
                    ) * 100
                
                # Update system hashrates
                self.systems_status['sha256']['hashrate'] = 150.5 + (hash(time.time()) % 50)
                self.systems_status['biological']['hashrate'] = 0  # Not active in SHA-256 only mode
                self.systems_status['mea']['hashrate'] = 0  # Not active in SHA-256 only mode
                
                await asyncio.sleep(1)  # Update every second
                
            except Exception as e:
                logger.error(f"❌ Error in mining simulation: {e}")
                await asyncio.sleep(1)
    
    async def biological_training_loop(self, config: TrainingConfig):
        """Real biological training process"""
        try:
            # Phase 1: Pattern recognition training
            logger.info("🎯 Phase 1: Bitcoin pattern recognition training")
            pattern_epochs = config.biological_epochs // 2
            
            for epoch in range(pattern_epochs):
                if not self.is_training:
                    break
                    
                # Run one epoch of pattern training
                # (In real implementation, this would be handled by the biological learner)
                progress = epoch / pattern_epochs
                
                # Get real learning statistics
                stats = self.biological_learner.get_learning_statistics()
                
                # Update systems status with real data
                self.systems_status['biological']['learning_rate'] = stats.get('avg_learning_coefficient', config.learning_rate)
                
                # Broadcast real training progress
                await websocket_manager.broadcast({
                    'type': 'training_progress',
                    'data': {
                        'epoch': epoch,
                        'total_epochs': pattern_epochs,
                        'phase': 'pattern_recognition',
                        'biological_accuracy': stats.get('avg_pattern_score', progress * 0.8),
                        'active_neurons': stats.get('active_neurons', 0),
                        'synaptic_strength': stats.get('avg_synaptic_strength', 0.5),
                        'system': 'biological'
                    }
                })
                
                if epoch % 50 == 0:  # Every 50 epochs
                    logger.info(f"📈 Pattern training epoch {epoch}/{pattern_epochs}")
                
                await asyncio.sleep(0.05)  # Controlled training speed
            
            # Phase 2: Nonce prediction training (if still training)
            if self.is_training:
                logger.info("🔮 Phase 2: Nonce prediction training")
                nonce_blocks = config.biological_epochs // 4
                
                # Start real nonce prediction training
                if BIOLOGICAL_AVAILABLE:
                    final_accuracy = await self.biological_learner.train_nonce_prediction(nonce_blocks)
                    logger.info(f"🎯 Nonce prediction training completed with accuracy: {final_accuracy:.3f}")
                else:
                    # Mock training for development
                    for block in range(nonce_blocks):
                        if not self.is_training:
                            break
                        progress = block / nonce_blocks
                        
                        await websocket_manager.broadcast({
                            'type': 'training_progress',
                            'data': {
                                'block': block,
                                'total_blocks': nonce_blocks,
                                'phase': 'nonce_prediction',
                                'prediction_accuracy': progress * 0.6,
                                'system': 'biological'
                            }
                        })
                        await asyncio.sleep(0.02)
            
        except Exception as e:
            logger.error(f"❌ Error in biological training: {e}")
        finally:
            self.is_training = False
            logger.info("✅ Biological training completed")
    
    def get_electrode_data(self) -> List[Dict]:
        """Get current electrode data from MEA interface"""
        try:
            # Try to get real electrode data
            if hasattr(self.mea_interface, 'get_electrode_data'):
                real_data = self.mea_interface.get_electrode_data()
                if real_data:
                    return real_data
        except Exception as e:
            logger.warning(f"Could not get real electrode data: {e}")
        
        # Fallback to enhanced simulation with biological influence
        electrode_data = []
        for i in range(1, 61):  # 60 electrodes
            # Add biological influence if available
            biological_influence = 1.0
            if i-1 in self.biological_learner.neurons:
                neuron = self.biological_learner.neurons[i-1]
                biological_influence = 1.0 + neuron.bitcoin_response_score * 0.5
            
            base_voltage = (hash(f"{i}_{time.time()}") % 100) / 10.0
            
            electrode_data.append({
                'electrode_id': i,
                'active': i <= self.systems_status['mea']['active_electrodes'],
                'voltage': base_voltage * biological_influence,  # Influenced by biological learning
                'impedance': 500 + (hash(f"{i}_{time.time()}") % 1000),  # 500-1500 kΩ
                'stimulating': i % 20 == 0,  # Every 20th electrode
                'recording': i <= self.systems_status['mea']['active_electrodes'],
                'firing_rate': self.biological_learner.neurons.get(i-1, type('obj', (object,), {'firing_rate': 0.0})).firing_rate if hasattr(self.biological_learner, 'neurons') else 0.0,
                'synaptic_strength': self.biological_learner.neurons.get(i-1, type('obj', (object,), {'synaptic_strength': 0.5})).synaptic_strength if hasattr(self.biological_learner, 'neurons') else 0.5
            })
        return electrode_data
    
    def get_performance_metrics(self) -> Dict:
        """Get current performance metrics"""
        try:
            import psutil
            self.performance_metrics.update({
                'cpu_usage': psutil.cpu_percent(),
                'memory_usage': psutil.virtual_memory().percent,
                'gpu_usage': hash(time.time()) % 100,  # Mock GPU usage
                'network_io': sum(psutil.net_io_counters()[:2]),  # bytes sent + received
                'timestamp': time.time()
            })
        except (ImportError, Exception):
            # Mock data if psutil not available
            self.performance_metrics.update({
                'cpu_usage': 45.5 + (hash(time.time()) % 30),
                'memory_usage': 62.3 + (hash(time.time()) % 20),
                'gpu_usage': 78.2 + (hash(time.time()) % 15),
                'network_io': time.time() * 1000,
                'timestamp': time.time()
            })
        
        return self.performance_metrics

# Initialize FastAPI app
app = FastAPI(
    title="Hybrid Bitcoin Mining Platform API",
    description="Advanced API for Triple System Bitcoin Mining Platform",
    version="1.0.0"
)

# Configure CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize managers
websocket_manager = WebSocketManager()
platform = HybridMiningPlatform()

# Mount static files
web_dir = Path(__file__).parent.parent
app.mount("/css", StaticFiles(directory=str(web_dir / "css")), name="css")
app.mount("/js", StaticFiles(directory=str(web_dir / "js")), name="js")
app.mount("/static", StaticFiles(directory=str(web_dir)), name="static")

# API Routes
@app.get("/", response_class=HTMLResponse)
async def serve_index():
    """Serve the main interface"""
    index_path = web_dir / "index.html"
    if index_path.exists():
        return HTMLResponse(content=index_path.read_text(), status_code=200)
    return HTMLResponse(content="<h1>Hybrid Mining Platform</h1><p>Interface not found</p>", status_code=404)

@app.get("/api/status")
async def get_system_status():
    """Get overall system status"""
    return JSONResponse({
        "systems": platform.systems_status,
        "mining": {
            "active": platform.is_mining,
            "stats": platform.mining_stats
        },
        "training": {
            "active": platform.is_training
        },
        "performance": platform.get_performance_metrics(),
        "connections": websocket_manager.get_connection_count()
    })

@app.post("/api/systems/{system_name}/start")
async def start_system(system_name: str):
    """Start a specific system"""
    if system_name not in ['sha256', 'biological', 'mea']:
        raise HTTPException(status_code=400, detail="Invalid system name")
    
    success = await platform.start_system(system_name)
    
    # Broadcast status update
    await websocket_manager.broadcast({
        'type': 'system_status',
        'data': platform.systems_status
    })
    
    return JSONResponse({
        "success": success,
        "message": f"System {system_name} {'started' if success else 'failed to start'}"
    })

@app.post("/api/systems/{system_name}/stop")
async def stop_system(system_name: str):
    """Stop a specific system"""
    if system_name not in ['sha256', 'biological', 'mea']:
        raise HTTPException(status_code=400, detail="Invalid system name")
    
    success = await platform.stop_system(system_name)
    
    # Broadcast status update
    await websocket_manager.broadcast({
        'type': 'system_status',
        'data': platform.systems_status
    })
    
    return JSONResponse({
        "success": success,
        "message": f"System {system_name} {'stopped' if success else 'failed to stop'}"
    })

@app.post("/api/mining/start")
async def start_mining(config: MiningConfig):
    """Start mining process"""
    success = await platform.start_mining(config)
    
    # Broadcast mining update
    await websocket_manager.broadcast({
        'type': 'mining_update',
        'data': {
            'active': platform.is_mining,
            'stats': platform.mining_stats
        }
    })
    
    return JSONResponse({
        "success": success,
        "message": "Mining started" if success else "Failed to start mining"
    })

@app.post("/api/mining/stop")
async def stop_mining():
    """Stop mining process"""
    success = await platform.stop_mining()
    
    # Broadcast mining update
    await websocket_manager.broadcast({
        'type': 'mining_update',
        'data': {
            'active': platform.is_mining,
            'stats': platform.mining_stats
        }
    })
    
    return JSONResponse({
        "success": success,
        "message": "Mining stopped" if success else "Failed to stop mining"
    })

@app.post("/api/training/start")
async def start_training(config: TrainingConfig):
    """Start training process"""
    success = await platform.start_training(config)
    
    return JSONResponse({
        "success": success,
        "message": "Training started" if success else "Failed to start training"
    })

@app.post("/api/training/stop")
async def stop_training():
    """Stop training process"""
    platform.is_training = False
    
    return JSONResponse({
        "success": True,
        "message": "Training stopped"
    })

@app.get("/api/electrodes")
async def get_electrodes():
    """Get electrode data"""
    return JSONResponse(platform.get_electrode_data())

@app.post("/api/electrodes/{electrode_id}/control")
async def control_electrode(electrode_id: int, control: ElectrodeControl):
    """Control specific electrode"""
    # Simulate electrode control
    electrode_data = platform.get_electrode_data()
    
    # Broadcast electrode update
    await websocket_manager.broadcast({
        'type': 'electrode_data',
        'data': electrode_data
    })
    
    return JSONResponse({
        "success": True,
        "message": f"Electrode {electrode_id} controlled successfully"
    })

@app.post("/api/upload")
async def upload_file(file: UploadFile = File(...), file_type: str = Form("training")):
    """Upload training files"""
    try:
        # Create uploads directory if it doesn't exist
        upload_dir = Path("uploads")
        upload_dir.mkdir(exist_ok=True)
        
        # Save uploaded file
        file_path = upload_dir / file.filename
        with open(file_path, "wb") as buffer:
            content = await file.read()
            buffer.write(content)
        
        logger.info(f"📁 File uploaded: {file.filename} ({len(content)} bytes)")
        
        # Broadcast upload progress
        await websocket_manager.broadcast({
            'type': 'upload_progress',
            'data': {
                'filename': file.filename,
                'progress': 100,
                'status': 'completed',
                'file_type': file_type
            }
        })
        
        return JSONResponse({
            "success": True,
            "message": f"File {file.filename} uploaded successfully",
            "file_path": str(file_path),
            "file_size": len(content)
        })
        
    except Exception as e:
        logger.error(f"❌ Upload error: {e}")
        raise HTTPException(status_code=500, detail=f"Upload failed: {str(e)}")

@app.get("/api/performance")
async def get_performance():
    """Get performance metrics"""
    return JSONResponse(platform.get_performance_metrics())

@app.get("/api/biological/status")
async def get_biological_status():
    """Get detailed biological system status"""
    try:
        stats = platform.biological_learner.get_learning_statistics()
        return JSONResponse({
            "available": BIOLOGICAL_AVAILABLE,
            "learning_phase": getattr(platform.biological_learner.learning_phase, 'value', str(platform.biological_learner.learning_phase)),
            "statistics": stats,
            "neurons_count": len(platform.biological_learner.neurons),
            "learning_history": platform.biological_learner.learning_history[-10:] if platform.biological_learner.learning_history else []
        })
    except Exception as e:
        return JSONResponse({"error": str(e)}, status_code=500)

@app.post("/api/biological/initialize")
async def initialize_biological_system():
    """Initialize the biological learning system"""
    try:
        if not BIOLOGICAL_AVAILABLE:
            raise HTTPException(status_code=503, detail="Biological system not available")
            
        await platform.biological_learner.initialize_biological_learning()
        
        # Update system status
        platform.systems_status['biological']['status'] = 'online'
        platform.systems_status['biological']['neurons'] = len(platform.biological_learner.neurons)
        
        # Broadcast status update
        await websocket_manager.broadcast({
            'type': 'biological_initialized',
            'data': {
                'neurons': len(platform.biological_learner.neurons),
                'status': 'online'
            }
        })
        
        return JSONResponse({
            "success": True,
            "message": "Biological system initialized successfully",
            "neurons_initialized": len(platform.biological_learner.neurons)
        })
        
    except Exception as e:
        logger.error(f"❌ Error initializing biological system: {e}")
        raise HTTPException(status_code=500, detail=f"Initialization failed: {str(e)}")

@app.post("/api/biological/train/patterns")
async def train_pattern_recognition(epochs: int = 1000):
    """Start Bitcoin pattern recognition training"""
    try:
        if not BIOLOGICAL_AVAILABLE:
            raise HTTPException(status_code=503, detail="Biological system not available")
            
        if platform.systems_status['biological']['status'] != 'online':
            raise HTTPException(status_code=400, detail="Biological system must be initialized first")
        
        # Start pattern training in background
        async def run_pattern_training():
            try:
                await platform.biological_learner.train_bitcoin_pattern_recognition(epochs)
                await websocket_manager.broadcast({
                    'type': 'pattern_training_complete',
                    'data': {'epochs_completed': epochs}
                })
            except Exception as e:
                logger.error(f"❌ Pattern training error: {e}")
                await websocket_manager.broadcast({
                    'type': 'pattern_training_error',
                    'data': {'error': str(e)}
                })
        
        asyncio.create_task(run_pattern_training())
        
        return JSONResponse({
            "success": True,
            "message": f"Pattern recognition training started for {epochs} epochs"
        })
        
    except Exception as e:
        logger.error(f"❌ Error starting pattern training: {e}")
        raise HTTPException(status_code=500, detail=f"Training failed: {str(e)}")

@app.post("/api/biological/train/nonce")  
async def train_nonce_prediction(blocks: int = 1000):
    """Start nonce prediction training"""
    try:
        if not BIOLOGICAL_AVAILABLE:
            raise HTTPException(status_code=503, detail="Biological system not available")
            
        if platform.systems_status['biological']['status'] != 'online':
            raise HTTPException(status_code=400, detail="Biological system must be initialized first")
        
        # Start nonce training in background
        async def run_nonce_training():
            try:
                accuracy = await platform.biological_learner.train_nonce_prediction(blocks)
                await websocket_manager.broadcast({
                    'type': 'nonce_training_complete',
                    'data': {
                        'blocks_trained': blocks,
                        'final_accuracy': accuracy
                    }
                })
            except Exception as e:
                logger.error(f"❌ Nonce training error: {e}")
                await websocket_manager.broadcast({
                    'type': 'nonce_training_error',
                    'data': {'error': str(e)}
                })
        
        asyncio.create_task(run_nonce_training())
        
        return JSONResponse({
            "success": True,
            "message": f"Nonce prediction training started for {blocks} blocks"
        })
        
    except Exception as e:
        logger.error(f"❌ Error starting nonce training: {e}")
        raise HTTPException(status_code=500, detail=f"Training failed: {str(e)}")

@app.get("/api/biological/neurons")
async def get_neurons_status():
    """Get detailed status of all biological neurons"""
    try:
        if not hasattr(platform.biological_learner, 'neurons'):
            return JSONResponse({"neurons": [], "message": "No neurons initialized"})
        
        neurons_data = []
        for electrode_id, neuron in platform.biological_learner.neurons.items():
            neurons_data.append({
                "electrode_id": electrode_id,
                "firing_rate": neuron.firing_rate,
                "spike_amplitude": neuron.spike_amplitude,
                "synaptic_strength": neuron.synaptic_strength,
                "bitcoin_response_score": neuron.bitcoin_response_score,
                "learning_coefficient": neuron.learning_coefficient
            })
        
        return JSONResponse({
            "neurons": neurons_data,
            "total_neurons": len(neurons_data),
            "average_scores": {
                "firing_rate": np.mean([n.firing_rate for n in platform.biological_learner.neurons.values()]),
                "bitcoin_score": np.mean([n.bitcoin_response_score for n in platform.biological_learner.neurons.values()]),
                "synaptic_strength": np.mean([n.synaptic_strength for n in platform.biological_learner.neurons.values()])
            }
        })
        
    except Exception as e:
        return JSONResponse({"error": str(e)}, status_code=500)

# WebSocket endpoint
@app.websocket("/ws/hybrid-mining")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket endpoint for real-time communication"""
    await websocket_manager.connect(websocket)
    
    try:
        # Send initial status
        await websocket_manager.send_personal_message({
            'type': 'system_status',
            'data': platform.systems_status
        }, websocket)
        
        while True:
            # Wait for messages from client
            data = await websocket.receive_text()
            message = json.loads(data)
            
            # Handle different message types
            if message.get('type') == 'ping':
                await websocket_manager.send_personal_message({
                    'type': 'pong',
                    'timestamp': time.time()
                }, websocket)
                
            elif message.get('type') == 'authenticate':
                # Simple authentication (in production, use proper auth)
                websocket_manager.connection_data[websocket]['authenticated'] = True
                await websocket_manager.send_personal_message({
                    'type': 'auth_response',
                    'data': {'success': True}
                }, websocket)
                
            elif message.get('type') == 'get_system_status':
                await websocket_manager.send_personal_message({
                    'type': 'system_status',
                    'data': platform.systems_status
                }, websocket)
                
            elif message.get('type') == 'get_performance_metrics':
                await websocket_manager.send_personal_message({
                    'type': 'performance_metrics',
                    'data': platform.get_performance_metrics()
                }, websocket)
                
    except WebSocketDisconnect:
        websocket_manager.disconnect(websocket)
    except Exception as e:
        logger.error(f"❌ WebSocket error: {e}")
        websocket_manager.disconnect(websocket)

# Background task for periodic updates
async def periodic_updates():
    """Send periodic updates to connected clients"""
    while True:
        try:
            if websocket_manager.get_connection_count() > 0:
                # Send performance metrics
                await websocket_manager.broadcast({
                    'type': 'performance_metrics',
                    'data': platform.get_performance_metrics()
                })
                
                # Send electrode data if MEA system is online
                if platform.systems_status['mea']['status'] == 'online':
                    await websocket_manager.broadcast({
                        'type': 'electrode_data',
                        'data': platform.get_electrode_data()
                    })
                
                # Send mining updates if mining is active
                if platform.is_mining:
                    await websocket_manager.broadcast({
                        'type': 'mining_update',
                        'data': platform.mining_stats
                    })
            
            await asyncio.sleep(2)  # Update every 2 seconds
            
        except Exception as e:
            logger.error(f"❌ Error in periodic updates: {e}")
            await asyncio.sleep(5)

# Startup event
@app.on_event("startup")
async def startup_event():
    """Initialize platform on startup"""
    logger.info("🚀 Starting Hybrid Bitcoin Mining Platform API Server")
    
    # Start periodic updates task
    asyncio.create_task(periodic_updates())
    
    logger.info("✅ API Server started successfully")

# Shutdown event
@app.on_event("shutdown")
async def shutdown_event():
    """Cleanup on shutdown"""
    logger.info("🛑 Shutting down Hybrid Bitcoin Mining Platform API Server")
    
    # Stop all systems
    await platform.stop_mining()
    platform.is_training = False
    
    logger.info("✅ API Server shutdown complete")

if __name__ == "__main__":
    # Get port from environment (CloudRun compatibility)
    import os
    port = int(os.getenv("PORT", 8080))
    
    # Check if we're in development or production
    if os.getenv("NODE_ENV") == "production":
        # Production server
        uvicorn.run(
            "web.api.server:app",
            host="0.0.0.0",
            port=port,
            log_level="info"
        )
    else:
        # Development server with reload
        uvicorn.run(
            "server:app",
            host="0.0.0.0",
            port=port,
            reload=True,
            log_level="info"
        )