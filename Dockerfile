# ================================================================
# BIOMINING PLATFORM - PURE PYTHON WEB INTERFACE DOCKERFILE
# Production-ready containerization for Pure Python Platform
# With Real Bitcoin Mining Support (Testnet/Mainnet)
# ================================================================

FROM python:3.11-slim

# Metadata
LABEL org.opencontainers.image.title="BioMining Platform - Pure Python"
LABEL org.opencontainers.image.description="Pure Python platform with Real Bitcoin Mining, Bio-Entropy, and Historical Training"
LABEL org.opencontainers.image.version="2.0.0"
LABEL org.opencontainers.image.vendor="BioMining Platform"

# Environment variables
ENV PYTHONUNBUFFERED=1
ENV PYTHONDONTWRITEBYTECODE=1
ENV PORT=8080
ENV HOST=0.0.0.0
ENV NODE_ENV=production
ENV PYTHONPATH="/app:$PYTHONPATH"

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    git \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

# Create non-root user for security
RUN groupadd -r appuser && useradd -r -g appuser appuser

# Copy requirements first for better Docker layer caching
COPY requirements.txt ./requirements.txt

# Install Python dependencies
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Copy application code
COPY . .

# Create necessary directories and set permissions
RUN mkdir -p uploads logs web/static && \
    chown -R appuser:appuser /app

# Switch to non-root user
USER appuser

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:${PORT}/api/status || exit 1

# Expose port
EXPOSE ${PORT}

# Start the web interface
CMD ["python", "web/api/server.py"]