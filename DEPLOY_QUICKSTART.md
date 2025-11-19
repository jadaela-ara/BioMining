# ⚡ Quick Deployment - BioMining Platform

## 🚀 One-Line Deployment

```bash
./deploy_python.sh
```

## 📋 Prerequisites (First Time Only)

### 1. Install Google Cloud SDK

**macOS**:
```bash
brew install google-cloud-sdk
```

**Linux**:
```bash
curl https://sdk.cloud.google.com | bash
exec -l $SHELL
```

**Windows**:
Download from https://cloud.google.com/sdk/docs/install

### 2. Authenticate

```bash
gcloud auth login
gcloud config set project YOUR_PROJECT_ID
```

### 3. Enable APIs

```bash
gcloud services enable cloudbuild.googleapis.com run.googleapis.com containerregistry.googleapis.com
```

## ✅ Verification Steps

After deployment completes, you'll see:

```
✅ Deployment completed successfully!

🌐 Service URL: https://biomining-platform-xxxxx-uc.a.run.app

🔗 Access your BioMining Platform:
   🌐 Web Interface: https://biomining-platform-xxxxx-uc.a.run.app
   📊 API Status: https://biomining-platform-xxxxx-uc.a.run.app/api/status
   ⛏️ Real Mining: Navigate to 'Real Bitcoin Mining' section
```

### Test the API

```bash
curl https://YOUR_SERVICE_URL/api/status
```

Expected response:
```json
{
  "status": "healthy",
  "version": "2.0.0",
  "features": {
    "bio_entropy": true,
    "real_mining": true
  }
}
```

## 🔧 Configuration

The deployment uses these defaults:

| Setting | Value | Purpose |
|---------|-------|---------|
| Memory | 4Gi | Required for mining operations |
| CPU | 4 cores | Optimal for parallel mining |
| Region | us-central1 | Low latency |
| Max Instances | 3 | Cost control |
| Port | 8080 | Standard Cloud Run port |

## 💰 Cost Estimate

- **Free tier**: 2M requests/month included
- **Active mining**: ~$0.048/hour
- **Idle**: $0 (auto-scales to zero)

## 🐛 Common Issues

### "Project not set"
```bash
gcloud config set project YOUR_PROJECT_ID
```

### "APIs not enabled"
```bash
gcloud services enable cloudbuild.googleapis.com run.googleapis.com
```

### "Permission denied"
```bash
gcloud auth login
```

## 📚 Full Documentation

For detailed information, see [DEPLOYMENT.md](./DEPLOYMENT.md)

---

**That's it!** Your BioMining Platform is now running on Google Cloud Run 🎉
