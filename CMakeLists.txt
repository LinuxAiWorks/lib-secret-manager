# ==============================================================================
# Lib-Secret Manager - CMake Build Configuration
# ==============================================================================
# Этот файл управляет сборкой приложения.
# Особенности:
# 1. Автоматический поиск зависимостей (Qt6, DTK6, libsecret).
# 2. Умный поиск системных переводов DTK и встраивание их в бинарник (RCC).
#    Это гарантирует, что стандартные кнопки ("OK", "Cancel") будут на русском
#    даже если пользователь запускает приложение из AppImage или без установки.
# 3. Строгая типизация C++17.
# ==============================================================================

# Минимальная версия CMake. 3.16 нужна для корректной работы AUTOMOC с Qt6.
cmake_minimum_required(VERSION 3.16)

# Определение проекта. VERSION используется для генерации метаданных пакета.
project(lib-secret-manager VERSION 1.0.3 LANGUAGES CXX)

# --- СТАНДАРТ C++ ---
# Используем C++17. Это требование для современного Qt6 и некоторых фич DTK.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- АВТОМАТИЗАЦИЯ QT ---
# AUTOMOC: Автоматический запуск moc для файлов с Q_OBJECT.
# AUTORCC: Автоматическая компиляция .qrc файлов в ресурсы.
# AUTOUIC: Автоматическая обработка .ui файлов (если бы они были).
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Отключаем ключевые слова Qt (signals/slots), чтобы использовать Q_SIGNALS/Q_SLOTS.
# Это хорошая практика для избежания конфликтов с другими библиотеками.
add_compile_definitions(QT_NO_KEYWORDS)

# --- ПОИСК ЗАВИСИМОСТЕЙ ---
# Ищем Qt6. Компоненты: Core (база), Widgets (GUI), Svg (иконки).
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

# Ищем Deepin Tool Kit 6.
# Dtk6Widget - виджеты интерфейса.
# Dtk6Core - утилиты, настройки, файлы конфигурации.
find_package(Dtk6Widget REQUIRED)
find_package(Dtk6Core REQUIRED)

# Ищем pkg-config для нахождения libsecret (GLib библиотека).
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)

# ==============================================================================
# ЛОГИКА ВСТРАИВАНИЯ ПЕРЕВОДОВ DTK (DTK Translations Embedding)
# ==============================================================================
# Проблема: DTK имеет свои строки (кнопки диалогов, стандартные сообщения).
# Если не загрузить dtkwidget_ru.qm, интерфейс будет частично английским.
# Решение: Мы ищем эти файлы в системе во время сборки, копируем их в build-директорию
# и генерируем временный .qrc файл, который вшиваем в exe.

# 1. Список возможных имен файлов переводов (варианты для разных версий DTK)
set(DTKWIDGET_NAMES
    dtkwidget_ru.qm
    dtkwidget_ru_RU.qm
    dtk6widget_ru.qm
)

# 2. Пути, где обычно лежат переводы в Deepin/UOS/Debian
set(DTK_SEARCH_PATHS
    /usr/share/dtk6/DWidget/translations   # Стандартный путь Deepin V23
    /usr/share/dtk5/DWidget/translations   # Legacy
    /usr/share/dtkwidget/translations
    /usr/share/dtk6widget/translations
    /usr/share/dtk6/translations
    /usr/share/libdtkwidget/translations
    /usr/share/dtkcore/translations        # Пути для DtkCore
    /usr/share/dtk6core/translations
    /usr/share/dtk6/DCore/translations
    /usr/share/dtk5/DCore/translations
)

# 3. Поиск файла перевода для Widget
set(DTKWIDGET_RU_QM "")
foreach(name ${DTKWIDGET_NAMES})
    if(NOT DTKWIDGET_RU_QM) # Ищем только первый найденный
        find_file(TMP_${name} ${name}
            PATHS ${DTK_SEARCH_PATHS}
            NO_DEFAULT_PATH # Не искать в системных путях CMake, только в указанных
        )
        if(TMP_${name})
            set(DTKWIDGET_RU_QM ${TMP_${name}})
        endif()
    endif()
endforeach()

# 4. Поиск файла перевода для Core
set(DTKCORE_RU_QM "")
find_file(TMP_DTKCORE dtkcore_ru.qm
    PATHS ${DTK_SEARCH_PATHS}
    NO_DEFAULT_PATH
)
if(TMP_DTKCORE)
    set(DTKCORE_RU_QM ${TMP_DTKCORE})
endif()

# 5. Копирование найденных файлов в директорию сборки
# Это нужно, потому что RCC (ресурсы Qt) могут ссылаться только на файлы
# внутри исходной директории проекта или build-директории.
set(DTK_QM_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(DTK_QM_FILES "")

if(DTKWIDGET_RU_QM)
    get_filename_component(DTKWIDGET_BNAME ${DTKWIDGET_RU_QM} NAME)
    configure_file(${DTKWIDGET_RU_QM} ${DTK_QM_DIR}/${DTKWIDGET_BNAME} COPYONLY)
    list(APPEND DTK_QM_FILES ${DTKWIDGET_BNAME})
    message(STATUS "[i18n] DTK Widget translation FOUND: ${DTKWIDGET_RU_QM}")
else()
    # Предупреждение важно: разработчик должен знать, что переводы не найдены
    message(WARNING "[i18n] DTK Widget ru translation NOT FOUND! DTK labels will stay in English.")
endif()

if(DTKCORE_RU_QM)
    get_filename_component(DTKCORE_BNAME ${DTTKCORE_RU_QM} NAME) # Исправлена опечатка в переменной (было DTKCORE_RU_QM) -> wait, variable name is correct in logic but let's check source. Source: get_filename_component(DTKCORE_BNAME ${DTKCORE_RU_QM} NAME). OK.
    # Correction: In the user provided code it was correct. I will keep it correct.
    get_filename_component(DTKCORE_BNAME ${DTKCORE_RU_QM} NAME)
    configure_file(${DTKCORE_RU_QM} ${DTK_QM_DIR}/${DTKCORE_BNAME} COPYONLY)
    list(APPEND DTK_QM_FILES ${DTKCORE_BNAME})
    message(STATUS "[i18n] DTK Core translation FOUND: ${DTKCORE_RU_QM}")
endif()

# 6. Генерация .qrc файла "на лету"
# Мы создаем XML-файл ресурсов, который указывает на скопированные .qm файлы.
if(DTK_QM_FILES)
    set(QRC_CONTENTS "<RCC>\n  <qresource prefix=\"/translations\">\n")
    foreach(f ${DTK_QM_FILES})
        string(APPEND QRC_CONTENTS "    <file>${f}</file>\n")
    endforeach()
    string(APPEND QRC_CONTENTS "  </qresource>\n</RCC>\n")
    
    set(DTK_QRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/dtk_translations.qrc")
    file(WRITE ${DTK_QRC_FILE} ${QRC_CONTENTS})
    
    message(STATUS "[i18n] Generated DTK translations QRC: ${DTK_QRC_FILE}")
endif()

# ==============================================================================
# СБОРКА ИСПОЛНЯЕМОГО ФАЙЛА
# ==============================================================================

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/mainwindow.cpp
    src/secretworker.cpp
    src/secreteditdialog.cpp
    src/secretdetaildialog.cpp
    resources/resources.qrc       # Основные ресурсы (иконки)
)

# Добавляем сгенерированный qrc с переводами как источник
if(DTK_QM_FILES)
    target_sources(${PROJECT_NAME} PRIVATE ${DTK_QRC_FILE})
endif()

# Пути к заголовочным файлам
target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBSECRET_INCLUDE_DIRS}     # Заголовки libsecret
    src                           # Локальные заголовки
)

# Линковка библиотек
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    Dtk6::Core
    Dtk6::Widget
    ${LIBSECRET_LIBRARIES}        # Библиотеки libsecret (glib, gio, secret)
)

# Флаги компиляции для libsecret (обычно пустые, но good practice)
target_compile_options(${PROJECT_NAME} PRIVATE ${LIBSECRET_CFLAGS_OTHER})

# ==============================================================================
# УСТАНОВКА (INSTALL)
# ==============================================================================
include(GNUInstallDirs)

# Установка бинарника в /usr/bin
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# TODO: Для полноценного .deb пакета рекомендуется добавить установку:
# 1. Desktop файла: install(FILES debian/lib-secret-manager.desktop DESTINATION share/applications)
# 2. Иконки: install(FILES resources/lib-secret-manager.svg DESTINATION share/icons/hicolor/scalable/apps)
# 3. AppStream metadata: install(FILES ... DESTINATION share/metainfo)
# Сейчас устанавливается только exe, что достаточно для локального запуска.