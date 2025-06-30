#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class ContactListWidget : public QWidget {
    Q_OBJECT

public:
    ContactListWidget(QWidget* parent = nullptr);

public slots:
    void addContact(const QString& contact);
    void removeContact(const QString& contact);
    void clearContacts();
    void setOnlineStatus(const QString& contact, bool online);

signals:
    void contactSelected(const QString& contact);
    void contactDoubleClicked(const QString& contact);

private slots:
    void onContactClicked(QListWidgetItem* item);
    void onContactDoubleClicked(QListWidgetItem* item);
    void onSearchTextChanged(const QString& text);

private:
    void setupUI();
    void updateContactDisplay(const QString& contact, bool online);

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLineEdit* m_searchInput;
    QListWidget* m_contactList;
    QMap<QString, bool> m_onlineStatus; // contact -> online status
}; 