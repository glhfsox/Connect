# Установка Docker для тестирования

## 🐳 Установка Docker Desktop

1. **Скачайте Docker Desktop для Windows:**
   - Перейдите на https://www.docker.com/products/docker-desktop/
   - Нажмите "Download for Windows"

2. **Установите Docker Desktop:**
   - Запустите скачанный файл
   - Следуйте инструкциям установщика
   - Перезагрузите компьютер при необходимости

3. **Проверьте установку:**
   ```cmd
   docker --version
   ```

4. **Запустите тест:**
   ```cmd
   test_build.bat
   ```

## 🚀 Альтернатива: Онлайн тестирование

Если не хотите устанавливать Docker, можете протестировать на Railway напрямую:

1. Загрузите код в GitHub
2. Подключите репозиторий к Railway
3. Railway автоматически соберет и запустит приложение

## 🔧 Ручная сборка (требует установки Qt)

Если хотите собрать локально без Docker:

1. Установите Visual Studio 2019/2022
2. Установите Qt6: https://www.qt.io/download
3. Установите vcpkg: https://github.com/microsoft/vcpkg
4. Соберите проект:
   ```cmd
   cmake -B build
   cmake --build build --config Release
   ``` 