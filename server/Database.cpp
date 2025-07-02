#include "../include/Database.h"
#include <iostream>
#include <filesystem>
#include <sqlite3.h>

Database::Database(const std::string& dbPath) : m_dbPath(dbPath), m_db(nullptr) {
}

Database::~Database() {
    if (m_db) {
        sqlite3_close(m_db);
    }
}

bool Database::initialize() {
    try {
        // Create data directory if it doesn't exist
        std::filesystem::path dbDir = std::filesystem::path(m_dbPath).parent_path();
        if (!dbDir.empty() && !std::filesystem::exists(dbDir)) {
            std::filesystem::create_directories(dbDir);
        }

        // Open database
        int rc = sqlite3_open(m_dbPath.c_str(), &m_db);
        if (rc != SQLITE_OK) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(m_db) << std::endl;
            return false;
        }

        createTables();
        std::cout << "Database initialized: " << m_dbPath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database initialization error: " << e.what() << std::endl;
        return false;
    }
}

void Database::createTables() {
    const char* sql_users = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char* sql_messages = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            receiver TEXT NOT NULL,
            text TEXT NOT NULL,
            message_type TEXT DEFAULT 'text',
            media_path TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char* sql_media = R"(
        CREATE TABLE IF NOT EXISTS media (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            receiver TEXT NOT NULL,
            path TEXT NOT NULL,
            type TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errMsg = 0;
    
    if (sqlite3_exec(m_db, sql_users, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error creating users table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    if (sqlite3_exec(m_db, sql_messages, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error creating messages table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    if (sqlite3_exec(m_db, sql_media, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error creating media table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool Database::saveMessage(const std::string& sender, const std::string& receiver, 
                          const std::string& text, const std::string& messageType,
                          const std::string& mediaPath) {
    const char* sql = R"(
        INSERT INTO messages (sender, receiver, text, message_type, media_path)
        VALUES (?, ?, ?, ?, ?);
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, messageType.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, mediaPath.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert message: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    return true;
}

std::vector<Message> Database::getMessages(const std::string& user1, const std::string& user2, int limit) {
    std::vector<Message> messages;
    
    const char* sql = R"(
        SELECT id, sender, receiver, text, timestamp, message_type, media_path
        FROM messages 
        WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)
        ORDER BY timestamp DESC 
        LIMIT ?;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return messages;
    }
    
    sqlite3_bind_text(stmt, 1, user1.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user2.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user2.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user1.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message msg;
        msg.id = sqlite3_column_int(stmt, 0);
        msg.sender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        msg.receiver = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        msg.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        msg.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        msg.messageType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        msg.mediaPath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

bool Database::saveMedia(const std::string& sender, const std::string& receiver,
                        const std::string& path, const std::string& type) {
    const char* sql = R"(
        INSERT INTO media (sender, receiver, path, type)
        VALUES (?, ?, ?, ?);
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, type.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert media: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    return true;
}

std::vector<Media> Database::getMedia(const std::string& user1, const std::string& user2) {
    std::vector<Media> media;
    
    const char* sql = R"(
        SELECT id, sender, receiver, path, type, timestamp
        FROM media 
        WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)
        ORDER BY timestamp DESC;
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return media;
    }
    
    sqlite3_bind_text(stmt, 1, user1.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user2.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user2.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user1.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Media m;
        m.id = sqlite3_column_int(stmt, 0);
        m.sender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        m.receiver = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        m.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        m.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        m.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        media.push_back(m);
    }
    
    sqlite3_finalize(stmt);
    return media;
}

bool Database::userExists(const std::string& username) {
    const char* sql = "SELECT COUNT(*) FROM users WHERE username = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

bool Database::createUser(const std::string& username) {
    const char* sql = "INSERT INTO users (username) VALUES (?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to create user: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    return true;
} 