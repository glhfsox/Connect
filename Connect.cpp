#include "Connect.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>

ChatWindow::ChatWindow(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_status = new QLabel("Idle", this);
    layout->addWidget(m_status);

    QHBoxLayout* configLayout = new QHBoxLayout();
    m_listenPort = new QLineEdit(this);
    m_listenPort->setPlaceholderText("Listen Port");
    QPushButton* listenButton = new QPushButton("Listen", this);
    m_peerHost = new QLineEdit(this);
    m_peerHost->setPlaceholderText("Peer Host");
    m_peerPort = new QLineEdit(this);
    m_peerPort->setPlaceholderText("Peer Port");
    QPushButton* connectButton = new QPushButton("Connect", this);

    configLayout->addWidget(m_listenPort);
    configLayout->addWidget(listenButton);
    configLayout->addWidget(m_peerHost);
    configLayout->addWidget(m_peerPort);
    configLayout->addWidget(connectButton);
    layout->addLayout(configLayout);

    m_chat = new QTextEdit(this);
    m_chat->setReadOnly(true);
    m_input = new QLineEdit(this);
    QPushButton* sendButton = new QPushButton("Send", this);

    layout->addWidget(m_chat);
    layout->addWidget(m_input);
    layout->addWidget(sendButton);

    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSend);
    connect(listenButton, &QPushButton::clicked, this, &ChatWindow::onStartServer);
    connect(connectButton, &QPushButton::clicked, this, &ChatWindow::onConnectClicked);
    connect(&m_messenger, &Messenger::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(&m_messenger, &Messenger::connectionStatusChanged, this, &ChatWindow::onStatusChanged);
    connect(&m_messenger, &Messenger::errorOccurred, this, &ChatWindow::onStatusChanged);
}

void ChatWindow::connectToPeer(const QString& host, quint16 port) {
    m_messenger.connectToHost(host, port);
}

void ChatWindow::onSend() {
    QByteArray msg = m_input->text().toUtf8();
    m_messenger.sendMessage(msg);
    m_chat->append("Me: " + m_input->text());
    m_input->clear();
}

void ChatWindow::onMessageReceived(const QByteArray& msg) {
    m_chat->append("Peer: " + QString::fromUtf8(msg));
}

void ChatWindow::onStartServer() {
    quint16 port = m_listenPort->text().toUShort();
    m_messenger.startServer(port);
}

void ChatWindow::onConnectClicked() {
    connectToPeer(m_peerHost->text(), m_peerPort->text().toUShort());
}

void ChatWindow::onStatusChanged(const QString& status) {
    m_status->setText(status);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ChatWindow window;
    window.show();
    return app.exec();
}
