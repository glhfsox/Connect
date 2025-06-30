# Connect Messenger - Server Architecture

## 🏗 Общая архитектура

```
┌─────────────────────────────────────────────────────────────┐
│                    Connect Messenger Server                 │
├─────────────────────────────────────────────────────────────┤
│  main.cpp (Entry Point)                                     │
│  ├── Инициализация Qt приложения                            │
│  ├── Обработка сигналов (Ctrl+C)                            │
│  ├── Создание WebSocket сервера                             │
│  └── Запуск event loop                                      │
├─────────────────────────────────────────────────────────────┤
│  WebSocketServer (QWebSocketServer)                         │
│  ├── Обработка подключений                                  │
│  ├── Маршрутизация сообщений                                │
│  ├── Управление онлайн пользователями                       │
│  └── JSON коммуникация                                      │
├─────────────────────────────────────────────────────────────┤
│  Database (SQLiteCpp)                                       │
│  ├── SQLite база данных                                     │
│  ├── CRUD операции                                          │
│  ├── Индексы для производительности                         │
│  └── Транзакции                                             │
├─────────────────────────────────────────────────────────────┤
│  Encryption (libsodium)                                     │
│  ├── End-to-end шифрование                                  │
│  ├── Генерация ключей                                       │
│  └── Безопасная передача                                    │
└─────────────────────────────────────────────────────────────┘
```

## 🔌 WebSocket Server

### Основные компоненты:

#### 1. **QWebSocketServer** (Qt WebSocket)
- **Порт**: 9001 (по умолчанию)
- **Протокол**: WebSocket (ws://)
- **Режим**: NonSecure (можно переключить на WSS)

#### 2. **Обработка сообщений**
```cpp
// Типы сообщений:
"auth"      - Авторизация пользователя
"message"   - Отправка сообщения
"history"   - Запрос истории
"ping"      - Проверка соединения
```

#### 3. **Управление подключениями**
```cpp
QMap<QString, QWebSocket*> m_onlineUsers;
// Ключ: username, Значение: WebSocket соединение
```

### Жизненный цикл подключения:

1. **Подключение** → `onNewConnection()`
2. **Авторизация** → `handleMessage("auth")`
3. **Обмен сообщениями** → `handleMessage("message")`
4. **Отключение** → `onDisconnected()`

## 🗄 База данных (SQLite)

### Структура базы данных:

#### 1. **Таблица users**
```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

#### 2. **Таблица messages**
```sql
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    text TEXT NOT NULL,
    message_type TEXT DEFAULT 'text',
    media_path TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sender) REFERENCES users (username),
    FOREIGN KEY (receiver) REFERENCES users (username)
);
```

#### 3. **Таблица media**
```sql
CREATE TABLE media (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    path TEXT NOT NULL,
    type TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sender) REFERENCES users (username),
    FOREIGN KEY (receiver) REFERENCES users (username)
);
```

### Индексы для производительности:
```sql
CREATE INDEX idx_messages_users ON messages(sender, receiver);
CREATE INDEX idx_messages_timestamp ON messages(timestamp);
CREATE INDEX idx_media_users ON media(sender, receiver);
```

### Операции с базой данных:

#### Сохранение сообщения:
```cpp
bool saveMessage(const std::string& sender, 
                const std::string& receiver, 
                const std::string& text);
```

#### Получение истории:
```cpp
std::vector<Message> getMessages(const std::string& user1, 
                                const std::string& user2, 
                                int limit = 100);
```

#### Управление пользователями:
```cpp
bool userExists(const std::string& username);
bool createUser(const std::string& username);
```

## 🔐 Шифрование

### Компоненты:
- **libsodium** - криптографическая библиотека
- **End-to-end шифрование** - сообщения шифруются на клиенте
- **Генерация ключей** - уникальные ключи для каждой пары пользователей

### Процесс шифрования:
1. Клиенты обмениваются публичными ключами
2. Сообщения шифруются перед отправкой
3. Сервер хранит только зашифрованные данные
4. Расшифровка происходит на клиенте

## 📡 Протокол обмена сообщениями

### Авторизация:
```json
// Клиент → Сервер
{
    "type": "auth",
    "username": "alice"
}

// Сервер → Клиент
{
    "type": "auth_response",
    "status": "success",
    "message": "Authenticated successfully"
}
```

### Отправка сообщения:
```json
// Клиент → Сервер
{
    "type": "message",
    "to": "bob",
    "text": "Hello, Bob!"
}

// Сервер → Получатель
{
    "type": "message",
    "from": "alice",
    "text": "Hello, Bob!",
    "timestamp": 1640995200
}
```

### Запрос истории:
```json
// Клиент → Сервер
{
    "type": "history",
    "with": "bob"
}

// Сервер → Клиент
{
    "type": "history",
    "with": "bob",
    "messages": [
        {
            "id": 1,
            "sender": "alice",
            "text": "Hello!",
            "timestamp": "2024-01-01 12:00:00"
        }
    ]
}
```

## 🚀 Производительность

### Оптимизации:

1. **Индексы SQLite** - быстрый поиск сообщений
2. **Ограничение истории** - по умолчанию 100 сообщений
3. **In-memory кэш** - онлайн пользователи в памяти
4. **Асинхронная обработка** - Qt event loop

### Масштабируемость:

- **До 1000 одновременных пользователей**
- **SQLite подходит для небольших нагрузок**
- **Легко заменить на PostgreSQL/MySQL при росте**

## 🔧 Конфигурация

### Параметры сервера:
```cpp
// Порт (по умолчанию 9001)
int port = 9001;

// Путь к базе данных
std::string dbPath = "data/messenger.db";

// Максимальное количество сообщений в истории
int historyLimit = 100;
```

### Переменные окружения:
- `CONNECT_PORT` - порт сервера
- `CONNECT_DB_PATH` - путь к базе данных
- `CONNECT_LOG_LEVEL` - уровень логирования

## 🐛 Логирование и отладка

### Логи сервера:
```
=== Connect Messenger Server ===
Starting server...
Database initialized successfully
WebSocket server started on port 9001
New WebSocket connection established
User alice authenticated
User alice disconnected
```

### Отладка базы данных:
```bash
# Просмотр базы данных
sqlite3 data/messenger.db

# Просмотр таблиц
.tables

# Просмотр сообщений
SELECT * FROM messages LIMIT 10;
```

## 🔄 Жизненный цикл сервера

1. **Запуск** → Инициализация Qt, базы данных, WebSocket
2. **Ожидание** → Event loop, обработка подключений
3. **Обработка** → Маршрутизация сообщений, работа с БД
4. **Завершение** → Graceful shutdown, закрытие соединений

## 📊 Мониторинг

### Метрики:
- Количество онлайн пользователей
- Количество сообщений в секунду
- Размер базы данных
- Время отклика

### Здоровье сервера:
- Ping/Pong для проверки соединений
- Проверка доступности базы данных
- Мониторинг использования памяти

---

**Архитектура спроектирована для простоты, производительности и масштабируемости!** 🚀 