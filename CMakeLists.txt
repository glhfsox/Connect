cmake_minimum_required(VERSION 3.10)
project(Connect)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

# Поиск пакетов
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets Multimedia)
find_package(sodium CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# uWebSockets (может потребоваться ручная настройка)
find_path(UWEBSOCKETS_INCLUDE_DIR uWebSockets/App.h)
if(NOT UWEBSOCKETS_INCLUDE_DIR)
    message(FATAL_ERROR "uWebSockets not found. Please install it via vcpkg or manually.")
endif()

# Исходные файлы сервера
set(SERVER_SOURCES
    main.cpp
    server/WebSocketServer.cpp
    server/Database.cpp
    server/Encryption.cpp
)

# Исходные файлы клиента (если есть)
set(CLIENT_SOURCES
    Connect.cpp
    Messenger.cpp
)

# Создание исполняемого файла сервера
add_executable(ConnectServer ${SERVER_SOURCES})

target_include_directories(ConnectServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${UWEBSOCKETS_INCLUDE_DIR}
)

target_link_libraries(ConnectServer PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    sodium::sodium
    nlohmann_json::nlohmann_json
    SQLiteCpp::SQLiteCpp
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Создание исполняемого файла клиента (если нужно)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Connect.cpp)
    add_executable(ConnectClient ${CLIENT_SOURCES})
    
    target_include_directories(ConnectClient PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    target_link_libraries(ConnectClient PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::WebSockets
        Qt6::Multimedia
        sodium::sodium
    )
endif()

# Копирование сертификатов (если есть)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/certs)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/certs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Создание директорий для данных
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/media/uploads)