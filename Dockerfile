# Multi-stage build for Connect Messenger Server
FROM ubuntu:22.04 as builder

# Install dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        pkg-config \
        libssl-dev \
        libsqlite3-dev \
        qtbase5-dev \
        qtwebsockets5-dev \
        libsodium-dev \
    || (sleep 10 && apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        pkg-config \
        libssl-dev \
        libsqlite3-dev \
        qtbase5-dev \
        qtwebsockets5-dev \
        libsodium-dev) \
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
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libssl3 \
        libsqlite3-0 \
        libqt5core5a \
        libqt5network5 \
        libqt5websockets5 \
        libsodium23 \
        dbus \
        libapparmor1 \
        libb2-1 \
        libbrotli1 \
        libdbus-1-3 \
        libdouble-conversion3 \
        libexpat1 \
        libglib2.0-0 \
        libglib2.0-data \
        libicu70 \
        libpcre2-16-0 \
        libproxy1v5 \
        libqt5dbus5 \
        libqt5gui5 \
        libqt5widgets5 \
        libxml2 \
        shared-mime-info \
        xdg-user-dirs \
    || (sleep 10 && apt-get update && apt-get install -y --no-install-recommends \
        libssl3 \
        libsqlite3-0 \
        libqt5core5a \
        libqt5network5 \
        libqt5websockets5 \
        libsodium23 \
        dbus \
        libapparmor1 \
        libb2-1 \
        libbrotli1 \
        libdbus-1-3 \
        libdouble-conversion3 \
        libexpat1 \
        libglib2.0-0 \
        libglib2.0-data \
        libicu70 \
        libpcre2-16-0 \
        libproxy1v5 \
        libqt5dbus5 \
        libqt5gui5 \
        libqt5widgets5 \
        libxml2 \
        shared-mime-info \
        xdg-user-dirs) \
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