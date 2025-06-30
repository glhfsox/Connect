# 🚀 Quick Start - Connect Messenger

## 📋 What to Install

1. **Visual Studio 2022 Community** (free)
   - Download: https://visualstudio.microsoft.com/downloads/
   - Select: "Desktop development with C++"
   - Include: CMake tools for Visual Studio

2. **CMake** (if not installed via Visual Studio)
   - Download: https://cmake.org/download/
   - Add to PATH during installation

## ⚡ Quick Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd Connect
   ```

2. **Run automatic installation**
   ```bash
   .\setup.bat
   ```

3. **Build the project**
   ```bash
   .\build.bat
   ```

## 🎯 Testing

### Option 1: Qt Client
1. Start server: `.\build\Release\ConnectServer.exe`
2. Start client: `.\build\Release\ConnectClient.exe`
3. Enter username and connect

### Option 2: Web Test
1. Start server: `.\build\Release\ConnectServer.exe`
2. Open `test_websocket.html` in browser
3. Click "Connect" and "Login"

## 🔧 If Something Doesn't Work

### Problem: CMake not found
```bash
# Install CMake or run from Developer Command Prompt
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
```

### Problem: Dependencies not installed
```bash
# Reinstall dependencies
.\vcpkg\vcpkg.exe install --triplet=x64-windows
```

### Problem: Port occupied
```bash
# Start server on different port
.\build\Release\ConnectServer.exe 8080
```

## 📱 Main Features

- ✅ **Real-time messages** via WebSocket
- ✅ **Message history** in SQLite
- ✅ **Encryption** with libsodium
- ✅ **Modern UI** on Qt
- ✅ **System notifications**
- ✅ **Multimedia support**

## 🎨 Interface

- **Left panel**: Connection and contact list
- **Right panel**: Chat with selected contact
- **Buttons**: Send files, voice messages
- **Tray**: System notifications

## 🔐 Security

- End-to-end encryption of all messages
- Secure key exchange
- Password hashing

## 📞 Support

If you have problems:
1. Check README.md for detailed instructions
2. Create Issue in repository
3. Check server logs in console

---

**Ready!** 🎉 Your messenger is ready to use. 