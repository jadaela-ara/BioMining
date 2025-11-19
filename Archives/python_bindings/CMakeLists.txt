cmake_minimum_required(VERSION 3.16)

project(biomining_python_bindings)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Network SerialPort)
find_package(pybind11 REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/cpp)

# Add the C++ source files
set(CPP_SOURCES
    ../src/cpp/hybrid_bitcoin_miner.cpp
    ../src/cpp/bitcoin_miner.cpp
    ../src/cpp/real_mea_interface.cpp
    ../src/cpp/biological_network.cpp
    ../src/cpp/mea_interface.cpp
    ../src/bio/ibio_compute_interface.cpp
)

# Create Python module
pybind11_add_module(biomining_cpp 
    biomining_python.cpp
    ${CPP_SOURCES}
)

# Link Qt libraries
target_link_libraries(biomining_cpp PRIVATE 
    Qt6::Core 
    Qt6::Network 
    Qt6::SerialPort
)

# Compiler-specific options
target_compile_definitions(biomining_cpp PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
target_compile_features(biomining_cpp PRIVATE cxx_std_17)

# Set output directory
set_target_properties(biomining_cpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../web/api/
)

# Additional compiler flags for Qt
if(MSVC)
    target_compile_options(biomining_cpp PRIVATE /bigobj)
endif()

# Install rules (optional)
install(TARGETS biomining_cpp
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
)
