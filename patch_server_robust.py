#!/usr/bin/env python3
"""
Robust patch for server.py to add lazy initialization with proper error handling
This version ensures ALL platform references are replaced
"""

import sys
import os
import re

SERVER_FILE = "/app/web/api/server.py"

# Check if running locally or in Docker
import os
if not os.path.exists(SERVER_FILE):
    # Try local path
    SERVER_FILE = os.path.join(os.path.dirname(__file__), "web/api/server.py")

def patch_server():
    """Apply comprehensive patch to server.py"""
    
    print("🔧 Applying robust lazy initialization patch...")
    
    try:
        with open(SERVER_FILE, 'r') as f:
            content = f.read()
        
        original_content = content
        
        # Step 1: Replace platform initialization
        old_init = "platform = BioMiningPlatform()"
        new_init = '''# Lazy initialization with error handling
platform = None
_platform_error = None

def get_platform():
    """Get platform instance with lazy initialization and error handling"""
    global platform, _platform_error
    
    if platform is None and _platform_error is None:
        try:
            print("🚀 Initializing BioMining Platform (lazy load)...")
            platform = BioMiningPlatform()
            print("✅ Platform initialized successfully")
        except Exception as e:
            _platform_error = str(e)
            print(f"⚠️ Failed to initialize BioMiningPlatform: {e}")
            print(f"   Error type: {type(e).__name__}")
            if 'pybind11' in str(e):
                print("   → Detected pybind11 error, using fallback")
            
            # Create fallback dummy platform
            class DummyPlatform:
                is_mining = False
                is_training = False
                systems_status = {
                    'mea_interface': {'status': 'fallback'},
                    'biological_network': {'status': 'fallback'},
                    'hybrid_miner': {'status': 'fallback'}
                }
                
                def get_platform_status(self):
                    return {
                        "status": "fallback", 
                        "mode": "python_only",
                        "error": _platform_error,
                        "systems": self.systems_status
                    }
                
                def get_performance_metrics(self):
                    return {"mode": "fallback", "error": _platform_error}
                
                async def stop_hybrid_mining(self):
                    pass
                
                async def start_hybrid_mining(self, *args, **kwargs):
                    return {"status": "error", "message": "C++ bindings unavailable"}
            
            platform = DummyPlatform()
            print("📦 Using DummyPlatform fallback")
    
    return platform'''
        
        if old_init in content:
            content = content.replace(old_init, new_init)
            print(f"✅ Step 1: Replaced platform initialization")
        else:
            print(f"⚠️ Step 1: Initialization line not found (may already be patched)")
        
        # Step 2: Replace ALL direct platform references
        # Use word boundaries to avoid replacing "platform_status" etc.
        
        # Count before
        before_count = len(re.findall(r'\bplatform\.', content))
        
        # Replace platform.method() and platform.attribute
        # But NOT platform = ... or get_platform
        content = re.sub(
            r'(?<!def )(?<!return )(?<!get_)\bplatform\.',
            'get_platform().',
            content
        )
        
        # Count after
        after_count = len(re.findall(r'\bplatform\.', content))
        replaced = before_count - after_count
        
        print(f"✅ Step 2: Replaced {replaced} platform references with get_platform()")
        
        # Step 3: Fix any remaining edge cases in specific contexts
        # In async functions, ensure proper usage
        content = re.sub(
            r'await platform\.stop_hybrid_mining\(\)',
            'await get_platform().stop_hybrid_mining()',
            content
        )
        
        content = re.sub(
            r'platform\.is_training = ',
            'get_platform().is_training = ',
            content
        )
        
        print(f"✅ Step 3: Fixed async and assignment edge cases")
        
        # Step 4: Verify changes were made
        if content != original_content:
            # Write back
            with open(SERVER_FILE, 'w') as f:
                f.write(content)
            
            print("✅ Patch applied successfully")
            print(f"   Total changes: {len(content) - len(original_content)} characters")
            return True
        else:
            print("⚠️ No changes made (file may already be patched)")
            return True
            
    except FileNotFoundError:
        print(f"❌ File not found: {SERVER_FILE}")
        return False
    except Exception as e:
        print(f"❌ Error patching server.py: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    success = patch_server()
    sys.exit(0 if success else 1)
