# Multi-stage build for Connect Messenger Server
FROM ubuntu:22.04 as builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    libsqlite3-dev \
    qt6-base-dev \
    qt6-websockets-dev \
    libsodium-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the project
RUN mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libsqlite3-0 \
    libqt6core6 \
    libqt6network6 \
    libqt6websockets6 \
    libsodium23 \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -m -u 1000 appuser

# Set working directory
WORKDIR /app

# Copy built executable
COPY --from=builder /app/build/ConnectServer /app/

# Create data directory
RUN mkdir -p /app/data && chown -R appuser:appuser /app

# Switch to app user
USER appuser

# Expose port
EXPOSE 9001

# Set environment variables
ENV CONNECT_PORT=9001
ENV CONNECT_DB_PATH=/app/data/messenger.db

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:9001/health || exit 1

# Run the server
CMD ["./ConnectServer"] 