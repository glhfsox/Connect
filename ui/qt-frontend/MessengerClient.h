#pragma once

#include <QMainWindow>
#include <QWebSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QAudioRecorder>
#include <QTimer>
#include <QSettings>
#include <memory>

class ChatWidget;
class ContactListWidget;

class MessengerClient : public QMainWindow {
    Q_OBJECT

public:
    MessengerClient(QWidget* parent = nullptr);
    ~MessengerClient();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onSendMessage();
    void onConnectClicked();
    void onAuthClicked();
    void onContactSelected(const QString& contact);
    void onFileAttachClicked();
    void onVoiceRecordClicked();
    void onVoiceRecordFinished();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMainWindow();
    void onQuitApplication();

private:
    void setupUI();
    void setupWebSocket();
    void setupTrayIcon();
    void loadSettings();
    void saveSettings();
    void sendJsonMessage(const QJsonObject& message);
    void handleIncomingMessage(const QJsonObject& message);
    void showNotification(const QString& title, const QString& message);
    void loadChatHistory(const QString& contact);
    QString saveMediaFile(const QString& filePath, const QString& type);

    // UI компоненты
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    ContactListWidget* m_contactList;
    ChatWidget* m_chatWidget;
    QLineEdit* m_usernameInput;
    QPushButton* m_authButton;
    QPushButton* m_connectButton;
    QLabel* m_statusLabel;

    // WebSocket
    QWebSocket* m_webSocket;
    QString m_currentUser;
    QString m_currentContact;
    bool m_authenticated = false;

    // Медиа
    QMediaPlayer* m_mediaPlayer;
    QAudioRecorder* m_audioRecorder;
    QString m_recordingPath;

    // Системный трей
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;

    // Настройки
    QSettings* m_settings;
    
    // Шифрование
    std::unique_ptr<class Encryption> m_encryption;
}; 