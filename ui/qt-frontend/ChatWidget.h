#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QDateTime>

class MessageWidget;

class ChatWidget : public QWidget {
    Q_OBJECT

public:
    ChatWidget(QWidget* parent = nullptr);

public slots:
    void addMessage(const QString& sender, const QString& text, const QDateTime& timestamp, bool isOwn = false);
    void addMediaMessage(const QString& sender, const QString& mediaPath, const QString& type, const QDateTime& timestamp, bool isOwn = false);
    void clearChat();
    void setCurrentContact(const QString& contact);

signals:
    void messageSent(const QString& text);
    void fileAttachRequested();
    void voiceRecordRequested();

private slots:
    void onSendClicked();
    void onAttachClicked();
    void onVoiceClicked();

private:
    void setupUI();
    void scrollToBottom();

    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_messagesContainer;
    QVBoxLayout* m_messagesLayout;
    QLineEdit* m_messageInput;
    QPushButton* m_sendButton;
    QPushButton* m_attachButton;
    QPushButton* m_voiceButton;
    QLabel* m_contactLabel;
    QString m_currentContact;
}; 