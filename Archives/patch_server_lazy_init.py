#!/usr/bin/env python3
"""
Patch server.py to use lazy initialization of BioMiningPlatform
This prevents pybind11 errors from crashing at module import time
"""

import sys
import os

# Path to server.py
SERVER_FILE = "/app/web/api/server.py"

def patch_server():
    """Patch server.py to use lazy initialization"""
    
    print("🔧 Patching server.py for lazy initialization...")
    
    try:
        with open(SERVER_FILE, 'r') as f:
            content = f.read()
        
        # Find and replace the platform initialization
        old_init = "platform = BioMiningPlatform()"
        new_init = """# Lazy initialization to prevent pybind11 errors at import time
platform = None

def get_platform():
    global platform
    if platform is None:
        try:
            platform = BioMiningPlatform()
        except Exception as e:
            print(f"⚠️ Failed to initialize BioMiningPlatform: {e}")
            print("📦 Service will continue without platform (limited functionality)")
            # Return a dummy object that won't crash
            class DummyPlatform:
                is_mining = False
                is_training = False
                def get_platform_status(self):
                    return {"status": "fallback", "systems": {}}
                def get_performance_metrics(self):
                    return {"mode": "fallback"}
                async def stop_hybrid_mining(self):
                    pass
            platform = DummyPlatform()
    return platform"""
        
        if old_init in content:
            content = content.replace(old_init, new_init)
            print(f"✅ Found and replaced: {old_init}")
            
            # Replace ALL platform usage with get_platform() calls
            # This is crucial to ensure lazy initialization works everywhere
            
            # Count replacements for logging
            replacements = 0
            
            # Pattern 1: platform.method() → get_platform().method()
            import re
            pattern1 = r'\bplatform\.([\w_]+)\('
            matches = re.findall(pattern1, content)
            replacements += len(matches)
            content = re.sub(pattern1, r'get_platform().\1(', content)
            
            # Pattern 2: platform.attribute → get_platform().attribute
            pattern2 = r'\bplatform\.([\w_]+)(?!\()'
            matches2 = re.findall(pattern2, content)
            replacements += len(matches2)
            content = re.sub(pattern2, r'get_platform().\1', content)
            
            # Pattern 3: return platform → return get_platform()
            content = content.replace('return platform\n', 'return get_platform()\n')
            
            print(f"✅ Replaced {replacements} platform references with get_platform()")
            
            # Write back
            with open(SERVER_FILE, 'w') as f:
                f.write(content)
            
            print("✅ server.py patched successfully")
            return True
        else:
            print(f"⚠️ Could not find initialization line: {old_init}")
            return False
            
    except Exception as e:
        print(f"❌ Error patching server.py: {e}")
        return False

if __name__ == "__main__":
    success = patch_server()
    sys.exit(0 if success else 1)
