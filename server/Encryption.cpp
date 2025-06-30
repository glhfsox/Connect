#include "../include/Encryption.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Encryption::Encryption() {
    if (sodium_init() < 0) {
        std::cerr << "Failed to initialize libsodium" << std::endl;
    }
}

Encryption::~Encryption() = default;

bool Encryption::generateKeyPair() {
    try {
        crypto_box_keypair(m_publicKey, m_privateKey);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to generate key pair: " << e.what() << std::endl;
        return false;
    }
}

std::string Encryption::getPublicKey() const {
    return std::string(reinterpret_cast<const char*>(m_publicKey), crypto_box_PUBLICKEYBYTES);
}

std::string Encryption::getPrivateKey() const {
    return std::string(reinterpret_cast<const char*>(m_privateKey), crypto_box_SECRETKEYBYTES);
}

std::string Encryption::encryptMessage(const std::string& message, const std::string& recipientPublicKey) {
    if (recipientPublicKey.length() != crypto_box_PUBLICKEYBYTES) {
        std::cerr << "Invalid recipient public key length" << std::endl;
        return "";
    }
    
    try {
        // Генерируем случайный nonce
        unsigned char nonce[crypto_box_NONCEBYTES];
        randombytes_buf(nonce, crypto_box_NONCEBYTES);
        
        // Шифруем сообщение
        std::vector<unsigned char> ciphertext(message.length() + crypto_box_MACBYTES);
        
        if (crypto_box_easy(ciphertext.data(), 
                           reinterpret_cast<const unsigned char*>(message.c_str()),
                           message.length(),
                           nonce,
                           reinterpret_cast<const unsigned char*>(recipientPublicKey.c_str()),
                           m_privateKey) != 0) {
            std::cerr << "Encryption failed" << std::endl;
            return "";
        }
        
        // Объединяем nonce и зашифрованное сообщение
        std::string result;
        result.append(reinterpret_cast<const char*>(nonce), crypto_box_NONCEBYTES);
        result.append(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Encryption error: " << e.what() << std::endl;
        return "";
    }
}

std::string Encryption::decryptMessage(const std::string& encryptedMessage, const std::string& senderPublicKey) {
    if (senderPublicKey.length() != crypto_box_PUBLICKEYBYTES) {
        std::cerr << "Invalid sender public key length" << std::endl;
        return "";
    }
    
    if (encryptedMessage.length() < crypto_box_NONCEBYTES + crypto_box_MACBYTES) {
        std::cerr << "Encrypted message too short" << std::endl;
        return "";
    }
    
    try {
        // Извлекаем nonce и зашифрованное сообщение
        const unsigned char* nonce = reinterpret_cast<const unsigned char*>(encryptedMessage.c_str());
        const unsigned char* ciphertext = reinterpret_cast<const unsigned char*>(encryptedMessage.c_str() + crypto_box_NONCEBYTES);
        size_t ciphertextLen = encryptedMessage.length() - crypto_box_NONCEBYTES;
        
        // Расшифровываем сообщение
        std::vector<unsigned char> plaintext(ciphertextLen - crypto_box_MACBYTES);
        
        if (crypto_box_open_easy(plaintext.data(),
                                ciphertext,
                                ciphertextLen,
                                nonce,
                                reinterpret_cast<const unsigned char*>(senderPublicKey.c_str()),
                                m_privateKey) != 0) {
            std::cerr << "Decryption failed" << std::endl;
            return "";
        }
        
        return std::string(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
    } catch (const std::exception& e) {
        std::cerr << "Decryption error: " << e.what() << std::endl;
        return "";
    }
}

bool Encryption::establishSession(const std::string& peerPublicKey) {
    if (peerPublicKey.length() != crypto_box_PUBLICKEYBYTES) {
        std::cerr << "Invalid peer public key length" << std::endl;
        return false;
    }
    
    try {
        // Генерируем сессионные ключи
        unsigned char rx[crypto_kx_SESSIONKEYBYTES];
        unsigned char tx[crypto_kx_SESSIONKEYBYTES];
        
        if (crypto_kx_client_session_keys(rx, tx,
                                        m_publicKey, m_privateKey,
                                        reinterpret_cast<const unsigned char*>(peerPublicKey.c_str())) != 0) {
            std::cerr << "Failed to establish session" << std::endl;
            return false;
        }
        
        // Сохраняем сессионные ключи
        std::memcpy(m_sessionKey, tx, crypto_kx_SESSIONKEYBYTES);
        m_sessionEstablished = true;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Session establishment error: " << e.what() << std::endl;
        return false;
    }
}

std::string Encryption::getSessionKey() const {
    if (!m_sessionEstablished) {
        return "";
    }
    return std::string(reinterpret_cast<const char*>(m_sessionKey), crypto_kx_SESSIONKEYBYTES);
}

std::string Encryption::hashPassword(const std::string& password) {
    try {
        unsigned char hash[crypto_pwhash_STRBYTES];
        
        if (crypto_pwhash_str(reinterpret_cast<char*>(hash),
                             password.c_str(),
                             password.length(),
                             crypto_pwhash_OPSLIMIT_INTERACTIVE,
                             crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
            std::cerr << "Password hashing failed" << std::endl;
            return "";
        }
        
        return std::string(reinterpret_cast<const char*>(hash), crypto_pwhash_STRBYTES);
    } catch (const std::exception& e) {
        std::cerr << "Password hashing error: " << e.what() << std::endl;
        return "";
    }
}

bool Encryption::verifyPassword(const std::string& password, const std::string& hash) {
    try {
        return crypto_pwhash_str_verify(hash.c_str(),
                                       password.c_str(),
                                       password.length()) == 0;
    } catch (const std::exception& e) {
        std::cerr << "Password verification error: " << e.what() << std::endl;
        return false;
    }
} 