#include "Messenger.h"
#include <QDataStream>
#include <QDebug>

Messenger::Messenger(QObject* parent)
    : QObject(parent), m_socket(new QTcpSocket(this)), m_server(new QTcpServer(this)) {
    if (sodium_init() < 0) {
        qFatal("Failed to initialize libsodium");
    }
    loadIdentity();
    connect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        m_isServer = false;
        sendPublicKey();
        emit connectionStatusChanged("Connected");
    });
    connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
        m_handshakeDone = false;
        emit connectionStatusChanged("Disconnected");
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError){
        emit errorOccurred(m_socket->errorString());
    });
    connect(m_server, &QTcpServer::newConnection, this, &Messenger::onNewConnection);
}

Messenger::~Messenger() {}

bool Messenger::startServer(quint16 port) {
    bool ok = m_server->listen(QHostAddress::Any, port);
    if (!ok)
        emit errorOccurred(m_server->errorString());
    else
        emit connectionStatusChanged(QString("Listening on %1").arg(port));
    return ok;
}

void Messenger::connectToHost(const QString& host, quint16 port) {
    if (m_socket->isOpen())
        m_socket->disconnectFromHost();
    m_socket->connectToHost(host, port);
}

void Messenger::sendMessage(const QByteArray& message) {
    if (!m_handshakeDone || m_socket->state() != QAbstractSocket::ConnectedState) {
        emit errorOccurred("Not connected");
        return;
    }

    QByteArray cipher(message.size() + crypto_secretbox_MACBYTES, 0);
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    crypto_secretbox_easy((unsigned char*)cipher.data(),
                    (const unsigned char*)message.data(), message.size(),
                    nonce, m_tx);

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    quint32 len = sizeof nonce + cipher.size();
    out << len;
    out.writeRawData((char*)nonce, sizeof nonce);
    out.writeRawData(cipher.data(), cipher.size());

    qint64 written = m_socket->write(packet);
    if (written != packet.size())
        emit errorOccurred("Failed to send packet");
}

void Messenger::onReadyRead() {
    m_buffer.append(m_socket->readAll());

    while (true) {
        if (!m_handshakeDone) {
            if (m_buffer.size() < crypto_kx_PUBLICKEYBYTES)
                return;
            memcpy(m_peerPk, m_buffer.constData(), crypto_kx_PUBLICKEYBYTES);
            m_buffer.remove(0, crypto_kx_PUBLICKEYBYTES);
            int ret;
            if (m_isServer)
                ret = crypto_kx_server_session_keys(m_rx, m_tx, m_pk, m_sk, m_peerPk);
            else
                ret = crypto_kx_client_session_keys(m_rx, m_tx, m_pk, m_sk, m_peerPk);
            if (ret != 0) {
                emit errorOccurred("Handshake failed");
                m_socket->disconnectFromHost();
                return;
            }
            m_handshakeDone = true;
            emit connectionStatusChanged("Secure channel established");
            continue;
        }

        if (m_buffer.size() < 4)
            return;
        QDataStream in(m_buffer);
        in.setByteOrder(QDataStream::LittleEndian);
        quint32 len;
        in >> len;
        if (m_buffer.size() < 4 + len)
            return;
        m_buffer.remove(0, 4);
        QByteArray payload = m_buffer.left(len);
        m_buffer.remove(0, len);

        const unsigned char* nonce = (const unsigned char*)payload.constData();
        QByteArray cipher = payload.mid(crypto_secretbox_NONCEBYTES);
        QByteArray message(cipher.size() - crypto_secretbox_MACBYTES, 0);
        if (crypto_secretbox_open_easy((unsigned char*)message.data(),
                                 (const unsigned char*)cipher.data(), cipher.size(),
                                 nonce, m_rx) == 0) {
            emit messageReceived(message);
        } else {
            emit errorOccurred("Decryption failed");
        }
    }
}

void Messenger::onNewConnection() {
    QTcpSocket* client = m_server->nextPendingConnection();
    if (!client) return;

    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        client->close();
        client->deleteLater();
        return;
    }

    m_socket->deleteLater();
    m_socket = client;
    m_isServer = true;
    m_handshakeDone = false;
    connect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
        m_handshakeDone = false;
        emit connectionStatusChanged("Disconnected");
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError){
        emit errorOccurred(m_socket->errorString());
    });
    sendPublicKey();
    emit connectionStatusChanged("Client connected");
}

void Messenger::sendPublicKey() {
    m_socket->write(QByteArray((char*)m_pk, crypto_kx_PUBLICKEYBYTES));
}

void Messenger::loadIdentity() {
    QFile f("identity.key");
    if (f.open(QIODevice::ReadOnly)) {
        f.read((char*)m_pk, crypto_kx_PUBLICKEYBYTES);
        f.read((char*)m_sk, crypto_kx_SECRETKEYBYTES);
        f.close();
    } else {
        crypto_kx_keypair(m_pk, m_sk);
        if (f.open(QIODevice::WriteOnly)) {
            f.write((char*)m_pk, crypto_kx_PUBLICKEYBYTES);
            f.write((char*)m_sk, crypto_kx_SECRETKEYBYTES);
            f.close();
        }
    }
}
