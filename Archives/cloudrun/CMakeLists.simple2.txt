cmake_minimum_required(VERSION 3.16)
project(BioMining VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Create executable
add_executable(biomining_server src/simple_server.cpp)

# Link libraries
target_link_libraries(biomining_server 
    PRIVATE 
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Compiler options
target_compile_options(biomining_server PRIVATE 
    -O3 -DNDEBUG 
    -Wall -Wextra 
    -Wno-deprecated-declarations
)

message(STATUS "BioMining Platform - Simple Build")
message(STATUS "Executable: biomining_server")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")