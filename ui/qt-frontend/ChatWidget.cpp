#include "ChatWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QDateTime>
#include <QKeyEvent>

ChatWidget::ChatWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ChatWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Заголовок чата
    m_contactLabel = new QLabel("Select a contact to start chatting");
    m_contactLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 10px;");
    m_mainLayout->addWidget(m_contactLabel);
    
    // Область сообщений
    m_scrollArea = new QScrollArea();
    m_messagesContainer = new QWidget();
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    m_messagesLayout->addStretch(); // Добавляем растяжку в конец
    
    m_scrollArea->setWidget(m_messagesContainer);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_mainLayout->addWidget(m_scrollArea);
    
    // Панель ввода
    QWidget* inputPanel = new QWidget();
    QHBoxLayout* inputLayout = new QHBoxLayout(inputPanel);
    
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Type your message...");
    m_messageInput->installEventFilter(this);
    
    m_sendButton = new QPushButton("Send");
    m_attachButton = new QPushButton("📎");
    m_voiceButton = new QPushButton("🎤");
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_attachButton);
    inputLayout->addWidget(m_voiceButton);
    inputLayout->addWidget(m_sendButton);
    
    m_mainLayout->addWidget(inputPanel);
    
    // Подключение сигналов
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(m_attachButton, &QPushButton::clicked, this, &ChatWidget::onAttachClicked);
    connect(m_voiceButton, &QPushButton::clicked, this, &ChatWidget::onVoiceClicked);
}

void ChatWidget::addMessage(const QString& sender, const QString& text, const QDateTime& timestamp, bool isOwn) {
    QFrame* messageFrame = new QFrame();
    QHBoxLayout* messageLayout = new QHBoxLayout(messageFrame);
    
    // Создаем виджет сообщения
    QWidget* messageWidget = new QWidget();
    QVBoxLayout* messageWidgetLayout = new QVBoxLayout(messageWidget);
    
    QLabel* senderLabel = new QLabel(sender);
    senderLabel->setStyleSheet("font-weight: bold; color: #0078d4;");
    
    QTextEdit* textEdit = new QTextEdit();
    textEdit->setPlainText(text);
    textEdit->setMaximumHeight(100);
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("border: none; background: transparent;");
    
    QLabel* timeLabel = new QLabel(timestamp.toString("hh:mm"));
    timeLabel->setStyleSheet("color: gray; font-size: 10px;");
    
    messageWidgetLayout->addWidget(senderLabel);
    messageWidgetLayout->addWidget(textEdit);
    messageWidgetLayout->addWidget(timeLabel);
    
    // Выравнивание сообщений
    if (isOwn) {
        messageLayout->addStretch();
        messageWidget->setStyleSheet("background-color: #0078d4; color: white; border-radius: 10px; padding: 5px;");
        senderLabel->setStyleSheet("font-weight: bold; color: white;");
        timeLabel->setStyleSheet("color: lightgray; font-size: 10px;");
    } else {
        messageWidget->setStyleSheet("background-color: #f0f0f0; border-radius: 10px; padding: 5px;");
        messageLayout->addWidget(messageWidget);
        messageLayout->addStretch();
    }
    
    // Вставляем сообщение перед растяжкой
    m_messagesLayout->insertWidget(m_messagesLayout->count() - 1, messageFrame);
    
    // Прокручиваем к последнему сообщению
    scrollToBottom();
}

void ChatWidget::addMediaMessage(const QString& sender, const QString& mediaPath, const QString& type, const QDateTime& timestamp, bool isOwn) {
    QFrame* messageFrame = new QFrame();
    QHBoxLayout* messageLayout = new QHBoxLayout(messageFrame);
    
    QWidget* messageWidget = new QWidget();
    QVBoxLayout* messageWidgetLayout = new QVBoxLayout(messageWidget);
    
    QLabel* senderLabel = new QLabel(sender);
    senderLabel->setStyleSheet("font-weight: bold; color: #0078d4;");
    
    QLabel* mediaLabel = new QLabel();
    if (type == "image") {
        mediaLabel->setText("📷 Image");
    } else if (type == "video") {
        mediaLabel->setText("🎥 Video");
    } else if (type == "voice") {
        mediaLabel->setText("🎤 Voice Message");
    } else {
        mediaLabel->setText("📎 File");
    }
    
    QLabel* timeLabel = new QLabel(timestamp.toString("hh:mm"));
    timeLabel->setStyleSheet("color: gray; font-size: 10px;");
    
    messageWidgetLayout->addWidget(senderLabel);
    messageWidgetLayout->addWidget(mediaLabel);
    messageWidgetLayout->addWidget(timeLabel);
    
    if (isOwn) {
        messageLayout->addStretch();
        messageWidget->setStyleSheet("background-color: #0078d4; color: white; border-radius: 10px; padding: 5px;");
        senderLabel->setStyleSheet("font-weight: bold; color: white;");
        timeLabel->setStyleSheet("color: lightgray; font-size: 10px;");
    } else {
        messageWidget->setStyleSheet("background-color: #f0f0f0; border-radius: 10px; padding: 5px;");
        messageLayout->addWidget(messageWidget);
        messageLayout->addStretch();
    }
    
    m_messagesLayout->insertWidget(m_messagesLayout->count() - 1, messageFrame);
    scrollToBottom();
}

void ChatWidget::clearChat() {
    // Удаляем все виджеты сообщений, кроме растяжки
    QLayoutItem* item;
    while ((item = m_messagesLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_messagesLayout->addStretch(); // Добавляем растяжку обратно
}

void ChatWidget::setCurrentContact(const QString& contact) {
    m_currentContact = contact;
    m_contactLabel->setText(QString("Chat with %1").arg(contact));
}

void ChatWidget::onSendClicked() {
    QString text = m_messageInput->text().trimmed();
    if (!text.isEmpty()) {
        emit messageSent(text);
        m_messageInput->clear();
    }
}

void ChatWidget::onAttachClicked() {
    emit fileAttachRequested();
}

void ChatWidget::onVoiceClicked() {
    emit voiceRecordRequested();
}

void ChatWidget::scrollToBottom() {
    QTimer::singleShot(100, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
    });
}

bool ChatWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return && !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            onSendClicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
} 