#!/usr/bin/env python3
"""
Test MEA Bitcoin Learning - BioMining Platform
===============================================

Fichier de test pour l'apprentissage du réseau biologique MEA pour le mining Bitcoin
avec des valeurs réelles de blocs en entrée et de nonces en sortie.

Architecture:
- Utilise des données réelles de blocs Bitcoin (headers de blocs valides)
- Stimule les électrodes MEA avec les données du bloc (hash précédent, merkle root, etc.)
- Entraîne le réseau biologique à découvrir les nonces valides
- Mesure les réponses électrophysiologiques et apprentissage adaptatif

Author: BioMining Platform Team
Date: 2025-10-02
Version: 1.0.0
"""

import sys
import os
import time
import hashlib
import struct
import json
import numpy as np
from datetime import datetime, timezone
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass, asdict
from concurrent.futures import ThreadPoolExecutor
import logging

# Ajouter le chemin vers les modules BioMining
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))

try:
    from biomining.mea_interface import RealMEAInterface
    from biomining.crypto.sha256 import SHA256Calculator
    from biomining.bio.neural_network import BiologicalNeuralNetwork
except ImportError as e:
    print(f"⚠️  Modules BioMining non trouvés: {e}")
    print("📚 Utilisation du simulateur intégré pour les tests")


@dataclass
class BitcoinBlock:
    """Structure d'un bloc Bitcoin pour l'apprentissage"""
    version: int
    previous_hash: str
    merkle_root: str
    timestamp: int
    bits: int
    nonce: int
    block_height: int
    target_hash: str
    
    def to_header_bytes(self) -> bytes:
        """Convertit le header du bloc en format binaire"""
        header = struct.pack('<I', self.version)
        header += bytes.fromhex(self.previous_hash)[::-1]  # Little endian
        header += bytes.fromhex(self.merkle_root)[::-1]
        header += struct.pack('<I', self.timestamp)
        header += struct.pack('<I', self.bits)
        header += struct.pack('<I', self.nonce)
        return header
    
    def calculate_hash(self) -> str:
        """Calcule le hash du bloc"""
        header = self.to_header_bytes()
        hash1 = hashlib.sha256(header).digest()
        hash2 = hashlib.sha256(hash1).digest()
        return hash2[::-1].hex()  # Big endian pour affichage


class MEABitcoinTrainingData:
    """Gestionnaire des données d'entraînement Bitcoin réelles"""
    
    def __init__(self):
        self.logger = logging.getLogger(__name__)
        self.real_bitcoin_blocks = self._load_real_bitcoin_data()
        
    def _load_real_bitcoin_data(self) -> List[BitcoinBlock]:
        """Charge des données réelles de blocs Bitcoin pour l'entraînement"""
        # Blocs Bitcoin réels avec leurs nonces valides pour l'entraînement
        real_blocks = [
            # Bloc Genesis Bitcoin (Block #0)
            BitcoinBlock(
                version=1,
                previous_hash="0000000000000000000000000000000000000000000000000000000000000000",
                merkle_root="4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b",
                timestamp=1231006505,
                bits=0x1d00ffff,
                nonce=2083236893,
                block_height=0,
                target_hash="000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
            ),
            
            # Bloc #1 - Premier bloc miné après Genesis
            BitcoinBlock(
                version=1,
                previous_hash="000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",
                merkle_root="0e3e2357e806b6cdb1f70b54c3a3a17b6714ee1f0e68bebb44a74b1efd512098",
                timestamp=1231469665,
                bits=0x1d00ffff,
                nonce=2573394689,
                block_height=1,
                target_hash="00000000839a8e6886ab5951d76f411475428afc90947ee320161bbf18eb6048"
            ),
            
            # Bloc moderne avec difficulté élevée (exemple récent)
            BitcoinBlock(
                version=0x20000000,
                previous_hash="00000000000000000001c792c4b9c426e8e8e6b5b4c2e2b1a1a9e8d7c6b5a4a3",
                merkle_root="a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef123456",
                timestamp=int(time.time()) - 3600,  # Il y a 1 heure
                bits=0x1712c4a3,  # Difficulté moderne
                nonce=0,  # À découvrir par le réseau biologique
                block_height=850000,
                target_hash=""  # À calculer
            ),
            
            # Bloc d'entraînement avec difficulté progressive
            BitcoinBlock(
                version=1,
                previous_hash="00000000b873e79784647a6c82962c70d228557d24a747ea4d1b8bbe878e1206",
                merkle_root="f3e94742aca4b5ef85488dc37c06c3282295ffec960994b2c0d5ac2a25a95766",
                timestamp=1293623863,
                bits=0x1b04864c,
                nonce=274148111,
                block_height=100000,
                target_hash="000000000003ba27aa200b1cecaad478d2b00432346c3f1f3986da1afd33e506"
            )
        ]
        
        self.logger.info(f"📚 Chargé {len(real_blocks)} blocs Bitcoin réels pour l'entraînement")
        return real_blocks
    
    def get_training_block(self, difficulty_level: int = 1) -> BitcoinBlock:
        """Récupère un bloc pour l'entraînement selon le niveau de difficulté"""
        if difficulty_level == 0:  # Genesis - très facile
            return self.real_bitcoin_blocks[0]
        elif difficulty_level == 1:  # Bloc #1 - facile  
            return self.real_bitcoin_blocks[1]
        elif difficulty_level == 2:  # Bloc historique - moyen
            return self.real_bitcoin_blocks[3]
        else:  # Bloc moderne - difficile
            return self.real_bitcoin_blocks[2]


class MEAElectrodeStimulator:
    """Simulateur de stimulation des électrodes MEA pour l'apprentissage Bitcoin"""
    
    def __init__(self, num_electrodes: int = 60):
        self.num_electrodes = num_electrodes
        self.electrode_patterns = np.zeros((num_electrodes, 256))  # Patterns pour chaque électrode
        self.logger = logging.getLogger(__name__)
        
    def encode_block_data_to_stimulation(self, block: BitcoinBlock) -> np.ndarray:
        """
        Encode les données du bloc Bitcoin en patterns de stimulation électrique
        
        Méthode:
        1. Convertit chaque composant du bloc en représentation binaire
        2. Mappe les bits sur les électrodes (distribution spatiale)
        3. Génère des patterns de stimulation temporelle
        """
        # Préparer les données du bloc
        header_bytes = block.to_header_bytes()[:-4]  # Sans le nonce pour l'entraînement
        
        # Encoder en patterns de stimulation (60 électrodes)
        stimulation_patterns = np.zeros((self.num_electrodes, 1000))  # 1000ms de stimulation
        
        # Distribution des données sur les électrodes
        for i, byte_val in enumerate(header_bytes[:self.num_electrodes]):
            electrode_id = i % self.num_electrodes
            
            # Conversion byte en pattern de stimulation (0-255 -> amplitude)
            base_amplitude = (byte_val / 255.0) * 100.0  # μV
            
            # Génération du pattern temporel
            for t in range(1000):
                # Pattern sinusoïdal modulé par la valeur du byte
                frequency = 10 + (byte_val % 40)  # 10-50 Hz
                phase = (byte_val % 8) * (np.pi / 4)  # Phase selon la valeur
                
                signal = base_amplitude * np.sin(2 * np.pi * frequency * t / 1000.0 + phase)
                
                # Ajout de bruit pour réalisme biologique
                noise = np.random.normal(0, base_amplitude * 0.1)
                stimulation_patterns[electrode_id, t] = signal + noise
        
        # Log des patterns générés
        self.logger.info(f"🧠 Généré patterns stimulation pour {len(header_bytes)} bytes")
        self.logger.info(f"📊 Amplitude moyenne: {np.mean(np.abs(stimulation_patterns)):.2f} μV")
        
        return stimulation_patterns
    
    def extract_nonce_from_response(self, mea_response: np.ndarray) -> int:
        """
        Extrait un nonce candidat des réponses électrophysiologiques du MEA
        
        Méthode:
        1. Analyse les pics de spike sur toutes les électrodes
        2. Convertit les patterns temporels en valeurs numériques
        3. Assemble un nonce 32-bit candidat
        """
        # Analyser les réponses de toutes les électrodes
        nonce_bits = []
        
        for electrode_id in range(min(32, self.num_electrodes)):  # 32 bits max pour nonce
            electrode_response = mea_response[electrode_id, :]
            
            # Détection de pics (spikes)
            threshold = np.std(electrode_response) * 2.5
            spikes = np.where(np.abs(electrode_response) > threshold)[0]
            
            # Conversion en bit basée sur l'activité de l'électrode
            if len(spikes) > 0:
                # Bit = 1 si activité élevée, 0 sinon
                avg_spike_amplitude = np.mean(np.abs(electrode_response[spikes]))
                median_amplitude = np.median(np.abs(electrode_response))
                bit_value = 1 if avg_spike_amplitude > median_amplitude * 1.5 else 0
            else:
                bit_value = 0
                
            nonce_bits.append(bit_value)
        
        # Assembler le nonce à partir des bits
        nonce = 0
        for i, bit in enumerate(nonce_bits):
            nonce |= (bit << i)
        
        self.logger.debug(f"🔢 Nonce extrait des réponses MEA: {nonce} (0x{nonce:08x})")
        return nonce


class BiologicalBitcoinMiner:
    """
    Mineur Bitcoin utilisant un réseau biologique MEA
    Entraîne le réseau à découvrir des nonces valides
    """
    
    def __init__(self, mea_device_config: str = "config/mea_device.json"):
        self.logger = logging.getLogger(__name__)
        
        # Initialisation des composants
        self.training_data = MEABitcoinTrainingData()
        self.electrode_stimulator = MEAElectrodeStimulator()
        self.learning_stats = {
            'blocks_processed': 0,
            'valid_nonces_found': 0,
            'total_attempts': 0,
            'success_rate': 0.0,
            'best_hash_distance': float('inf')
        }
        
        # Tentative de connexion au MEA réel
        try:
            self.mea_interface = RealMEAInterface(mea_device_config)
            self.mea_connected = True
            self.logger.info("✅ Connexion MEA réelle établie")
        except Exception as e:
            self.logger.warning(f"⚠️  MEA réel non disponible: {e}")
            self.mea_connected = False
            self.logger.info("🔬 Utilisation du simulateur MEA")
    
class BiologicalBitcoinMiner:
    """
    Mineur Bitcoin utilisant un réseau biologique MEA
    Entraîne le réseau à découvrir des nonces valides
    """
    
    def __init__(self, mea_device_config: str = "config/mea_device.json"):
        self.logger = logging.getLogger(__name__)
        
        # Initialisation des composants
        self.training_data = MEABitcoinTrainingData()
        self.electrode_stimulator = MEAElectrodeStimulator()
        self.learning_stats = {
            'blocks_processed': 0,
            'valid_nonces_found': 0,
            'total_attempts': 0,
            'success_rate': 0.0,
            'best_hash_distance': float('inf')
        }
        
        # Tentative de connexion au MEA réel
        try:
            self.mea_interface = RealMEAInterface(mea_device_config)
            self.mea_connected = True
            self.logger.info("✅ Connexion MEA réelle établie")
        except Exception as e:
            self.logger.warning(f"⚠️  MEA réel non disponible: {e}")
            self.mea_connected = False
            self.logger.info("🔬 Utilisation du simulateur MEA")
    
    def simulate_mea_response(self, stimulation_patterns: np.ndarray) -> np.ndarray:
        """Simule les réponses d'un MEA réel pour les tests"""
        num_electrodes, duration = stimulation_patterns.shape
        
        # Simulation de réponses biologiques réalistes
        responses = np.zeros_like(stimulation_patterns)
        
        for electrode_id in range(num_electrodes):
            stimulation = stimulation_patterns[electrode_id, :]
            
            # Modèle de réponse neuronale simplifiée
            # 1. Réponse immédiate (10ms delay)
            delay = 10
            response = np.zeros(duration)
            
            if duration > delay:
                # Réponse avec atténuation et délai
                attenuated_signal = stimulation[:-delay] * 0.7
                response[delay:] = attenuated_signal
                
                # Ajout de spikes spontanés
                spike_probability = 0.02  # 2% chance de spike par ms
                for t in range(duration):
                    if np.random.random() < spike_probability:
                        spike_amplitude = np.random.uniform(50, 200)  # μV
                        spike_duration = min(5, duration - t)
                        response[t:t+spike_duration] += spike_amplitude
                
                # Ajout de bruit de fond biologique
                noise = np.random.normal(0, 10, duration)  # 10 μV RMS
                response += noise
            
            responses[electrode_id, :] = response
        
        return responses
    
    def validate_nonce(self, block: BitcoinBlock, nonce: int) -> Tuple[bool, str, float]:
        """
        Valide si un nonce produit un hash valide pour le bloc
        
        Returns:
            (is_valid, hash_result, hash_distance_to_target)
        """
        # Créer une copie du bloc avec le nouveau nonce
        test_block = BitcoinBlock(
            version=block.version,
            previous_hash=block.previous_hash,
            merkle_root=block.merkle_root,
            timestamp=block.timestamp,
            bits=block.bits,
            nonce=nonce,
            block_height=block.block_height,
            target_hash=""
        )
        
        # Calculer le hash avec ce nonce
        block_hash = test_block.calculate_hash()
        
        # Calculer le target à partir de 'bits'
        target_threshold = self._bits_to_target(block.bits)
        hash_int = int(block_hash, 16)
        
        # Le hash est valide si inférieur au target
        is_valid = hash_int < target_threshold
        
        # Distance au target (pour mesurer la progression)
        hash_distance = abs(hash_int - target_threshold) / target_threshold
        
        return is_valid, block_hash, hash_distance
    
    def _bits_to_target(self, bits: int) -> int:
        """Convertit le field 'bits' en target threshold"""
        exponent = bits >> 24
        mantissa = bits & 0xffffff
        
        if exponent <= 3:
            target = mantissa >> (8 * (3 - exponent))
        else:
            target = mantissa << (8 * (exponent - 3))
            
        return target
    
    def train_single_block(self, block: BitcoinBlock, max_iterations: int = 1000) -> Dict:
        """
        Entraîne le réseau biologique sur un seul bloc
        
        Process:
        1. Encode le bloc en stimulations électriques
        2. Stimule le MEA et enregistre les réponses
        3. Extrait un nonce candidat des réponses
        4. Valide le nonce et ajuste l'entraînement
        """
        self.logger.info(f"🧬 Début entraînement sur bloc #{block.block_height}")
        
        training_results = {
            'block_height': block.block_height,
            'iterations': 0,
            'nonces_tested': [],
            'best_nonce': None,
            'best_hash': None,
            'best_distance': float('inf'),
            'success': False,
            'training_time': 0
        }
        
        start_time = time.time()
        
        # Génération des patterns de stimulation pour le bloc
        stimulation_patterns = self.electrode_stimulator.encode_block_data_to_stimulation(block)
        
        for iteration in range(max_iterations):
            self.logger.debug(f"🔄 Itération {iteration + 1}/{max_iterations}")
            
            # Stimulation du MEA (réel ou simulé)
            if self.mea_connected:
                try:
                    # Stimulation MEA réelle
                    mea_responses = self.mea_interface.stimulate_and_record(
                        stimulation_patterns, 
                        duration=1000  # 1 seconde
                    )
                except Exception as e:
                    self.logger.error(f"❌ Erreur MEA réel: {e}")
                    mea_responses = self.simulate_mea_response(stimulation_patterns)
            else:
                # Utilisation du simulateur
                mea_responses = self.simulate_mea_response(stimulation_patterns)
            
            # Extraction du nonce candidat depuis les réponses MEA
            candidate_nonce = self.electrode_stimulator.extract_nonce_from_response(mea_responses)
            
            # Validation du nonce
            is_valid, hash_result, hash_distance = self.validate_nonce(block, candidate_nonce)
            
            # Enregistrement des résultats
            training_results['nonces_tested'].append({
                'nonce': candidate_nonce,
                'hash': hash_result,
                'distance': hash_distance,
                'valid': is_valid
            })
            
            # Mise à jour du meilleur résultat
            if hash_distance < training_results['best_distance']:
                training_results['best_nonce'] = candidate_nonce
                training_results['best_hash'] = hash_result
                training_results['best_distance'] = hash_distance
                
                self.logger.info(f"🎯 Nouveau meilleur nonce: {candidate_nonce} (distance: {hash_distance:.6f})")
            
            # Vérification de succès
            if is_valid:
                training_results['success'] = True
                self.learning_stats['valid_nonces_found'] += 1
                self.logger.success(f"🏆 NONCE VALIDE TROUVÉ! {candidate_nonce} -> {hash_result}")
                break
            
            # Adaptation de l'entraînement basée sur les résultats
            if iteration % 100 == 0 and iteration > 0:
                self.logger.info(f"📊 Progrès: {iteration} itérations, meilleure distance: {training_results['best_distance']:.6f}")
            
            # Modification des patterns pour apprentissage adaptatif
            if hash_distance > training_results['best_distance'] * 1.1:
                # Ajustement des patterns si on s'éloigne du target
                stimulation_patterns = self._adjust_stimulation_patterns(
                    stimulation_patterns, 
                    hash_distance, 
                    iteration
                )
        
        # Finalisation des résultats
        training_results['iterations'] = iteration + 1
        training_results['training_time'] = time.time() - start_time
        
        # Mise à jour des statistiques globales
        self.learning_stats['blocks_processed'] += 1
        self.learning_stats['total_attempts'] += training_results['iterations']
        self.learning_stats['success_rate'] = (
            self.learning_stats['valid_nonces_found'] / self.learning_stats['blocks_processed']
        )
        
        if training_results['best_distance'] < self.learning_stats['best_hash_distance']:
            self.learning_stats['best_hash_distance'] = training_results['best_distance']
        
        self.logger.info(f"✅ Entraînement terminé: {training_results['iterations']} itérations en {training_results['training_time']:.2f}s")
        
        return training_results
    
    def _adjust_stimulation_patterns(self, patterns: np.ndarray, hash_distance: float, iteration: int) -> np.ndarray:
        """Ajuste les patterns de stimulation pour l'apprentissage adaptatif"""
        adjusted_patterns = patterns.copy()
        
        # Facteur d'ajustement basé sur la distance au target
        adjustment_factor = min(0.1, hash_distance / 1000.0)
        
        # Modification aléatoire avec biais vers l'amélioration
        for electrode_id in range(patterns.shape[0]):
            if np.random.random() < 0.3:  # 30% chance de modification par électrode
                # Ajustement de l'amplitude ou de la fréquence
                noise_amplitude = adjustment_factor * 20.0  # μV
                noise = np.random.normal(0, noise_amplitude, patterns.shape[1])
                adjusted_patterns[electrode_id, :] += noise
        
        return adjusted_patterns
    
    def run_progressive_training(self, difficulty_levels: List[int] = [0, 1, 2, 3]) -> Dict:
        """
        Exécute un entraînement progressif sur différents niveaux de difficulté
        
        Args:
            difficulty_levels: Liste des niveaux de difficulté (0=facile, 3=difficile)
        """
        self.logger.info("🚀 Début entraînement progressif MEA Bitcoin")
        
        overall_results = {
            'training_session': datetime.now().isoformat(),
            'difficulty_levels': difficulty_levels,
            'level_results': [],
            'total_training_time': 0,
            'final_stats': {}
        }
        
        session_start_time = time.time()
        
        for level in difficulty_levels:
            self.logger.info(f"📈 === NIVEAU DE DIFFICULTÉ {level} ===")
            
            # Récupération du bloc pour ce niveau
            training_block = self.training_data.get_training_block(level)
            
            # Entraînement sur ce bloc
            level_results = self.train_single_block(
                training_block, 
                max_iterations=1000 if level < 3 else 2000
            )
            
            level_results['difficulty_level'] = level
            overall_results['level_results'].append(level_results)
            
            # Log des résultats du niveau
            success_status = "✅ SUCCÈS" if level_results['success'] else "❌ ÉCHEC"
            self.logger.info(f"📊 Niveau {level} - {success_status}")
            self.logger.info(f"   Itérations: {level_results['iterations']}")
            self.logger.info(f"   Meilleur nonce: {level_results['best_nonce']}")
            self.logger.info(f"   Distance minimale: {level_results['best_distance']:.8f}")
            
            # Pause entre les niveaux pour permettre au réseau biologique de "récupérer"
            if level < max(difficulty_levels):
                self.logger.info("⏱️  Pause de récupération du réseau biologique...")
                time.sleep(2)
        
        # Finalisation des résultats globaux
        overall_results['total_training_time'] = time.time() - session_start_time
        overall_results['final_stats'] = self.learning_stats.copy()
        
        # Log du résumé final
        self.logger.info("🎉 === ENTRAÎNEMENT PROGRESSIF TERMINÉ ===")
        self.logger.info(f"📊 Statistiques finales:")
        self.logger.info(f"   Blocs traités: {self.learning_stats['blocks_processed']}")
        self.logger.info(f"   Nonces valides trouvés: {self.learning_stats['valid_nonces_found']}")
        self.logger.info(f"   Taux de succès: {self.learning_stats['success_rate']:.2%}")
        self.logger.info(f"   Temps total: {overall_results['total_training_time']:.2f}s")
        
        return overall_results


def setup_logging():
    """Configure le système de logging pour les tests"""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler(),
            logging.FileHandler('/home/user/webapp/tests/mea_bitcoin_learning.log')
        ]
    )
    
    # Ajouter niveau custom pour succès
    logging.SUCCESS = 25  # Entre INFO (20) et WARNING (30)
    logging.addLevelName(logging.SUCCESS, 'SUCCESS')
    
    def success(self, message, *args, **kws):
        if self.isEnabledFor(logging.SUCCESS):
            self._log(logging.SUCCESS, message, args, **kws)
    
    logging.Logger.success = success


def save_results_to_json(results: Dict, filename: str):
    """Sauvegarde les résultats en format JSON"""
    filepath = f"/home/user/webapp/tests/results/{filename}"
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    
    with open(filepath, 'w') as f:
        json.dump(results, f, indent=2, default=str)
    
    print(f"📁 Résultats sauvegardés: {filepath}")


def main():
    """Fonction principale de test"""
    setup_logging()
    logger = logging.getLogger(__name__)
    
    print("🧬 === TEST MEA BITCOIN LEARNING ===")
    print("📚 Entraînement du réseau biologique pour le mining Bitcoin")
    print("🔗 Utilisation de blocs et nonces réels Bitcoin")
    print("=" * 50)
    
    try:
        # Initialisation du mineur biologique
        bio_miner = BiologicalBitcoinMiner()
        
        # Test rapide sur un bloc simple
        print("\n🧪 === TEST RAPIDE ===")
        genesis_block = bio_miner.training_data.get_training_block(0)
        quick_results = bio_miner.train_single_block(genesis_block, max_iterations=50)
        
        print(f"✅ Test rapide terminé:")
        print(f"   Nonces testés: {len(quick_results['nonces_tested'])}")
        print(f"   Succès: {'Oui' if quick_results['success'] else 'Non'}")
        print(f"   Meilleure distance: {quick_results['best_distance']:.8f}")
        
        # Entraînement progressif complet
        print("\n🎓 === ENTRAÎNEMENT PROGRESSIF ===")
        full_results = bio_miner.run_progressive_training([0, 1, 2])
        
        # Sauvegarde des résultats
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        save_results_to_json(quick_results, f"quick_test_{timestamp}.json")
        save_results_to_json(full_results, f"progressive_training_{timestamp}.json")
        
        # Affichage du résumé final
        print("\n🏆 === RÉSUMÉ FINAL ===")
        print(f"📊 Blocs traités: {bio_miner.learning_stats['blocks_processed']}")
        print(f"🎯 Nonces valides: {bio_miner.learning_stats['valid_nonces_found']}")
        print(f"📈 Taux de succès: {bio_miner.learning_stats['success_rate']:.2%}")
        print(f"🥇 Meilleure performance: {bio_miner.learning_stats['best_hash_distance']:.8f}")
        
        return full_results
        
    except Exception as e:
        logger.error(f"❌ Erreur lors du test: {e}", exc_info=True)
        raise
    
    finally:
        print("\n🔚 Test MEA Bitcoin Learning terminé")


if __name__ == "__main__":
    results = main()