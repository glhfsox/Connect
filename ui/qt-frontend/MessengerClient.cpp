#include "MessengerClient.h"
#include "ChatWidget.h"
#include "ContactListWidget.h"
#include "../../include/Encryption.h"
#include <QApplication>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QDateTime>

MessengerClient::MessengerClient(QWidget* parent)
    : QMainWindow(parent)
    , m_webSocket(new QWebSocket())
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_audioRecorder(new QAudioRecorder(this))
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_trayMenu(new QMenu())
    , m_settings(new QSettings("Connect", "Messenger", this))
    , m_encryption(std::make_unique<Encryption>())
    , m_connected(false)
    , m_authenticated(false)
{
    setWindowTitle("Connect Messenger");
    setMinimumSize(800, 600);
    
    // Initialize UI
    setupUI();
    setupTrayIcon();
    setupWebSocket();
    
    // Create data directories
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir().mkpath(documentsPath + "/Connect/Media");
    
    // Генерируем ключи шифрования
    m_encryption->generateKeyPair();
}

MessengerClient::~MessengerClient() {
    saveSettings();
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->close();
    }
}

void MessengerClient::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    // Left panel - Connection and contacts
    QGroupBox* leftPanel = new QGroupBox("Connection & Contacts");
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    // Connection section
    QGroupBox* connectionGroup = new QGroupBox("Server Connection");
    QVBoxLayout* connectionLayout = new QVBoxLayout(connectionGroup);
    
    m_serverInput = new QLineEdit("localhost:9001");
    m_usernameInput = new QLineEdit();
    m_usernameInput->setPlaceholderText("Enter username");
    
    m_connectButton = new QPushButton("Connect to Server");
    m_loginButton = new QPushButton("Login");
    m_loginButton->setEnabled(false);
    
    connectionLayout->addWidget(new QLabel("Server:"));
    connectionLayout->addWidget(m_serverInput);
    connectionLayout->addWidget(new QLabel("Username:"));
    connectionLayout->addWidget(m_usernameInput);
    connectionLayout->addWidget(m_connectButton);
    connectionLayout->addWidget(m_loginButton);
    
    // Contacts section
    QGroupBox* contactsGroup = new QGroupBox("Contacts");
    QVBoxLayout* contactsLayout = new QVBoxLayout(contactsGroup);
    
    m_contactList = new ContactListWidget();
    contactsLayout->addWidget(m_contactList);
    
    leftLayout->addWidget(connectionGroup);
    leftLayout->addWidget(contactsGroup);
    
    // Right panel - Chat
    QGroupBox* rightPanel = new QGroupBox("Chat");
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    
    m_chatWidget = new ChatWidget();
    rightLayout->addWidget(m_chatWidget);
    
    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 2);
    
    // Connect signals
    connect(m_connectButton, &QPushButton::clicked, this, &MessengerClient::connectToServer);
    connect(m_loginButton, &QPushButton::clicked, this, &MessengerClient::login);
    connect(m_contactList, &ContactListWidget::contactSelected, this, &MessengerClient::onContactSelected);
    connect(m_chatWidget, &ChatWidget::messageSent, this, &MessengerClient::sendMessage);
}

void MessengerClient::setupTrayIcon() {
    // Create tray icon
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    m_trayIcon->setToolTip("Connect Messenger");
    
    // Create tray menu
    QMenu* trayMenu = new QMenu();
    QAction* showAction = new QAction("Show", this);
    QAction* quitAction = new QAction("Quit", this);
    
    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    
    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
    
    // Connect signals
    connect(showAction, &QAction::triggered, this, &QWidget::show);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MessengerClient::onTrayIconActivated);
}

void MessengerClient::setupWebSocket() {
    connect(m_webSocket, &QWebSocket::connected, this, &MessengerClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &MessengerClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &MessengerClient::onMessageReceived);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &MessengerClient::onError);
}

void MessengerClient::connectToServer() {
    QString serverUrl = m_serverInput->text();
    if (!serverUrl.startsWith("ws://")) {
        serverUrl = "ws://" + serverUrl;
    }
    
    m_webSocket->open(QUrl(serverUrl));
    m_connectButton->setEnabled(false);
    m_connectButton->setText("Connecting...");
}

void MessengerClient::login() {
    QString username = m_usernameInput->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a username");
        return;
    }
    
    QJsonObject authMessage = {
        {"type", "auth"},
        {"username", username}
    };
    
    sendJsonMessage(authMessage);
}

void MessengerClient::onConnected() {
    m_connected = true;
    m_connectButton->setText("Connected");
    m_connectButton->setEnabled(false);
    m_loginButton->setEnabled(true);
    
    m_trayIcon->showMessage("Connect Messenger", "Connected to server", QSystemTrayIcon::Information, 2000);
}

void MessengerClient::onDisconnected() {
    m_connected = false;
    m_authenticated = false;
    m_connectButton->setText("Connect to Server");
    m_connectButton->setEnabled(true);
    m_loginButton->setEnabled(false);
    
    m_trayIcon->showMessage("Connect Messenger", "Disconnected from server", QSystemTrayIcon::Warning, 2000);
}

void MessengerClient::onMessageReceived(const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject j = doc.object();
    QString type = j["type"].toString();
    
    if (type == "auth_response") {
        if (j["status"].toString() == "success") {
            m_authenticated = true;
            m_loginButton->setText("Logged In");
            m_loginButton->setEnabled(false);
            
            // Load contacts (for demo, add some dummy contacts)
            m_contactList->addContact("Alice");
            m_contactList->addContact("Bob");
            m_contactList->addContact("Charlie");
            
            m_trayIcon->showMessage("Connect Messenger", "Successfully logged in", QSystemTrayIcon::Information, 2000);
        } else {
            QMessageBox::warning(this, "Authentication Error", j["message"].toString());
        }
    }
    else if (type == "message") {
        QString from = j["from"].toString();
        QString text = j["text"].toString();
        qint64 timestamp = j["timestamp"].toVariant().toLongLong();
        
        // Add contact if not exists
        m_contactList->addContact(from);
        
        // Show message in chat if this contact is selected
        if (m_currentContact == from) {
            m_chatWidget->addMessage(from, text, QDateTime::fromSecsSinceEpoch(timestamp));
        }
        
        // Show notification if window is not active
        if (!isActiveWindow()) {
            m_trayIcon->showMessage("New message from " + from, text, QSystemTrayIcon::Information, 3000);
        }
    }
    else if (type == "history") {
        QJsonArray messages = j["messages"].toArray();
        for (const QJsonValue& msg : messages) {
            QJsonObject messageObj = msg.toObject();
            QString sender = messageObj["sender"].toString();
            QString text = messageObj["text"].toString();
            qint64 timestamp = messageObj["timestamp"].toString().toLongLong();
            
            m_chatWidget->addMessage(sender, text, QDateTime::fromSecsSinceEpoch(timestamp));
        }
    }
    else if (type == "message_ack") {
        // Message sent successfully
        m_chatWidget->messageSent();
    }
    else if (type == "error") {
        QMessageBox::warning(this, "Server Error", j["message"].toString());
    }
}

void MessengerClient::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error)
    QMessageBox::critical(this, "Connection Error", "Failed to connect to server: " + m_webSocket->errorString());
    m_connectButton->setText("Connect to Server");
    m_connectButton->setEnabled(true);
    m_loginButton->setEnabled(false);
}

void MessengerClient::onContactSelected(const QString& contact) {
    m_currentContact = contact;
    m_chatWidget->clearMessages();
    m_chatWidget->setCurrentContact(contact);
    
    // Request message history
    if (m_authenticated) {
        QJsonObject historyRequest = {
            {"type", "history"},
            {"with", contact}
        };
        sendJsonMessage(historyRequest);
    }
}

void MessengerClient::sendMessage(const QString& text) {
    if (!m_authenticated || m_currentContact.isEmpty()) {
        return;
    }
    
    QJsonObject message = {
        {"type", "message"},
        {"to", m_currentContact},
        {"text", text}
    };
    
    sendJsonMessage(message);
}

void MessengerClient::sendJsonMessage(const QJsonObject& message) {
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(message);
        m_webSocket->sendTextMessage(doc.toJson());
    }
}

void MessengerClient::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
        raise();
        activateWindow();
    }
}

void MessengerClient::closeEvent(QCloseEvent* event) {
    if (m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MessengerClient::loadSettings() {
    m_usernameInput->setText(m_settings->value("username", "").toString());
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());
}

void MessengerClient::saveSettings() {
    m_settings->setValue("username", m_usernameInput->text());
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
} 