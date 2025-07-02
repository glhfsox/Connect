# 🚀 Connect Messenger - Quick Start Guide

## Railway Deployment (1-2 minutes)

### Option 1: One-Click Deploy
1. **Fork** this repository on GitHub
2. Go to [railway.app](https://railway.app) and sign up/login
3. Click **"New Project"** → **"Deploy from GitHub repo"**
4. Select your forked **Connect** repository
5. Wait 3-5 minutes for automatic build and deployment
6. Your messenger will be live at `https://your-app-name.up.railway.app`

### Option 2: Railway CLI
```bash
npm install -g @railway/cli
railway login
railway link
railway up
```

**That's it!** Your messenger supports:
- ✅ Real-time WebSocket messaging
- ✅ Up to 1000 concurrent users
- ✅ Message history with SQLite
- ✅ Health checks at `/health`
- ✅ Automatic HTTPS

## Local Development (5 minutes)

### Windows (Visual Studio)
1. **Double-click** `build_project.bat` → Wait for build completion
2. **Double-click** `start_server.bat` → Server starts on port 9001
3. Test: Open `http://localhost:9002/health` in browser

### Linux (Ubuntu/Debian)
```bash
chmod +x build_project.sh start_server.sh
./build_project.sh
./start_server.sh
```

### Docker (Any OS)
```bash
docker-compose up --build
```

## Testing Your Deployment

### Health Check
```bash
curl https://your-app-name.up.railway.app/health
# Expected: {"status":"healthy","online_users":0}
```

### WebSocket Connection Test
```javascript
const ws = new WebSocket('wss://your-app-name.up.railway.app');
ws.onopen = () => console.log('Connected to Connect Messenger!');
ws.send(JSON.stringify({
    type: "auth",
    username: "testuser"
}));
```

## Client Connection Example

```javascript
// Connect to your deployed messenger
const serverUrl = 'wss://your-app-name.up.railway.app';
const ws = new WebSocket(serverUrl);

ws.onopen = () => {
    // Authenticate
    ws.send(JSON.stringify({
        type: "auth", 
        username: "your-username"
    }));
};

ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    console.log('Received:', data);
};

// Send a message
function sendMessage(to, text) {
    ws.send(JSON.stringify({
        type: "message",
        to: to,
        text: text
    }));
}
```

## Troubleshooting

### Railway Issues
- **Build fails**: Check build logs in Railway dashboard
- **App crashes**: Check runtime logs for error messages
- **Memory limit**: Free tier has 512MB RAM (upgrade if needed)

### Local Issues
- **Windows**: Install Visual Studio 2022 with C++ tools
- **Linux**: Run `sudo apt install build-essential cmake qtbase6-dev`
- **Port busy**: Change port in start scripts

### Quick Fixes
```bash
# Check if server is running
curl http://localhost:9002/health

# Kill process on port 9001 (if stuck)
# Windows: netstat -ano | findstr :9001
# Linux: sudo lsof -t -i:9001 | xargs kill -9

# Check Railway logs
railway logs

# Test WebSocket locally
wscat -c ws://localhost:9001
```

## Next Steps

1. **Customize**: Edit server code in `server/` directory
2. **Client**: Build Qt client with `build_project.bat`/`.sh`
3. **Database**: SQLite files stored in `data/` directory
4. **Monitoring**: Use Railway metrics dashboard
5. **Scale**: Upgrade Railway plan for more resources

---

**🎉 Your Connect Messenger is now running!**

- **Railway URL**: `https://your-app-name.up.railway.app`
- **WebSocket**: `wss://your-app-name.up.railway.app`
- **Health**: `https://your-app-name.up.railway.app/health`
- **Local Server**: `http://localhost:9001` (WebSocket) + `http://localhost:9002/health` 