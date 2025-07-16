# Multi-stage build for Connect Messenger Server
FROM ubuntu:22.04 as builder

# Avoid interactive prompts during package installation
ARG DEBIAN_FRONTEND=noninteractive

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

# Build the server (server-only version)
RUN cp CMakeLists_server.txt CMakeLists.txt && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target ConnectServer -- -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ARG DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies (server-only)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libssl3 \
        libsqlite3-0 \
        libqt6core6 \
        libqt6network6 \
        libqt6websockets6 \
        libsodium23 \
        curl \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -m -u 1000 appuser

# Set working directory
WORKDIR /app

# Copy built executable and web client
COPY --from=builder /app/build/ConnectServer /app/
COPY --from=builder /app/web_client.html /app/

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
ENV PORT=9001

# Health check using the HTTP endpoint on port 9002
HEALTHCHECK --interval=30s --timeout=10s --start-period=30s --retries=3 \
    CMD curl -f http://localhost:9002/health || exit 1

# Run the server
CMD ["./ConnectServer"] 