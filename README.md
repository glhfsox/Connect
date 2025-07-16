# Connect Messenger

A modern C++ messenger with WebSocket support, encryption, and multimedia capabilities. Ready for cloud deployment!

## 🚀 Features

- **Real-time communication** via WebSocket
- **End-to-end encryption** using libsodium
- **Message history** in SQLite database
- **Multimedia support** (photos, videos, voice messages)
- **Modern Qt interface** with dark theme
- **System notifications** in tray
- **User authentication**
- **Support for up to 1000 concurrent users**
- **Cloud-ready** with Docker support
- **Health checks** for monitoring

## 🌐 Quick Deploy on Railway

### One-Click Deploy
1. Fork this repository on GitHub
2. Go to [Railway](https://railway.app)
3. Click "New Project" → "Deploy from GitHub repo"
4. Select your forked repository
5. Railway will automatically detect and deploy using the Dockerfile
6. Your app will be available at `https://your-app-name.up.railway.app`

### Manual Railway Setup
```bash
# Install Railway CLI
npm install -g @railway/cli

# Login to Railway
railway login

# Deploy the project
railway link
railway up
```

The server will automatically:
- Use port 9001 for WebSocket connections
- Use port 9002 for health checks
- Create SQLite database in `/app/data/`
- Support up to 1000 concurrent users

## 📋 Requirements

### For Local Development
- **Windows**: Windows 10/11, Visual Studio 2022, CMake 3.10+
- **Linux**: Ubuntu 20.04+, GCC 9+, CMake 3.10+
- **Dependencies**: Qt6/Qt5, SQLite3, libsodium, OpenSSL

### For Cloud Deployment
- GitHub repository
- Railway/Render/Fly.io account

## 🛠 Local Development

### Quick Start (Recommended)

#### Windows
1. Double-click `build_project.bat` to build the project
2. Double-click `start_server.bat` to start the server
3. Double-click `start_client.bat` to start the client

#### Linux
```bash
# Make scripts executable
chmod +x build_project.sh start_server.sh start_client.sh

# Build the project
./build_project.sh

# Start the server
./start_server.sh

# Start the client (in another terminal)
./start_client.sh
```

### Manual Build

#### Windows with Visual Studio
```bash
# Install dependencies via vcpkg
.\vcpkg\vcpkg.exe install --triplet=x64-windows

# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### Linux
```bash
# Install system dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libssl-dev libsqlite3-dev libsodium-dev

# Install Qt6 or Qt5
sudo apt-get install -y qt6-base-dev qt6-websockets-dev qt6-multimedia-dev
# OR for Qt5: sudo apt-get install -y qtbase5-dev qtwebsockets5-dev qtmultimedia5-dev

# Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 🐳 Docker Development

### Quick Start with Docker
```bash
# Build and run with docker-compose
docker-compose up --build

# Or manually
docker build -t connect-messenger .
docker run -p 9001:9001 -p 9002:9002 connect-messenger
```

### Test Health Check
```bash
curl http://localhost:9002/health
# Expected response: {"status":"healthy","online_users":0}
```

## 🚀 Running the Application

### Server
The server can be started in several ways:

1. **Using shortcuts**:
   - Windows: Double-click `start_server.bat`
   - Linux: `./start_server.sh`

2. **Direct execution**:
   ```bash
   # Windows
   build\Release\ConnectServer.exe [port]
   
   # Linux
   ./build/ConnectServer [port]
   ```

3. **With environment variables**:
   ```bash
   export PORT=9001
   export CONNECT_DB_PATH=data/messenger.db
   ./build/ConnectServer
   ```

### Client (if built)
- Windows: Double-click `start_client.bat`
- Linux: `./build/ConnectClient`

## 📱 Using the Messenger

### Server Connection
The server automatically starts on:
- **WebSocket**: Port 9001 (or specified port)
- **Health Check**: Port 9002 (or port + 1)

### WebSocket API
```javascript
// Connect to server
const ws = new WebSocket('ws://localhost:9001');

// Authenticate
ws.send(JSON.stringify({
    type: "auth",
    username: "your-username"
}));

// Send message
ws.send(JSON.stringify({
    type: "message",
    to: "recipient-username",
    text: "Hello world!"
}));

// Request message history
ws.send(JSON.stringify({
    type: "history",
    with: "other-username"
}));
```

## 🏗 Architecture

### Server Components
- **WebSocketServer**: Real-time communication (Port 9001)
- **HTTPServer**: Health checks and monitoring (Port 9002)
- **Database**: SQLite message storage
- **Encryption**: libsodium for message security

### Client Components
- **MessengerClient**: Main Qt application window
- **ChatWidget**: Message display and input
- **ContactListWidget**: User management

## 📊 Database Structure

### Tables
```sql
-- Users
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Messages
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    text TEXT NOT NULL,
    message_type TEXT DEFAULT 'text',
    media_path TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Media Files
CREATE TABLE media (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    path TEXT NOT NULL,
    type TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

## 🔐 Security Features

- **Message Encryption**: All messages encrypted with libsodium
- **Secure WebSocket**: Automatic TLS on cloud platforms
- **Input Validation**: SQL injection prevention
- **Memory Safety**: Modern C++ practices

## 🌐 Cloud Deployment

### Railway (Recommended)
✅ **Free tier**: $5/month credit  
✅ **Auto-scaling**: Built-in  
✅ **Zero config**: Works out of the box  

### Render
✅ **Free tier**: 750 hours/month  
✅ **Custom domains**: Available  
✅ **Auto-sleep**: After 15 minutes inactive  

### Fly.io
✅ **Free tier**: 3 shared-cpu VMs  
✅ **Global deployment**: Multiple regions  
✅ **CLI deployment**: Fast and efficient  

### Environment Variables
For production deployment, set:
```bash
CONNECT_PORT=9001
CONNECT_DB_PATH=/app/data/messenger.db
QT_QPA_PLATFORM=offscreen
```

## 🔧 Development

### Building Debug Version
```bash
# Windows
build_project.bat debug

# Linux
./build_project.sh debug
```

### Project Structure
```
Connect/
├── include/                 # Header files
│   ├── WebSocketServer.h
│   ├── Database.h
│   └── Encryption.h
├── server/                  # Server implementation
│   ├── WebSocketServer.cpp
│   ├── Database.cpp
│   └── Encryption.cpp
├── ui/qt-frontend/          # Qt client
├── data/                    # Database files
├── media/uploads/           # Media storage
├── start_server.bat         # Windows server shortcut
├── start_client.bat         # Windows client shortcut
├── start_server.sh          # Linux server shortcut
├── build_project.bat        # Windows build script
├── build_project.sh         # Linux build script
├── ConnectMessenger.desktop # Linux desktop entry
├── main.cpp                 # Server entry point
├── CMakeLists.txt          # Build configuration
├── Dockerfile              # Container configuration
├── docker-compose.yml      # Local development
└── railway.json            # Railway deployment
```

## 🚨 Troubleshooting

### Build Issues
1. **Qt not found**: Install Qt6 or Qt5 development packages
2. **libsodium missing**: Install libsodium-dev
3. **SQLite errors**: Install libsqlite3-dev
4. **CMake errors**: Update CMake to 3.10+

### Runtime Issues
1. **Port in use**: Change port in start scripts or environment
2. **Database errors**: Check write permissions to `data/` directory
3. **Connection refused**: Verify firewall settings
4. **Qt platform errors**: Set `QT_QPA_PLATFORM=offscreen` for headless

### Railway Deployment Issues
1. **Build timeout**: Railway builds within 10 minutes
2. **Memory limits**: Free tier has 512MB RAM limit
3. **Port binding**: Railway auto-assigns PORT variable
4. **Health check fails**: Verify `/health` endpoint works

## 📞 Support

- **Documentation**: See `ARCHITECTURE.md` and `DEPLOYMENT.md`
- **Issues**: Open GitHub issues for bug reports
- **Health Check**: `GET /health` for server status

## 📄 License

This project is open source. See LICENSE file for details.

---

### Quick Commands Reference

```bash
# Build and start (Windows)
build_project.bat && start_server.bat

# Build and start (Linux)
./build_project.sh && ./start_server.sh

# Docker development
docker-compose up --build

# Test health check
curl http://localhost:9002/health

# Deploy to Railway
railway login && railway up
``` 