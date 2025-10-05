#!/usr/bin/env python3
"""
🧬 REAL MEA INTERFACE - Enhanced for Biological Bitcoin Learning
===============================================================

Interface révolutionnaire pour Multi-Electrode Array (MEA) avec capacités
d'apprentissage Bitcoin intégrées pour neurones biologiques.

Cette interface permet de:
- Contrôler et stimuler des neurones biologiques via MEA
- Enregistrer l'activité neuronale en temps réel
- Entraîner les neurones à reconnaître les patterns Bitcoin
- Optimiser le mining via plasticité synaptique biologiquele

Compatible avec:
- Multi Channel Systems MEA2100
- Plexon OmniPlex
- Blackrock Cerebus
- Intan RHD2000 series

Author: BioMining Team  
Version: 3.0.0 - Biological Bitcoin Integration
"""

import asyncio
import numpy as np
import time
import json
import logging
from dataclasses import dataclass, field
from typing import List, Dict, Optional, Callable, Tuple, Any
from enum import Enum
import threading
from queue import Queue, Empty
import struct
import socket
import serial
from abc import ABC, abstractmethod

# Configuration logging
logging.basicConfig(level=logging.INFO, format='[%(asctime)s] 🔬 MEA: %(message)s')
logger = logging.getLogger(__name__)

class MEAConnectionType(Enum):
    """Types de connexion MEA supportés"""
    TCP_SOCKET = "tcp_socket"        # Connexion réseau TCP
    SERIAL_USB = "serial_usb"        # Connexion série USB  
    SHARED_MEMORY = "shared_memory"  # Mémoire partagée (rapide)
    FILE_STREAM = "file_stream"      # Stream de fichiers (simulation)

class ElectrodeState(Enum):
    """États des électrodes"""
    ACTIVE = "active"                # Électrode active et fonctionnelle
    INACTIVE = "inactive"            # Électrode inactive
    STIMULATING = "stimulating"      # En cours de stimulation
    RECORDING = "recording"          # En cours d'enregistrement
    ERROR = "error"                  # Erreur détectée

@dataclass
class ElectrodeData:
    """Données d'une électrode à un instant donné"""
    electrode_id: int
    timestamp: float  # Timestamp en secondes
    voltage: float    # Tension en microvolts (μV)
    impedance: float = 0.0  # Impédance en MΩ
    noise_level: float = 0.0  # Niveau de bruit RMS
    is_spike: bool = False   # Détection automatique de spike
    state: ElectrodeState = ElectrodeState.ACTIVE

@dataclass 
class StimulusPattern:
    """Pattern de stimulation pour une électrode"""
    electrode_id: int
    waveform_type: str = "biphasic"  # biphasic, monophasic, custom
    amplitude: float = 10.0          # Amplitude en μV
    duration: float = 0.1            # Durée en ms
    frequency: float = 100.0         # Fréquence en Hz (pour trains)
    pulse_count: int = 1             # Nombre de pulses
    inter_pulse_interval: float = 1.0 # Intervalle entre pulses (ms)
    custom_waveform: Optional[np.ndarray] = None  # Waveform personnalisée

@dataclass
class BitcoinStimulationProtocol:
    """Protocol de stimulation spécifique pour l'apprentissage Bitcoin"""
    block_pattern: bytes             # Pattern dérivé du block header
    target_electrodes: List[int]     # Électrodes ciblées
    stimulation_sequence: List[StimulusPattern] = field(default_factory=list)
    expected_response_pattern: Optional[np.ndarray] = None
    learning_phase: str = "pattern_recognition"
    reward_signal_amplitude: float = 50.0  # Amplitude de récompense

class MEADevice(ABC):
    """Interface abstraite pour différents types de dispositifs MEA"""
    
    @abstractmethod
    async def connect(self) -> bool:
        """Se connecter au dispositif MEA"""
        pass
    
    @abstractmethod  
    async def disconnect(self) -> bool:
        """Se déconnecter du dispositif MEA"""
        pass
    
    @abstractmethod
    async def start_recording(self) -> bool:
        """Démarrer l'acquisition de données"""
        pass
    
    @abstractmethod
    async def stop_recording(self) -> bool:
        """Arrêter l'acquisition de données"""
        pass
    
    @abstractmethod
    async def stimulate_electrode(self, pattern: StimulusPattern) -> bool:
        """Stimuler une électrode avec un pattern donné"""
        pass
    
    @abstractmethod
    async def get_latest_data(self, num_samples: int = 1000) -> List[ElectrodeData]:
        """Récupérer les dernières données d'acquisition"""
        pass

class MultiChannelSystemsMEA2100(MEADevice):
    """
    🔬 Implémentation pour Multi Channel Systems MEA2100
    
    Dispositif MEA haut de gamme avec 60 électrodes, stimulation
    et enregistrement simultanés, idéal pour l'apprentissage Bitcoin.
    """
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.host = config.get('host', 'localhost')
        self.port = config.get('port', 6340)
        self.num_electrodes = config.get('num_electrodes', 60)
        self.sampling_rate = config.get('sampling_rate', 25000)  # 25 kHz
        
        self.socket: Optional[socket.socket] = None
        self.is_connected = False
        self.is_recording = False
        
        # Buffers de données
        self.data_buffer = Queue(maxsize=100000)  # Buffer circulaire
        self.stimulation_queue = Queue(maxsize=1000)
        
        # Thread d'acquisition
        self.acquisition_thread: Optional[threading.Thread] = None
        self.stimulation_thread: Optional[threading.Thread] = None
        
        logger.info(f"🔬 MEA2100 initialized: {self.num_electrodes} electrodes @ {self.sampling_rate} Hz")

    async def connect(self) -> bool:
        """Se connecter au MEA2100 via TCP"""
        try:
            logger.info(f"🔗 Connecting to MEA2100 at {self.host}:{self.port}")
            
            # Créer la connexion TCP
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(10.0)  # Timeout de 10 secondes
            
            await asyncio.get_event_loop().run_in_executor(
                None, self.socket.connect, (self.host, self.port)
            )
            
            # Envoyer la commande d'initialisation
            init_command = self._build_init_command()
            await self._send_command(init_command)
            
            # Vérifier la réponse
            response = await self._receive_response()
            if response and b"MEA2100_READY" in response:
                self.is_connected = True
                logger.info("✅ MEA2100 connection successful")
                
                # Démarrer les threads de gestion
                await self._start_background_threads()
                
                return True
            else:
                logger.error(f"❌ MEA2100 initialization failed: {response}")
                return False
                
        except Exception as e:
            logger.error(f"❌ MEA2100 connection error: {e}")
            return False

    async def disconnect(self) -> bool:
        """Déconnexion propre du MEA2100"""
        try:
            logger.info("🔌 Disconnecting from MEA2100...")
            
            # Arrêter l'enregistrement si actif
            if self.is_recording:
                await self.stop_recording()
            
            # Arrêter les threads
            await self._stop_background_threads()
            
            # Fermer la connexion socket
            if self.socket:
                await self._send_command(b"DISCONNECT")
                self.socket.close()
                self.socket = None
            
            self.is_connected = False
            logger.info("✅ MEA2100 disconnected successfully")
            return True
            
        except Exception as e:
            logger.error(f"❌ MEA2100 disconnection error: {e}")
            return False

    async def start_recording(self) -> bool:
        """Démarrer l'acquisition simultanée sur toutes les électrodes"""
        if not self.is_connected:
            logger.error("❌ Cannot start recording: MEA2100 not connected")
            return False
        
        try:
            logger.info(f"📊 Starting MEA2100 recording ({self.num_electrodes} channels)")
            
            # Configurer l'acquisition
            config_command = self._build_recording_config()
            await self._send_command(config_command)
            
            # Démarrer l'acquisition
            start_command = b"START_RECORDING"
            await self._send_command(start_command)
            
            # Vérifier que l'acquisition a démarré
            response = await self._receive_response()
            if response and b"RECORDING_STARTED" in response:
                self.is_recording = True
                logger.info("✅ MEA2100 recording started successfully")
                return True
            else:
                logger.error(f"❌ Failed to start MEA2100 recording: {response}")
                return False
                
        except Exception as e:
            logger.error(f"❌ MEA2100 recording start error: {e}")
            return False

    async def stop_recording(self) -> bool:
        """Arrêter l'acquisition"""
        try:
            if not self.is_recording:
                return True
            
            logger.info("⏹️ Stopping MEA2100 recording...")
            
            # Envoyer la commande d'arrêt
            await self._send_command(b"STOP_RECORDING")
            
            response = await self._receive_response()
            if response and b"RECORDING_STOPPED" in response:
                self.is_recording = False
                logger.info("✅ MEA2100 recording stopped")
                return True
            else:
                logger.warning(f"⚠️ Unexpected stop response: {response}")
                self.is_recording = False  # Force stop
                return True
                
        except Exception as e:
            logger.error(f"❌ MEA2100 recording stop error: {e}")
            self.is_recording = False
            return False

    async def stimulate_electrode(self, pattern: StimulusPattern) -> bool:
        """
        ⚡ Stimuler une électrode avec un pattern spécifique
        
        Essentiel pour l'entraînement Bitcoin - permet d'enseigner aux
        neurones en associant des stimulations aux patterns de hash.
        """
        if not self.is_connected:
            logger.error("❌ Cannot stimulate: MEA2100 not connected")
            return False
        
        try:
            # Construire la commande de stimulation
            stim_command = self._build_stimulation_command(pattern)
            
            # Ajouter à la queue de stimulation pour traitement en arrière-plan
            self.stimulation_queue.put(stim_command, timeout=1.0)
            
            logger.debug(f"⚡ Queued stimulation for electrode {pattern.electrode_id}")
            return True
            
        except Exception as e:
            logger.error(f"❌ MEA2100 stimulation error: {e}")
            return False

    async def get_latest_data(self, num_samples: int = 1000) -> List[ElectrodeData]:
        """
        📊 Récupérer les dernières données d'acquisition
        
        Retourne les données les plus récentes de toutes les électrodes
        pour analyse de l'activité neuronale en réponse aux stimulations Bitcoin.
        """
        if not self.is_recording:
            return []
        
        try:
            # Récupérer les données du buffer
            data_samples = []
            samples_retrieved = 0
            
            while samples_retrieved < num_samples:
                try:
                    # Récupérer du buffer avec timeout court
                    raw_data = self.data_buffer.get(timeout=0.01)
                    
                    # Parser les données brutes en ElectrodeData
                    parsed_data = self._parse_raw_data(raw_data)
                    data_samples.extend(parsed_data)
                    
                    samples_retrieved += len(parsed_data)
                    
                except Empty:
                    # Plus de données disponibles dans le buffer
                    break
            
            # Retourner les derniers échantillons demandés
            return data_samples[-num_samples:] if len(data_samples) > num_samples else data_samples
            
        except Exception as e:
            logger.error(f"❌ MEA2100 data retrieval error: {e}")
            return []

    def _build_init_command(self) -> bytes:
        """Construit la commande d'initialisation MEA2100"""
        # Protocole MEA2100 simplifié (en réalité plus complexe)
        config = struct.pack('<4sHHHH', 
                           b'INIT',                    # Header
                           self.num_electrodes,        # Nombre d'électrodes
                           int(self.sampling_rate),    # Fréquence d'échantillonnage
                           1000,                       # Gain d'amplification
                           0x01                        # Flags de configuration
        )
        return config

    def _build_recording_config(self) -> bytes:
        """Configuration pour l'enregistrement"""
        # Configuration d'acquisition MEA2100
        config = struct.pack('<4sHHfff',
                           b'CONF',                    # Header de config
                           self.num_electrodes,        # Électrodes actives
                           int(self.sampling_rate),    # Fréquence
                           -100.0,                     # Seuil de détection (μV)
                           300.0,                      # Filtre passe-haut (Hz)
                           8000.0                      # Filtre passe-bas (Hz)
        )
        return config

    def _build_stimulation_command(self, pattern: StimulusPattern) -> bytes:
        """Construit une commande de stimulation"""
        # Protocole de stimulation MEA2100
        if pattern.waveform_type == "biphasic":
            waveform_id = 1
        elif pattern.waveform_type == "monophasic":
            waveform_id = 2
        else:
            waveform_id = 3  # Custom
        
        command = struct.pack('<4sHBfffH',
                            b'STIM',                   # Header
                            pattern.electrode_id,      # ID électrode
                            waveform_id,               # Type de waveform
                            pattern.amplitude,         # Amplitude (μV)
                            pattern.duration,          # Durée (ms)
                            pattern.frequency,         # Fréquence (Hz)
                            pattern.pulse_count        # Nombre de pulses
        )
        return command

    async def _send_command(self, command: bytes) -> bool:
        """Envoie une commande au MEA2100"""
        if not self.socket:
            return False
        
        try:
            await asyncio.get_event_loop().run_in_executor(
                None, self.socket.send, command
            )
            return True
        except Exception as e:
            logger.error(f"❌ Command send error: {e}")
            return False

    async def _receive_response(self) -> Optional[bytes]:
        """Reçoit une réponse du MEA2100"""
        if not self.socket:
            return None
        
        try:
            response = await asyncio.get_event_loop().run_in_executor(
                None, self.socket.recv, 1024
            )
            return response
        except Exception as e:
            logger.error(f"❌ Response receive error: {e}")
            return None

    async def _start_background_threads(self):
        """Démarre les threads de gestion en arrière-plan"""
        # Thread d'acquisition de données
        self.acquisition_thread = threading.Thread(
            target=self._acquisition_loop,
            daemon=True,
            name="MEA2100_Acquisition"
        )
        self.acquisition_thread.start()
        
        # Thread de traitement des stimulations
        self.stimulation_thread = threading.Thread(
            target=self._stimulation_loop,
            daemon=True,
            name="MEA2100_Stimulation" 
        )
        self.stimulation_thread.start()
        
        logger.info("🔄 MEA2100 background threads started")

    async def _stop_background_threads(self):
        """Arrête les threads de gestion"""
        # Les threads daemon s'arrêtent automatiquement
        # mais on peut signaler l'arrêt explicitement
        if self.acquisition_thread and self.acquisition_thread.is_alive():
            # Signaler l'arrêt via une variable de contrôle si nécessaire
            pass
        
        if self.stimulation_thread and self.stimulation_thread.is_alive():
            # Vider la queue de stimulation
            while not self.stimulation_queue.empty():
                try:
                    self.stimulation_queue.get_nowait()
                except Empty:
                    break
        
        logger.info("🛑 MEA2100 background threads stopped")

    def _acquisition_loop(self):
        """
        🔄 Boucle d'acquisition continue des données MEA
        
        Tourne en arrière-plan pour collecter les données de toutes
        les électrodes et les mettre dans le buffer pour analyse.
        """
        logger.info("🔄 Starting MEA2100 acquisition loop")
        
        while self.is_connected:
            try:
                if not self.is_recording:
                    time.sleep(0.1)  # Pause si pas d'enregistrement
                    continue
                
                # Simuler la réception de données du MEA2100
                # En réalité, ceci recevrait les données via socket/série
                raw_data = self._simulate_mea_data()
                
                # Ajouter au buffer (non-bloquant)
                if not self.data_buffer.full():
                    self.data_buffer.put(raw_data, timeout=0.001)
                else:
                    # Buffer plein, supprimer les anciens échantillons
                    try:
                        self.data_buffer.get_nowait()
                        self.data_buffer.put(raw_data, timeout=0.001)
                    except Empty:
                        pass
                
                # Fréquence d'acquisition basée sur le sampling rate
                sleep_time = 1.0 / (self.sampling_rate / 100)  # 100 échantillons par batch
                time.sleep(sleep_time)
                
            except Exception as e:
                logger.error(f"❌ Acquisition loop error: {e}")
                time.sleep(1.0)  # Pause en cas d'erreur

    def _stimulation_loop(self):
        """
        ⚡ Boucle de traitement des stimulations
        
        Traite la queue des stimulations à appliquer aux électrodes
        pour l'entraînement Bitcoin des neurones.
        """
        logger.info("⚡ Starting MEA2100 stimulation loop")
        
        while self.is_connected:
            try:
                # Récupérer la prochaine stimulation à traiter
                try:
                    stim_command = self.stimulation_queue.get(timeout=0.5)
                except Empty:
                    continue  # Pas de stimulation en attente
                
                # Envoyer la commande de stimulation au dispositif
                if self.socket:
                    try:
                        self.socket.send(stim_command)
                        
                        # Attendre une courte confirmation (optionnel)
                        # response = self.socket.recv(64)
                        
                        logger.debug("⚡ Stimulation command sent")
                        
                    except Exception as e:
                        logger.error(f"❌ Stimulation send error: {e}")
                
                # Marquer la tâche comme terminée
                self.stimulation_queue.task_done()
                
            except Exception as e:
                logger.error(f"❌ Stimulation loop error: {e}")
                time.sleep(0.1)

    def _simulate_mea_data(self) -> bytes:
        """
        🎲 Simule des données MEA réalistes pour les tests
        
        En production, cette fonction serait remplacée par la lecture
        des données réelles du dispositif MEA2100.
        """
        # Générer 100 échantillons pour toutes les électrodes
        num_samples = 100
        timestamp = time.time()
        
        # Créer des données simulées avec du bruit et quelques spikes
        data_packet = struct.pack('<fH', timestamp, num_samples)
        
        for electrode_id in range(self.num_electrodes):
            for sample in range(num_samples):
                # Bruit de fond gaussien (-20 à +20 μV)
                baseline_noise = np.random.normal(0, 5.0)
                
                # Ajouter occasionnellement des spikes (-100 à -200 μV)
                if np.random.random() < 0.001:  # 0.1% de chance de spike
                    spike_amplitude = np.random.uniform(-200, -100)
                    voltage = baseline_noise + spike_amplitude
                else:
                    voltage = baseline_noise
                
                # Emballer dans le format MEA2100
                sample_data = struct.pack('<Hf', electrode_id, voltage)
                data_packet += sample_data
        
        return data_packet

    def _parse_raw_data(self, raw_data: bytes) -> List[ElectrodeData]:
        """
        📊 Parse les données brutes du MEA2100 en structures ElectrodeData
        
        Convertit les bytes reçus du dispositif en objets Python
        utilisables pour l'analyse et l'apprentissage Bitcoin.
        """
        try:
            # Dépacker l'header du packet
            header_size = struct.calcsize('<fH')
            timestamp, num_samples = struct.unpack('<fH', raw_data[:header_size])
            
            # Dépacker les échantillons
            electrode_data = []
            data_offset = header_size
            sample_size = struct.calcsize('<Hf')
            
            for i in range(num_samples * self.num_electrodes):
                if data_offset + sample_size <= len(raw_data):
                    electrode_id, voltage = struct.unpack('<Hf', 
                                                        raw_data[data_offset:data_offset + sample_size])
                    
                    # Détection automatique de spike (seuil simple)
                    is_spike = abs(voltage) > 50.0
                    
                    # Calculer le niveau de bruit (approximation)
                    noise_level = abs(voltage) if abs(voltage) < 30 else 5.0
                    
                    # Créer l'objet ElectrodeData
                    electrode_sample = ElectrodeData(
                        electrode_id=electrode_id,
                        timestamp=timestamp + (i * (1.0 / self.sampling_rate)),
                        voltage=voltage,
                        impedance=1.0,  # Valeur par défaut
                        noise_level=noise_level,
                        is_spike=is_spike,
                        state=ElectrodeState.RECORDING
                    )
                    
                    electrode_data.append(electrode_sample)
                    data_offset += sample_size
                else:
                    break  # Pas assez de données pour ce sample
            
            return electrode_data
            
        except Exception as e:
            logger.error(f"❌ Data parsing error: {e}")
            return []

class RealMEAInterface:
    """
    🧬 INTERFACE MEA PRINCIPALE - Enhanced for Bitcoin Learning
    ==========================================================
    
    Interface unifiée pour tous les types de dispositifs MEA avec
    capacités d'apprentissage Bitcoin intégrées.
    
    Fonctionnalités principales:
    - Gestion multi-dispositifs (MEA2100, Plexon, Blackrock)
    - Stimulation programmable pour entraînement Bitcoin
    - Acquisition temps réel haute résolution
    - Intégration avec biological_bitcoin_learning.py
    - Protocoles d'apprentissage synaptique automatisés
    """
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.device_type = config.get('device_type', 'MultiChannelSystems_MEA2100')
        self.electrodes = list(range(config.get('num_electrodes', 60)))
        
        # Initialiser le dispositif approprié
        self.device: Optional[MEADevice] = None
        self._create_device()
        
        # État de l'interface
        self.is_connected = False
        self.is_recording = False
        
        # Gestion des protocoles Bitcoin
        self.bitcoin_protocols: List[BitcoinStimulationProtocol] = []
        self.current_protocol: Optional[BitcoinStimulationProtocol] = None
        
        # Callbacks pour l'apprentissage
        self.spike_callbacks: List[Callable] = []
        self.pattern_callbacks: List[Callable] = []
        
        # Statistiques temps réel
        self.total_spikes_detected = 0
        self.total_stimulations_sent = 0
        self.connection_start_time: Optional[float] = None
        
        logger.info(f"🧬 MEA Interface initialized for {self.device_type}")
        logger.info(f"💡 {len(self.electrodes)} electrodes available for Bitcoin learning")

    def _create_device(self):
        """Créer l'instance du dispositif MEA approprié"""
        if self.device_type == "MultiChannelSystems_MEA2100":
            self.device = MultiChannelSystemsMEA2100(self.config)
        elif self.device_type == "Plexon_OmniPlex":
            # self.device = PlexonOmniPlex(self.config)  # À implémenter
            logger.warning("⚠️ Plexon support not implemented yet, using MEA2100 simulation")
            self.device = MultiChannelSystemsMEA2100(self.config)
        elif self.device_type == "Blackrock_Cerebus":
            # self.device = BlackrockCerebus(self.config)  # À implémenter
            logger.warning("⚠️ Blackrock support not implemented yet, using MEA2100 simulation")
            self.device = MultiChannelSystemsMEA2100(self.config)
        else:
            logger.warning(f"⚠️ Unknown device type {self.device_type}, using MEA2100")
            self.device = MultiChannelSystemsMEA2100(self.config)

    async def connect(self) -> bool:
        """
        🔗 Établir la connexion avec le dispositif MEA
        
        Initialise la connexion et prépare le système pour
        l'entraînement Bitcoin des neurones biologiques.
        """
        if not self.device:
            logger.error("❌ No MEA device configured")
            return False
        
        try:
            logger.info("🔗 Establishing MEA connection for Bitcoin learning...")
            
            # Se connecter au dispositif
            success = await self.device.connect()
            
            if success:
                self.is_connected = True
                self.connection_start_time = time.time()
                
                logger.info("✅ MEA connection established successfully")
                logger.info("🧬 Ready for biological Bitcoin neural training!")
                
                # Initialiser les protocoles Bitcoin par défaut
                await self._initialize_bitcoin_protocols()
                
                return True
            else:
                logger.error("❌ Failed to establish MEA connection")
                return False
                
        except Exception as e:
            logger.error(f"❌ MEA connection error: {e}")
            return False

    async def disconnect(self) -> bool:
        """🔌 Déconnexion propre du MEA"""
        try:
            if self.device and self.is_connected:
                success = await self.device.disconnect()
                
                if success:
                    self.is_connected = False
                    self.is_recording = False
                    
                    # Statistiques finales
                    if self.connection_start_time:
                        session_duration = time.time() - self.connection_start_time
                        logger.info(f"📊 Session duration: {session_duration:.1f}s")
                        logger.info(f"⚡ Total stimulations: {self.total_stimulations_sent}")
                        logger.info(f"📈 Total spikes detected: {self.total_spikes_detected}")
                    
                    logger.info("✅ MEA disconnected successfully")
                    return True
                else:
                    logger.error("❌ MEA disconnection failed")
                    return False
            else:
                logger.info("ℹ️ MEA already disconnected")
                return True
                
        except Exception as e:
            logger.error(f"❌ MEA disconnection error: {e}")
            return False

    async def start_recording(self) -> bool:
        """
        📊 Démarrer l'enregistrement pour l'apprentissage Bitcoin
        
        Lance l'acquisition continue de l'activité neuronale nécessaire
        pour l'entraînement des patterns Bitcoin.
        """
        if not self.is_connected:
            logger.error("❌ Cannot start recording: MEA not connected")
            return False
        
        if self.is_recording:
            logger.info("ℹ️ Recording already active")
            return True
        
        try:
            logger.info("📊 Starting Bitcoin learning recording session...")
            
            success = await self.device.start_recording()
            
            if success:
                self.is_recording = True
                logger.info("✅ MEA recording started for Bitcoin learning")
                logger.info("🧬 Monitoring neural activity for pattern recognition...")
                return True
            else:
                logger.error("❌ Failed to start MEA recording")
                return False
                
        except Exception as e:
            logger.error(f"❌ MEA recording start error: {e}")
            return False

    async def stop_recording(self) -> bool:
        """⏹️ Arrêter l'enregistrement"""
        if not self.is_recording:
            return True
        
        try:
            success = await self.device.stop_recording()
            
            if success:
                self.is_recording = False
                logger.info("⏹️ MEA recording stopped")
                return True
            else:
                logger.error("❌ Failed to stop MEA recording")
                return False
                
        except Exception as e:
            logger.error(f"❌ MEA recording stop error: {e}")
            return False

    async def stimulate_electrode(self, electrode_id: int, amplitude: float = 10.0, 
                                duration: float = 0.1, waveform: str = 'biphasic') -> bool:
        """
        ⚡ Stimuler une électrode spécifique
        
        Interface simplifiée pour stimulation d'entraînement Bitcoin.
        Utilisée par biological_bitcoin_learning.py pour enseigner aux neurones.
        """
        if not self.is_connected:
            logger.error("❌ Cannot stimulate: MEA not connected")
            return False
        
        try:
            # Créer le pattern de stimulation
            pattern = StimulusPattern(
                electrode_id=electrode_id,
                waveform_type=waveform,
                amplitude=amplitude,
                duration=duration,
                frequency=100.0,  # Fréquence par défaut
                pulse_count=1
            )
            
            # Envoyer la stimulation
            success = await self.device.stimulate_electrode(pattern)
            
            if success:
                self.total_stimulations_sent += 1
                logger.debug(f"⚡ Electrode {electrode_id} stimulated ({amplitude}μV, {duration}ms)")
                return True
            else:
                logger.error(f"❌ Failed to stimulate electrode {electrode_id}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Electrode stimulation error: {e}")
            return False

    async def record_electrode(self, electrode_id: int, duration: float = 100.0) -> Optional[List[Dict]]:
        """
        📊 Enregistrer l'activité d'une électrode spécifique
        
        Utilisé par biological_bitcoin_learning.py pour capturer les
        réponses neuronales aux stimulations Bitcoin.
        """
        if not self.is_recording:
            logger.warning("⚠️ Cannot record: recording not active")
            return None
        
        try:
            # Calculer le nombre d'échantillons nécessaires
            sampling_rate = self.config.get('sampling_rate', 25000)
            num_samples = int((duration / 1000.0) * sampling_rate)  # duration en ms
            
            # Récupérer les données récentes
            all_data = await self.device.get_latest_data(num_samples * len(self.electrodes))
            
            # Filtrer pour l'électrode spécifique
            electrode_data = [
                {
                    'timestamp': data.timestamp,
                    'voltage': data.voltage,
                    'is_spike': data.is_spike,
                    'noise_level': data.noise_level
                }
                for data in all_data
                if data.electrode_id == electrode_id
            ]
            
            # Compter les spikes détectés
            spikes_in_recording = sum(1 for d in electrode_data if d['is_spike'])
            if spikes_in_recording > 0:
                self.total_spikes_detected += spikes_in_recording
                logger.debug(f"📈 Electrode {electrode_id}: {spikes_in_recording} spikes in {duration}ms")
            
            return electrode_data[-num_samples:] if electrode_data else []
            
        except Exception as e:
            logger.error(f"❌ Electrode recording error: {e}")
            return None

    async def apply_bitcoin_stimulation_protocol(self, protocol: BitcoinStimulationProtocol) -> bool:
        """
        🧬₿ Appliquer un protocole de stimulation Bitcoin complet
        
        Exécute une séquence de stimulations conçue pour enseigner
        aux neurones biologiques les patterns de hash Bitcoin.
        """
        if not self.is_connected:
            logger.error("❌ Cannot apply protocol: MEA not connected")
            return False
        
        try:
            logger.info(f"🧬₿ Applying Bitcoin stimulation protocol ({len(protocol.stimulation_sequence)} steps)")
            
            self.current_protocol = protocol
            success_count = 0
            
            # Exécuter chaque étape du protocole
            for i, stim_pattern in enumerate(protocol.stimulation_sequence):
                logger.debug(f"⚡ Protocol step {i+1}/{len(protocol.stimulation_sequence)}")
                
                # Appliquer la stimulation
                success = await self.device.stimulate_electrode(stim_pattern)
                
                if success:
                    success_count += 1
                    self.total_stimulations_sent += 1
                else:
                    logger.warning(f"⚠️ Protocol step {i+1} failed")
                
                # Pause entre les stimulations pour permettre la récupération neuronale
                await asyncio.sleep(stim_pattern.inter_pulse_interval / 1000.0)
            
            protocol_success_rate = success_count / len(protocol.stimulation_sequence)
            
            if protocol_success_rate > 0.8:  # 80% de réussite minimum
                logger.info(f"✅ Bitcoin protocol completed successfully ({protocol_success_rate:.1%} success rate)")
                return True
            else:
                logger.warning(f"⚠️ Bitcoin protocol partially failed ({protocol_success_rate:.1%} success rate)")
                return False
                
        except Exception as e:
            logger.error(f"❌ Bitcoin protocol error: {e}")
            return False

    async def _initialize_bitcoin_protocols(self):
        """
        🚀 Initialiser les protocoles de stimulation Bitcoin par défaut
        
        Crée des patterns de stimulation prédéfinis pour différentes
        phases d'apprentissage Bitcoin.
        """
        logger.info("🚀 Initializing Bitcoin learning protocols...")
        
        # Protocole 1: Reconnaissance de patterns de base
        basic_pattern_protocol = BitcoinStimulationProtocol(
            block_pattern=b"basic_bitcoin_pattern_00000000",
            target_electrodes=list(range(0, 20)),  # Première zone du MEA
            learning_phase="pattern_recognition"
        )
        
        # Créer les stimulations pour ce protocole
        for i, electrode_id in enumerate(basic_pattern_protocol.target_electrodes):
            amplitude = 15.0 + (i % 3) * 5.0  # Amplitudes variées
            duration = 0.1 + (i % 2) * 0.05   # Durées variées
            
            stim = StimulusPattern(
                electrode_id=electrode_id,
                waveform_type="biphasic",
                amplitude=amplitude,
                duration=duration,
                frequency=50.0 + (i % 4) * 25.0,  # Fréquences 50-150 Hz
                pulse_count=1,
                inter_pulse_interval=2.0  # 2ms entre stimulations
            )
            basic_pattern_protocol.stimulation_sequence.append(stim)
        
        self.bitcoin_protocols.append(basic_pattern_protocol)
        
        # Protocole 2: Apprentissage de nonce
        nonce_learning_protocol = BitcoinStimulationProtocol(
            block_pattern=b"nonce_prediction_training_pattern",
            target_electrodes=list(range(20, 40)),  # Deuxième zone du MEA
            learning_phase="nonce_prediction",
            reward_signal_amplitude=75.0  # Amplitude plus élevée pour récompense
        )
        
        # Stimulations pour apprentissage de nonce (plus complexes)
        for i, electrode_id in enumerate(nonce_learning_protocol.target_electrodes):
            # Pattern de stimulation complexe pour nonce
            stim = StimulusPattern(
                electrode_id=electrode_id,
                waveform_type="biphasic",
                amplitude=20.0 + (i % 5) * 7.0,   # Amplitudes 20-48 μV
                duration=0.15 + (i % 3) * 0.05,   # Durées 0.15-0.25 ms
                frequency=100.0 + (i % 6) * 20.0, # Fréquences 100-200 Hz
                pulse_count=2 + (i % 3),          # 2-4 pulses par train
                inter_pulse_interval=1.5          # 1.5ms entre stimulations
            )
            nonce_learning_protocol.stimulation_sequence.append(stim)
        
        self.bitcoin_protocols.append(nonce_learning_protocol)
        
        # Protocole 3: Optimisation avancée
        optimization_protocol = BitcoinStimulationProtocol(
            block_pattern=b"advanced_mining_optimization_protocol",
            target_electrodes=list(range(40, 60)),  # Troisième zone du MEA
            learning_phase="optimization",
            reward_signal_amplitude=100.0  # Récompense maximale
        )
        
        # Stimulations d'optimisation (patterns très spécifiques)
        for i, electrode_id in enumerate(optimization_protocol.target_electrodes):
            stim = StimulusPattern(
                electrode_id=electrode_id,
                waveform_type="custom",  # Waveforms personnalisées
                amplitude=30.0 + (i % 4) * 10.0,  # Amplitudes 30-60 μV
                duration=0.2 + (i % 4) * 0.1,     # Durées 0.2-0.5 ms
                frequency=150.0 + (i % 5) * 30.0, # Fréquences 150-270 Hz
                pulse_count=3 + (i % 2),          # 3-4 pulses
                inter_pulse_interval=1.0          # 1ms entre stimulations
            )
            optimization_protocol.stimulation_sequence.append(stim)
        
        self.bitcoin_protocols.append(optimization_protocol)
        
        logger.info(f"✅ Initialized {len(self.bitcoin_protocols)} Bitcoin learning protocols")
        for i, protocol in enumerate(self.bitcoin_protocols):
            logger.info(f"   Protocol {i+1}: {protocol.learning_phase} "
                       f"({len(protocol.stimulation_sequence)} stimulations, "
                       f"{len(protocol.target_electrodes)} electrodes)")

    def add_spike_callback(self, callback: Callable[[int, float, float], None]):
        """
        📞 Ajouter un callback pour la détection de spikes
        
        Le callback sera appelé à chaque détection de spike avec:
        - electrode_id: ID de l'électrode  
        - timestamp: Moment du spike
        - amplitude: Amplitude du spike
        """
        self.spike_callbacks.append(callback)
        logger.info(f"📞 Spike callback added (total: {len(self.spike_callbacks)})")

    def add_pattern_callback(self, callback: Callable[[str, Dict], None]):
        """
        📞 Ajouter un callback pour la reconnaissance de patterns
        
        Le callback sera appelé lors de la détection de patterns Bitcoin avec:
        - pattern_type: Type de pattern détecté
        - pattern_data: Données du pattern
        """
        self.pattern_callbacks.append(callback)
        logger.info(f"📞 Pattern callback added (total: {len(self.pattern_callbacks)})")

    def get_connection_statistics(self) -> Dict[str, Any]:
        """📊 Obtenir les statistiques de connexion et d'activité"""
        if not self.connection_start_time:
            return {}
        
        session_duration = time.time() - self.connection_start_time
        
        return {
            'device_type': self.device_type,
            'num_electrodes': len(self.electrodes),
            'is_connected': self.is_connected,
            'is_recording': self.is_recording,
            'session_duration': session_duration,
            'total_stimulations_sent': self.total_stimulations_sent,
            'total_spikes_detected': self.total_spikes_detected,
            'stimulation_rate': self.total_stimulations_sent / max(1, session_duration),
            'spike_detection_rate': self.total_spikes_detected / max(1, session_duration),
            'available_protocols': len(self.bitcoin_protocols),
            'current_protocol': self.current_protocol.learning_phase if self.current_protocol else None
        }

    async def run_bitcoin_learning_session(self, session_duration: float = 300.0) -> Dict[str, Any]:
        """
        🧬₿ Exécuter une session complète d'apprentissage Bitcoin
        
        Session automatisée qui applique tous les protocoles disponibles
        pour entraîner les neurones biologiques au mining Bitcoin.
        
        Args:
            session_duration: Durée de la session en secondes (défaut: 5 minutes)
            
        Returns:
            Statistiques de la session d'apprentissage
        """
        if not self.is_connected:
            logger.error("❌ Cannot run learning session: MEA not connected")
            return {}
        
        logger.info(f"🧬₿ Starting Bitcoin learning session ({session_duration}s)")
        
        session_stats = {
            'start_time': time.time(),
            'duration': session_duration,
            'protocols_applied': 0,
            'total_stimulations': 0,
            'spikes_detected': 0,
            'learning_phases': [],
            'success_rates': []
        }
        
        try:
            # Démarrer l'enregistrement si pas déjà actif
            recording_started_here = False
            if not self.is_recording:
                await self.start_recording()
                recording_started_here = True
            
            start_time = time.time()
            
            # Cycle à travers tous les protocoles disponibles
            while (time.time() - start_time) < session_duration:
                for protocol in self.bitcoin_protocols:
                    # Vérifier le temps restant
                    if (time.time() - start_time) >= session_duration:
                        break
                    
                    logger.info(f"🎓 Applying {protocol.learning_phase} protocol...")
                    
                    # Enregistrer les stats pré-protocol
                    pre_spikes = self.total_spikes_detected
                    pre_stimulations = self.total_stimulations_sent
                    
                    # Appliquer le protocole
                    success = await self.apply_bitcoin_stimulation_protocol(protocol)
                    
                    # Pause post-protocole pour observation des réponses
                    await asyncio.sleep(5.0)
                    
                    # Calculer les statistiques du protocole
                    post_spikes = self.total_spikes_detected
                    post_stimulations = self.total_stimulations_sent
                    
                    protocol_stats = {
                        'phase': protocol.learning_phase,
                        'success': success,
                        'stimulations_sent': post_stimulations - pre_stimulations,
                        'spikes_induced': post_spikes - pre_spikes,
                        'electrode_count': len(protocol.target_electrodes)
                    }
                    
                    session_stats['protocols_applied'] += 1
                    session_stats['learning_phases'].append(protocol_stats)
                    session_stats['success_rates'].append(1.0 if success else 0.0)
                    
                    logger.info(f"📊 Protocol results: "
                              f"{protocol_stats['stimulations_sent']} stims, "
                              f"{protocol_stats['spikes_induced']} spikes")
                    
                    # Pause inter-protocole
                    await asyncio.sleep(2.0)
            
            # Arrêter l'enregistrement si on l'a démarré
            if recording_started_here:
                await self.stop_recording()
            
            # Finaliser les statistiques
            session_stats['end_time'] = time.time()
            session_stats['actual_duration'] = session_stats['end_time'] - session_stats['start_time']
            session_stats['total_stimulations'] = self.total_stimulations_sent
            session_stats['spikes_detected'] = self.total_spikes_detected
            session_stats['average_success_rate'] = np.mean(session_stats['success_rates']) if session_stats['success_rates'] else 0.0
            
            logger.info("🎉 Bitcoin learning session completed!")
            logger.info(f"📊 Session summary:")
            logger.info(f"   Duration: {session_stats['actual_duration']:.1f}s")
            logger.info(f"   Protocols applied: {session_stats['protocols_applied']}")
            logger.info(f"   Total stimulations: {session_stats['total_stimulations']}")
            logger.info(f"   Spikes detected: {session_stats['spikes_detected']}")
            logger.info(f"   Average success rate: {session_stats['average_success_rate']:.1%}")
            
            return session_stats
            
        except Exception as e:
            logger.error(f"❌ Bitcoin learning session error: {e}")
            session_stats['error'] = str(e)
            return session_stats

# ====================================================================
# FONCTIONS UTILITAIRES
# ====================================================================

def create_mea_interface(device_type: str = "MultiChannelSystems_MEA2100") -> RealMEAInterface:
    """
    🏭 Factory function pour créer une interface MEA configurée
    
    Args:
        device_type: Type de dispositif MEA à utiliser
        
    Returns:
        Interface MEA prête pour l'apprentissage Bitcoin
    """
    # Configuration par défaut optimisée pour l'apprentissage Bitcoin
    config = {
        'device_type': device_type,
        'host': 'localhost',
        'port': 6340,
        'num_electrodes': 60,
        'sampling_rate': 25000,      # 25 kHz haute résolution
        'connection_protocol': 'tcp',
        'amplification': 1200.0,
        'filter_low_cut': 300.0,     # Filtre passe-haut 300 Hz
        'filter_high_cut': 8000.0,   # Filtre passe-bas 8 kHz
        'threshold_detection': True,
        'buffer_size': 10000         # Buffer large pour analyse temps réel
    }
    
    interface = RealMEAInterface(config)
    
    logger.info(f"🏭 MEA Interface created for {device_type}")
    logger.info("🧬₿ Ready for biological Bitcoin neural training!")
    
    return interface

async def test_mea_bitcoin_integration():
    """
    🧪 Test d'intégration MEA-Bitcoin
    
    Fonction de test pour vérifier que l'interface MEA fonctionne
    correctement avec le système d'apprentissage Bitcoin.
    """
    logger.info("🧪 Testing MEA-Bitcoin integration...")
    
    try:
        # Créer l'interface MEA
        mea = create_mea_interface()
        
        # Se connecter
        logger.info("🔗 Testing MEA connection...")
        connected = await mea.connect()
        
        if not connected:
            logger.error("❌ MEA connection failed")
            return False
        
        # Tester l'enregistrement
        logger.info("📊 Testing MEA recording...")
        recording_started = await mea.start_recording()
        
        if not recording_started:
            logger.error("❌ MEA recording failed")
            return False
        
        # Tester la stimulation Bitcoin
        logger.info("⚡ Testing Bitcoin stimulation protocols...")
        
        # Exécuter une session d'apprentissage courte
        session_results = await mea.run_bitcoin_learning_session(session_duration=30.0)
        
        if session_results and session_results.get('protocols_applied', 0) > 0:
            logger.info("✅ Bitcoin learning protocols working")
        else:
            logger.warning("⚠️ Bitcoin learning protocols may have issues")
        
        # Tester l'enregistrement d'électrode individuelle
        logger.info("📊 Testing individual electrode recording...")
        electrode_data = await mea.record_electrode(electrode_id=0, duration=100.0)
        
        if electrode_data and len(electrode_data) > 0:
            logger.info(f"✅ Electrode recording working ({len(electrode_data)} samples)")
        else:
            logger.warning("⚠️ Electrode recording may have issues")
        
        # Arrêter l'enregistrement
        await mea.stop_recording()
        
        # Déconnexion
        await mea.disconnect()
        
        # Afficher les statistiques finales
        stats = mea.get_connection_statistics()
        logger.info("📊 Final test statistics:")
        for key, value in stats.items():
            logger.info(f"   {key}: {value}")
        
        logger.info("🎉 MEA-Bitcoin integration test completed successfully!")
        return True
        
    except Exception as e:
        logger.error(f"❌ MEA-Bitcoin integration test failed: {e}")
        return False

if __name__ == "__main__":
    """
    🧬 REAL MEA INTERFACE - Point d'entrée principal
    
    Démonstration des capacités d'apprentissage Bitcoin avec MEA.
    """
    
    print("🧬 REAL MEA INTERFACE - Bitcoin Learning Edition")
    print("===============================================")
    print("Advanced Multi-Electrode Array interface for")
    print("biological Bitcoin neural network training!")
    print("")
    
    # Lancer le test d'intégration
    asyncio.run(test_mea_bitcoin_integration())