#!/usr/bin/env python3
"""
Script de diagnostic pour vérifier l'état des bindings C++ déployés
"""

import requests
import json
import sys

def check_deployment(url):
    """Vérifier l'état d'un déploiement BioMining"""
    
    print(f"🔍 Diagnostic de déploiement : {url}")
    print("=" * 60)
    
    # Test 1: API Status
    try:
        print("\n📊 Test 1: API Status")
        response = requests.get(f"{url}/api/status", timeout=10)
        if response.status_code == 200:
            print("✅ API accessible")
            data = response.json()
            env = data.get('environment', 'unknown')
            print(f"   Environment: {env}")
        else:
            print(f"❌ API erreur: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ Erreur connexion API: {e}")
        return False
    
    # Test 2: Bindings Check  
    try:
        print("\n🔧 Test 2: C++ Bindings")
        response = requests.get(f"{url}/api/bindings", timeout=10)
        if response.status_code == 200:
            bindings = response.json()
            cpp_available = bindings.get('cpp_available', False)
            
            if cpp_available:
                print("🎉 SUCCÈS: C++ bindings chargés !")
                print(f"   Module: {bindings.get('module_location', 'N/A')}")
                
                classes = bindings.get('classes', {})
                for name, available in classes.items():
                    status = "✅" if available else "❌"
                    print(f"   {status} {name}")
                    
            else:
                print("⚠️ PROBLÈME: Fallback Python utilisé")
                print(f"   Message: {bindings.get('message', 'N/A')}")
                print(f"   Environment: {bindings.get('environment', 'N/A')}")
                
            return cpp_available
            
        else:
            print(f"❌ Bindings API erreur: {response.status_code}")
            return False
            
    except Exception as e:
        print(f"❌ Erreur bindings check: {e}")
        return False

def test_biological_network(url):
    """Tester le BiologicalNetwork"""
    
    print(f"\n🧠 Test 3: BiologicalNetwork")
    
    try:
        # Start biological system
        response = requests.post(f"{url}/api/systems/biological/start", timeout=10)
        if response.status_code == 200:
            print("✅ Biological system démarré")
        else:
            print(f"⚠️ Biological start: {response.status_code}")
            
        # Start training
        training_config = {
            "learning_rate": 0.01,
            "epochs": 100, 
            "difficulty": 4
        }
        
        response = requests.post(
            f"{url}/api/training/start",
            json=training_config,
            headers={"Content-Type": "application/json"},
            timeout=10
        )
        
        if response.status_code == 200:
            print("✅ Training démarré")
            data = response.json()
            print(f"   Config: {data.get('training_config', {})}")
            return True
        else:
            print(f"⚠️ Training start: {response.status_code}")
            return False
            
    except Exception as e:
        print(f"❌ Erreur test BiologicalNetwork: {e}")
        return False

def main():
    if len(sys.argv) != 2:
        print("Usage: python diagnose_deployment.py <URL>")
        print("Exemple: python diagnose_deployment.py https://biomining-cpp-xxx.run.app")
        sys.exit(1)
        
    url = sys.argv[1].rstrip('/')
    
    # Diagnostic complet
    bindings_ok = check_deployment(url)
    
    if bindings_ok:
        print("\n🎊 DIAGNOSTIC: C++ BINDINGS OK")
        print("   Votre déploiement utilise les VRAIES classes C++")
        test_biological_network(url)
    else:
        print("\n⚠️ DIAGNOSTIC: FALLBACK PYTHON")
        print("   Votre déploiement utilise les fallbacks Python")
        print("\n🔧 SOLUTION:")
        print("   1. Redéployer avec Dockerfile.cpp-enabled")
        print("   2. Ou utiliser ./deploy_new_cpp_service.sh")
        print("   3. Vérifier la configuration Cloud Run")
        
    print("\n" + "=" * 60)

if __name__ == "__main__":
    main()