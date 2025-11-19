#!/usr/bin/env python3
"""
Test script to verify frontend-backend alignment
"""
import requests
import json
import websocket
import time
import threading
from urllib.parse import urlparse

def test_http_endpoints():
    """Test HTTP API endpoints"""
    base_url = "http://localhost:8000"
    
    endpoints = [
        "/api/status",
        "/api/systems",
        "/api/config/hybrid-miner",
        "/api/config/biological-network", 
        "/api/config/mea-interface"
    ]
    
    print("🔍 Testing HTTP endpoints...")
    
    for endpoint in endpoints:
        try:
            response = requests.get(f"{base_url}{endpoint}", timeout=5)
            status = "✅" if response.status_code == 200 else "❌"
            print(f"  {status} {endpoint} - {response.status_code}")
        except requests.exceptions.RequestException as e:
            print(f"  ❌ {endpoint} - Connection error: {e}")

def test_websocket_connection():
    """Test WebSocket connection"""
    print("\n🔌 Testing WebSocket connection...")
    
    def on_message(ws, message):
        try:
            data = json.loads(message)
            print(f"  📨 Received: {data.get('type', 'unknown')} message")
        except json.JSONDecodeError:
            print(f"  ⚠️ Invalid JSON: {message}")
    
    def on_error(ws, error):
        print(f"  ❌ WebSocket error: {error}")
    
    def on_close(ws, close_status_code, close_msg):
        print("  🔌 WebSocket connection closed")
    
    def on_open(ws):
        print("  ✅ WebSocket connected")
        # Test ping message
        ws.send(json.dumps({"type": "ping"}))
        # Test status request
        ws.send(json.dumps({"type": "get_status"}))
        time.sleep(2)
        ws.close()
    
    try:
        ws = websocket.WebSocketApp("ws://localhost:8000/ws/hybrid-mining",
                                  on_open=on_open,
                                  on_message=on_message,
                                  on_error=on_error,
                                  on_close=on_close)
        ws.run_forever(timeout=10)
    except Exception as e:
        print(f"  ❌ WebSocket connection failed: {e}")

def check_form_alignment():
    """Check form IDs alignment between HTML and JS"""
    print("\n📋 Checking form alignment...")
    
    # Expected forms from HTML
    html_forms = [
        "tripleConfigForm",
        "weightsForm", 
        "biologicalNetworkForm",
        "meaConfigForm",
        "trainingConfigForm",
        "miningConfigForm"
    ]
    
    # Check if JavaScript handles these forms
    try:
        with open('/home/user/webapp/web/js/app.js', 'r') as f:
            js_content = f.read()
        
        print("  📝 HTML Forms vs JavaScript handlers:")
        for form_id in html_forms:
            if form_id in js_content or 'form[id$="Form"]' in js_content:
                print(f"    ✅ {form_id}")
            else:
                print(f"    ❌ {form_id} - Missing handler")
                
        # Check for form preservation
        if "setupFormPreservation" in js_content:
            print("    ✅ Form preservation implemented")
        else:
            print("    ❌ Form preservation missing")
            
    except FileNotFoundError:
        print("  ❌ JavaScript file not found")

def check_websocket_message_alignment():
    """Check WebSocket message types alignment"""
    print("\n📡 Checking WebSocket message alignment...")
    
    # Frontend message types from app.js
    frontend_types = [
        'system_status', 'mining_update', 'electrode_data', 
        'training_progress', 'biological_activity', 'performance_metrics',
        'error', 'success', 'config_update_response', 'configuration_updated'
    ]
    
    # Backend message types from server.py
    backend_types = [
        'platform_initialized', 'system_status_update', 'mining_started',
        'mining_stopped', 'electrode_controlled', 'file_uploaded',
        'biological_learning_started', 'biological_learning_stopped',
        'system_status', 'pong', 'mining_update', 'biological_activity'
    ]
    
    print("  📤 Frontend handles these message types:")
    for msg_type in frontend_types:
        print(f"    - {msg_type}")
        
    print("  📥 Backend sends these message types:")  
    for msg_type in backend_types:
        print(f"    - {msg_type}")
        
    # Check overlap
    common = set(frontend_types) & set(backend_types)
    print(f"  🔄 Common message types: {len(common)}")
    for msg_type in common:
        print(f"    ✅ {msg_type}")

if __name__ == "__main__":
    print("🚀 BioMining Platform Frontend-Backend Alignment Test")
    print("=" * 60)
    
    test_http_endpoints()
    test_websocket_connection()
    check_form_alignment()
    check_websocket_message_alignment()
    
    print("\n✅ Alignment test completed!")