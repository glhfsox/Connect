#pragma once

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QMap>
#include <QString>
#include <QTcpServer>
#include <memory>

class WebSocketServer : public QObject {
    Q_OBJECT

public:
    WebSocketServer(QObject* parent = nullptr);
    ~WebSocketServer();

    bool start(int port = 9001);
    void stop();
    bool isRunning() const;

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString& message);
    void onDisconnected();
    void onHttpRequest();

private:
    void handleMessage(QWebSocket* client, const QString& message);
    void sendJsonMessage(QWebSocket* client, const QJsonObject& message);
    void handleHttpRequest(QTcpSocket* socket, const QByteArray& request);

    std::unique_ptr<QWebSocketServer> m_server;
    std::unique_ptr<QTcpServer> m_httpServer;
    QMap<QString, QWebSocket*> m_onlineUsers;
    bool m_running = false;
}; 