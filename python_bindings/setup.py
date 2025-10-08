#!/usr/bin/env python3
"""
Setup script for BioMining C++ Python bindings
"""

import os
import sys
import subprocess
from pathlib import Path

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup, Extension

# Check if Qt6 is available
def find_qt6():
    """Find Qt6 installation"""
    qt_paths = [
        "/opt/qt6",
        "/usr/local/opt/qt6", 
        "/usr/lib/x86_64-linux-gnu/qt6",
        "/usr/share/qt6",
        os.environ.get("QT_DIR", "")
    ]
    
    for path in qt_paths:
        if os.path.exists(path):
            return path
    
    # Try to find with qmake6
    try:
        result = subprocess.run(['qmake6', '-query', 'QT_INSTALL_PREFIX'], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()
    except:
        pass
    
    return None

# Get Qt6 path
qt_path = find_qt6()
if not qt_path:
    print("Warning: Qt6 not found. Using system default paths.")
    qt_include = []
    qt_lib_dirs = []
    qt_libs = []
else:
    qt_include = [
        os.path.join(qt_path, "include"),
        os.path.join(qt_path, "include", "QtCore"),
        os.path.join(qt_path, "include", "QtNetwork"),
        os.path.join(qt_path, "include", "QtSerialPort"),
    ]
    qt_lib_dirs = [os.path.join(qt_path, "lib")]
    qt_libs = ["Qt6Core", "Qt6Network", "Qt6SerialPort"]

# Define the extension module
ext_modules = [
    Pybind11Extension(
        "biomining_cpp",
        [
            "biomining_python.cpp",
            "../src/cpp/hybrid_bitcoin_miner.cpp",
            "../src/cpp/bitcoin_miner.cpp", 
            "../src/cpp/real_mea_interface.cpp",
            "../src/cpp/biological_network.cpp",
            "../src/cpp/mea_interface.cpp",
        ],
        include_dirs=[
            "../include",
            "../src/cpp",
        ] + qt_include,
        library_dirs=qt_lib_dirs,
        libraries=qt_libs,
        language='c++',
        cxx_std=17,
    ),
]

# Build configuration
class CustomBuildExt(build_ext):
    """Custom build extension to handle Qt-specific compilation"""
    
    def build_extensions(self):
        # Add Qt-specific compiler flags
        for ext in self.extensions:
            ext.extra_compile_args.extend([
                '-DQT_CORE_LIB',
                '-DQT_NETWORK_LIB',
                '-DQT_SERIALPORT_LIB',
                '-std=c++17',
            ])
            
            if sys.platform == 'darwin':  # macOS
                ext.extra_compile_args.extend(['-stdlib=libc++'])
            elif sys.platform.startswith('linux'):  # Linux
                ext.extra_link_args.extend(['-Wl,-rpath,$ORIGIN'])
                
        super().build_extensions()

setup(
    name="biomining-cpp",
    version="1.0.0",
    author="BioMining Team",
    description="Python bindings for BioMining C++ classes",
    long_description="Python bindings for HybridBitcoinMiner and RealMEAInterface C++ classes",
    ext_modules=ext_modules,
    cmdclass={'build_ext': CustomBuildExt},
    zip_safe=False,
    python_requires=">=3.8",
    install_requires=[
        "pybind11>=2.6",
    ],
)