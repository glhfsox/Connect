#include "include/WebSocketServer.h"
#include "include/Database.h"
#include "include/Encryption.h"
#include <QCoreApplication>
#include <QTimer>
#include <iostream>
#include <csignal>
#include <memory>

std::unique_ptr<WebSocketServer> g_server;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down server..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    QCoreApplication::quit();
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== Connect Messenger Server ===" << std::endl;
    std::cout << "Starting server..." << std::endl;
    
    // Initialize database
    Database db;
    if (!db.initialize()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    std::cout << "Database initialized successfully" << std::endl;
    
    // Create and start WebSocket server
    g_server = std::make_unique<WebSocketServer>();
    
    int port = 9001;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    if (!g_server->start(port)) {
        std::cerr << "Failed to start WebSocket server" << std::endl;
        return 1;
    }
    
    std::cout << "Server is running on port " << port << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    return app.exec();
} 