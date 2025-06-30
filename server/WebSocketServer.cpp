#include "../include/WebSocketServer.h"
#include "../include/Database.h"
#include "../include/Encryption.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QTcpSocket>
#include <QHostAddress>
#include <iostream>

WebSocketServer::WebSocketServer(QObject* parent)
    : QObject(parent)
    , m_server(new QWebSocketServer("Connect Messenger", QWebSocketServer::NonSecureMode, this))
    , m_httpServer(new QTcpServer(this))
{
    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
    connect(m_httpServer, &QTcpServer::newConnection, this, &WebSocketServer::onHttpRequest);
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start(int port) {
    // Start WebSocket server
    if (!m_server->listen(QHostAddress::Any, port)) {
        std::cerr << "Failed to start WebSocket server: " << m_server->errorString().toStdString() << std::endl;
        return false;
    }
    
    // Start HTTP server for health checks on port + 1
    if (!m_httpServer->listen(QHostAddress::Any, port + 1)) {
        std::cerr << "Failed to start HTTP server: " << m_httpServer->errorString().toStdString() << std::endl;
        return false;
    }
    
    m_running = true;
    std::cout << "WebSocket server started on port " << port << std::endl;
    std::cout << "HTTP health check server started on port " << (port + 1) << std::endl;
    return true;
}

void WebSocketServer::stop() {
    if (m_running) {
        m_server->close();
        m_httpServer->close();
        m_running = false;
        std::cout << "Servers stopped" << std::endl;
    }
}

bool WebSocketServer::isRunning() const {
    return m_running;
}

void WebSocketServer::onNewConnection() {
    QWebSocket* client = m_server->nextPendingConnection();
    
    connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &WebSocketServer::onDisconnected);
    
    std::cout << "New WebSocket connection established" << std::endl;
}

void WebSocketServer::onHttpRequest() {
    QTcpSocket* socket = m_httpServer->nextPendingConnection();
    
    connect(socket, &QTcpSocket::readyRead, [this, socket]() {
        QByteArray request = socket->readAll();
        handleHttpRequest(socket, request);
    });
    
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void WebSocketServer::handleHttpRequest(QTcpSocket* socket, const QByteArray& request) {
    QString requestStr = QString::fromUtf8(request);
    
    if (requestStr.startsWith("GET /health")) {
        // Health check endpoint
        QString response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: 45\r\n"
                          "\r\n"
                          "{\"status\":\"healthy\",\"online_users\":" + 
                          QString::number(m_onlineUsers.size()) + "}";
        
        socket->write(response.toUtf8());
    } else if (requestStr.startsWith("GET /")) {
        // Root endpoint
        QString response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Content-Length: 89\r\n"
                          "\r\n"
                          "<html><body><h1>Connect Messenger Server</h1><p>Server is running</p></body></html>";
        
        socket->write(response.toUtf8());
    } else {
        // 404 Not Found
        QString response = "HTTP/1.1 404 Not Found\r\n"
                          "Content-Type: text/plain\r\n"
                          "Content-Length: 13\r\n"
                          "\r\n"
                          "Not Found";
        
        socket->write(response.toUtf8());
    }
    
    socket->disconnectFromHost();
}

void WebSocketServer::onTextMessageReceived(const QString& message) {
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        handleMessage(client, message);
    }
}

void WebSocketServer::onDisconnected() {
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        // Remove user from online list
        QString username = m_onlineUsers.key(client);
        if (!username.isEmpty()) {
            m_onlineUsers.remove(username);
            std::cout << "User " << username.toStdString() << " disconnected" << std::endl;
        }
        client->deleteLater();
    }
}

void WebSocketServer::handleMessage(QWebSocket* client, const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        QJsonObject error = {
            {"type", "error"},
            {"message", "Invalid JSON format"}
        };
        sendJsonMessage(client, error);
        return;
    }
    
    QJsonObject j = doc.object();
    QString type = j["type"].toString();
    
    if (type == "auth") {
        // Authentication
        QString username = j["username"].toString();
        
        // Save user to database
        Database db;
        if (db.initialize()) {
            if (!db.userExists(username.toStdString())) {
                db.createUser(username.toStdString());
            }
        }
        
        m_onlineUsers[username] = client;
        
        QJsonObject response = {
            {"type", "auth_response"},
            {"status", "success"},
            {"message", "Authenticated successfully"}
        };
        sendJsonMessage(client, response);
        
        std::cout << "User " << username.toStdString() << " authenticated" << std::endl;
    }
    else if (type == "message") {
        // Send message
        QString to = j["to"].toString();
        QString text = j["text"].toString();
        
        // Find sender
        QString sender = m_onlineUsers.key(client);
        if (sender.isEmpty()) {
            QJsonObject error = {
                {"type", "error"},
                {"message", "Not authenticated"}
            };
            sendJsonMessage(client, error);
            return;
        }
        
        // Save to database
        Database db;
        if (db.initialize()) {
            db.saveMessage(sender.toStdString(), to.toStdString(), text.toStdString());
        }
        
        // Send to recipient if online
        if (m_onlineUsers.contains(to)) {
            QJsonObject messageJson = {
                {"type", "message"},
                {"from", sender},
                {"text", text},
                {"timestamp", QDateTime::currentSecsSinceEpoch()}
            };
            sendJsonMessage(m_onlineUsers[to], messageJson);
        }
        
        // Acknowledgment to sender
        QJsonObject ack = {
            {"type", "message_ack"},
            {"status", "sent"}
        };
        sendJsonMessage(client, ack);
    }
    else if (type == "history") {
        // Request message history
        QString with = j["with"].toString();
        QString currentUser = m_onlineUsers.key(client);
        
        if (currentUser.isEmpty()) {
            QJsonObject error = {
                {"type", "error"},
                {"message", "Not authenticated"}
            };
            sendJsonMessage(client, error);
            return;
        }
        
        Database db;
        if (db.initialize()) {
            auto messages = db.getMessages(currentUser.toStdString(), with.toStdString());
            QJsonObject history = {
                {"type", "history"},
                {"with", with}
            };
            
            QJsonArray messagesArray;
            for (const auto& msg : messages) {
                QJsonObject messageObj = {
                    {"id", msg.id},
                    {"sender", QString::fromStdString(msg.sender)},
                    {"text", QString::fromStdString(msg.text)},
                    {"timestamp", QString::fromStdString(msg.timestamp)}
                };
                messagesArray.append(messageObj);
            }
            history["messages"] = messagesArray;
            
            sendJsonMessage(client, history);
        }
    }
    else if (type == "ping") {
        // Pong for connection check
        QJsonObject pong = {
            {"type", "pong"},
            {"timestamp", QDateTime::currentSecsSinceEpoch()}
        };
        sendJsonMessage(client, pong);
    }
    else {
        QJsonObject error = {
            {"type", "error"},
            {"message", "Unknown message type"}
        };
        sendJsonMessage(client, error);
    }
}

void WebSocketServer::sendJsonMessage(QWebSocket* client, const QJsonObject& message) {
    QJsonDocument doc(message);
    client->sendTextMessage(doc.toJson());
} 