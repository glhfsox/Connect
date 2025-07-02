#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declaration
struct sqlite3;

struct Message {
    int id;
    std::string sender;
    std::string receiver;
    std::string text;
    std::string timestamp;
    std::string messageType; // "text", "image", "video", "voice"
    std::string mediaPath;   // путь к медиафайлу, если есть
};

struct Media {
    int id;
    std::string sender;
    std::string receiver;
    std::string path;
    std::string type; // "photo", "video", "voice"
    std::string timestamp;
};

class Database {
public:
    Database(const std::string& dbPath = "data/messenger.db");
    ~Database();

    bool initialize();
    
    // Сообщения
    bool saveMessage(const std::string& sender, const std::string& receiver, 
                    const std::string& text, const std::string& messageType = "text",
                    const std::string& mediaPath = "");
    std::vector<Message> getMessages(const std::string& user1, const std::string& user2, int limit = 100);
    
    // Медиафайлы
    bool saveMedia(const std::string& sender, const std::string& receiver,
                  const std::string& path, const std::string& type);
    std::vector<Media> getMedia(const std::string& user1, const std::string& user2);
    
    // Пользователи
    bool userExists(const std::string& username);
    bool createUser(const std::string& username);

private:
    std::string m_dbPath;
    sqlite3* m_db;
    
    void createTables();
}; 