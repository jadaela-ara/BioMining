cmake_minimum_required(VERSION 3.16)

project(BioCryptoMiningPlatform 
        VERSION 1.0.0 
        DESCRIPTION "Plateforme Hybride Bio-Informatique pour Mining Bitcoin"
        LANGUAGES CXX)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options de compilation
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Options du projet
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_QUANTUM_OPTIMIZATION "Enable Q+ quantum optimization" OFF)
option(ENABLE_PROFILING "Enable performance profiling" OFF)

# Recherche des dépendances Qt6
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Network 
    Concurrent
)

# Charts est optionnel pour certains environnements
find_package(Qt6 COMPONENTS Charts QUIET)
if(Qt6Charts_FOUND)
    message(STATUS "Qt6 Charts found - GUI features enabled")
    set(CHARTS_AVAILABLE TRUE)
else()
    message(STATUS "Qt6 Charts not found - GUI will use basic components")
    set(CHARTS_AVAILABLE FALSE)
endif()

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Boost (optionnel pour certaines optimisations)
# Compatible avec Cloud Shell et autres environnements
set(Boost_NO_WARN_NEW_VERSIONS 1)
if(NOT DEFINED CMAKE_DISABLE_FIND_PACKAGE_Boost)
    find_package(Boost COMPONENTS system filesystem thread QUIET)
endif()

if(Boost_FOUND AND NOT CMAKE_DISABLE_FIND_PACKAGE_Boost)
    message(STATUS "Boost found - enabling advanced features")
    add_compile_definitions(BOOST_AVAILABLE)
else()
    message(STATUS "Boost not found or disabled - using standard library alternatives")
    add_compile_definitions(NO_BOOST_AVAILABLE)
endif()

# Configuration des répertoires
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Répertoires d'inclusion
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# === BIBLIOTHÈQUE PRINCIPALE ===

# Sources de la bibliothèque core
set(LIB_SOURCES
    src/cpp/mea_interface.cpp
    src/cpp/bitcoin_miner.cpp
    src/cpp/biological_network.cpp
    src/cpp/hybrid_bitcoin_miner.cpp
)

# Headers
set(LIB_HEADERS
    include/bio/mea_interface.h
    include/crypto/bitcoin_miner.h
    include/bio/biological_network.h
    include/crypto/hybrid_bitcoin_miner.h
)

# Création de la bibliothèque statique
add_library(biomining_core STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# Activation du MOC pour la bibliothèque
set_target_properties(biomining_core PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

target_link_libraries(biomining_core
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

if(Boost_FOUND)
    target_link_libraries(biomining_core ${Boost_LIBRARIES})
    target_include_directories(biomining_core PRIVATE ${Boost_INCLUDE_DIRS})
endif()

# Propriétés de la bibliothèque
set_target_properties(biomining_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# === APPLICATION PRINCIPALE ===

# Sources de l'interface graphique
set(GUI_SOURCES
    src/cpp/mainwindow.cpp
    src/cpp/mainwindow_slots.cpp
    src/cpp/main.cpp
)

set(GUI_HEADERS
    include/mainwindow.h
)

# Création de l'exécutable principal
add_executable(biomining_gui ${GUI_SOURCES} ${GUI_HEADERS})

target_link_libraries(biomining_gui
    biomining_core
    Qt6::Widgets
)

# Ajouter Qt6::Charts seulement si disponible
if(CHARTS_AVAILABLE)
    target_link_libraries(biomining_gui Qt6::Charts)
    target_compile_definitions(biomining_gui PRIVATE CHARTS_AVAILABLE)
endif()

# MOC pour Qt
set_target_properties(biomining_gui PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# === APPLICATION CONSOLE (CLI) ===

set(CLI_SOURCES
    src/cpp/cli_main.cpp
)

add_executable(biomining_cli ${CLI_SOURCES})

target_link_libraries(biomining_cli
    biomining_core
)

set_target_properties(biomining_cli PROPERTIES
    AUTOMOC ON
)

# === COMPOSANTS QUANTUM (Q+) ===

if(ENABLE_QUANTUM_OPTIMIZATION)
    message(STATUS "Quantum optimization enabled")
    
    # Recherche de Qiskit ou autres bibliothèques quantiques
    find_path(QISKIT_INCLUDE_DIR qiskit.h PATHS /usr/local/include /opt/qiskit/include)
    find_library(QISKIT_LIBRARY qiskit PATHS /usr/local/lib /opt/qiskit/lib)
    
    if(QISKIT_INCLUDE_DIR AND QISKIT_LIBRARY)
        add_compile_definitions(QUANTUM_ENABLED)
        
        set(QUANTUM_SOURCES
            src/qplus/quantum_optimizer.cpp
            src/qplus/quantum_hash_accelerator.cpp
        )
        
        add_library(biomining_quantum STATIC ${QUANTUM_SOURCES})
        target_include_directories(biomining_quantum PRIVATE ${QISKIT_INCLUDE_DIR})
        target_link_libraries(biomining_quantum ${QISKIT_LIBRARY})
        
        # Lien avec les autres composants
        target_link_libraries(biomining_core biomining_quantum)
    else()
        message(WARNING "Quantum libraries not found - quantum features disabled")
    endif()
endif()

# === TESTS UNITAIRES ===

if(BUILD_TESTS)
    enable_testing()
    
    find_package(Qt6 REQUIRED COMPONENTS Test)
    
    set(TEST_SOURCES
        tests/test_mea_interface.cpp
        tests/test_bitcoin_miner.cpp
        tests/test_biological_network.cpp
        tests/test_hybrid_bitcoin_miner.cpp
        tests/test_integration.cpp
    )
    
    foreach(test_source ${TEST_SOURCES})
        get_filename_component(test_name ${test_source} NAME_WE)
        add_executable(${test_name} ${test_source})
        
        target_link_libraries(${test_name}
            biomining_core
            Qt6::Test
            Qt6::Core
        )
        
        set_target_properties(${test_name} PROPERTIES
            AUTOMOC ON
        )
        
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

# === EXEMPLES ===

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# === INSTALLATION ===

# Installation des binaires
install(TARGETS biomining_gui biomining_cli biomining_core
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Installation des headers
install(DIRECTORY include/ 
    DESTINATION include/biomining
    FILES_MATCHING PATTERN "*.h"
)

# Installation de la documentation
install(FILES README.md DESTINATION share/doc/biomining)
install(DIRECTORY docs/ DESTINATION share/doc/biomining OPTIONAL)

# Installation des exemples
if(BUILD_EXAMPLES)
    install(DIRECTORY examples/ DESTINATION share/biomining/examples)
endif()

# === CONFIGURATION POUR LE PACKAGING ===

set(CPACK_PACKAGE_NAME "BioCryptoMiningPlatform")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Bio-Crypto Research Lab")
set(CPACK_PACKAGE_CONTACT "research@biocrypto.lab")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Générateurs de packages
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-dev, libssl-dev")

include(CPack)

# === INFORMATIONS DE BUILD ===

message(STATUS "=== BIO-CRYPTO MINING PLATFORM ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "OpenSSL Version: ${OPENSSL_VERSION}")

if(Boost_FOUND)
    message(STATUS "Boost Version: ${Boost_VERSION}")
endif()

message(STATUS "Options:")
message(STATUS "  BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "  BUILD_EXAMPLES: ${BUILD_EXAMPLES}")
message(STATUS "  ENABLE_QUANTUM_OPTIMIZATION: ${ENABLE_QUANTUM_OPTIMIZATION}")
message(STATUS "  ENABLE_PROFILING: ${ENABLE_PROFILING}")

message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "===================================")

# === TARGETS PERSONNALISÉS ===

# Target pour nettoyer complètement
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all build files"
)

# Target pour générer la documentation
find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation with Doxygen"
    )
endif()

# Target pour les benchmarks de performance
if(ENABLE_PROFILING)
    add_custom_target(benchmark
        COMMAND ${CMAKE_BINARY_DIR}/bin/biomining_cli --benchmark
        DEPENDS biomining_cli
        COMMENT "Running performance benchmarks"
    )
endif()

# Target pour validation du code
add_custom_target(check
    COMMAND cppcheck --enable=all --std=c++17 ${CMAKE_SOURCE_DIR}/src
    COMMENT "Running static code analysis"
)

# === CONFIGURATION DÉVELOPPEMENT ===

# Support pour les IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(biomining_core biomining_gui biomining_cli
    PROPERTIES FOLDER "Main"
)

# Export des symboles de compilation pour les IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)