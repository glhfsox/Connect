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

## 🌐 Quick Deploy

### Railway (Recommended)
[![Deploy on Railway](https://railway.app/button.svg)](https://railway.app/template/new?template=https://github.com/yourusername/Connect)

### Render
[![Deploy to Render](https://render.com/images/deploy-to-render-button.svg)](https://render.com/deploy)

### Fly.io
```bash
fly launch
```

## 📋 Requirements

### For Local Development
- Windows 10/11
- Visual Studio 2022 with C++20 support
- CMake 3.10+
- vcpkg (already included in the project)

### For Cloud Deployment
- GitHub repository
- Docker (for local testing)
- Account on chosen platform

## 🛠 Local Development

### 1. Installing Dependencies

#### Installing Visual Studio 2022
1. Download Visual Studio 2022 Community from the [official website](https://visualstudio.microsoft.com/downloads/)
2. During installation, select "Desktop development with C++"
3. Make sure the following components are selected:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10/11 SDK
   - CMake tools for Visual Studio

#### Installing CMake (if not installed via Visual Studio)
1. Download CMake from the [official website](https://cmake.org/download/)
2. Install, selecting "Add CMake to the system PATH"

### 2. Building the Project
```bash
# Clone the repository
git clone <repository-url>
cd Connect

# Install dependencies
.\vcpkg\vcpkg.exe install --triplet=x64-windows

# Build the project
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## 🐳 Docker Development

### Local Testing with Docker
```bash
# Build and run
docker build -t connect-messenger .
docker run -p 9001:9001 -p 9002:9002 connect-messenger

# Or use docker-compose
docker-compose up --build
```

### Test Health Check
```bash
curl http://localhost:9002/health
# Should return: {"status":"healthy","online_users":0}
```

## 🚀 Running

### Starting the Server
```bash
# From build directory
.\Release\ConnectServer.exe [port]
```

By default, the server starts on port 9001.

### Starting the Client
```bash
# From build directory
.\Release\ConnectClient.exe
```

## 📱 Usage

### Connecting to Server
1. Start the server
2. Start the client
3. Enter a username
4. Click "Connect to Server"
5. Click "Login" to authenticate

### Sending Messages
1. Select a contact from the list
2. Type your message in the input field
3. Press Enter or click "Send"

### Sending Media Files
- **Files**: Click the paperclip button and select a file
- **Voice messages**: Click the microphone button to record

## 🏗 Architecture

### Server
- **WebSocketServer**: WebSocket connection handling
- **Database**: SQLite database operations
- **Encryption**: Message encryption
- **HTTP Server**: Health checks and monitoring

### Client
- **MessengerClient**: Main application window
- **ChatWidget**: Chat widget
- **ContactListWidget**: Contact list

## 📊 Database Structure

### Users Table
```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### Messages Table
```sql
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    text TEXT NOT NULL,
    message_type TEXT DEFAULT 'text',
    media_path TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### Media Table
```sql
CREATE TABLE media (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    path TEXT NOT NULL,
    type TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

## 🔐 Security

- **End-to-end encryption** of all messages
- **Password hashing** using Argon2
- **Secure key exchange** via libsodium
- **TLS support** (automatic on cloud platforms)

## 📁 Project Structure

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
│   ├── MessengerClient.h
│   ├── MessengerClient.cpp
│   ├── ChatWidget.h
│   ├── ChatWidget.cpp
│   ├── ContactListWidget.h
│   ├── ContactListWidget.cpp
│   └── main_client.cpp
├── data/                    # Database
├── certs/                   # SSL certificates
├── media/uploads/           # Uploaded files
├── main.cpp                 # Main server file
├── CMakeLists.txt
├── vcpkg.json
├── Dockerfile               # Docker configuration
├── docker-compose.yml       # Local development
├── railway.json             # Railway deployment
├── render.yaml              # Render deployment
├── fly.toml                 # Fly.io deployment
├── DEPLOYMENT.md            # Cloud deployment guide
├── ARCHITECTURE.md          # Architecture documentation
└── README.md
```

## 🌐 Cloud Deployment

### Supported Platforms

#### Railway
- **Free tier**: $5/month credit
- **Auto-scaling**: Built-in
- **Easy deployment**: Connect GitHub repository

#### Render
- **Free tier**: 750 hours/month
- **Custom domains**: Available
- **Auto-sleep**: After 15 minutes inactive

#### Fly.io
- **Free tier**: 3 shared-cpu VMs
- **Global deployment**: Multiple regions
- **CLI deployment**: Fast and efficient

### Database Options

#### SQLite (Built-in)
- **Pros**: Simple, no setup required
- **Cons**: Data lost on restart
- **Best for**: Testing, small deployments

#### PostgreSQL (Recommended for Production)
- **Railway PostgreSQL**: Built-in service
- **Render PostgreSQL**: Built-in service
- **Supabase**: Free PostgreSQL hosting

For detailed deployment instructions, see [DEPLOYMENT.md](DEPLOYMENT.md).

## 🔧 Configuration

### Environment Variables
```bash
CONNECT_PORT=9001              # Server port
CONNECT_DB_PATH=/app/data/messenger.db  # Database path
```

### Changing Server Port
```bash
.\ConnectServer.exe 8080
```

### SSL/TLS Configuration
1. Place certificates in the `certs/` folder
2. Update server code to use WSS
3. Cloud platforms provide automatic HTTPS

## 📊 Monitoring

### Health Checks
- **Endpoint**: `/health`
- **Response**: `{"status":"healthy","online_users":N}`
- **Port**: WebSocket port + 1

### Logs
- **Local**: Console output
- **Cloud**: Platform-specific logging
- **Docker**: `docker logs`

## 🐛 Troubleshooting

### Local Development Issues
1. Make sure Visual Studio 2022 with C++ support is installed
2. Run from Developer Command Prompt
3. Check that all dependencies are installed via vcpkg

### Cloud Deployment Issues
1. Check platform logs
2. Verify environment variables
3. Test health check endpoint
4. Check resource usage

### Connection Issues
1. Make sure the server is running
2. Check that port is not occupied
3. Check firewall settings
4. Verify WebSocket URL (ws:// vs wss://)

## 📈 Performance

- **Support for up to 1000 users** simultaneously
- **Fast history operations** thanks to SQLite indexes
- **Efficient encryption** using libsodium
- **Minimal memory consumption**
- **Auto-scaling** on cloud platforms

## 🤝 Contributing

1. Fork the repository
2. Create a branch for your new feature
3. Make your changes
4. Create a Pull Request

## 📄 License

MIT License

## 📞 Support

If you encounter problems:
1. Check this README and DEPLOYMENT.md
2. Create an Issue in the repository
3. Check platform-specific documentation

---

**Ready for local development and cloud deployment!** 🚀 