#include "../include/Database.h"
#include <iostream>
#include <filesystem>
#include <ctime>

Database::Database(const std::string& dbPath) : m_dbPath(dbPath) {
    // Create database directory if it doesn't exist
    std::filesystem::path path(m_dbPath);
    std::filesystem::create_directories(path.parent_path());
}

Database::~Database() = default;

bool Database::initialize() {
    try {
        m_db = std::make_unique<SQLite::Database>(m_dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        createTables();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "Database initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void Database::createTables() {
    // Users table
    m_db->exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");
    
    // Messages table
    m_db->exec(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            receiver TEXT NOT NULL,
            text TEXT NOT NULL,
            message_type TEXT DEFAULT 'text',
            media_path TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (sender) REFERENCES users (username),
            FOREIGN KEY (receiver) REFERENCES users (username)
        )
    )");
    
    // Media table
    m_db->exec(R"(
        CREATE TABLE IF NOT EXISTS media (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            receiver TEXT NOT NULL,
            path TEXT NOT NULL,
            type TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (sender) REFERENCES users (username),
            FOREIGN KEY (receiver) REFERENCES users (username)
        )
    )");
    
    // Indexes for fast search
    m_db->exec("CREATE INDEX IF NOT EXISTS idx_messages_users ON messages(sender, receiver)");
    m_db->exec("CREATE INDEX IF NOT EXISTS idx_messages_timestamp ON messages(timestamp)");
    m_db->exec("CREATE INDEX IF NOT EXISTS idx_media_users ON media(sender, receiver)");
}

bool Database::saveMessage(const std::string& sender, const std::string& receiver, 
                          const std::string& text, const std::string& messageType,
                          const std::string& mediaPath) {
    try {
        SQLite::Statement query(*m_db, R"(
            INSERT INTO messages (sender, receiver, text, message_type, media_path)
            VALUES (?, ?, ?, ?, ?)
        )");
        
        query.bind(1, sender);
        query.bind(2, receiver);
        query.bind(3, text);
        query.bind(4, messageType);
        query.bind(5, mediaPath);
        
        query.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "Failed to save message: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Message> Database::getMessages(const std::string& user1, const std::string& user2, int limit) {
    std::vector<Message> messages;
    
    try {
        SQLite::Statement query(*m_db, R"(
            SELECT id, sender, receiver, text, message_type, media_path, timestamp
            FROM messages 
            WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)
            ORDER BY timestamp DESC
            LIMIT ?
        )");
        
        query.bind(1, user1);
        query.bind(2, user2);
        query.bind(3, user2);
        query.bind(4, user1);
        query.bind(5, limit);
        
        while (query.executeStep()) {
            Message msg;
            msg.id = query.getColumn(0);
            msg.sender = query.getColumn(1);
            msg.receiver = query.getColumn(2);
            msg.text = query.getColumn(3);
            msg.messageType = query.getColumn(4);
            msg.mediaPath = query.getColumn(5);
            msg.timestamp = query.getColumn(6);
            
            messages.push_back(msg);
        }
    } catch (const SQLite::Exception& e) {
        std::cerr << "Failed to get messages: " << e.what() << std::endl;
    }
    
    return messages;
}

bool Database::saveMedia(const std::string& sender, const std::string& receiver,
                        const std::string& path, const std::string& type) {
    try {
        SQLite::Statement query(*m_db, R"(
            INSERT INTO media (sender, receiver, path, type)
            VALUES (?, ?, ?, ?)
        )");
        
        query.bind(1, sender);
        query.bind(2, receiver);
        query.bind(3, path);
        query.bind(4, type);
        
        query.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "Failed to save media: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Media> Database::getMedia(const std::string& user1, const std::string& user2) {
    std::vector<Media> media;
    
    try {
        SQLite::Statement query(*m_db, R"(
            SELECT id, sender, receiver, path, type, timestamp
            FROM media 
            WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)
            ORDER BY timestamp DESC
        )");
        
        query.bind(1, user1);
        query.bind(2, user2);
        query.bind(3, user2);
        query.bind(4, user1);
        
        while (query.executeStep()) {
            Media m;
            m.id = query.getColumn(0);
            m.sender = query.getColumn(1);
            m.receiver = query.getColumn(2);
            m.path = query.getColumn(3);
            m.type = query.getColumn(4);
            m.timestamp = query.getColumn(5);
            
            media.push_back(m);
        }
    } catch (const SQLite::Exception& e) {
        std::cerr << "Failed to get media: " << e.what() << std::endl;
    }
    
    return media;
}

bool Database::userExists(const std::string& username) {
    try {
        SQLite::Statement query(*m_db, "SELECT COUNT(*) FROM users WHERE username = ?");
        query.bind(1, username);
        
        if (query.executeStep()) {
            return query.getColumn(0) > 0;
        }
    } catch (const SQLite::Exception& e) {
        std::cerr << "Failed to check user existence: " << e.what() << std::endl;
    }
    
    return false;
}

bool Database::createUser(const std::string& username) {
    try {
        SQLite::Statement query(*m_db, "INSERT INTO users (username) VALUES (?)");
        query.bind(1, username);
        query.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "Failed to create user: " << e.what() << std::endl;
        return false;
    }
} 