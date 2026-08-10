/**
 * @file main.cpp
 * @brief Точка входа в приложение Lib-Secret Manager.
 * 
 * Отвечает за:
 * 1. Инициализацию Qt6 и DTK6 приложения.
 * 2. Настройку HiDPI (масштабирование на экранах с высоким разрешением).
 * 3. Загрузку переводов DTK (сначала из ресурсов, затем из системы).
 * 4. Настройку нативного окна "О программе" (DAboutDialog).
 * 5. Регистрацию кастомных типов для межпоточного взаимодействия (QueuedConnection).
 * 6. Запуск главного окна.
 */

#include <DApplication>       // Основной класс приложения DTK
#include <DMainWindow>        // Базовый класс главного окна (используется косвенно)
#include <DWidgetUtil>        // Утилиты DTK (например, moveToCenter)
#include <DAboutDialog>       // Нативный диалог "О программе" в стиле Deepin/UOS
#include <DGuiApplicationHelper> // Помощник для работы с темами и GUI

#include <QApplication>       // Базовый класс Qt приложения
#include <QIcon>              // Работа с иконками
#include <QTranslator>        // Загрузка .qm файлов переводов
#include <QLabel>             // Для поиска виджетов в DAboutDialog
#include <QAbstractButton>    // Для поиска кнопок в DAboutDialog
#include <QDir>               // Для перебора файлов в ресурсах
#include <QDebug>             // Для отладочного вывода (загрузка переводов)

#include "mainwindow.h"       // Главное окно приложения
#include "secretworker.h"     // Структура SecretItemData нужна для регистрации метатипа

// Макросы для использования пространств имен DTK без префиксов
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

/**
 * @brief Регистрация кастомных мета-типов Qt.
 * 
 * КРИТИЧНО ДЛЯ МНОГОПОТОЧНОСТИ:
 * Чтобы передавать QList<SecretItemData> через сигналы-слоты между разными потоками
 * (GUI поток <-> Worker поток) с использованием Qt::QueuedConnection,
 * Qt должен знать, как копировать этот тип. Без этой регистрации приложение
 * будет падать или выводить предупреждение "QObject::connect: Cannot queue arguments...".
 */
static void registerMetaTypes()
{
    qRegisterMetaType<QList<SecretItemData>>("QList<SecretItemData>");
}

/**
 * @brief Принудительный перевод внутренних меток DAboutDialog.
 * 
 * ЗАЧЕМ ЭТО НУЖНО:
 * DAboutDialog — это сложный композитный виджет DTK. Иногда системные переводы DTK
 * отсутствуют, неполны или не подхватываются автоматически для внутренних лейблов
 * ("Version", "License" и т.д.).
 * Эта функция служит "страховкой": она находит все QLabel и QPushButton внутри диалога
 * и заменяет английские тексты на русские, если они совпадают с картой перевода.
 * 
 * @param dlg Указатель на экземпляр DAboutDialog.
 */
static void translateAboutDialogLabels(DAboutDialog *dlg)
{
    if (!dlg) return;

    // Карта замены: Ключ = английский текст (как в исходниках DTK), Значение = русский перевод
    const QHash<QString, QString> labelMap = {
        {"Version",          "Версия"},
        {"Homepage",         "Домашняя страница"},
        {"Description",      "Описание"},
        {"Acknowledgements", "Благодарности"},
        {"License",          "Лицензия"},
    };

    // 1. Переводим все текстовые метки (QLabel)
    QList<QLabel*> labels = dlg->findChildren<QLabel*>();
    for (QLabel *lbl : labels) {
        QString txt = lbl->text();
        if (labelMap.contains(txt)) {
            lbl->setText(labelMap.value(txt));
        }
    }

    // 2. Переводим кнопки (например, кнопка лицензии или благодарностей, если они есть)
    QList<QAbstractButton*> buttons = dlg->findChildren<QAbstractButton*>();
    for (QAbstractButton *btn : buttons) {
        QString txt = btn->text();
        if (labelMap.contains(txt)) {
            btn->setText(labelMap.value(txt));
        }
    }
}

/**
 * @brief Главная функция приложения.
 */
int main(int argc, char *argv[])
{
    /* ---------- 1. НАСТРОЙКА HiDPI ---------- */
    // Для Qt6 политика масштабирования PassThrough позволяет DTK самому управлять
    // дробным масштабированием, что критично для экранов 2K/4K в Deepin/UOS.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#else
    // Fallback для Qt5 (на случай сборки под старые системы)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    /* ---------- 2. ИНИЦИАЛИЗАЦИЯ ПРИЛОЖЕНИЯ ---------- */
    DApplication app(argc, argv);

    // Метаданные приложения (используются в DAboutDialog, логах и менеджере задач)
    app.setOrganizationName("linuxaiworks");
    app.setApplicationName("lib-secret-manager");
    app.setApplicationVersion("1.0.3");
    
    // Иконки: загружаются из Qt Resource System (файл resources.qrc)
    app.setProductIcon(QIcon(":/lib-secret-manager.svg"));
    app.setWindowIcon(QIcon(":/lib-secret-manager.svg"));
    app.setProductName("Менеджер секретов");

    /* ---------- 3. ЗАГРУЗКА ПЕРЕВОДОВ DTK ---------- */
    // DTK имеет свои собственные строки (кнопки "OK", "Cancel", стандартные диалоги).
    // Нам нужно загрузить их переводы, чтобы интерфейс был полностью русским.
    
    // ВАЖНО: Трансляторы объявлены как static, чтобы они жили всё время работы программы.
    // Если они будут уничтожены в конце main(), перевод сломается.
    
    // --- A. Переводы DtkWidget (кнопки, инпуты, диалоги) ---
    static QTranslator dtkWidgetTr;
    bool dtkWidgetOk = false;

    // Стратегия 1: Ищем во встроенных ресурсах (:/translations).
    // Это гарантирует работу переводов даже в изолированном окружении (AppImage, Snap),
    // если CMakeLists.txt правильно собрал ресурсы.
    QDir resDir(":/translations");
    for (const QString &f : resDir.entryList(QStringList() << "*.qm", QDir::Files)) {
        // Фильтруем только файлы виджетов
        if (f.contains("dtkwidget") || f.contains("dtk6widget")) {
            if (dtkWidgetTr.load(":/translations/" + f)) {
                app.installTranslator(&dtkWidgetTr);
                dtkWidgetOk = true;
                qDebug() << "[i18n] Loaded DTK Widget translation from resources:" << f;
                break; // Нашли, выходим из цикла
            }
        }
    }

    // Стратегия 2: Fallback на системные пути.
    // Если в ресурсах пусто (например, при отладке без генерации qrc), ищем в /usr/share.
    if (!dtkWidgetOk) {
        const QStringList paths = {
            "/usr/share/dtk6/DWidget/translations", // Стандартный путь Deepin V23/UOS
            "/usr/share/dtk5/DWidget/translations", // Legacy
            "/usr/share/dtkwidget/translations",
            "/usr/share/dtk6widget/translations",
            "/usr/share/dtk6/translations",
            "/usr/share/libdtkwidget/translations",
        };
        for (const QString &p : paths) {
            // Пробуем разные варианты имен файлов (ru, ru_RU, dtk6widget_ru)
            if (dtkWidgetTr.load("dtkwidget_ru", p) ||
                dtkWidgetTr.load("dtkwidget_ru_RU", p) ||
                dtkWidgetTr.load("dtk6widget_ru", p)) {
                app.installTranslator(&dtkWidgetTr);
                dtkWidgetOk = true;
                qDebug() << "[i18n] Loaded DTK Widget translation from system:" << p;
                break;
            }
        }
    }

    // --- B. Переводы DtkCore (системные сообщения, утилиты) ---
    static QTranslator dtkCoreTr;
    bool dtkCoreOk = false;
    
    // Сначала ресурсы
    for (const QString &f : resDir.entryList(QStringList() << "*.qm", QDir::Files)) {
        if (f.contains("dtkcore") || f.contains("dtk6core")) {
            if (dtkCoreTr.load(":/translations/" + f)) {
                app.installTranslator(&dtkCoreTr);
                dtkCoreOk = true;
                qDebug() << "[i18n] Loaded DTK Core translation from resources:" << f;
                break;
            }
        }
    }
    
    // Затем система
    if (!dtkCoreOk) {
        const QStringList paths = {
            "/usr/share/dtkcore/translations",
            "/usr/share/dtk6core/translations",
            "/usr/share/dtk6/DCore/translations",
            "/usr/share/dtk5/DCore/translations",
            "/usr/share/dtk6/translations",
            "/usr/share/libdtkcore/translations",
        };
        for (const QString &p : paths) {
            if (dtkCoreTr.load("dtkcore_ru", p) ||
                dtkCoreTr.load("dtkcore_ru_RU", p) ||
                dtkCoreTr.load("dtk6core_ru", p)) {
                app.installTranslator(&dtkCoreTr);
                dtkCoreOk = true;
                qDebug() << "[i18n] Loaded DTK Core translation from system:" << p;
                break;
            }
        }
    }

    /* ---------- 4. НАСТРОЙКА ОКНА «О ПРОГРАММЕ» ---------- */
    // Создаем статический экземпляр диалога. Static нужен, так как DApplication
    // хранит указатель на него и может обращаться к нему в любой момент (например, при клике в меню).
    static DAboutDialog aboutDialog;
    
    aboutDialog.setProductIcon(QIcon(":/lib-secret-manager.svg"));
    aboutDialog.setProductName("Менеджер секретов");
    aboutDialog.setVersion("1.0.3");
    aboutDialog.setDescription(
        "Простой и безопасный менеджер паролей для Linux.\n"
        "Управляйте секретами, хранящимися в системном хранилище ключей через libsecret.");
    
    // Ссылки на проект
    aboutDialog.setWebsiteName("linuxaiworks");
    aboutDialog.setWebsiteLink("https://github.com/linuxaiworks/lib-secret-manager");
    aboutDialog.setLicense("GPLv3"); // Лицензия

    // Регистрируем диалог в приложении (теперь он доступен через меню帮助 -> О программе)
    app.setAboutDialog(&aboutDialog);
    
    // Страница благодарностей (опционально)
    app.setApplicationAcknowledgementPage(
        "https://github.com/linuxaiworks/lib-secret-manager/blob/main/AUTHORS.md");

    // Описание для тултипов и системных меню
    app.setApplicationDisplayName("Менеджер секретов");
    app.setApplicationDescription(
        "<span style='font-size:8pt; font-weight:600;'>Простой и безопасный менеджер паролей для Linux.</span><br/>"
        "Управляйте секретами, хранящимися в системном хранилище ключей через libsecret.<br/>"
        "<a href='https://github.com/linuxaiworks/lib-secret-manager'>https://github.com/linuxaiworks/lib-secret-manager</a>");

    // Применяем наш "костыль" для гарантированного перевода лейблов внутри диалога
    translateAboutDialogLabels(&aboutDialog);

    /* ---------- 5. РЕГИСТРАЦИЯ ТИПОВ И ЗАПУСК ---------- */
    
    // Регистрируем типы ДО создания MainWindow, чтобы воркер мог сразу отправлять сигналы
    registerMetaTypes();

    // Создаем и показываем главное окно
    MainWindow w;
    
    // Минимальный размер: 1200x700 выбрано для комфортного отображения таблицы с длинными метками.
    // Можно уменьшить до 900x600 для совместимости с маленькими ноутбуками.
    w.setMinimumSize(900, 600);
    w.show();

    // Центрируем окно на экране (утилита DTK)
    Dtk::Widget::moveToCenter(&w);

    // Запуск основного цикла событий Qt
    return app.exec();
}