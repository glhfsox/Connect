#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "Messenger.h"

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    ChatWindow(QWidget* parent = nullptr);

    void connectToPeer(const QString& host, quint16 port);
private slots:
    void onSend();
    void onMessageReceived(const QByteArray& msg);
    void onStartServer();
    void onConnectClicked();
    void onStatusChanged(const QString& status);

private:
    Messenger m_messenger;
    QTextEdit* m_chat;
    QLineEdit* m_input;
    QLineEdit* m_listenPort;
    QLineEdit* m_peerHost;
    QLineEdit* m_peerPort;
    QLabel* m_status;
}; 

