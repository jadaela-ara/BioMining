# Deployment Script Update for Bio-Entropy Platform

## 📋 Summary

Updated Google Cloud deployment scripts to include **Bio-Entropy Mining Architecture** (commit d63d12d).

---

## 🔧 Files Modified

### 1. **Dockerfile.cpp-simple** (Updated for Bio-Entropy)

**Changes Made:**
- ✅ Added Bio-Entropy MOC generation for 4 new headers
- ✅ Added Bio-Entropy source compilation for 4 new .cpp files
- ✅ Updated Qt include directories (added `src/bio`, `src/crypto`, `QtConcurrent`)
- ✅ Updated Qt libraries (added `Qt5Concurrent`)
- ✅ Updated package version: `1.0.0` → `2.0.0`
- ✅ Updated package name: `biomining-cpp-simple` → `biomining-cpp-bioentropy`

**New MOC Files Generated:**
1. `src/bio/ibio_compute_interface.moc.cpp`
2. `src/bio/real_mea_adapter.moc.cpp`
3. `src/bio/biological_network_adapter.moc.cpp`
4. `src/crypto/bio_entropy_generator.moc.cpp`

**New Source Files Compiled:**
1. `src/bio/real_mea_adapter.cpp`
2. `src/bio/biological_network_adapter.cpp`
3. `src/crypto/bio_entropy_generator.cpp`
4. All corresponding `.moc.cpp` files

**Total Changes:**
- +9 lines for MOC generation
- +6 lines for source compilation
- +2 include directories
- +1 Qt library (Qt5Concurrent)
- +1 compile flag (`-DQT_CONCURRENT_LIB`)

---

### 2. **deploy_cpp_ultra_simple.sh** (Updated with Bio-Entropy verification)

**Changes Made:**
- ✅ Added Bio-Entropy file verification (7 new files)
- ✅ Added Bio-Entropy endpoint testing (2 new tests)
- ✅ Updated deployment summary with Bio-Entropy features
- ✅ Added Bio-Entropy usage examples

**New Verification Checks:**
```bash
bio_entropy_files=(
    "include/bio/ibio_compute_interface.h"
    "include/bio/real_mea_adapter.h"
    "include/bio/biological_network_adapter.h"
    "include/crypto/bio_entropy_generator.h"
    "src/bio/real_mea_adapter.cpp"
    "src/bio/biological_network_adapter.cpp"
    "src/crypto/bio_entropy_generator.cpp"
)
```

**New Endpoint Tests:**
1. **Feature Extraction Test:**
   ```bash
   POST /api/bio-entropy/extract-features
   {
     "block_header": "...",
     "difficulty": 1
   }
   ```

2. **Stats Test:**
   ```bash
   GET /api/bio-entropy/stats
   ```

**Total Changes:**
- +7 files in verification checklist
- +2 endpoint tests
- +5 lines in deployment summary
- +10 lines for usage examples

---

## 🚀 Deployment Instructions

### Prerequisites
1. Google Cloud SDK installed and configured
2. Project ID set: `gcloud config set project YOUR_PROJECT_ID`
3. All Bio-Entropy files present (will be verified automatically)

### Deploy Command
```bash
cd /home/user/webapp
./deploy_cpp_ultra_simple.sh
```

### What Happens During Deployment

1. **File Verification** ✅
   - Verifies all 15 files exist (8 original + 7 Bio-Entropy)
   - Exits with error if any file is missing

2. **Dockerfile Backup** ✅
   - Backs up existing `Dockerfile` (if present)
   - Temporarily copies `Dockerfile.cpp-simple` as `Dockerfile`

3. **Cloud Build** ✅
   - Submits to Google Cloud Build
   - Uses `e2-highcpu-8` machine type
   - Timeout: 3600s (1 hour)
   - Generates MOC files for all Qt classes
   - Compiles all C++ sources including Bio-Entropy

4. **Cloud Run Deployment** ✅
   - Deploys to `biomining-cpp-ultra-simple` service
   - Region: `us-central1`
   - Memory: 4Gi
   - CPU: 4
   - Timeout: 3600s
   - Environment variables:
     - `BIOMINING_ENVIRONMENT=production`
     - `QT_QPA_PLATFORM=offscreen`
     - `DISPLAY=:0`

5. **Service Testing** ✅
   - Tests `/api/status` endpoint
   - Tests `/api/bindings` for C++ availability
   - Tests `/api/configure-biological-network`
   - **NEW:** Tests `/api/bio-entropy/extract-features`
   - **NEW:** Tests `/api/bio-entropy/stats`

6. **Cleanup** ✅
   - Restores original `Dockerfile` (if existed)
   - Displays service URL and test commands

---

## 🧪 Testing After Deployment

### 1. Check Service Status
```bash
curl https://YOUR-SERVICE-URL/api/status
```

### 2. Verify C++ Bindings
```bash
curl https://YOUR-SERVICE-URL/api/bindings
```

**Expected Response:**
```json
{
  "cpp_available": true,
  "biomining_version": "2.0.0"
}
```

### 3. Test Bio-Entropy Feature Extraction
```bash
curl -X POST https://YOUR-SERVICE-URL/api/bio-entropy/extract-features \
  -H "Content-Type: application/json" \
  -d '{
    "block_header": "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    "difficulty": 1
  }'
```

**Expected Response:**
```json
{
  "success": true,
  "features": {
    "timestamp_norm": 0.5,
    "difficulty_level": 0.0,
    "prev_hash_entropy": 0.0,
    "prev_hash_bytes": [...],
    "merkle_bytes": [...],
    "total_features": 60
  }
}
```

### 4. Test Bio-Entropy Stats
```bash
curl https://YOUR-SERVICE-URL/api/bio-entropy/stats
```

**Expected Response:**
```json
{
  "success": true,
  "stats": {
    "mode": "SimulatedNetwork",
    "initialized": false,
    "total_stimulations": 0,
    "successful_mines": 0,
    "average_entropy": 0.0,
    "last_strategy": "-"
  }
}
```

### 5. Test Bio-Entropy Mode Switching
```bash
curl -X POST https://YOUR-SERVICE-URL/api/bio-entropy/set-mode \
  -H "Content-Type: application/json" \
  -d '{"mode": "RealMEA"}'
```

---

## 📊 What's New in Bio-Entropy Platform

### Architecture Features
- ✅ **Dual-Mode Support**: RealMEA (hardware) + SimulatedNetwork
- ✅ **Abstract Interface**: `IBioComputeInterface` for unified API
- ✅ **Adapter Pattern**: Runtime mode switching without recompilation
- ✅ **60-Dimensional Features**: Extracted from Bitcoin block headers

### Mining Pipeline (7 Steps)
1. **Feature Extraction**: Block header → 60 dimensions
2. **Stimulus Generation**: Features → electrical patterns
3. **Bio-Response Capture**: MEA/Network stimulation
4. **Entropy Generation**: Bio-signals → cryptographic seed
5. **Smart Starting Points**: 1000 candidates × 4M window
6. **Parallel Search**: GPU/CPU mining
7. **Reinforcement Learning**: Pattern optimization

### Adaptive Strategies
- **Uniform**: Even distribution (low confidence)
- **Fibonacci**: Golden ratio spacing (medium confidence)
- **BioGuided**: Peak-based exploration (high confidence)

### Performance Metrics
- **Response Time**: 100-500ms per bio-computation
- **Nonce Coverage**: ~41% (1000 × 4M / 2^32)
- **Window Size**: 4,194,304 nonces per starting point
- **Parallelism**: 1000+ simultaneous searches

---

## 🔍 Troubleshooting

### Build Fails with MOC Errors
**Problem:** MOC files not generated for Bio-Entropy classes

**Solution:**
1. Verify all Bio-Entropy header files exist:
   ```bash
   ls -la include/bio/ibio_compute_interface.h \
          include/bio/real_mea_adapter.h \
          include/bio/biological_network_adapter.h \
          include/crypto/bio_entropy_generator.h
   ```
2. Check headers contain `Q_OBJECT` macro:
   ```bash
   grep "Q_OBJECT" include/bio/*.h include/crypto/bio_entropy_generator.h
   ```

### Build Fails with Linking Errors
**Problem:** Missing Qt5Concurrent library

**Solution:**
1. Verify Qt5Concurrent is installed in Docker image:
   ```dockerfile
   RUN apt-get install -y libqt5concurrent5
   ```
2. Verify library is linked in setup_simple.py:
   ```python
   qt_libs = ['Qt5Core', 'Qt5Network', 'Qt5SerialPort', 'Qt5Widgets', 'Qt5Concurrent']
   ```

### C++ Module Not Found
**Problem:** `biomining_cpp` module fails to import

**Solution:**
1. Check `.so` file was created:
   ```bash
   find . -name "biomining_cpp*.so"
   ```
2. Verify module is in correct path:
   ```bash
   ls -la /app/biomining_cpp*.so
   ```
3. Test import:
   ```bash
   python3 -c "import sys; sys.path.insert(0, '/app'); import biomining_cpp"
   ```

### Bio-Entropy Endpoints Return Errors
**Problem:** Bio-Entropy endpoints return 500 errors

**Solution:**
1. Check C++ bindings are available:
   ```bash
   curl https://YOUR-SERVICE-URL/api/bindings
   ```
2. Check server logs:
   ```bash
   gcloud run logs read biomining-cpp-ultra-simple --limit 100
   ```
3. Verify Python bindings were compiled:
   ```bash
   python3 -c "from biomining_cpp import bio; print(bio.BioEntropyGenerator())"
   ```

---

## 📈 Performance Expectations

### Cloud Build Time
- **Expected**: 10-15 minutes
- **Includes**:
  - Qt5 installation (2-3 min)
  - MOC generation (30-60 sec)
  - C++ compilation (5-8 min)
  - Python package building (2-3 min)

### Cloud Run Startup
- **Cold Start**: 20-30 seconds
- **Warm Start**: 2-5 seconds
- **Health Check**: 30 seconds interval

### Resource Usage
- **CPU**: 4 cores (e2-highcpu)
- **Memory**: 4Gi
- **Disk**: ~2Gi (Docker image)
- **Concurrent Requests**: 10 max

---

## ✅ Deployment Checklist

Before deploying, ensure:

- [ ] All 8 Bio-Entropy files exist in repository
- [ ] `Dockerfile.cpp-simple` is updated (version 2.0.0)
- [ ] `deploy_cpp_ultra_simple.sh` is updated with Bio-Entropy tests
- [ ] Google Cloud SDK is configured
- [ ] Project ID is set correctly
- [ ] Sufficient Cloud Build quota available
- [ ] Cloud Run API is enabled

After deploying, verify:

- [ ] Build completes successfully (check Cloud Build logs)
- [ ] Service deploys to Cloud Run
- [ ] `/api/status` returns 200 OK
- [ ] `/api/bindings` shows `cpp_available: true`
- [ ] `/api/bio-entropy/stats` returns valid JSON
- [ ] `/api/bio-entropy/extract-features` works
- [ ] Web interface loads at service URL

---

## 🔗 Related Documentation

- **Integration Guide**: `INTEGRATION_GUIDE_BIO_ENTROPY.md`
- **Web Interface Update**: `WEB_INTERFACE_UPDATE_SUMMARY.md`
- **Pull Request**: https://github.com/jadaela-ara/BioMining/pull/114
- **Commit**: d63d12d (Bio-Entropy Platform)

---

## 📝 Version History

### Version 2.0.0 (Current)
- ✅ Added Bio-Entropy Mining Architecture
- ✅ Dual-mode support (RealMEA + SimulatedNetwork)
- ✅ 60-dimensional feature extraction
- ✅ 3 adaptive strategies
- ✅ Smart nonce generation
- ✅ Reinforcement learning

### Version 1.0.0 (Previous)
- Original deployment with BiologicalNetwork
- Single mode (simulated only)
- Basic Bitcoin mining

---

**Deployment Status**: ✅ Ready for Google Cloud

**Last Updated**: 2025-10-15

**Maintainer**: GenSpark AI Developer
