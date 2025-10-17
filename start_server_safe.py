#!/usr/bin/env python3
"""
Safe server startup wrapper for BioMining Platform
Handles C++ binding errors gracefully
"""

import os
import sys

# Set environment for fallback mode if C++ fails
os.environ.setdefault('PYTHONPATH', '/app')
os.environ.setdefault('BIOMINING_ENVIRONMENT', 'production')
os.environ.setdefault('QT_QPA_PLATFORM', 'offscreen')
os.environ.setdefault('DISPLAY', ':0')
os.environ.setdefault('PYTHONUNBUFFERED', '1')

print("🚀 BioMining Platform - Safe Startup Wrapper")
print("=" * 60)

# Test 1: Try importing the server module
print("\n📦 Testing server module import...")
try:
    sys.path.insert(0, '/app')
    from web.api import server
    print("✅ Server module imported successfully")
    
    # Test 2: Check if C++ bindings are available
    if hasattr(server, 'CPP_BINDINGS_AVAILABLE'):
        if server.CPP_BINDINGS_AVAILABLE:
            print("✅ C++ bindings detected as available")
            print(f"   Module: {server.biomining_cpp}")
        else:
            print("⚠️  C++ bindings not available - using Python fallback")
    
except Exception as e:
    print(f"❌ Error during server module import: {e}")
    print(f"   Error type: {type(e).__name__}")
    print(f"   Error details: {str(e)}")
    
    # Check if it's a pybind11 error
    if 'pybind11' in str(e) or 'get_value_and_holder' in str(e):
        print("\n🔧 Detected pybind11 binding error")
        print("   → Forcing Python fallback mode")
        
        # Force disable C++ bindings
        os.environ['CPP_BINDINGS_DISABLED'] = '1'
        
        # Try to patch the module before import
        print("\n🔄 Attempting to reload with Python fallback...")
        
        # Remove the failed import from cache
        if 'web.api.server' in sys.modules:
            del sys.modules['web.api.server']
        if 'biomining_cpp' in sys.modules:
            del sys.modules['biomining_cpp']
        
        # Create a dummy biomining_cpp module that fails import
        import types
        dummy_module = types.ModuleType('biomining_cpp')
        sys.modules['biomining_cpp'] = dummy_module
        
        # Try importing again
        try:
            from web.api import server
            print("✅ Server module loaded in Python fallback mode")
        except Exception as e2:
            print(f"❌ Failed to load server even in fallback mode: {e2}")
            sys.exit(1)
    else:
        print("❌ Unrecoverable error - cannot start server")
        import traceback
        traceback.print_exc()
        sys.exit(1)

# Start the server
print("\n" + "=" * 60)
print("🌐 Starting Uvicorn server...")
print("=" * 60 + "\n")

import uvicorn

uvicorn.run(
    "web.api.server:app",
    host="0.0.0.0",
    port=8080,
    log_level="info",
    access_log=True
)
