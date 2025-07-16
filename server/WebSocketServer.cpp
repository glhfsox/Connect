#include "../include/WebSocketServer.h"
#include "../include/Database.h"
#include "../include/Encryption.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QFile>
#include <iostream>

WebSocketServer::WebSocketServer(QObject* parent)
    : QObject(parent)
    , m_server(new QWebSocketServer("Connect Messenger", QWebSocketServer::NonSecureMode, this))
    , m_httpServer(new QTcpServer(this))
{
    connect(m_server.get(), &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
    connect(m_httpServer.get(), &QTcpServer::newConnection, this, &WebSocketServer::onTcpConnection);
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start(int port) {
    // Start a single TCP server that will handle both WebSocket upgrades and HTTP requests.
    if (!m_httpServer->listen(QHostAddress::Any, port)) {
        std::cerr << "Failed to start TCP listener: " << m_httpServer->errorString().toStdString() << std::endl;
        return false;
    }

    m_running = true;
    std::cout << "Server listening (WebSocket+HTTP) on port " << port << std::endl;
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

void WebSocketServer::onTcpConnection() {
    QTcpSocket* socket = m_httpServer->nextPendingConnection();

    // Handle the very first data chunk to decide if this is a plain HTTP request or a WebSocket upgrade.
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        if (!socket->bytesAvailable()) {
            return;
        }

        // Peek so we do not consume bytes in case we hand over the socket to the WebSocket server.
        QByteArray data = socket->peek(2048);
        QString requestStr = QString::fromUtf8(data);

        // Health-check endpoint.
        if (requestStr.startsWith("GET /health")) {
            // Consume the request.
            socket->readAll();
            QString body = QStringLiteral("{\"status\":\"healthy\",\"online_users\":%1}").arg(m_onlineUsers.size());
            QByteArray response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: application/json\r\n" +
                                   QByteArray("Content-Length: ") + QByteArray::number(body.toUtf8().size()) + "\r\n\r\n" +
                                   body.toUtf8();
            socket->write(response);
            socket->disconnectFromHost();
            return;
        }

        // Serve web client
        if (requestStr.startsWith("GET / ") || requestStr.startsWith("GET /index.html")) {
            socket->readAll();
            
            // Read web_client.html file
            QFile file("web_client.html");
            QByteArray html;
            if (file.open(QIODevice::ReadOnly)) {
                html = file.readAll();
            } else {
                // Fallback HTML if file not found
                html = R"(<html><body><h1>Connect Messenger Server</h1><p>Server is running</p><p><a href="/client">Web Client</a></p></body></html>)";
            }
            
            QByteArray response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n" +
                                   QByteArray("Content-Length: ") + QByteArray::number(html.size()) + "\r\n\r\n" +
                                   html;
            socket->write(response);
            socket->disconnectFromHost();
            return;
        }
        
        // Serve web client at /client endpoint
        if (requestStr.startsWith("GET /client")) {
            socket->readAll();
            
            QFile file("web_client.html");
            QByteArray html;
            if (file.open(QIODevice::ReadOnly)) {
                html = file.readAll();
            } else {
                html = "<html><body><h1>Web Client Not Found</h1></body></html>";
            }
            
            QByteArray response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n" +
                                   QByteArray("Content-Length: ") + QByteArray::number(html.size()) + "\r\n\r\n" +
                                   html;
            socket->write(response);
            socket->disconnectFromHost();
            return;
        }

        // If it contains an Upgrade header assume it is a WebSocket handshake, delegate.
        if (requestStr.contains("Upgrade: websocket", Qt::CaseInsensitive)) {
            // Disconnect the lambda to avoid re-entry after handing over.
            socket->disconnect();
            m_server->handleConnection(socket);  // QWebSocketServer takes ownership.
            return;
        }

        // Any other HTTP request -> 404
        socket->readAll();
        const QByteArray notFound = "Not Found";
        QByteArray response = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n" +
                               QByteArray("Content-Length: ") + QByteArray::number(notFound.size()) + "\r\n\r\n" +
                               notFound;
        socket->write(response);
        socket->disconnectFromHost();
    });

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
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