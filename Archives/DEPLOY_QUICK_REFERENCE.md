# 🚀 Quick Deployment Reference - Bio-Entropy Platform

## ⚡ One-Command Deployment

```bash
cd /home/user/webapp && ./deploy_cpp_ultra_simple.sh
```

---

## 📋 What Gets Deployed

### C++ Components (16 files compiled)
| Component | Files | Purpose |
|-----------|-------|---------|
| **Original** | 6 .cpp + 6 .moc.cpp | BiologicalNetwork, BitcoinMiner, MEA interfaces |
| **Bio-Entropy** | 3 .cpp + 3 .moc.cpp | Adapters, entropy generator |
| **Bindings** | 1 biomining_python.cpp | Pybind11 interface |

### Python Backend
- FastAPI server with Bio-Entropy endpoints
- CppBioEntropyGenerator wrapper
- WebSocket handlers for real-time updates
- 4 new REST endpoints

### Web Frontend
- Bio-Entropy dashboard card
- Configuration panel (mode selector)
- Statistics panel with Chart.js
- 29 new UI element IDs

---

## 🧪 Quick Test Commands

### 1. Check C++ Bindings Available
```bash
curl https://YOUR-URL/api/bindings | jq .
```

### 2. Extract Features from Block Header
```bash
curl -X POST https://YOUR-URL/api/bio-entropy/extract-features \
  -H "Content-Type: application/json" \
  -d '{"block_header":"0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000","difficulty":1}' | jq .
```

### 3. Get Bio-Entropy Statistics
```bash
curl https://YOUR-URL/api/bio-entropy/stats | jq .
```

### 4. Switch to RealMEA Mode
```bash
curl -X POST https://YOUR-URL/api/bio-entropy/set-mode \
  -H "Content-Type: application/json" \
  -d '{"mode":"RealMEA"}' | jq .
```

---

## 🔍 Verification Checklist

After deployment, verify these 5 things:

1. ✅ **Service URL**: Deployment script prints service URL
2. ✅ **C++ Available**: `/api/bindings` returns `cpp_available: true`
3. ✅ **Bio-Entropy**: `/api/bio-entropy/stats` returns valid JSON
4. ✅ **Web Interface**: Open service URL in browser
5. ✅ **Mode Switching**: POST to `/api/bio-entropy/set-mode` succeeds

---

## 🐛 Common Issues & Fixes

### Issue 1: Build Timeout
**Symptom**: Cloud Build exceeds 3600s timeout

**Fix**: Script already uses `--timeout=3600s` and `--machine-type=e2-highcpu-8`

**Alternative**: Split build into stages if needed

---

### Issue 2: MOC Files Missing
**Symptom**: Build fails with "undefined reference to vtable"

**Fix**: Verify all 4 Bio-Entropy headers have `Q_OBJECT`:
```bash
grep -l "Q_OBJECT" include/bio/ibio_compute_interface.h \
                   include/bio/real_mea_adapter.h \
                   include/bio/biological_network_adapter.h \
                   include/crypto/bio_entropy_generator.h
```

---

### Issue 3: Python Import Fails
**Symptom**: Server logs show "ModuleNotFoundError: No module named 'biomining_cpp'"

**Fix**: Check `.so` file exists in `/app/`:
```bash
# In Cloud Run logs
ls -la /app/biomining_cpp*.so
```

---

### Issue 4: Endpoint Returns 500
**Symptom**: Bio-Entropy endpoints return internal server error

**Fix**: Check Cloud Run logs:
```bash
gcloud run logs read biomining-cpp-ultra-simple --limit 50
```

Look for:
- C++ module import errors
- Missing Qt libraries
- Segmentation faults

---

## 📊 Expected Performance

| Metric | Value | Notes |
|--------|-------|-------|
| **Build Time** | 10-15 min | Includes Qt5 installation + C++ compilation |
| **Cold Start** | 20-30 sec | First request after deployment |
| **Warm Start** | 2-5 sec | Subsequent requests |
| **Memory Usage** | ~2-3 Gi | Out of 4Gi allocated |
| **CPU Usage** | 2-3 cores | Out of 4 cores allocated |
| **Image Size** | ~2 Gi | Includes Qt5 libraries |

---

## 🎯 Key Differences from v1.0.0

| Feature | v1.0.0 | v2.0.0 (Bio-Entropy) |
|---------|--------|----------------------|
| **Modes** | Simulation only | RealMEA + Simulation |
| **Strategies** | None | Uniform, Fibonacci, BioGuided |
| **Features** | None | 60-dimensional extraction |
| **Starting Points** | Brute force | 1000 smart points |
| **Learning** | Basic | Reinforcement learning |
| **Qt Libraries** | 4 | 5 (added Qt5Concurrent) |
| **C++ Files** | 12 | 16 (+4 Bio-Entropy) |
| **Package Name** | biomining-cpp-simple | biomining-cpp-bioentropy |
| **Version** | 1.0.0 | 2.0.0 |

---

## 🔗 Resources

- **Full Guide**: `DEPLOYMENT_UPDATE_BIO_ENTROPY.md`
- **Integration**: `INTEGRATION_GUIDE_BIO_ENTROPY.md`
- **Web UI**: `WEB_INTERFACE_UPDATE_SUMMARY.md`
- **PR #114**: https://github.com/jadaela-ara/BioMining/pull/114

---

## 💡 Pro Tips

### Tip 1: Monitor Build Progress
```bash
# Watch build in real-time
gcloud builds list --ongoing --limit 1
gcloud builds log $(gcloud builds list --ongoing --format="value(id)" --limit 1)
```

### Tip 2: Quick Redeploy
If only Python code changed (no C++ changes):
```bash
# Use existing image
gcloud run deploy biomining-cpp-ultra-simple \
  --image gcr.io/PROJECT_ID/biomining-cpp-ultra-simple:latest \
  --region us-central1
```

### Tip 3: Check Service Logs Live
```bash
gcloud run logs tail biomining-cpp-ultra-simple --region us-central1
```

### Tip 4: Test Locally First
```bash
# Build Dockerfile locally
docker build -f Dockerfile.cpp-simple -t biomining-test .

# Run locally
docker run -p 8080:8080 biomining-test

# Test endpoints
curl http://localhost:8080/api/bindings
```

---

**Happy Deploying! 🚀**

*Generated: 2025-10-15*
*Version: 2.0.0 (Bio-Entropy Platform)*
