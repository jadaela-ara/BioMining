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
    from biological_bitcoin_learning import BiologicalBitcoinLearner
    from real_mea_interface import RealMEAInterface
except ImportError as e:
    print(f"⚠️ Platform modules not available: {e}")
    # Mock classes for development
    class HybridBitcoinMiner:
        def __init__(self): self.status = "offline"
        def start_mining(self): pass
        def stop_mining(self): pass
        def get_stats(self): return {}
    
    class BiologicalBitcoinLearner:
        def __init__(self): self.status = "offline"
        def start_training(self): pass
        def get_metrics(self): return {}
    
    class RealMEAInterface:
        def __init__(self): self.status = "offline"
        def get_electrode_data(self): return []

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
        self.biological_learner = BiologicalBitcoinLearner()
        self.mea_interface = RealMEAInterface()
        
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
                # Start biological network
                self.systems_status['biological']['status'] = 'starting'
                await asyncio.sleep(2)  # Longer initialization for neural networks
                self.systems_status['biological']['status'] = 'online'
                self.systems_status['biological']['neurons'] = 1000
                self.systems_status['biological']['synapses'] = 10000
                
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
        """Start the mining process"""
        try:
            if self.is_mining:
                return False
                
            # Check if required systems are online
            required_systems = ['sha256', 'biological', 'mea']
            for system in required_systems:
                if self.systems_status[system]['status'] != 'online':
                    await self.start_system(system)
            
            self.is_mining = True
            logger.info(f"⛏️ Mining started with method: {config.method}")
            
            # Start mining simulation loop
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
        """Start the training process"""
        try:
            if self.is_training:
                return False
                
            self.is_training = True
            logger.info("🧠 Training started")
            
            # Start training simulation loop
            asyncio.create_task(self.training_simulation_loop(config))
            
            return True
            
        except Exception as e:
            logger.error(f"❌ Error starting training: {e}")
            return False
    
    async def mining_simulation_loop(self):
        """Simulate mining activity for demonstration"""
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
                self.systems_status['biological']['hashrate'] = 75.2 + (hash(time.time()) % 25)
                self.systems_status['mea']['hashrate'] = 45.8 + (hash(time.time()) % 15)
                
                await asyncio.sleep(1)  # Update every second
                
            except Exception as e:
                logger.error(f"❌ Error in mining simulation: {e}")
                await asyncio.sleep(1)
    
    async def training_simulation_loop(self, config: TrainingConfig):
        """Simulate training process for demonstration"""
        current_epoch = 0
        
        while self.is_training and current_epoch < config.biological_epochs:
            try:
                current_epoch += 1
                
                # Simulate training progress
                biological_loss = max(0.001, 1.0 - (current_epoch / config.biological_epochs) * 0.8)
                mea_loss = max(0.001, 0.9 - (current_epoch / config.biological_epochs) * 0.7)
                
                biological_accuracy = min(0.99, (current_epoch / config.biological_epochs) * 0.85)
                mea_accuracy = min(0.95, (current_epoch / config.biological_epochs) * 0.80)
                
                # Update learning rate
                self.systems_status['biological']['learning_rate'] = config.learning_rate * (0.95 ** (current_epoch / 100))
                
                # Broadcast training progress
                await websocket_manager.broadcast({
                    'type': 'training_progress',
                    'data': {
                        'epoch': current_epoch,
                        'total_epochs': config.biological_epochs,
                        'biological_loss': biological_loss,
                        'mea_loss': mea_loss,
                        'biological_accuracy': biological_accuracy,
                        'mea_accuracy': mea_accuracy,
                        'system': 'combined'
                    }
                })
                
                await asyncio.sleep(0.1)  # Fast simulation for demo
                
            except Exception as e:
                logger.error(f"❌ Error in training simulation: {e}")
                await asyncio.sleep(1)
        
        self.is_training = False
        logger.info("✅ Training completed")
    
    def get_electrode_data(self) -> List[Dict]:
        """Get current electrode data"""
        electrode_data = []
        for i in range(1, 61):  # 60 electrodes
            electrode_data.append({
                'electrode_id': i,
                'active': i <= self.systems_status['mea']['active_electrodes'],
                'voltage': (hash(f"{i}_{time.time()}") % 100) / 10.0,  # 0-10mV
                'impedance': 500 + (hash(f"{i}_{time.time()}") % 1000),  # 500-1500 kΩ
                'stimulating': i % 20 == 0,  # Every 20th electrode
                'recording': i <= self.systems_status['mea']['active_electrodes']
            })
        return electrode_data
    
    def get_performance_metrics(self) -> Dict:
        """Get current performance metrics"""
        import psutil
        
        try:
            self.performance_metrics.update({
                'cpu_usage': psutil.cpu_percent(),
                'memory_usage': psutil.virtual_memory().percent,
                'gpu_usage': hash(time.time()) % 100,  # Mock GPU usage
                'network_io': sum(psutil.net_io_counters()[:2]),  # bytes sent + received
                'timestamp': time.time()
            })
        except ImportError:
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
    # Development server
    uvicorn.run(
        "server:app",
        host="0.0.0.0",
        port=8080,
        reload=True,
        log_level="info"
    )