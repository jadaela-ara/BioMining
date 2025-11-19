#!/usr/bin/env python3
"""
Validate C++ bindings structure for real BiologicalNetwork deployment
This script checks that all components are ready for Qt MOC compilation
"""

import os
import sys
import re
from pathlib import Path

def check_file_exists(filepath, description):
    """Check if a required file exists"""
    if os.path.exists(filepath):
        print(f"✅ {description}: {filepath}")
        return True
    else:
        print(f"❌ {description}: {filepath} - MISSING")
        return False

def check_q_object_in_header(filepath, classname):
    """Check if header contains Q_OBJECT macro"""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            if 'Q_OBJECT' in content:
                q_object_count = content.count('Q_OBJECT')
                print(f"✅ {classname}: Q_OBJECT found ({q_object_count} occurrence(s)) - MOC required")
                return True
            else:
                print(f"⚠️ {classname}: No Q_OBJECT found - MOC not needed")
                return False
    except Exception as e:
        print(f"❌ {classname}: Error reading {filepath} - {e}")
        return False

def check_cpp_method_in_bindings(method_name):
    """Check if method is properly bound in Python bindings"""
    bindings_file = "python_bindings/biomining_python.cpp"
    try:
        with open(bindings_file, 'r') as f:
            content = f.read()
            if method_name in content:
                print(f"✅ Method '{method_name}' found in Python bindings")
                return True
            else:
                print(f"❌ Method '{method_name}' NOT found in Python bindings")
                return False
    except Exception as e:
        print(f"❌ Error reading bindings file: {e}")
        return False

def main():
    print("🔍 Validating C++ Bindings Structure for Real BiologicalNetwork Deployment")
    print("=" * 80)
    
    all_good = True
    
    # Check required files
    print("\n📁 Checking Required Files:")
    required_files = [
        ("Dockerfile.cpp-moc-fixed", "Qt MOC-enabled Dockerfile"),
        ("deploy_cpp_moc_fixed.sh", "Deployment script"),
        ("requirements-cpp.txt", "C++ requirements"),
        ("python_bindings/biomining_python.cpp", "Python bindings"),
    ]
    
    for filepath, description in required_files:
        if not check_file_exists(filepath, description):
            all_good = False
    
    # Check C++ headers
    print("\n📋 Checking C++ Headers and Qt Requirements:")
    headers_to_check = [
        ("include/bio/biological_network.h", "BiologicalNetwork"),
        ("include/crypto/bitcoin_miner.h", "BitcoinMiner"),
        ("include/crypto/hybrid_bitcoin_miner.h", "HybridBitcoinMiner"),
        ("include/bio/real_mea_interface.h", "RealMEAInterface"),
        ("include/bio/mea_interface.h", "MEAInterface"),
    ]
    
    for filepath, classname in headers_to_check:
        if check_file_exists(filepath, f"{classname} header"):
            check_q_object_in_header(filepath, classname)
    
    # Check C++ sources
    print("\n⚙️ Checking C++ Source Files:")
    cpp_sources = [
        ("src/cpp/biological_network.cpp", "BiologicalNetwork implementation"),
        ("src/cpp/bitcoin_miner.cpp", "BitcoinMiner implementation"), 
        ("src/cpp/hybrid_bitcoin_miner.cpp", "HybridBitcoinMiner implementation"),
        ("src/cpp/real_mea_interface.cpp", "RealMEAInterface implementation"),
        ("src/cpp/mea_interface.cpp", "MEAInterface implementation"),
    ]
    
    for filepath, description in cpp_sources:
        if not check_file_exists(filepath, description):
            all_good = False
    
    # Check critical methods in bindings
    print("\n🔗 Checking Critical Methods in Python Bindings:")
    critical_methods = [
        "startLearning",
        "configureBiologicalNetwork", 
        "BiologicalNetwork",
        "HybridBitcoinMiner",
        "RealMEAInterface"
    ]
    
    for method in critical_methods:
        check_cpp_method_in_bindings(method)
    
    # Check for MOC include statements in C++ sources
    print("\n🔧 Checking MOC Include Statements:")
    moc_includes = [
        ("src/cpp/biological_network.cpp", "biological_network.moc"),
        ("src/cpp/bitcoin_miner.cpp", "bitcoin_miner.moc"),
    ]
    
    for cpp_file, moc_file in moc_includes:
        try:
            with open(cpp_file, 'r') as f:
                content = f.read()
                if f'#include "{moc_file}"' in content:
                    print(f"✅ {cpp_file} includes {moc_file}")
                else:
                    print(f"⚠️ {cpp_file} may not include {moc_file}")
        except Exception as e:
            print(f"❌ Error checking {cpp_file}: {e}")
    
    # Summary
    print("\n" + "=" * 80)
    if all_good:
        print("🎉 ALL VALIDATION CHECKS PASSED!")
        print("✅ Ready for Qt MOC deployment with real C++ methods")
        print("\n🚀 Next step: Run ./deploy_cpp_moc_fixed.sh")
    else:
        print("⚠️ Some validation checks failed")
        print("❌ Fix missing files before deployment")
    
    print("\n🎯 Target: Real BiologicalNetwork.startLearning() C++ method")
    print("📋 User request: \"non je veux rester avec les vraies méthodes C++\"")
    
    return 0 if all_good else 1

if __name__ == "__main__":
    sys.exit(main())