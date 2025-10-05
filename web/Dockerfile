# ================================================================
# HYBRID BITCOIN MINING PLATFORM - WEB INTERFACE DOCKERFILE
# Advanced containerization for Triple System Web Interface
# ================================================================

FROM python:3.11-slim

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/*

# Copy requirements and install Python dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Install additional web dependencies
RUN pip install \
    fastapi \
    uvicorn \
    websockets \
    python-multipart \
    psutil \
    aiofiles

# Copy application files
COPY . .

# Create necessary directories
RUN mkdir -p uploads logs static/js static/css

# Set environment variables
ENV PYTHONPATH="/app:$PYTHONPATH"
ENV PYTHONUNBUFFERED=1
ENV PORT=8080

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=30s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/api/status || exit 1

# Run the application
CMD ["python", "api/server.py"]