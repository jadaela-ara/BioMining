#!/usr/bin/env python3
"""
Script de correction automatique des erreurs de compilation BioMining
Corrige les déclarations manquantes et les problèmes de namespace
"""

import os
import re
import shutil
import sys
from pathlib import Path

class BioMiningFixer:
    def __init__(self, project_path):
        self.project_path = Path(project_path)
        self.backup_dir = self.project_path / "backup_before_fix"
        
    def create_backup(self):
        """Crée une sauvegarde des fichiers avant modification"""
        if not self.backup_dir.exists():
            self.backup_dir.mkdir(parents=True)
        
        files_to_backup = [
            "include/bio/biological_network.h",
            "include/crypto/hybrid_bitcoin_miner.h", 
            "src/cpp/biological_network.cpp"
        ]
        
        for file_path in files_to_backup:
            src = self.project_path / file_path
            if src.exists():
                dst = self.backup_dir / file_path.replace("/", "_")
                shutil.copy2(src, dst)
                print(f"✓ Sauvegarde: {file_path}")
    
    def fix_biological_network_header(self):
        """Corrige le fichier biological_network.h"""
        header_file = self.project_path / "include/bio/biological_network.h"
        if not header_file.exists():
            print(f"❌ Fichier non trouvé: {header_file}")
            return False
            
        with open(header_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Trouver la fin de la classe (avant le dernier };)
        # Ajouter les déclarations manquantes
        missing_declarations = '''
    // Méthodes ajoutées pour corriger les erreurs de compilation
private:
    void generateTrainingData();
    void calculateNetworkEfficiency();  
    double calculateNetworkStability() const;
    double sigmoidDerivative(double sigmoidOutput) const;
    double estimateNonceEfficiency(uint64_t nonce, uint64_t difficulty);
    void adaptNetworkThresholds();
    double adaptiveThreshold(const Neuron& neuron);
'''
        
        # Insérer avant la fermeture de la classe
        if 'class BiologicalNetwork' in content:
            # Trouver la dernière accolade fermante de la classe
            lines = content.splitlines()
            insert_pos = -1
            brace_count = 0
            in_class = False
            
            for i, line in enumerate(lines):
                if 'class BiologicalNetwork' in line:
                    in_class = True
                    brace_count = 0
                if in_class:
                    brace_count += line.count('{') - line.count('}')
                    if brace_count == 0 and in_class and '}' in line:
                        insert_pos = i
                        break
            
            if insert_pos > 0:
                lines.insert(insert_pos, missing_declarations)
                content = '\n'.join(lines)
                
                with open(header_file, 'w', encoding='utf-8') as f:
                    f.write(content)
                print("✓ biological_network.h corrigé")
                return True
        
        print("❌ Impossible de modifier biological_network.h")
        return False
    
    def fix_hybrid_miner_header(self):
        """Corrige le fichier hybrid_bitcoin_miner.h"""
        header_file = self.project_path / "include/crypto/hybrid_bitcoin_miner.h"
        if not header_file.exists():
            print(f"❌ Fichier non trouvé: {header_file}")
            return False
            
        with open(header_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Corriger la référence au namespace
        content = content.replace(
            'BioMining::Bio::BiologicalNetwork',
            'BiologicalNetwork'
        )
        
        # Ajouter l'include si nécessaire
        if '#include "bio/biological_network.h"' not in content:
            includes_section = content.find('#include')
            if includes_section != -1:
                lines = content.splitlines()
                # Trouver la première ligne d'include
                for i, line in enumerate(lines):
                    if line.strip().startswith('#include'):
                        lines.insert(i, '#include "bio/biological_network.h"')
                        break
                content = '\n'.join(lines)
        
        # Définir MiningConfig si c'est juste une forward declaration
        if 'struct MiningConfig;' in content and 'struct MiningConfig {' not in content:
            mining_config_def = '''
struct MiningConfig {
    uint64_t difficulty = 1;
    uint32_t threads = 1; 
    bool useGPU = false;
    double targetEfficiency = 0.5;
};
'''
            content = content.replace('struct MiningConfig;', mining_config_def)
        
        with open(header_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print("✓ hybrid_bitcoin_miner.h corrigé")
        return True
    
    def fix_biological_network_cpp(self):
        """Corrige le fichier biological_network.cpp"""
        cpp_file = self.project_path / "src/cpp/biological_network.cpp"
        if not cpp_file.exists():
            print(f"❌ Fichier non trouvé: {cpp_file}")
            return False
            
        with open(cpp_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Corriger l'utilisation récursive d'adaptiveThreshold
        content = re.sub(
            r'double adaptiveThreshold = adaptiveThreshold\(neuron\);',
            'double threshold = adaptiveThreshold(neuron);',
            content
        )
        
        # Ajouter les includes nécessaires si manquants
        includes_to_add = ['#include <cmath>', '#include <algorithm>', '#include <numeric>']
        lines = content.splitlines()
        
        first_include_idx = -1
        for i, line in enumerate(lines):
            if line.strip().startswith('#include'):
                first_include_idx = i
                break
        
        if first_include_idx != -1:
            for include in reversed(includes_to_add):
                if include not in content:
                    lines.insert(first_include_idx, include)
        
        content = '\n'.join(lines)
        
        with open(cpp_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print("✓ biological_network.cpp corrigé")
        return True
    
    def run_fixes(self):
        """Exécute toutes les corrections"""
        print("🔧 Démarrage des corrections BioMining...")
        print(f"📁 Projet: {self.project_path}")
        
        # Créer une sauvegarde
        self.create_backup()
        
        # Appliquer les corrections
        success = True
        success &= self.fix_biological_network_header()
        success &= self.fix_hybrid_miner_header() 
        success &= self.fix_biological_network_cpp()
        
        if success:
            print("\n✅ Toutes les corrections ont été appliquées avec succès!")
            print("\n📋 Prochaines étapes:")
            print("1. Vérifiez les fichiers modifiés")
            print("2. Compilez le projet: make clean && make")
            print("3. Si des erreurs persistent, consultez les fichiers de patch pour les corrections manuelles")
            print(f"4. Sauvegarde disponible dans: {self.backup_dir}")
        else:
            print("\n❌ Certaines corrections ont échoué. Vérifiez les chemins des fichiers.")
        
        return success

def main():
    if len(sys.argv) > 1:
        project_path = sys.argv[1]
    else:
        project_path = input("Entrez le chemin vers le projet BioMining [/home/jadaela/BioMining]: ").strip()
        if not project_path:
            project_path = "/home/jadaela/BioMining"
    
    fixer = BioMiningFixer(project_path)
    fixer.run_fixes()

if __name__ == "__main__":
    main()