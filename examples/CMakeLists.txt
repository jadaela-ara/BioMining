# CMakeLists.txt pour les exemples

# Exemple simple d'utilisation MEA
add_executable(example_mea_basic
    basic_mea_usage.cpp
)
target_link_libraries(example_mea_basic
    biomining_core
    Qt6::Core
)

# Exemple de mining simple
add_executable(example_mining_basic
    basic_mining_usage.cpp  
)
target_link_libraries(example_mining_basic
    biomining_core
    Qt6::Core
)

# Exemple d'intégration complète
add_executable(example_complete_workflow
    complete_workflow.cpp
)
target_link_libraries(example_complete_workflow
    biomining_core
    Qt6::Core
)

# Exemple de benchmark personnalisé
add_executable(example_custom_benchmark
    custom_benchmark.cpp
)
target_link_libraries(example_custom_benchmark
    biomining_core
    Qt6::Core
)

# Installation des exemples
install(TARGETS 
    example_mea_basic 
    example_mining_basic 
    example_complete_workflow
    example_custom_benchmark
    RUNTIME DESTINATION bin/examples
)