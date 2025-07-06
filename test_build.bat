@echo off
echo 🧪 Testing Connect Messenger build in Docker...

echo 📦 Building Docker image...
docker build -t connect-test .

if %ERRORLEVEL% EQU 0 (
    echo ✅ Build successful!
    
    echo 🚀 Testing container startup...
    docker run --rm -d --name connect-test-container -p 9001:9001 -p 9002:9002 connect-test
    
    if %ERRORLEVEL% EQU 0 (
        echo ✅ Container started successfully!
        
        echo Waiting for server to start...
        timeout /t 3 /nobreak >nul
        
        echo 🏥 Testing health check...
        curl -f http://localhost:9002/health
        
        if %ERRORLEVEL% EQU 0 (
            echo ✅ Health check passed!
        ) else (
            echo ❌ Health check failed!
        )
        
        echo 🛑 Stopping container...
        docker stop connect-test-container
        echo Container stopped
    ) else (
        echo ❌ Container failed to start!
    )
) else (
    echo ❌ Build failed!
    exit /b 1
)

echo 🎉 Test completed!
pause 