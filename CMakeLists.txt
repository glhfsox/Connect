cmake_minimum_required(VERSION 3.10)
project(Connect)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to find Qt6 first, then fall back to Qt5
find_package(Qt6 QUIET COMPONENTS Core Network WebSockets)
if(Qt6_FOUND)
    find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Network WebSockets)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
endif()

# Try to find other dependencies
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBSODIUM libsodium)
    pkg_check_modules(SQLITE3 sqlite3)
endif()

# Fallback for libsodium if not found via pkg-config
if(NOT LIBSODIUM_FOUND)
    find_library(LIBSODIUM_LIBRARIES NAMES sodium libsodium)
    find_path(LIBSODIUM_INCLUDE_DIRS NAMES sodium.h)
    if(LIBSODIUM_LIBRARIES AND LIBSODIUM_INCLUDE_DIRS)
        set(LIBSODIUM_FOUND TRUE)
        message(STATUS "Found libsodium: ${LIBSODIUM_LIBRARIES}")
    endif()
endif()

# Fallback for SQLite3 if not found via pkg-config
if(NOT SQLITE3_FOUND)
    find_library(SQLITE3_LIBRARIES NAMES sqlite3)
    find_path(SQLITE3_INCLUDE_DIRS NAMES sqlite3.h)
    if(SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIRS)
        set(SQLITE3_FOUND TRUE)
        message(STATUS "Found SQLite3: ${SQLITE3_LIBRARIES}")
    endif()
endif()

# Try to find OpenSSL
find_package(OpenSSL QUIET)

# Исходные файлы сервера
set(SERVER_SOURCES
    main.cpp
    server/WebSocketServer.cpp
    server/Database.cpp
    server/Encryption.cpp
)

# Создание исполняемого файла сервера
add_executable(ConnectServer ${SERVER_SOURCES})

target_include_directories(ConnectServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBSODIUM_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
)

# Link libraries based on Qt version
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(ConnectServer PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
    )
else()
    target_link_libraries(ConnectServer PRIVATE
        Qt5::Core
        Qt5::Network
        Qt5::WebSockets
    )
endif()

# Link other libraries if found
if(LIBSODIUM_FOUND)
    target_link_libraries(ConnectServer PRIVATE ${LIBSODIUM_LIBRARIES})
    target_compile_definitions(ConnectServer PRIVATE HAVE_LIBSODIUM)
endif()

if(SQLITE3_FOUND)
    target_link_libraries(ConnectServer PRIVATE ${SQLITE3_LIBRARIES})
else()
    # Use built-in SQLite if external not found
    message(WARNING "SQLite3 not found, using built-in version")
endif()

if(OpenSSL_FOUND)
    target_link_libraries(ConnectServer PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
    )
endif()

# Создание директорий для данных
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/media/uploads)

# Set RPATH for Linux
if(UNIX AND NOT APPLE)
    set_target_properties(ConnectServer PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(ConnectServer PRIVATE -Wall -Wextra)
endif()

# Enable debug symbols in release mode for better crash reports
set_target_properties(ConnectServer PROPERTIES
    COMPILE_FLAGS_RELEASE "-O2 -g"
)