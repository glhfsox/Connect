#include "ContactListWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidgetItem>

ContactListWidget::ContactListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ContactListWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Заголовок
    m_titleLabel = new QLabel("Contacts");
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    m_mainLayout->addWidget(m_titleLabel);
    
    // Поиск
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Search contacts...");
    m_mainLayout->addWidget(m_searchInput);
    
    // Список контактов
    m_contactList = new QListWidget();
    m_contactList->setAlternatingRowColors(true);
    m_mainLayout->addWidget(m_contactList);
    
    // Подключение сигналов
    connect(m_contactList, &QListWidget::itemClicked, this, &ContactListWidget::onContactClicked);
    connect(m_contactList, &QListWidget::itemDoubleClicked, this, &ContactListWidget::onContactDoubleClicked);
    connect(m_searchInput, &QLineEdit::textChanged, this, &ContactListWidget::onSearchTextChanged);
}

void ContactListWidget::addContact(const QString& contact) {
    // Проверяем, есть ли уже такой контакт
    for (int i = 0; i < m_contactList->count(); ++i) {
        QListWidgetItem* item = m_contactList->item(i);
        if (item->text() == contact) {
            return; // Контакт уже существует
        }
    }
    
    QListWidgetItem* item = new QListWidgetItem(contact);
    item->setIcon(QIcon(":/icons/user.png")); // Можно добавить иконку
    m_contactList->addItem(item);
    
    // Устанавливаем статус онлайн по умолчанию
    m_onlineStatus[contact] = true;
    updateContactDisplay(contact, true);
}

void ContactListWidget::removeContact(const QString& contact) {
    for (int i = 0; i < m_contactList->count(); ++i) {
        QListWidgetItem* item = m_contactList->item(i);
        if (item->text() == contact) {
            delete m_contactList->takeItem(i);
            m_onlineStatus.remove(contact);
            break;
        }
    }
}

void ContactListWidget::clearContacts() {
    m_contactList->clear();
    m_onlineStatus.clear();
}

void ContactListWidget::setOnlineStatus(const QString& contact, bool online) {
    m_onlineStatus[contact] = online;
    updateContactDisplay(contact, online);
}

void ContactListWidget::onContactClicked(QListWidgetItem* item) {
    if (item) {
        emit contactSelected(item->text());
    }
}

void ContactListWidget::onContactDoubleClicked(QListWidgetItem* item) {
    if (item) {
        emit contactDoubleClicked(item->text());
    }
}

void ContactListWidget::onSearchTextChanged(const QString& text) {
    for (int i = 0; i < m_contactList->count(); ++i) {
        QListWidgetItem* item = m_contactList->item(i);
        bool visible = item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!visible);
    }
}

void ContactListWidget::updateContactDisplay(const QString& contact, bool online) {
    for (int i = 0; i < m_contactList->count(); ++i) {
        QListWidgetItem* item = m_contactList->item(i);
        if (item->text() == contact) {
            QString displayText = contact;
            if (online) {
                displayText += " ●"; // Зеленая точка для онлайн
                item->setForeground(Qt::black);
            } else {
                displayText += " ○"; // Серая точка для оффлайн
                item->setForeground(Qt::gray);
            }
            item->setText(displayText);
            break;
        }
    }
} 