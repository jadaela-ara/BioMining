#!/usr/bin/env python3
"""
🧬₿ BIOLOGICAL BITCOIN LEARNING INTERFACE
=========================================

Interface révolutionnaire pour apprendre aux neurones biologiques 
connectés au MEA à miner Bitcoin et prédire des nonces.

Utilise la plasticité synaptique et l'apprentissage Hebbian pour 
entraîner les neurones à reconnaître les patterns Bitcoin et 
optimiser le processus de mining.

Author: BioMining Team
Version: 2.0.0 - Biological Neural Bitcoin Predictor
"""

import numpy as np
import asyncio
import hashlib
import struct
import time
import json
import logging
from dataclasses import dataclass, field
from typing import List, Optional, Dict, Tuple
from enum import Enum
import threading
from queue import Queue
import matplotlib.pyplot as plt

# Configuration logging pour le training biologique
logging.basicConfig(
    level=logging.INFO,
    format='[%(asctime)s] 🧬 %(levelname)s: %(message)s',
    datefmt='%H:%M:%S'
)
logger = logging.getLogger(__name__)

class NeuralLearningPhase(Enum):
    """Phases d'apprentissage des neurones biologiques"""
    INITIALIZATION = "initialization"
    PATTERN_RECOGNITION = "pattern_recognition" 
    BITCOIN_TRAINING = "bitcoin_training"
    NONCE_PREDICTION = "nonce_prediction"
    OPTIMIZATION = "optimization"

@dataclass
class BiologicalNeuralState:
    """État des neurones biologiques en temps réel"""
    electrode_id: int
    firing_rate: float  # Hz
    spike_amplitude: float  # μV
    inter_spike_interval: float  # ms
    synaptic_strength: float  # Poids synaptique adaptatif
    learning_coefficient: float = 0.001
    last_stimulation: float = 0.0
    bitcoin_response_score: float = 0.0  # Score de réponse aux patterns Bitcoin

@dataclass 
class BitcoinLearningTarget:
    """Cible d'apprentissage pour les neurones"""
    block_header: bytes
    target_hash: str
    expected_nonce: int
    difficulty: float
    pattern_signature: np.ndarray  # Signature électrique du pattern
    reward_signal: float = 1.0

class BiologicalBitcoinLearner:
    """
    🧬₿ Système d'apprentissage Bitcoin pour neurones biologiques
    
    Utilise l'interface MEA pour:
    1. Stimuler les neurones avec des patterns Bitcoin
    2. Mesurer leurs réponses et adapter les connexions synaptiques  
    3. Entraîner la prédiction de nonce via reinforcement learning
    4. Optimiser le mining via la plasticité neuronale
    """
    
    def __init__(self, mea_interface):
        self.mea = mea_interface
        self.neurons: Dict[int, BiologicalNeuralState] = {}
        self.learning_phase = NeuralLearningPhase.INITIALIZATION
        self.learning_history = []
        
        # Configuration Bitcoin learning
        self.bitcoin_patterns = Queue(maxsize=1000)
        self.successful_predictions = 0
        self.total_attempts = 0
        self.current_block_template = None
        
        # Matrices d'apprentissage synaptique
        self.synaptic_matrix = np.zeros((60, 60))  # 60x60 electrodes
        self.plasticity_matrix = np.ones((60, 60)) * 0.5
        
        # Monitoring biologique
        self.neural_activity_buffer = np.zeros((60, 10000))  # Buffer circulaire
        self.buffer_index = 0
        
        # Bitcoin mining state
        self.mining_active = False
        self.current_difficulty = 1.0
        self.nonce_predictions = []
        
        logger.info("🧬 Biological Bitcoin Learner initialized")
        logger.info(f"💡 {len(self.mea.electrodes)} electrodes available for learning")

    async def initialize_biological_learning(self):
        """
        🚀 Initialise l'apprentissage biologique Bitcoin
        
        1. Calibre les neurones avec des stimulations de base
        2. Établit les connexions synaptiques initiales
        3. Prépare les patterns Bitcoin pour l'entraînement
        """
        logger.info("🧬 Initializing biological Bitcoin learning system...")
        
        self.learning_phase = NeuralLearningPhase.INITIALIZATION
        
        # Initialiser l'état des neurones
        for electrode_id in range(60):
            self.neurons[electrode_id] = BiologicalNeuralState(
                electrode_id=electrode_id,
                firing_rate=0.0,
                spike_amplitude=0.0,
                inter_spike_interval=float('inf'),
                synaptic_strength=np.random.normal(0.5, 0.1)
            )
        
        # Calibration initiale avec stimulations douces
        await self._calibrate_neural_baseline()
        
        # Établir les connexions synaptiques initiales
        await self._initialize_synaptic_connections()
        
        logger.info("✅ Biological learning system ready for Bitcoin training")

    async def _calibrate_neural_baseline(self):
        """Calibre la ligne de base de l'activité neuronale"""
        logger.info("🔬 Calibrating neural baseline activity...")
        
        # Stimulations de calibration sur tous les électrodes
        for electrode_id in range(60):
            # Stimulation douce pour évaluer la réactivité
            await self.mea.stimulate_electrode(
                electrode_id, 
                amplitude=10.0,  # μV
                duration=0.1,    # ms
                waveform='biphasic'
            )
            
            # Mesurer la réponse
            response = await self.mea.record_electrode(electrode_id, duration=50.0)
            
            if response and len(response) > 0:
                # Analyser l'activité spontanée
                baseline_rate = self._calculate_firing_rate(response)
                self.neurons[electrode_id].firing_rate = baseline_rate
                
                # Calculer l'amplitude moyenne des spikes
                spikes = self._detect_spikes(response)
                if spikes:
                    self.neurons[electrode_id].spike_amplitude = np.mean([s['amplitude'] for s in spikes])
        
        logger.info("📊 Neural baseline established for all electrodes")

    async def _initialize_synaptic_connections(self):
        """Initialise les connexions synaptiques entre électrodes"""
        logger.info("🔗 Initializing synaptic connection matrix...")
        
        # Créer une matrice de connectivité basée sur la proximité physique
        for i in range(60):
            for j in range(60):
                if i != j:
                    # Distance physique sur le MEA (supposé 8x8 avec quelques électrodes de référence)
                    distance = self._calculate_electrode_distance(i, j)
                    
                    # Connexion synaptique inversement proportionnelle à la distance
                    base_strength = 1.0 / (1.0 + distance * 0.1)
                    noise = np.random.normal(0, 0.05)
                    
                    self.synaptic_matrix[i][j] = max(0, base_strength + noise)
                    self.plasticity_matrix[i][j] = np.random.uniform(0.3, 0.7)
        
        logger.info("🧠 Synaptic connections initialized with distance-based weights")

    def _calculate_electrode_distance(self, electrode1: int, electrode2: int) -> float:
        """Calcule la distance physique entre deux électrodes sur le MEA"""
        # Supposons un arrangement 8x8 des électrodes
        row1, col1 = electrode1 // 8, electrode1 % 8
        row2, col2 = electrode2 // 8, electrode2 % 8
        
        return np.sqrt((row1 - row2)**2 + (col1 - col2)**2)

    async def train_bitcoin_pattern_recognition(self, num_epochs: int = 1000):
        """
        🎯 Entraîne les neurones à reconnaître les patterns Bitcoin
        
        Utilise la stimulation directe pour associer les patterns de hash
        Bitcoin avec des réponses neuronales spécifiques.
        """
        logger.info(f"🎓 Starting Bitcoin pattern recognition training ({num_epochs} epochs)")
        
        self.learning_phase = NeuralLearningPhase.PATTERN_RECOGNITION
        
        for epoch in range(num_epochs):
            # Générer un block template Bitcoin réaliste
            block_template = self._generate_bitcoin_block_template()
            
            # Convertir le pattern en stimulation électrique
            stimulation_pattern = self._bitcoin_to_electrical_pattern(block_template)
            
            # Appliquer le pattern aux neurones
            response = await self._apply_learning_stimulation(stimulation_pattern)
            
            # Évaluer la qualité de la réponse
            pattern_score = self._evaluate_pattern_recognition(response, block_template)
            
            # Adapter les connexions synaptiques (Hebbian learning)
            await self._update_synaptic_weights(response, pattern_score)
            
            # Logging du progrès
            if epoch % 100 == 0:
                avg_score = np.mean([n.bitcoin_response_score for n in self.neurons.values()])
                logger.info(f"📈 Epoch {epoch}: Pattern recognition score = {avg_score:.3f}")
                
                # Sauvegarder l'historique d'apprentissage
                self.learning_history.append({
                    'epoch': epoch,
                    'phase': self.learning_phase.value,
                    'average_score': avg_score,
                    'best_neuron': max(self.neurons.keys(), key=lambda x: self.neurons[x].bitcoin_response_score),
                    'synaptic_strength': np.mean(self.synaptic_matrix)
                })
        
        logger.info("✅ Bitcoin pattern recognition training completed")

    def _generate_bitcoin_block_template(self) -> Dict:
        """Génère un template de block Bitcoin réaliste pour l'entraînement"""
        return {
            'version': 0x20000000,
            'previous_hash': hashlib.sha256(f"prev_block_{time.time()}".encode()).hexdigest(),
            'merkle_root': hashlib.sha256(f"transactions_{time.time()}".encode()).hexdigest(), 
            'timestamp': int(time.time()),
            'difficulty_bits': 0x1d00ffff,  # Difficulté d'entraînement
            'nonce': 0  # À prédire !
        }

    def _bitcoin_to_electrical_pattern(self, block_template: Dict) -> np.ndarray:
        """
        🔌 Convertit un block Bitcoin en pattern de stimulation électrique
        
        Transforme les données du block (hash, nonce, etc.) en signaux
        électriques appropriés pour stimuler les neurones biologiques.
        """
        # Créer le header du block
        header = struct.pack('<L', block_template['version'])
        header += bytes.fromhex(block_template['previous_hash'])
        header += bytes.fromhex(block_template['merkle_root'])
        header += struct.pack('<L', block_template['timestamp'])
        header += struct.pack('<L', block_template['difficulty_bits'])
        header += struct.pack('<L', block_template['nonce'])
        
        # Convertir en pattern électrique
        # Chaque byte du header influence l'amplitude et la fréquence de stimulation
        pattern = np.zeros((60, 100))  # 60 electrodes, 100ms de stimulation
        
        for i, byte_val in enumerate(header[:60]):  # Limiter aux 60 premiers bytes
            # Amplitude basée sur la valeur du byte
            amplitude = (byte_val / 255.0) * 50.0  # 0-50 μV
            
            # Fréquence basée sur les bits
            frequency = 10 + (byte_val & 0x0F) * 5  # 10-85 Hz
            
            # Générer le signal sinusoïdal
            time_points = np.linspace(0, 0.1, 100)  # 100ms
            signal = amplitude * np.sin(2 * np.pi * frequency * time_points)
            
            pattern[i] = signal
        
        return pattern

    async def _apply_learning_stimulation(self, pattern: np.ndarray) -> Dict[int, List]:
        """Applique le pattern de stimulation et enregistre les réponses"""
        responses = {}
        
        # Stimuler tous les électrodes simultanément avec le pattern
        stimulation_tasks = []
        
        for electrode_id in range(60):
            if electrode_id < pattern.shape[0]:
                # Créer la tâche de stimulation asynchrone
                task = self._stimulate_with_pattern(electrode_id, pattern[electrode_id])
                stimulation_tasks.append(task)
        
        # Exécuter toutes les stimulations en parallèle
        await asyncio.gather(*stimulation_tasks)
        
        # Enregistrer les réponses post-stimulation
        for electrode_id in range(60):
            response = await self.mea.record_electrode(electrode_id, duration=200.0)  # 200ms post-stimulation
            responses[electrode_id] = response if response else []
            
            # Mettre à jour le buffer circulaire d'activité
            if response and len(response) > 0:
                activity_level = self._calculate_activity_strength(response)
                self.neural_activity_buffer[electrode_id][self.buffer_index % 10000] = activity_level
        
        self.buffer_index += 1
        return responses

    async def _stimulate_with_pattern(self, electrode_id: int, signal: np.ndarray):
        """Applique un signal de stimulation complexe à un électrode"""
        # Appliquer le signal point par point (simulation - en réalité ce serait via DAC)
        for i, amplitude in enumerate(signal):
            if abs(amplitude) > 1.0:  # Seuil minimal de stimulation
                await self.mea.stimulate_electrode(
                    electrode_id,
                    amplitude=abs(amplitude),
                    duration=1.0,  # 1ms par point
                    waveform='custom'
                )
                await asyncio.sleep(0.001)  # 1ms entre points

    def _evaluate_pattern_recognition(self, responses: Dict[int, List], block_template: Dict) -> float:
        """Évalue la qualité de la reconnaissance de pattern par les neurones"""
        total_score = 0.0
        active_neurons = 0
        
        # Hash du block pour créer la signature de référence
        target_hash = self._calculate_block_hash(block_template)
        target_signature = self._hash_to_signature(target_hash)
        
        for electrode_id, response in responses.items():
            if response and len(response) > 0:
                # Calculer la signature de la réponse neuronale
                neural_signature = self._response_to_signature(response)
                
                # Comparer avec la signature cible (corrélation)
                correlation = np.corrcoef(neural_signature, target_signature)[0, 1]
                if not np.isnan(correlation):
                    score = max(0, correlation)  # Corrélation positive seulement
                    
                    # Mettre à jour le score du neurone
                    self.neurons[electrode_id].bitcoin_response_score = score
                    total_score += score
                    active_neurons += 1
        
        return total_score / max(1, active_neurons)

    def _calculate_block_hash(self, block_template: Dict) -> str:
        """Calcule le hash SHA-256 du block header"""
        header = struct.pack('<L', block_template['version'])
        header += bytes.fromhex(block_template['previous_hash'])  
        header += bytes.fromhex(block_template['merkle_root'])
        header += struct.pack('<L', block_template['timestamp'])
        header += struct.pack('<L', block_template['difficulty_bits'])
        header += struct.pack('<L', block_template['nonce'])
        
        return hashlib.sha256(hashlib.sha256(header).digest()).hexdigest()

    def _hash_to_signature(self, hash_str: str) -> np.ndarray:
        """Convertit un hash en signature numérique pour comparaison"""
        # Utiliser les premiers 32 caractères du hash
        hash_bytes = bytes.fromhex(hash_str[:32])
        return np.array([b / 255.0 for b in hash_bytes], dtype=np.float32)

    def _response_to_signature(self, response: List) -> np.ndarray:
        """Convertit une réponse neuronale en signature pour comparaison"""
        if len(response) < 16:
            return np.zeros(16, dtype=np.float32)
        
        # Extraire les caractéristiques temporelles et spectrales
        signal = np.array([r['voltage'] if isinstance(r, dict) else r for r in response[:1000]])
        
        # Calculer les bins de distribution d'amplitude
        hist, _ = np.histogram(signal, bins=16, range=(-100, 100))
        signature = hist / (np.sum(hist) + 1e-10)  # Normaliser
        
        return signature.astype(np.float32)

    async def _update_synaptic_weights(self, responses: Dict[int, List], pattern_score: float):
        """
        🧠 Met à jour les poids synaptiques selon l'apprentissage Hebbian
        
        "Neurons that fire together, wire together"
        Les connexions entre neurones qui répondent bien aux patterns Bitcoin
        sont renforcées.
        """
        # Calculer l'activité de chaque neurone
        neuron_activities = {}
        
        for electrode_id, response in responses.items():
            if response and len(response) > 0:
                activity = self._calculate_activity_strength(response)
                neuron_activities[electrode_id] = activity
                
                # Mettre à jour le coefficient d'apprentissage adaptatif
                if pattern_score > 0.5:  # Bonne réponse
                    self.neurons[electrode_id].learning_coefficient *= 1.01  # Augmenter
                else:  # Mauvaise réponse  
                    self.neurons[electrode_id].learning_coefficient *= 0.99  # Diminuer
                
                # Borner le coefficient
                self.neurons[electrode_id].learning_coefficient = np.clip(
                    self.neurons[electrode_id].learning_coefficient, 0.0001, 0.01
                )
            else:
                neuron_activities[electrode_id] = 0.0
        
        # Mise à jour Hebbian des connexions synaptiques
        for i in range(60):
            for j in range(60):
                if i != j and i in neuron_activities and j in neuron_activities:
                    # Règle de Hebb: Δw = η * activité_i * activité_j
                    activity_i = neuron_activities[i] 
                    activity_j = neuron_activities[j]
                    
                    learning_rate = (self.neurons[i].learning_coefficient + 
                                   self.neurons[j].learning_coefficient) / 2
                    
                    # Renforcement si les deux neurones sont actifs
                    weight_change = learning_rate * activity_i * activity_j * pattern_score
                    
                    # Appliquer la plasticité
                    self.synaptic_matrix[i][j] += weight_change * self.plasticity_matrix[i][j]
                    
                    # Normaliser les poids (éviter l'explosion)
                    self.synaptic_matrix[i][j] = np.clip(self.synaptic_matrix[i][j], 0.0, 2.0)

    def _calculate_activity_strength(self, response: List) -> float:
        """Calcule la force d'activité d'une réponse neuronale"""
        if not response or len(response) == 0:
            return 0.0
        
        # Convertir en signal numérique
        if isinstance(response[0], dict):
            voltages = [r.get('voltage', 0) for r in response]
        else:
            voltages = list(response)
        
        if len(voltages) == 0:
            return 0.0
        
        # Calculer plusieurs métriques d'activité
        signal = np.array(voltages)
        
        # 1. Variance (activité globale)
        variance = np.var(signal)
        
        # 2. Nombre de pics (spikes)
        spikes = self._detect_spikes(response)
        spike_rate = len(spikes) / (len(voltages) * 0.001)  # spikes/sec
        
        # 3. Amplitude maximale
        max_amplitude = np.max(np.abs(signal)) if len(signal) > 0 else 0
        
        # Combinaison pondérée des métriques
        activity = (0.4 * variance + 0.4 * spike_rate + 0.2 * max_amplitude) / 100.0
        
        return min(1.0, activity)  # Normaliser à [0,1]

    async def train_nonce_prediction(self, num_training_blocks: int = 5000):
        """
        🎯 Entraîne les neurones à prédire les nonces Bitcoin
        
        Phase avancée où les neurones apprennent à associer les patterns
        de block header avec les nonces qui produisent des hash valides.
        """
        logger.info(f"🔮 Starting nonce prediction training ({num_training_blocks} blocks)")
        
        self.learning_phase = NeuralLearningPhase.NONCE_PREDICTION
        prediction_accuracy = []
        
        for block_num in range(num_training_blocks):
            # Générer un block avec un nonce solution connu
            block_template, solution_nonce = self._generate_training_block_with_solution()
            
            # Phase 1: Présenter le block header sans le nonce
            header_pattern = self._bitcoin_to_electrical_pattern(block_template)
            response = await self._apply_learning_stimulation(header_pattern)
            
            # Phase 2: Demander aux neurones de prédire le nonce
            predicted_nonce = self._neural_nonce_prediction(response)
            
            # Phase 3: Révéler la solution et calculer la récompense
            accuracy = self._calculate_nonce_accuracy(predicted_nonce, solution_nonce)
            
            # Phase 4: Apprentissage par renforcement
            await self._apply_reinforcement_learning(response, accuracy)
            
            prediction_accuracy.append(accuracy)
            
            # Logging du progrès
            if block_num % 500 == 0:
                recent_accuracy = np.mean(prediction_accuracy[-100:]) if len(prediction_accuracy) >= 100 else np.mean(prediction_accuracy)
                logger.info(f"🎯 Block {block_num}: Prediction accuracy = {recent_accuracy:.3f}")
                
                self.learning_history.append({
                    'block': block_num,
                    'phase': self.learning_phase.value,
                    'accuracy': recent_accuracy,
                    'predicted_nonce': predicted_nonce,
                    'actual_nonce': solution_nonce,
                    'neural_confidence': np.mean([n.bitcoin_response_score for n in self.neurons.values()])
                })
        
        final_accuracy = np.mean(prediction_accuracy[-1000:]) if len(prediction_accuracy) >= 1000 else np.mean(prediction_accuracy)
        logger.info(f"✅ Nonce prediction training completed. Final accuracy: {final_accuracy:.3f}")
        
        return final_accuracy

    def _generate_training_block_with_solution(self) -> Tuple[Dict, int]:
        """Génère un block d'entraînement avec une solution de nonce connue"""
        base_template = self._generate_bitcoin_block_template()
        
        # Réduire la difficulté pour l'entraînement (plus de solutions valides)
        base_template['difficulty_bits'] = 0x207fffff  # Difficulté très faible
        
        # Trouver un nonce valide par force brute (rapide avec faible difficulté)
        for nonce in range(1000000):  # Limiter la recherche
            base_template['nonce'] = nonce
            block_hash = self._calculate_block_hash(base_template)
            
            # Vérifier si le hash respecte la difficulté
            if block_hash.startswith('0000'):  # 4 zéros = difficulté faible
                return base_template, nonce
        
        # Fallback: retourner un nonce aléatoire
        return base_template, np.random.randint(0, 1000000)

    def _neural_nonce_prediction(self, responses: Dict[int, List]) -> int:
        """
        🔮 Utilise les réponses neuronales pour prédire un nonce
        
        Combine les activités neuronales selon les poids synaptiques
        appris pour générer une prédiction de nonce.
        """
        # Extraire les caractéristiques de chaque réponse neuronale
        neural_features = []
        
        for electrode_id in range(60):
            if electrode_id in responses and responses[electrode_id]:
                # Calculer les features temporelles
                response = responses[electrode_id]
                features = self._extract_prediction_features(response)
                neural_features.append(features)
            else:
                # Neurone inactif
                neural_features.append(np.zeros(8))
        
        # Combiner les features avec la matrice synaptique
        neural_vector = np.array(neural_features).flatten()  # 60 * 8 = 480 features
        
        # Projection via les poids synaptiques (réduction de dimension)
        projected = np.dot(self.synaptic_matrix.flatten(), neural_vector[:3600])  # Adapter la taille
        
        # Convertir en nonce (32-bit unsigned integer)
        # Utiliser une fonction de hachage pour mapper les activités neuronales
        feature_bytes = struct.pack('f', projected)
        hash_digest = hashlib.md5(feature_bytes).digest()
        nonce = struct.unpack('<I', hash_digest[:4])[0]
        
        # Limiter à une plage raisonnable pour l'entraînement
        nonce = nonce % 1000000
        
        return nonce

    def _extract_prediction_features(self, response: List) -> np.ndarray:
        """Extrait des features prédictives d'une réponse neuronale"""
        if not response or len(response) == 0:
            return np.zeros(8)
        
        # Convertir la réponse en signal
        if isinstance(response[0], dict):
            signal = np.array([r.get('voltage', 0) for r in response[:1000]])
        else:
            signal = np.array(response[:1000])
        
        if len(signal) == 0:
            return np.zeros(8)
        
        # Calculer diverses features temporelles et spectrales
        features = np.zeros(8)
        
        try:
            features[0] = np.mean(signal)           # Moyenne
            features[1] = np.std(signal)            # Écart-type  
            features[2] = np.max(signal)            # Maximum
            features[3] = np.min(signal)            # Minimum
            features[4] = len(self._detect_spikes(response))  # Nombre de spikes
            features[5] = np.sum(np.abs(np.diff(signal)))     # Variation totale
            
            # Features spectrales (FFT simplifié)
            if len(signal) > 10:
                fft = np.fft.fft(signal)
                features[6] = np.abs(fft[1])        # Composante basse fréquence
                features[7] = np.abs(fft[len(fft)//4])  # Composante moyenne fréquence
        
        except Exception as e:
            logger.warning(f"Feature extraction error: {e}")
            return np.zeros(8)
        
        return features

    def _calculate_nonce_accuracy(self, predicted: int, actual: int) -> float:
        """Calcule la précision de la prédiction de nonce"""
        # Différence absolue normalisée
        diff = abs(predicted - actual)
        max_diff = 1000000  # Plage maximale pour l'entraînement
        
        # Accuracy inversement proportionnelle à la différence
        accuracy = max(0.0, 1.0 - (diff / max_diff))
        
        # Bonus pour les prédictions très proches (dans les 1000)
        if diff < 1000:
            accuracy = min(1.0, accuracy + 0.1)
        
        # Bonus pour les prédictions exactes
        if diff == 0:
            accuracy = 1.0
        
        return accuracy

    async def _apply_reinforcement_learning(self, responses: Dict[int, List], accuracy: float):
        """
        🏆 Applique l'apprentissage par renforcement basé sur la précision
        
        Récompense/punition des neurones selon leur contribution à la
        prédiction de nonce correcte.
        """
        # Signal de récompense global
        reward_signal = (accuracy - 0.5) * 2  # Centrer sur [-1, 1]
        
        for electrode_id, response in responses.items():
            if response and len(response) > 0:
                neuron = self.neurons[electrode_id]
                
                # Calculer la contribution du neurone à la prédiction
                neuron_activity = self._calculate_activity_strength(response)
                contribution_score = neuron_activity * neuron.bitcoin_response_score
                
                # Mise à jour du score Bitcoin basée sur la récompense
                if reward_signal > 0:  # Bonne prédiction
                    # Renforcer les neurones qui ont contribué
                    score_update = reward_signal * contribution_score * 0.1
                    neuron.bitcoin_response_score += score_update
                    
                    # Augmenter légèrement la force synaptique
                    neuron.synaptic_strength *= 1.01
                    
                else:  # Mauvaise prédiction
                    # Diminuer le score des neurones très actifs (ils ont mal guidé)
                    if neuron_activity > 0.5:
                        neuron.bitcoin_response_score += reward_signal * 0.05
                    
                    # Diminuer la force synaptique
                    neuron.synaptic_strength *= 0.99
                
                # Borner les valeurs
                neuron.bitcoin_response_score = np.clip(neuron.bitcoin_response_score, 0.0, 2.0)
                neuron.synaptic_strength = np.clip(neuron.synaptic_strength, 0.1, 2.0)
        
        # Mise à jour adaptative de la matrice de plasticité
        for i in range(60):
            for j in range(60):
                if i != j:
                    # Augmenter la plasticité des connexions qui ont bien performé
                    if accuracy > 0.7:
                        self.plasticity_matrix[i][j] *= 1.001
                    else:
                        self.plasticity_matrix[i][j] *= 0.999
                    
                    # Borner la plasticité
                    self.plasticity_matrix[i][j] = np.clip(self.plasticity_matrix[i][j], 0.1, 1.0)

    async def start_biological_bitcoin_mining(self):
        """
        ⚡ Lance le mining Bitcoin avec assistance neuronale biologique !
        
        Utilise les neurones entraînés pour prédire des nonces et 
        optimiser le processus de mining en temps réel.
        """
        logger.info("⚡ Starting BIOLOGICAL Bitcoin mining with trained neurons!")
        
        self.learning_phase = NeuralLearningPhase.OPTIMIZATION
        self.mining_active = True
        
        mining_stats = {
            'blocks_attempted': 0,
            'neural_predictions': 0,
            'successful_predictions': 0,
            'mining_start_time': time.time()
        }
        
        while self.mining_active:
            try:
                # Générer un nouveau block candidat
                block_candidate = self._generate_bitcoin_block_template()
                mining_stats['blocks_attempted'] += 1
                
                logger.info(f"🧬 Mining block #{mining_stats['blocks_attempted']} with biological assistance")
                
                # Phase 1: Consultation des neurones biologiques
                neural_nonce_suggestions = await self._consult_biological_neurons(block_candidate)
                mining_stats['neural_predictions'] += len(neural_nonce_suggestions)
                
                # Phase 2: Test des suggestions neuronales
                for suggested_nonce in neural_nonce_suggestions[:10]:  # Tester les 10 meilleures
                    block_candidate['nonce'] = suggested_nonce
                    block_hash = self._calculate_block_hash(block_candidate)
                    
                    # Vérifier si c'est une solution valide
                    if self._is_valid_hash(block_hash, self.current_difficulty):
                        logger.info(f"🎉 BIOLOGICAL BITCOIN BLOCK FOUND!")
                        logger.info(f"💎 Block hash: {block_hash}")
                        logger.info(f"🧠 Neural nonce: {suggested_nonce}")
                        
                        mining_stats['successful_predictions'] += 1
                        
                        # Récompenser massivement les neurones pour cette réussite
                        await self._reward_successful_mining(block_candidate, block_hash)
                        
                        # Passer au block suivant
                        break
                
                # Phase 3: Mining traditionnel en complément (si pas de solution neuronale)
                else:
                    # Continuer avec mining traditionnel mais guidé par les neurones
                    await self._hybrid_mining_completion(block_candidate, neural_nonce_suggestions)
                
                # Statistiques périodiques
                if mining_stats['blocks_attempted'] % 10 == 0:
                    self._log_mining_statistics(mining_stats)
                    
                    # Adapter la difficulté d'entraînement
                    await self._adapt_mining_difficulty(mining_stats)
                
                # Pause courte avant le prochain block
                await asyncio.sleep(1.0)
                
            except Exception as e:
                logger.error(f"❌ Mining error: {e}")
                await asyncio.sleep(5.0)  # Pause plus longue en cas d'erreur
        
        logger.info("🛑 Biological Bitcoin mining stopped")

    async def _consult_biological_neurons(self, block_candidate: Dict) -> List[int]:
        """
        🧠 Consulte les neurones biologiques pour des suggestions de nonce
        
        Stimule les neurones avec le pattern du block et interprète
        leurs réponses comme des prédictions de nonce.
        """
        # Convertir le block en pattern de stimulation
        stimulation_pattern = self._bitcoin_to_electrical_pattern(block_candidate)
        
        # Appliquer le pattern et enregistrer les réponses
        responses = await self._apply_learning_stimulation(stimulation_pattern)
        
        # Collecter les prédictions de tous les neurones actifs
        nonce_suggestions = []
        confidence_scores = []
        
        for electrode_id, response in responses.items():
            if response and len(response) > 0:
                # Générer une prédiction de nonce basée sur la réponse
                predicted_nonce = self._neural_nonce_prediction({electrode_id: response})
                
                # Calculer la confiance basée sur le score Bitcoin du neurone
                confidence = self.neurons[electrode_id].bitcoin_response_score
                
                nonce_suggestions.append(predicted_nonce)
                confidence_scores.append(confidence)
        
        # Trier les suggestions par confiance (meilleures en premier)
        if nonce_suggestions:
            sorted_pairs = sorted(zip(nonce_suggestions, confidence_scores), 
                                key=lambda x: x[1], reverse=True)
            return [nonce for nonce, _ in sorted_pairs]
        else:
            return []

    def _is_valid_hash(self, block_hash: str, difficulty: float) -> bool:
        """Vérifie si un hash respecte la difficulté requise"""
        # Pour l'entraînement, utiliser une difficulté simple (nombre de zéros)
        required_zeros = max(1, int(difficulty))
        return block_hash.startswith('0' * required_zeros)

    async def _reward_successful_mining(self, block_candidate: Dict, winning_hash: str):
        """
        🏆 Récompense massivement les neurones pour un block trouvé
        
        Renforce dramatiquement les connexions neuronales qui ont
        contribué à trouver une solution Bitcoin valide.
        """
        logger.info("🏆 Rewarding neurons for successful Bitcoin mining!")
        
        # Signal de récompense maximal
        super_reward = 2.0
        
        # Recalculer les réponses pour ce block gagnant
        stimulation_pattern = self._bitcoin_to_electrical_pattern(block_candidate)
        responses = await self._apply_learning_stimulation(stimulation_pattern)
        
        for electrode_id, response in responses.items():
            if response and len(response) > 0:
                neuron = self.neurons[electrode_id]
                
                # Calculer la contribution à la réussite
                activity = self._calculate_activity_strength(response)
                contribution = activity * neuron.bitcoin_response_score
                
                # Récompense proportionnelle à la contribution
                reward_boost = super_reward * contribution
                
                # Mise à jour massive des scores et forces
                neuron.bitcoin_response_score += reward_boost * 0.5
                neuron.synaptic_strength += reward_boost * 0.2
                neuron.learning_coefficient += reward_boost * 0.001
                
                # Borner les valeurs améliorées
                neuron.bitcoin_response_score = min(5.0, neuron.bitcoin_response_score)
                neuron.synaptic_strength = min(5.0, neuron.synaptic_strength)
                neuron.learning_coefficient = min(0.1, neuron.learning_coefficient)
        
        # Renforcer massivement la matrice synaptique
        for i in range(60):
            for j in range(60):
                if i != j:
                    # Renforcement basé sur l'activité conjointe lors de la réussite
                    if (i in responses and j in responses and 
                        responses[i] and responses[j]):
                        
                        activity_i = self._calculate_activity_strength(responses[i])
                        activity_j = self._calculate_activity_strength(responses[j])
                        
                        reinforcement = super_reward * activity_i * activity_j * 0.1
                        self.synaptic_matrix[i][j] += reinforcement
                        
                        # Augmenter la plasticité des connexions gagnantes
                        self.plasticity_matrix[i][j] = min(2.0, self.plasticity_matrix[i][j] * 1.1)
        
        # Enregistrer ce succès dans l'historique
        self.successful_predictions += 1
        
        logger.info(f"✅ Neural network rewarded for mining success #{self.successful_predictions}")

    async def _hybrid_mining_completion(self, block_candidate: Dict, neural_suggestions: List[int]):
        """
        ⚙️ Complete le mining avec une approche hybride neuronale + traditionnelle
        
        Si les neurones n'ont pas trouvé directement, utilise leurs suggestions
        comme points de départ pour un mining traditionnel optimisé.
        """
        # Utiliser les suggestions neuronales comme points de départ
        search_ranges = []
        
        for suggestion in neural_suggestions[:5]:  # Top 5 suggestions
            # Créer une plage de recherche autour de chaque suggestion
            range_start = max(0, suggestion - 10000)
            range_end = min(4294967295, suggestion + 10000)  # Max uint32
            search_ranges.append((range_start, range_end))
        
        # Mining guidé par les neurones
        for range_start, range_end in search_ranges:
            for nonce in range(range_start, range_end, 1000):  # Échantillonnage
                block_candidate['nonce'] = nonce
                block_hash = self._calculate_block_hash(block_candidate)
                
                if self._is_valid_hash(block_hash, self.current_difficulty):
                    logger.info(f"🎯 Hybrid mining success! Neural-guided nonce: {nonce}")
                    
                    # Récompense partielle pour guidage réussi
                    await self._reward_guided_mining(block_candidate, neural_suggestions)
                    return True
        
        return False  # Aucune solution trouvée

    async def _reward_guided_mining(self, block_candidate: Dict, neural_suggestions: List[int]):
        """Récompense modérée pour un mining guidé par les neurones"""
        moderate_reward = 0.5
        
        # Récompenser proportionnellement à la qualité du guidage
        for electrode_id in range(60):
            neuron = self.neurons[electrode_id]
            
            # Légère amélioration pour tous les neurones actifs
            if neuron.bitcoin_response_score > 0.1:
                neuron.bitcoin_response_score += moderate_reward * 0.1
                neuron.synaptic_strength += moderate_reward * 0.05

    def _log_mining_statistics(self, stats: Dict):
        """Log les statistiques de mining biologique"""
        runtime = time.time() - stats['mining_start_time']
        
        success_rate = (stats['successful_predictions'] / max(1, stats['neural_predictions'])) * 100
        blocks_per_hour = (stats['blocks_attempted'] / max(1, runtime)) * 3600
        
        logger.info(f"📊 BIOLOGICAL MINING STATS:")
        logger.info(f"   🕒 Runtime: {runtime:.1f}s")
        logger.info(f"   📦 Blocks attempted: {stats['blocks_attempted']}")
        logger.info(f"   🧠 Neural predictions: {stats['neural_predictions']}")
        logger.info(f"   ✅ Successful predictions: {stats['successful_predictions']}")
        logger.info(f"   🎯 Success rate: {success_rate:.2f}%")
        logger.info(f"   ⚡ Blocks/hour: {blocks_per_hour:.1f}")
        
        # Afficher les meilleurs neurones
        best_neurons = sorted(self.neurons.items(), 
                            key=lambda x: x[1].bitcoin_response_score, reverse=True)[:5]
        
        logger.info(f"   🏆 Top neurons:")
        for electrode_id, neuron in best_neurons:
            logger.info(f"     Electrode {electrode_id}: score={neuron.bitcoin_response_score:.3f}")

    async def _adapt_mining_difficulty(self, stats: Dict):
        """Adapte dynamiquement la difficulté selon les performances"""
        success_rate = stats['successful_predictions'] / max(1, stats['blocks_attempted'])
        
        if success_rate > 0.1:  # Plus de 10% de réussite
            self.current_difficulty += 0.1  # Augmenter la difficulté
            logger.info(f"📈 Difficulty increased to {self.current_difficulty}")
        elif success_rate < 0.01:  # Moins de 1% de réussite
            self.current_difficulty = max(1.0, self.current_difficulty - 0.1)  # Diminuer
            logger.info(f"📉 Difficulty decreased to {self.current_difficulty}")

    def _detect_spikes(self, response: List) -> List[Dict]:
        """Détecte les spikes dans une réponse neuronale"""
        if not response or len(response) < 3:
            return []
        
        spikes = []
        
        # Convertir en array numpy
        if isinstance(response[0], dict):
            voltages = np.array([r.get('voltage', 0) for r in response])
            timestamps = np.array([r.get('timestamp', i) for i, r in enumerate(response)])
        else:
            voltages = np.array(response)
            timestamps = np.arange(len(voltages))
        
        # Seuil de détection adaptatif
        baseline = np.median(voltages)
        noise_level = np.std(voltages)
        threshold = baseline - 3 * noise_level  # Spikes négatifs typiques
        
        # Détecter les croisements de seuil
        below_threshold = voltages < threshold
        spike_starts = np.where(np.diff(below_threshold.astype(int)) == 1)[0]
        
        for start_idx in spike_starts:
            if start_idx + 5 < len(voltages):  # Assurer une fenêtre suffisante
                # Trouver le minimum local (pic du spike)
                window = voltages[start_idx:start_idx+10]
                min_idx = np.argmin(window)
                actual_idx = start_idx + min_idx
                
                spike = {
                    'timestamp': timestamps[actual_idx],
                    'amplitude': voltages[actual_idx],
                    'index': actual_idx
                }
                spikes.append(spike)
        
        return spikes

    def _calculate_firing_rate(self, response: List, window_duration: float = 1.0) -> float:
        """Calcule le taux de décharge en Hz"""
        spikes = self._detect_spikes(response)
        
        if not spikes or len(response) == 0:
            return 0.0
        
        # Durée totale d'enregistrement (supposée en ms)
        total_duration = len(response) * 0.001  # Convertir en secondes
        
        return len(spikes) / max(total_duration, 0.001)

    def save_learning_progress(self, filename: str = "biological_bitcoin_learning.json"):
        """
        💾 Sauvegarde le progrès d'apprentissage des neurones
        
        Inclut les poids synaptiques, scores des neurones, et historique complet
        pour reprendre l'entraînement plus tard.
        """
        logger.info(f"💾 Saving biological learning progress to {filename}")
        
        # Préparer les données de sauvegarde
        save_data = {
            'metadata': {
                'version': '2.0.0',
                'timestamp': time.time(),
                'learning_phase': self.learning_phase.value,
                'total_predictions': self.successful_predictions,
                'total_attempts': self.total_attempts,
                'current_difficulty': self.current_difficulty
            },
            'neurons': {
                str(electrode_id): {
                    'firing_rate': neuron.firing_rate,
                    'spike_amplitude': neuron.spike_amplitude,
                    'inter_spike_interval': neuron.inter_spike_interval,
                    'synaptic_strength': neuron.synaptic_strength,
                    'learning_coefficient': neuron.learning_coefficient,
                    'bitcoin_response_score': neuron.bitcoin_response_score
                }
                for electrode_id, neuron in self.neurons.items()
            },
            'synaptic_matrix': self.synaptic_matrix.tolist(),
            'plasticity_matrix': self.plasticity_matrix.tolist(),
            'learning_history': self.learning_history,
            'statistics': {
                'successful_predictions': self.successful_predictions,
                'total_attempts': self.total_attempts,
                'success_rate': self.successful_predictions / max(1, self.total_attempts),
                'best_neuron_id': max(self.neurons.keys(), 
                                    key=lambda x: self.neurons[x].bitcoin_response_score,
                                    default=0),
                'average_synaptic_strength': np.mean(self.synaptic_matrix),
                'network_connectivity': np.count_nonzero(self.synaptic_matrix) / (60*60)
            }
        }
        
        # Sauvegarder en JSON
        with open(filename, 'w') as f:
            json.dump(save_data, f, indent=2, default=str)
        
        logger.info(f"✅ Learning progress saved successfully")
        logger.info(f"📈 Success rate: {save_data['statistics']['success_rate']:.3f}")
        logger.info(f"🧠 Best neuron: #{save_data['statistics']['best_neuron_id']}")

    def load_learning_progress(self, filename: str = "biological_bitcoin_learning.json"):
        """📥 Charge un progrès d'apprentissage précédemment sauvegardé"""
        try:
            logger.info(f"📥 Loading biological learning progress from {filename}")
            
            with open(filename, 'r') as f:
                save_data = json.load(f)
            
            # Restaurer les métadonnées
            metadata = save_data.get('metadata', {})
            self.learning_phase = NeuralLearningPhase(metadata.get('learning_phase', 'initialization'))
            self.successful_predictions = metadata.get('total_predictions', 0)
            self.total_attempts = metadata.get('total_attempts', 0)
            self.current_difficulty = metadata.get('current_difficulty', 1.0)
            
            # Restaurer les neurones
            neurons_data = save_data.get('neurons', {})
            for electrode_id_str, neuron_data in neurons_data.items():
                electrode_id = int(electrode_id_str)
                
                self.neurons[electrode_id] = BiologicalNeuralState(
                    electrode_id=electrode_id,
                    firing_rate=neuron_data.get('firing_rate', 0.0),
                    spike_amplitude=neuron_data.get('spike_amplitude', 0.0),
                    inter_spike_interval=neuron_data.get('inter_spike_interval', float('inf')),
                    synaptic_strength=neuron_data.get('synaptic_strength', 0.5),
                    learning_coefficient=neuron_data.get('learning_coefficient', 0.001),
                    bitcoin_response_score=neuron_data.get('bitcoin_response_score', 0.0)
                )
            
            # Restaurer les matrices
            self.synaptic_matrix = np.array(save_data.get('synaptic_matrix', np.zeros((60, 60)).tolist()))
            self.plasticity_matrix = np.array(save_data.get('plasticity_matrix', np.ones((60, 60)).tolist()))
            
            # Restaurer l'historique
            self.learning_history = save_data.get('learning_history', [])
            
            # Afficher les statistiques
            stats = save_data.get('statistics', {})
            logger.info(f"✅ Learning progress loaded successfully")
            logger.info(f"📊 Loaded {len(self.neurons)} neurons")
            logger.info(f"📈 Previous success rate: {stats.get('success_rate', 0):.3f}")
            logger.info(f"🧠 Best neuron: #{stats.get('best_neuron_id', 'N/A')}")
            logger.info(f"🔗 Network connectivity: {stats.get('network_connectivity', 0):.3f}")
            
        except FileNotFoundError:
            logger.warning(f"❌ Save file {filename} not found. Starting fresh training.")
        except Exception as e:
            logger.error(f"❌ Error loading progress: {e}. Starting fresh training.")

    def plot_learning_progress(self):
        """
        📊 Visualise le progrès d'apprentissage des neurones biologiques
        
        Crée des graphiques pour analyser l'évolution de l'apprentissage
        et identifier les neurones les plus performants.
        """
        if not self.learning_history:
            logger.warning("No learning history to plot")
            return
        
        # Créer une figure avec plusieurs subplots
        fig, axes = plt.subplots(2, 2, figsize=(15, 12))
        fig.suptitle('🧬₿ Biological Bitcoin Learning Progress', fontsize=16)
        
        # Historique des scores moyens
        if self.learning_history:
            epochs = [h.get('epoch', h.get('block', i)) for i, h in enumerate(self.learning_history)]
            scores = [h.get('average_score', h.get('accuracy', 0)) for h in self.learning_history]
            
            axes[0, 0].plot(epochs, scores, 'b-', linewidth=2)
            axes[0, 0].set_title('Average Learning Score Over Time')
            axes[0, 0].set_xlabel('Training Epoch/Block')
            axes[0, 0].set_ylabel('Score')
            axes[0, 0].grid(True, alpha=0.3)
        
        # Matrice synaptique (heatmap)
        im = axes[0, 1].imshow(self.synaptic_matrix, cmap='viridis', aspect='auto')
        axes[0, 1].set_title('Synaptic Connection Matrix')
        axes[0, 1].set_xlabel('Target Electrode')
        axes[0, 1].set_ylabel('Source Electrode')
        plt.colorbar(im, ax=axes[0, 1])
        
        # Distribution des scores Bitcoin des neurones
        scores = [neuron.bitcoin_response_score for neuron in self.neurons.values()]
        axes[1, 0].hist(scores, bins=20, alpha=0.7, color='orange')
        axes[1, 0].set_title('Distribution of Neuron Bitcoin Scores')
        axes[1, 0].set_xlabel('Bitcoin Response Score')
        axes[1, 0].set_ylabel('Number of Neurons')
        axes[1, 0].grid(True, alpha=0.3)
        
        # Performance des top neurones
        top_neurons = sorted(self.neurons.items(), 
                           key=lambda x: x[1].bitcoin_response_score, reverse=True)[:10]
        
        neuron_ids = [f"E{electrode_id}" for electrode_id, _ in top_neurons]
        neuron_scores = [neuron.bitcoin_response_score for _, neuron in top_neurons]
        
        axes[1, 1].bar(neuron_ids, neuron_scores, color='green', alpha=0.7)
        axes[1, 1].set_title('Top 10 Performing Neurons')
        axes[1, 1].set_xlabel('Electrode ID')
        axes[1, 1].set_ylabel('Bitcoin Response Score')
        axes[1, 1].tick_params(axis='x', rotation=45)
        
        plt.tight_layout()
        
        # Sauvegarder le graphique
        filename = f"biological_learning_progress_{int(time.time())}.png"
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        plt.show()
        
        logger.info(f"📊 Learning progress plot saved as {filename}")

    def get_learning_statistics(self) -> Dict:
        """📊 Retourne les statistiques complètes d'apprentissage"""
        if not self.neurons:
            return {}
        
        # Calculer les statistiques des neurones
        scores = [n.bitcoin_response_score for n in self.neurons.values()]
        firing_rates = [n.firing_rate for n in self.neurons.values()]
        synaptic_strengths = [n.synaptic_strength for n in self.neurons.values()]
        
        # Analyser la matrice synaptique
        synaptic_density = np.count_nonzero(self.synaptic_matrix) / (60 * 60)
        synaptic_mean = np.mean(self.synaptic_matrix)
        synaptic_std = np.std(self.synaptic_matrix)
        
        # Identifier le meilleur neurone
        best_neuron_id = max(self.neurons.keys(), 
                           key=lambda x: self.neurons[x].bitcoin_response_score)
        best_neuron = self.neurons[best_neuron_id]
        
        return {
            'learning_phase': self.learning_phase.value,
            'total_neurons': len(self.neurons),
            'active_neurons': sum(1 for n in self.neurons.values() if n.firing_rate > 0),
            
            'performance': {
                'successful_predictions': self.successful_predictions,
                'total_attempts': self.total_attempts,
                'success_rate': self.successful_predictions / max(1, self.total_attempts),
                'current_difficulty': self.current_difficulty
            },
            
            'neuron_scores': {
                'mean': np.mean(scores),
                'std': np.std(scores),
                'min': np.min(scores),
                'max': np.max(scores),
                'median': np.median(scores)
            },
            
            'neural_activity': {
                'mean_firing_rate': np.mean(firing_rates),
                'std_firing_rate': np.std(firing_rates),
                'mean_synaptic_strength': np.mean(synaptic_strengths),
                'std_synaptic_strength': np.std(synaptic_strengths)
            },
            
            'synaptic_network': {
                'connection_density': synaptic_density,
                'mean_weight': synaptic_mean,
                'weight_variability': synaptic_std,
                'plasticity_level': np.mean(self.plasticity_matrix)
            },
            
            'best_neuron': {
                'electrode_id': best_neuron_id,
                'bitcoin_score': best_neuron.bitcoin_response_score,
                'firing_rate': best_neuron.firing_rate,
                'synaptic_strength': best_neuron.synaptic_strength,
                'learning_coefficient': best_neuron.learning_coefficient
            },
            
            'training_history': {
                'total_epochs': len(self.learning_history),
                'learning_phases_completed': len(set(h.get('phase', 'unknown') for h in self.learning_history))
            }
        }

    def stop_mining(self):
        """🛑 Arrête le mining biologique"""
        self.mining_active = False
        logger.info("🛑 Biological Bitcoin mining stopped by user")

# ====================================================================
# FONCTION PRINCIPALE D'ENTRAÎNEMENT
# ====================================================================

async def main_biological_bitcoin_training():
    """
    🚀 Programme principal d'entraînement Bitcoin biologique
    
    Orchestre tout le processus d'apprentissage des neurones biologiques
    pour le mining et la prédiction de nonce Bitcoin.
    """
    logger.info("🧬₿ Starting BIOLOGICAL BITCOIN LEARNING SYSTEM")
    logger.info("=" * 60)
    
    try:
        # Importer l'interface MEA réelle
        from real_mea_interface import RealMEAInterface
        
        # Initialiser l'interface MEA avec les neurones biologiques
        mea_config = {
            'device_type': 'MultiChannelSystems_MEA2100',
            'num_electrodes': 60,
            'sampling_rate': 25000,  # 25 kHz
            'connection_protocol': 'tcp',
            'host': 'localhost',
            'port': 6340
        }
        
        mea = RealMEAInterface(mea_config)
        await mea.connect()
        
        # Créer le système d'apprentissage biologique
        bio_learner = BiologicalBitcoinLearner(mea)
        
        # Étape 1: Initialisation du système biologique
        logger.info("🔬 Step 1: Initializing biological learning system...")
        await bio_learner.initialize_biological_learning()
        
        # Étape 2: Apprentissage de reconnaissance de patterns Bitcoin
        logger.info("🎓 Step 2: Training Bitcoin pattern recognition...")
        await bio_learner.train_bitcoin_pattern_recognition(num_epochs=2000)
        
        # Étape 3: Entraînement à la prédiction de nonce
        logger.info("🎯 Step 3: Training nonce prediction capabilities...")
        accuracy = await bio_learner.train_nonce_prediction(num_training_blocks=10000)
        
        logger.info(f"📊 Training completed! Final prediction accuracy: {accuracy:.3f}")
        
        # Étape 4: Sauvegarde du progrès
        logger.info("💾 Step 4: Saving learning progress...")
        bio_learner.save_learning_progress("biological_bitcoin_model.json")
        
        # Étape 5: Visualisation des résultats
        logger.info("📊 Step 5: Generating learning progress plots...")
        bio_learner.plot_learning_progress()
        
        # Étape 6: Affichage des statistiques finales
        stats = bio_learner.get_learning_statistics()
        logger.info("📈 FINAL LEARNING STATISTICS:")
        logger.info(f"   🧠 Active neurons: {stats['active_neurons']}/{stats['total_neurons']}")
        logger.info(f"   🎯 Success rate: {stats['performance']['success_rate']:.3f}")
        logger.info(f"   ⭐ Best neuron: #{stats['best_neuron']['electrode_id']} (score: {stats['best_neuron']['bitcoin_score']:.3f})")
        logger.info(f"   🔗 Network connectivity: {stats['synaptic_network']['connection_density']:.3f}")
        
        # Étape 7: Lancement du mining biologique (optionnel)
        if accuracy > 0.1:  # Seulement si l'apprentissage est prometteur
            logger.info("⚡ Step 7: Starting biological Bitcoin mining!")
            
            # Lancer le mining en arrière-plan
            mining_task = asyncio.create_task(bio_learner.start_biological_bitcoin_mining())
            
            # Laisser tourner pendant un certain temps
            try:
                await asyncio.wait_for(mining_task, timeout=3600)  # 1 heure de mining
            except asyncio.TimeoutError:
                logger.info("⏰ Mining timeout reached, stopping...")
                bio_learner.stop_mining()
        else:
            logger.warning("⚠️  Accuracy too low for mining. Consider more training.")
        
        # Sauvegarde finale
        bio_learner.save_learning_progress("final_biological_bitcoin_model.json")
        
        # Déconnexion propre
        await mea.disconnect()
        
        logger.info("🎉 BIOLOGICAL BITCOIN LEARNING COMPLETED SUCCESSFULLY!")
        
    except Exception as e:
        logger.error(f"❌ Critical error in biological Bitcoin training: {e}")
        raise

if __name__ == "__main__":
    """
    🧬₿ BIOLOGICAL BITCOIN LEARNING - POINT D'ENTRÉE
    
    Lance l'entraînement complet des neurones biologiques pour le Bitcoin mining.
    
    Usage:
        python biological_bitcoin_learning.py
        
    Le système va automatiquement:
    1. Se connecter au MEA avec les neurones biologiques
    2. Calibrer et initialiser le réseau neuronal
    3. Entraîner la reconnaissance de patterns Bitcoin
    4. Apprendre la prédiction de nonces 
    5. Optimiser le mining via plasticité synaptique
    6. Sauvegarder tous les progrès d'apprentissage
    """
    
    print("🧬₿ BIOLOGICAL BITCOIN LEARNING SYSTEM")
    print("=====================================")
    print("Connecting biological neurons to Bitcoin blockchain...")
    print("Teaching neural networks to mine cryptocurrency!")
    print("")
    
    # Lancer l'entraînement biologique
    asyncio.run(main_biological_bitcoin_training())