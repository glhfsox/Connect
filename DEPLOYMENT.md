# Connect Messenger - Cloud Deployment Guide

This guide explains how to deploy Connect Messenger on free cloud platforms.

## 🚀 Quick Deploy Options

### Option 1: Railway (Recommended)
- **Free tier**: $5/month credit
- **Easy deployment**: Connect GitHub repository
- **Auto-scaling**: Built-in

### Option 2: Render
- **Free tier**: 750 hours/month
- **Easy setup**: Connect GitHub repository
- **Custom domains**: Available

### Option 3: Fly.io
- **Free tier**: 3 shared-cpu VMs
- **Global deployment**: Multiple regions
- **CLI deployment**: Fast and efficient

## 📋 Prerequisites

1. **GitHub repository** with your code
2. **Docker** (optional, for local testing)
3. **Account** on chosen platform

## 🚀 Railway Deployment

### 1. Create Railway Account
1. Go to [railway.app](https://railway.app)
2. Sign up with GitHub
3. Get $5/month free credit

### 2. Deploy from GitHub
1. Click "New Project"
2. Select "Deploy from GitHub repo"
3. Choose your repository
4. Railway will automatically detect Dockerfile

### 3. Configure Environment Variables
```bash
CONNECT_PORT=9001
CONNECT_DB_PATH=/app/data/messenger.db
```

### 4. Deploy
- Railway will build and deploy automatically
- Get your public URL (e.g., `https://connect-messenger-production.up.railway.app`)

## 🌐 Render Deployment

### 1. Create Render Account
1. Go to [render.com](https://render.com)
2. Sign up with GitHub
3. Get 750 free hours/month

### 2. Create Web Service
1. Click "New +" → "Web Service"
2. Connect your GitHub repository
3. Configure:
   - **Name**: `connect-messenger`
   - **Environment**: `Docker`
   - **Region**: Choose closest to you
   - **Branch**: `main`

### 3. Environment Variables
```bash
CONNECT_PORT=10000
CONNECT_DB_PATH=/app/data/messenger.db
```

### 4. Deploy
- Render will build and deploy
- Get your URL (e.g., `https://connect-messenger.onrender.com`)

## 🦅 Fly.io Deployment

### 1. Install Fly CLI
```bash
# Windows
winget install Fly.Flyctl

# Or download from: https://fly.io/docs/hands-on/install-flyctl/
```

### 2. Sign Up
```bash
fly auth signup
```

### 3. Deploy
```bash
# Login
fly auth login

# Deploy (first time)
fly launch

# Follow prompts:
# - App name: connect-messenger
# - Region: Choose closest
# - Deploy now: Yes
```

### 4. Configure
```bash
# Set environment variables
fly secrets set CONNECT_PORT=8080
fly secrets set CONNECT_DB_PATH=/app/data/messenger.db

# Deploy updates
fly deploy
```

## 🗄 Database Options

### Option 1: SQLite (Built-in)
- **Pros**: Simple, no setup required
- **Cons**: Data lost on restart, no scaling
- **Best for**: Testing, small deployments

### Option 2: Railway PostgreSQL
1. Add PostgreSQL service in Railway
2. Get connection string
3. Update code to use PostgreSQL

### Option 3: Render PostgreSQL
1. Add PostgreSQL service in Render
2. Get connection string
3. Update code to use PostgreSQL

### Option 4: Supabase (Free PostgreSQL)
1. Go to [supabase.com](https://supabase.com)
2. Create free project
3. Get connection string
4. Update code to use PostgreSQL

## 🔧 Local Testing

### Using Docker
```bash
# Build and run locally
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

## 📱 Client Configuration

### Update Client Connection
```cpp
// In your client code, update server URL
QString serverUrl = "wss://your-app-name.railway.app";
// or
QString serverUrl = "wss://your-app-name.onrender.com";
// or
QString serverUrl = "wss://your-app-name.fly.dev";
```

### WebSocket Test
```javascript
// Test in browser console
const ws = new WebSocket('wss://your-app-name.railway.app');
ws.onopen = () => console.log('Connected!');
ws.onmessage = (event) => console.log('Message:', event.data);
```

## 🔐 Security Considerations

### SSL/TLS
- **Railway**: Automatic HTTPS
- **Render**: Automatic HTTPS
- **Fly.io**: Automatic HTTPS

### Environment Variables
- Never commit secrets to Git
- Use platform's secret management
- Rotate keys regularly

### Database Security
- Use connection pooling
- Implement proper authentication
- Regular backups

## 📊 Monitoring

### Health Checks
All platforms support health checks:
- **Endpoint**: `/health`
- **Expected response**: `{"status":"healthy","online_users":N}`

### Logs
- **Railway**: Built-in logging
- **Render**: Built-in logging
- **Fly.io**: `fly logs`

### Metrics
- Monitor CPU/Memory usage
- Track WebSocket connections
- Monitor database performance

## 🚨 Troubleshooting

### Common Issues

#### Build Failures
```bash
# Check Docker build locally
docker build -t test .

# Check platform logs
# Railway/Render: View in dashboard
# Fly.io: fly logs
```

#### Connection Issues
```bash
# Test WebSocket connection
wscat -c wss://your-app-name.railway.app

# Test HTTP endpoint
curl https://your-app-name.railway.app/health
```

#### Database Issues
```bash
# Check database connection
# For SQLite: Check file permissions
# For PostgreSQL: Check connection string
```

### Platform-Specific

#### Railway
- Check resource usage (CPU/Memory)
- Verify environment variables
- Check build logs

#### Render
- Check build logs
- Verify service configuration
- Monitor resource usage

#### Fly.io
```bash
# Check app status
fly status

# View logs
fly logs

# SSH into app
fly ssh console
```

## 💰 Cost Optimization

### Railway
- Use $5/month credit efficiently
- Monitor usage in dashboard
- Scale down when not needed

### Render
- Free tier: 750 hours/month
- Auto-sleep after 15 minutes inactive
- Wake up on first request

### Fly.io
- Free tier: 3 shared-cpu VMs
- Auto-stop when not in use
- Scale to 0 when possible

## 🔄 Continuous Deployment

### GitHub Actions (Optional)
```yaml
name: Deploy to Railway
on:
  push:
    branches: [main]
jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - uses: railway/deploy@v1
        with:
          railway_token: ${{ secrets.RAILWAY_TOKEN }}
```

## 📈 Scaling

### Horizontal Scaling
- **Railway**: Auto-scaling based on load
- **Render**: Manual scaling
- **Fly.io**: Auto-scaling with rules

### Vertical Scaling
- Upgrade to paid plans for more resources
- Monitor performance metrics
- Optimize code and database queries

---

**Your Connect Messenger is now ready for the cloud!** 🚀 