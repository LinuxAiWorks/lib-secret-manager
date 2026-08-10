cmake_minimum_required(VERSION 3.16)
project(lib-secret-manager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_compile_definitions(QT_NO_KEYWORDS)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)
find_package(Dtk6Widget REQUIRED)
find_package(Dtk6Core REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)

# --- Ищем системные переводы DTK ---
set(DTKWIDGET_NAMES
    dtkwidget_ru.qm
    dtkwidget_ru_RU.qm
    dtk6widget_ru.qm
)

set(DTK_SEARCH_PATHS
    /usr/share/dtk6/DWidget/translations
    /usr/share/dtk5/DWidget/translations
    /usr/share/dtkwidget/translations
    /usr/share/dtk6widget/translations
    /usr/share/dtk6/translations
    /usr/share/libdtkwidget/translations
    /usr/share/dtkcore/translations
    /usr/share/dtk6core/translations
    /usr/share/dtk6/DCore/translations
    /usr/share/dtk5/DCore/translations
)

set(DTKWIDGET_RU_QM "")
foreach(name ${DTKWIDGET_NAMES})
    if(NOT DTKWIDGET_RU_QM)
        find_file(TMP_${name} ${name}
            PATHS ${DTK_SEARCH_PATHS}
            NO_DEFAULT_PATH
        )
        if(TMP_${name})
            set(DTKWIDGET_RU_QM ${TMP_${name}})
        endif()
    endif()
endforeach()

set(DTKCORE_RU_QM "")
find_file(TMP_DTKCORE dtkcore_ru.qm
    PATHS ${DTK_SEARCH_PATHS}
    NO_DEFAULT_PATH
)
if(TMP_DTKCORE)
    set(DTKCORE_RU_QM ${TMP_DTKCORE})
endif()

# --- Копируем .qm в build dir (рядом с будущим .qrc) ---
set(DTK_QM_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(DTK_QM_FILES "")

if(DTKWIDGET_RU_QM)
    get_filename_component(DTKWIDGET_BNAME ${DTKWIDGET_RU_QM} NAME)
    configure_file(${DTKWIDGET_RU_QM} ${DTK_QM_DIR}/${DTKWIDGET_BNAME} COPYONLY)
    list(APPEND DTK_QM_FILES ${DTKWIDGET_BNAME})
    message(STATUS "DTK Widget translation FOUND: ${DTKWIDGET_RU_QM}")
else()
    message(WARNING "DTK Widget ru translation NOT FOUND! DTK labels will stay in English.")
endif()

if(DTKCORE_RU_QM)
    get_filename_component(DTKCORE_BNAME ${DTKCORE_RU_QM} NAME)
    configure_file(${DTKCORE_RU_QM} ${DTK_QM_DIR}/${DTKCORE_BNAME} COPYONLY)
    list(APPEND DTK_QM_FILES ${DTKCORE_BNAME})
    message(STATUS "DTK Core translation FOUND: ${DTKCORE_RU_QM}")
endif()

# --- Генерируем .qrc файл в build dir ---
if(DTK_QM_FILES)
    set(QRC_CONTENTS "<RCC>\n  <qresource prefix=\"/translations\">\n")
    foreach(f ${DTK_QM_FILES})
        string(APPEND QRC_CONTENTS "    <file>${f}</file>\n")
    endforeach()
    string(APPEND QRC_CONTENTS "  </qresource>\n</RCC>\n")
    
    set(DTK_QRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/dtk_translations.qrc")
    file(WRITE ${DTK_QRC_FILE} ${QRC_CONTENTS})
    
    message(STATUS "Generated DTK translations QRC: ${DTK_QRC_FILE}")
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/mainwindow.cpp
    src/secretworker.cpp
    src/secreteditdialog.cpp
    src/secretdetaildialog.cpp
    resources/resources.qrc
)

if(DTK_QM_FILES)
    target_sources(${PROJECT_NAME} PRIVATE ${DTK_QRC_FILE})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBSECRET_INCLUDE_DIRS}
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    Dtk6::Core
    Dtk6::Widget
    ${LIBSECRET_LIBRARIES}
)

# --- Установка ---
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

target_compile_options(${PROJECT_NAME} PRIVATE ${LIBSECRET_CFLAGS_OTHER})