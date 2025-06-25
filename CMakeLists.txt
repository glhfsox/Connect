cmake_minimum_required(VERSION 3.14)
project("Connect")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

add_executable(Connect
    Connect.cpp
    Connect.h
    Messenger.cpp
    Messenger.h
)

target_include_directories(Connect PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(Connect PRIVATE Qt5::Widgets Qt5::Network ${SODIUM_LIBRARIES})

set_target_properties(Connect PROPERTIES CXX_STANDARD 20)
