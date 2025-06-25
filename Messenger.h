#ifndef MESSENGER_H
#define MESSENGER_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QObject>
#include <QFile>
#include <sodium.h>

class Messenger : public QObject {
    Q_OBJECT
public:
    explicit Messenger(QObject* parent = nullptr);
    ~Messenger();

    bool startServer(quint16 port);
    void connectToHost(const QString& host, quint16 port);

    void sendMessage(const QByteArray& message);

signals:
    void messageReceived(const QByteArray& message);
    void connectionStatusChanged(const QString& status);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();
    void onNewConnection();

private:
    void sendPublicKey();
    void loadIdentity();

    unsigned char m_pk[crypto_kx_PUBLICKEYBYTES];
    unsigned char m_sk[crypto_kx_SECRETKEYBYTES];
    unsigned char m_peerPk[crypto_kx_PUBLICKEYBYTES];
    unsigned char m_rx[crypto_kx_SESSIONKEYBYTES];
    unsigned char m_tx[crypto_kx_SESSIONKEYBYTES];

    QByteArray m_buffer;
    bool m_isServer = false;
    bool m_handshakeDone = false;

    QTcpSocket* m_socket;
    QTcpServer* m_server;

};

#endif // MESSENGER_H
