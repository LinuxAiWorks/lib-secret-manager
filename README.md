<div align="center">

![Logo](resources/lib-secret-manager.svg)

# 🔐 Lib-Secret Manager

**Простой и безопасный менеджер паролей для Linux**

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Qt6](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io)
[![DTK6](https://img.shields.io/badge/DTK-6-orange.svg)](https://github.com/linuxdeepin/dtk)

</div>

---

## 📋 Описание

**Lib-Secret Manager** — графический менеджер секретов для Linux, работающий через системное хранилище ключей [**libsecret**](https://wiki.gnome.org/Projects/Libsecret) (GNOME Keyring, KWallet, KeePassXC и др.).

Программа написана на **Qt6 / DTK6** и полностью русифицирована.

### Возможности

- 🔐 Просмотр всех секретов из системного хранилища
- ➕ Добавление новых записей (метка, имя пользователя, сервис, пароль)
- 👁️ Просмотр деталей по двойному клику с копированием пароля
- 🗑️ Удаление с подтверждением
- 🖱️ Контекстное меню: добавить / удалить / обновить
- 🇷🇺 Полностью русскоязычный интерфейс
- 🎨 Нативная интеграция с **Deepin Tool Kit (DTK6)**
- 🌗 Поддержка тёмной и светлой тем

---

## 📦 Установка

### Из .deb (рекомендуется)

```bash
wget https://github.com/linuxaiworks/lib-secret-manager/releases/download/v1.0.0/lib-secret-manager_1.0.0-1_amd64.deb
sudo dpkg -i lib-secret-manager_1.0.0-1_amd64.deb
sudo apt-get install -f
```

### Из исходников

**Зависимости для сборки:**

```bash
sudo apt install build-essential cmake qt6-base-dev libqt6svg6-dev     libdtk6widget-dev libdtk6core-dev libsecret-1-dev libglib2.0-dev pkg-config
```

**Сборка и установка:**

```bash
git clone https://github.com/linuxaiworks/lib-secret-manager.git
cd lib-secret-manager
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

**Удаление (если ставил через `make install`):**

```bash
sudo make uninstall
```

---

## 🏗️ Сборка .deb пакета

```bash
cd lib-secret-manager
sudo apt install debhelper fakeroot devscripts
dpkg-buildpackage -us -uc -b
```

Готовый пакет появится в родительской директории:
```
../lib-secret-manager_1.0.0-1_amd64.deb
```

---

## 🖥️ Системные требования

| Компонент | Минимум |
|-----------|---------|
| ОС | Linux (Deepin / UOS / Debian / Ubuntu) |
| Qt | 6.x |
| DTK | 6.x |
| libsecret | 0.20+ |

---

## 📁 Структура проекта

```
lib-secret-manager/
├── CMakeLists.txt          # Система сборки CMake
├── src/
│   ├── main.cpp              # Точка входа
│   ├── mainwindow.cpp/h      # Главное окно
│   ├── secretworker.cpp/h    # Работа с libsecret (в потоке)
│   ├── secreteditdialog.cpp/h # Диалог добавления
│   └── secretdetaildialog.cpp/h # Диалог просмотра
├── resources/
│   ├── resources.qrc         # Ресурсы Qt
│   └── lib-secret-manager.svg # Иконка приложения
├── debian/                   # Файлы для сборки .deb
│   ├── control
│   ├── rules
│   ├── changelog
│   └── ...
├── CHANGELOG.md
├── README.md
└── LICENSE
```

---

## 🤝 Участие в проекте

Pull requests приветствуются! Для крупных изменений, пожалуйста, сначала откройте issue для обсуждения.

---

## 📄 Лицензия

Распространяется под лицензией **GPL-3.0**. См. файл [LICENSE](LICENSE).

---

<div align="center">

Сделано с ❤️ для Linux-сообщества

**[⬆ Наверх](#-lib-secret-manager)**

</div>
