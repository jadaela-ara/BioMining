#!/bin/bash
# BioMining Platform - Script de Test Complet
# Tests unitaires, intégration et performance

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
TEST_SERVER_URL="${TEST_SERVER_URL:-http://localhost:8080}"

echo "🧪 === BIOMINING PLATFORM TESTS ==="
echo "=================================="
echo "Project Dir: $PROJECT_DIR"
echo "Test Server: $TEST_SERVER_URL"
echo "=================================="

# ====================================================================
# FONCTIONS UTILITAIRES
# ====================================================================

log_test() {
    echo "🧪 $1..."
}

log_success() {
    echo "✅ $1"
}

log_error() {
    echo "❌ $1"
    return 1
}

wait_for_server() {
    local url="$1"
    local timeout="${2:-30}"
    
    log_test "Waiting for server at $url"
    
    for i in $(seq 1 $timeout); do
        if curl -f -s "$url/health" > /dev/null 2>&1; then
            log_success "Server is ready"
            return 0
        fi
        sleep 1
    done
    
    log_error "Server not ready after ${timeout}s"
    return 1
}

# ====================================================================
# TESTS DE BUILD
# ====================================================================

test_build() {
    log_test "Testing build system"
    
    cd "$PROJECT_DIR"
    
    # Nettoyage
    rm -rf build-test
    mkdir build-test
    cd build-test
    
    # Configuration CMake
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON .. || log_error "CMake configuration failed"
    
    # Build
    make -j$(nproc) || log_error "Build failed"
    
    # Vérifier que l'exécutable existe
    if [[ ! -f "biomining_server" ]]; then
        log_error "Executable not found"
    fi
    
    log_success "Build test passed"
}

# ====================================================================
# TESTS UNITAIRES
# ====================================================================

test_unit() {
    log_test "Running unit tests"
    
    cd "$PROJECT_DIR/build-test"
    
    # Test help
    if ! ./biomining_server --help | grep -q "BioMining Platform"; then
        log_error "Help output incorrect"
    fi
    
    # Test version
    if ! ./biomining_server --version 2>&1 | grep -q "1.0.0"; then
        log_error "Version output incorrect"  
    fi
    
    # Test configuration loading
    if ! ./biomining_server --config /nonexistent/file.json 2>&1 | grep -q "Config file not found"; then
        log_error "Config file handling incorrect"
    fi
    
    log_success "Unit tests passed"
}

# ====================================================================
# TESTS D'INTÉGRATION API
# ====================================================================

test_api_health() {
    log_test "Testing health endpoint"
    
    local response
    response=$(curl -f -s "$TEST_SERVER_URL/health")
    
    # Vérifier format JSON
    if ! echo "$response" | jq -e '.status == "healthy"' > /dev/null; then
        log_error "Health check failed: $response"
    fi
    
    log_success "Health endpoint test passed"
}

test_api_status() {
    log_test "Testing status endpoint"
    
    local response
    response=$(curl -f -s "$TEST_SERVER_URL/api/status")
    
    # Vérifier présence des champs requis
    if ! echo "$response" | jq -e '.timestamp and .network_type and .configuration' > /dev/null; then
        log_error "Status endpoint missing required fields: $response"
    fi
    
    log_success "Status endpoint test passed"
}

test_api_metrics() {
    log_test "Testing metrics endpoint"
    
    local response
    response=$(curl -f -s "$TEST_SERVER_URL/api/metrics")
    
    # Vérifier présence des métriques système
    if ! echo "$response" | jq -e '.timestamp and .system' > /dev/null; then
        log_error "Metrics endpoint missing system data: $response"
    fi
    
    log_success "Metrics endpoint test passed"
}

test_api_configure() {
    log_test "Testing configuration endpoint"
    
    local config='{
        "mea": {
            "num_electrodes": 30
        },
        "network": {
            "type": "biological"
        }
    }'
    
    local response
    response=$(curl -f -s -X POST \
        -H "Content-Type: application/json" \
        -d "$config" \
        "$TEST_SERVER_URL/api/configure")
    
    # Vérifier succès
    if ! echo "$response" | jq -e '.success == true' > /dev/null; then
        log_error "Configuration update failed: $response"
    fi
    
    log_success "Configuration endpoint test passed"
}

test_api_training() {
    log_test "Testing training endpoints"
    
    # Test start training (should fail without data)
    local response
    response=$(curl -s -X POST "$TEST_SERVER_URL/api/train/start" || echo '{"success":false}')
    
    # Devrait échouer sans données
    if echo "$response" | jq -e '.success == true' > /dev/null; then
        log_error "Training should fail without data"
    fi
    
    log_success "Training endpoint test passed"
}

test_api_mining() {
    log_test "Testing mining endpoints"
    
    # Start mining
    local response
    response=$(curl -f -s -X POST "$TEST_SERVER_URL/api/mine/start")
    
    if ! echo "$response" | jq -e '.success == true' > /dev/null; then
        log_error "Mining start failed: $response"
    fi
    
    # Check status
    sleep 2
    response=$(curl -f -s "$TEST_SERVER_URL/api/mine/status" || echo '{}')
    
    if ! echo "$response" | jq -e '.active == true' > /dev/null; then
        log_error "Mining status incorrect: $response"
    fi
    
    # Stop mining
    response=$(curl -f -s -X POST "$TEST_SERVER_URL/api/mine/stop")
    
    if ! echo "$response" | jq -e '.success == true' > /dev/null; then
        log_error "Mining stop failed: $response"
    fi
    
    log_success "Mining endpoints test passed"
}

# ====================================================================
# TESTS DE PERFORMANCE
# ====================================================================

test_performance_latency() {
    log_test "Testing API latency"
    
    # Test latence health endpoint
    local latency
    latency=$(curl -w "%{time_total}" -s -o /dev/null "$TEST_SERVER_URL/health")
    
    # Latence doit être < 100ms
    if (( $(echo "$latency > 0.1" | bc -l) )); then
        log_error "Health endpoint too slow: ${latency}s"
    fi
    
    log_success "Latency test passed (${latency}s)"
}

test_performance_concurrent() {
    log_test "Testing concurrent requests"
    
    # Test avec Apache Bench si disponible
    if command -v ab > /dev/null; then
        ab -n 50 -c 5 -q "$TEST_SERVER_URL/health" > /dev/null || log_error "Concurrent test failed"
        log_success "Concurrent requests test passed"
    else
        # Test manuel avec curl en parallèle
        for i in {1..10}; do
            curl -f -s "$TEST_SERVER_URL/health" > /dev/null &
        done
        wait
        log_success "Manual concurrent test passed"
    fi
}

test_performance_memory() {
    log_test "Testing memory usage"
    
    # Vérifier utilisation mémoire via métriques
    local response
    response=$(curl -f -s "$TEST_SERVER_URL/api/metrics")
    
    local memory_usage
    memory_usage=$(echo "$response" | jq -r '.system.memory_max_rss // 0')
    
    # Convertir en MB (RSS est en KB sur Linux)
    local memory_mb=$((memory_usage / 1024))
    
    if (( memory_mb > 2048 )); then
        log_error "Memory usage too high: ${memory_mb}MB"
    fi
    
    log_success "Memory test passed (${memory_mb}MB)"
}

# ====================================================================
# TESTS DE SÉCURITÉ
# ====================================================================

test_security_headers() {
    log_test "Testing security headers"
    
    # Test CORS headers
    local headers
    headers=$(curl -s -I "$TEST_SERVER_URL/api/status")
    
    if ! echo "$headers" | grep -q "Access-Control-Allow-Origin"; then
        log_error "CORS headers missing"
    fi
    
    log_success "Security headers test passed"
}

test_security_injection() {
    log_test "Testing injection attacks"
    
    # Test SQL injection tentative (should be safe)
    local malicious_config='{
        "mea": {
            "num_electrodes": "1; DROP TABLE users;--"
        }
    }'
    
    curl -s -X POST \
        -H "Content-Type: application/json" \
        -d "$malicious_config" \
        "$TEST_SERVER_URL/api/configure" > /dev/null || true
    
    # Serveur doit toujours répondre normalement
    if ! curl -f -s "$TEST_SERVER_URL/health" > /dev/null; then
        log_error "Server crashed after injection test"
    fi
    
    log_success "Injection attack test passed"
}

# ====================================================================
# TESTS DOCKER
# ====================================================================

test_docker_build() {
    log_test "Testing Docker build"
    
    cd "$PROJECT_DIR"
    
    # Build image de test
    docker build -t biomining:test . || log_error "Docker build failed"
    
    log_success "Docker build test passed"
}

test_docker_run() {
    log_test "Testing Docker container"
    
    # Lancer container en arrière-plan
    local container_id
    container_id=$(docker run -d -p 8081:8080 biomining:test)
    
    # Attendre que le container soit prêt
    sleep 10
    
    # Test health check
    if ! curl -f -s http://localhost:8081/health > /dev/null; then
        docker logs "$container_id"
        docker stop "$container_id"
        log_error "Docker container health check failed"
    fi
    
    # Nettoyage
    docker stop "$container_id" > /dev/null
    docker rm "$container_id" > /dev/null
    
    log_success "Docker container test passed"
}

# ====================================================================
# FONCTION PRINCIPALE
# ====================================================================

run_all_tests() {
    local failed_tests=0
    
    echo "🚀 Starting comprehensive test suite"
    echo ""
    
    # Tests de build
    test_build || ((failed_tests++))
    test_unit || ((failed_tests++))
    
    # Tests Docker (optionnels)
    if command -v docker > /dev/null; then
        test_docker_build || ((failed_tests++))
        # test_docker_run || ((failed_tests++))  # Commenté pour éviter les conflits de port
    fi
    
    # Tests API (nécessitent un serveur en cours)
    if curl -f -s "$TEST_SERVER_URL/health" > /dev/null 2>&1; then
        echo ""
        echo "🔗 Server detected, running API tests..."
        
        test_api_health || ((failed_tests++))
        test_api_status || ((failed_tests++))
        test_api_metrics || ((failed_tests++))
        test_api_configure || ((failed_tests++))
        test_api_training || ((failed_tests++))
        test_api_mining || ((failed_tests++))
        
        # Tests de performance
        echo ""
        echo "⚡ Running performance tests..."
        test_performance_latency || ((failed_tests++))
        test_performance_concurrent || ((failed_tests++))
        test_performance_memory || ((failed_tests++))
        
        # Tests de sécurité
        echo ""
        echo "🔒 Running security tests..."
        test_security_headers || ((failed_tests++))
        test_security_injection || ((failed_tests++))
        
    else
        echo ""
        echo "⚠️  No server detected at $TEST_SERVER_URL"
        echo "   Start server with: ./biomining_server --config config/biomining_config.json"
        echo "   Skipping API tests..."
    fi
    
    # Résumé final
    echo ""
    echo "📊 === TEST RESULTS ==="
    if (( failed_tests == 0 )); then
        echo "🎉 All tests passed successfully!"
        return 0
    else
        echo "❌ $failed_tests test(s) failed"
        return 1
    fi
}

# ====================================================================
# GESTION DES OPTIONS
# ====================================================================

show_help() {
    cat << EOF
BioMining Platform - Test Suite

Usage: $0 [OPTIONS] [TEST_TYPE]

Options:
  --help                 Show this help
  --server-url URL       Test server URL (default: http://localhost:8080)
  --timeout SECONDS      Server wait timeout (default: 30)

Test Types:
  build                  Build and unit tests only
  api                    API integration tests only
  performance            Performance tests only
  security               Security tests only
  docker                 Docker build/run tests only
  all                    All tests (default)

Examples:
  $0                     Run all tests
  $0 api                 Run API tests only
  $0 --server-url http://staging.example.com api
  
EOF
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_help
            exit 0
            ;;
        --server-url)
            TEST_SERVER_URL="$2"
            shift 2
            ;;
        --timeout)
            SERVER_TIMEOUT="$2"
            shift 2
            ;;
        build|api|performance|security|docker|all)
            TEST_TYPE="$1"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Exécution des tests
case "${TEST_TYPE:-all}" in
    build)
        test_build && test_unit
        ;;
    api)
        wait_for_server "$TEST_SERVER_URL" "${SERVER_TIMEOUT:-30}"
        test_api_health && test_api_status && test_api_metrics && test_api_configure && test_api_training && test_api_mining
        ;;
    performance)
        wait_for_server "$TEST_SERVER_URL" "${SERVER_TIMEOUT:-30}"
        test_performance_latency && test_performance_concurrent && test_performance_memory
        ;;
    security)
        wait_for_server "$TEST_SERVER_URL" "${SERVER_TIMEOUT:-30}"
        test_security_headers && test_security_injection
        ;;
    docker)
        test_docker_build && test_docker_run
        ;;
    all)
        run_all_tests
        ;;
esac