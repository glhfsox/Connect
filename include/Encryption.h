#pragma once

#include <sodium.h>
#include <string>
#include <vector>
#include <unordered_map>

class Encryption {
public:
    Encryption();
    ~Encryption();

    // Генерация ключей
    bool generateKeyPair();
    std::string getPublicKey() const;
    std::string getPrivateKey() const;
    
    // Шифрование/дешифрование
    std::string encryptMessage(const std::string& message, const std::string& recipientPublicKey);
    std::string decryptMessage(const std::string& encryptedMessage, const std::string& senderPublicKey);
    
    // Обмен ключами
    bool establishSession(const std::string& peerPublicKey);
    std::string getSessionKey() const;
    
    // Хеширование
    static std::string hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& hash);

private:
    unsigned char m_publicKey[crypto_box_PUBLICKEYBYTES];
    unsigned char m_privateKey[crypto_box_SECRETKEYBYTES];
    unsigned char m_sessionKey[crypto_secretbox_KEYBYTES];
    bool m_sessionEstablished = false;
    
    std::unordered_map<std::string, unsigned char[crypto_secretbox_KEYBYTES]> m_peerSessions;
}; 