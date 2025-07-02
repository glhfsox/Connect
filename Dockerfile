# Multi-stage build for Connect Messenger Server
FROM ubuntu:22.04 as builder

# Install dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        pkg-config \
        libssl-dev \
        libsqlite3-dev \
        qt6-base-dev \
        qt6-multimedia-dev \
        libsodium-dev \
        curl zip tar wget && \
    # Qt6 WebSockets dev package can be named differently, try common name and fallback alternative
    (apt-get install -y --no-install-recommends qt6-websockets-dev || \
     apt-get install -y --no-install-recommends libqt6websockets6-dev) && \
    rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Install vcpkg dependencies manually for Qt6
RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc) ConnectServer

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libssl3 \
        libsqlite3-0 \
        libqt6core6 \
        libqt6network6 \
        libqt6websockets6 \
        libsodium23 \
        dbus \
        curl \
        ca-certificates \
    || (sleep 10 && apt-get update && apt-get install -y --no-install-recommends \
        libssl3 \
        libsqlite3-0 \
        libqt6core6 \
        libqt6network6 \
        libqt6websockets6 \
        libsodium23 \
        dbus \
        curl \
        ca-certificates) \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -m -u 1000 appuser

# Set working directory
WORKDIR /app

# Copy built executable
COPY --from=builder /app/build/ConnectServer /app/

# Create data and media directories
RUN mkdir -p /app/data /app/media/uploads && chown -R appuser:appuser /app

# Switch to app user
USER appuser

# Expose ports (WebSocket and HTTP health check)
EXPOSE 9001 9002

# Set environment variables
ENV CONNECT_PORT=9001
ENV CONNECT_DB_PATH=/app/data/messenger.db
ENV QT_QPA_PLATFORM=offscreen

# Health check using the HTTP endpoint on port 9002
HEALTHCHECK --interval=30s --timeout=10s --start-period=30s --retries=3 \
    CMD curl -f http://localhost:9002/health || exit 1

# Run the server
CMD ["./ConnectServer"] 