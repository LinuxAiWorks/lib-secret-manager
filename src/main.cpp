#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <DGuiApplicationHelper>

#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLabel>
#include <QAbstractButton>
#include <QDir>

#include "mainwindow.h"
#include "secretworker.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static void registerMetaTypes()
{
    qRegisterMetaType<QList<SecretItemData>>("QList<SecretItemData>");
}

/* ---------- Явный перевод внутренних меток DAboutDialog ---------- */
static void translateAboutDialogLabels(DAboutDialog *dlg)
{
    if (!dlg) return;

    const QHash<QString, QString> labelMap = {
        {"Version",          "Версия"},
        {"Homepage",         "Домашняя страница"},
        {"Description",      "Описание"},
        {"Acknowledgements", "Благодарности"},
        {"License",          "Лицензия"},
    };

    QList<QLabel*> labels = dlg->findChildren<QLabel*>();
    for (QLabel *lbl : labels) {
        QString txt = lbl->text();
        if (labelMap.contains(txt)) {
            lbl->setText(labelMap.value(txt));
        }
    }

    QList<QAbstractButton*> buttons = dlg->findChildren<QAbstractButton*>();
    for (QAbstractButton *btn : buttons) {
        QString txt = btn->text();
        if (labelMap.contains(txt)) {
            btn->setText(labelMap.value(txt));
        }
    }
}

int main(int argc, char *argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#else
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    DApplication app(argc, argv);

    app.setOrganizationName("linuxaiworks");
    app.setApplicationName("lib-secret-manager");
    app.setApplicationVersion("1.0.0");
    app.setProductIcon(QIcon(":/lib-secret-manager.svg"));
    app.setWindowIcon(QIcon(":/lib-secret-manager.svg"));
    app.setProductName("Менеджер секретов");

    /* ---------- Системные переводы DTK ---------- */
    static QTranslator dtkWidgetTr;
    bool dtkWidgetOk = false;

    // Сначала пробуем из встроенных ресурсов
    QDir resDir(":/translations");
    for (const QString &f : resDir.entryList(QStringList() << "*.qm", QDir::Files)) {
        if (f.contains("dtkwidget") || f.contains("dtk6widget")) {
            if (dtkWidgetTr.load(":/translations/" + f)) {
                app.installTranslator(&dtkWidgetTr);
                dtkWidgetOk = true;
                qDebug() << "Loaded DTK Widget translation from resources:" << f;
                break;
            }
        }
    }

    // Fallback на системные пути
    if (!dtkWidgetOk) {
        const QStringList paths = {
            "/usr/share/dtk6/DWidget/translations",
            "/usr/share/dtk5/DWidget/translations",
            "/usr/share/dtkwidget/translations",
            "/usr/share/dtk6widget/translations",
            "/usr/share/dtk6/translations",
            "/usr/share/libdtkwidget/translations",
        };
        for (const QString &p : paths) {
            if (dtkWidgetTr.load("dtkwidget_ru", p) ||
                dtkWidgetTr.load("dtkwidget_ru_RU", p) ||
                dtkWidgetTr.load("dtk6widget_ru", p)) {
                app.installTranslator(&dtkWidgetTr);
                dtkWidgetOk = true;
                qDebug() << "Loaded DTK Widget translation from:" << p;
                break;
            }
        }
    }

    static QTranslator dtkCoreTr;
    bool dtkCoreOk = false;
    for (const QString &f : resDir.entryList(QStringList() << "*.qm", QDir::Files)) {
        if (f.contains("dtkcore") || f.contains("dtk6core")) {
            if (dtkCoreTr.load(":/translations/" + f)) {
                app.installTranslator(&dtkCoreTr);
                dtkCoreOk = true;
                qDebug() << "Loaded DTK Core translation from resources:" << f;
                break;
            }
        }
    }
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
                qDebug() << "Loaded DTK Core translation from:" << p;
                break;
            }
        }
    }

    /* ---------- НАТИВНОЕ ОКНО «О ПРОГРАММЕ» ---------- */
    static DAboutDialog aboutDialog;
    aboutDialog.setProductIcon(QIcon(":/lib-secret-manager.svg"));
    aboutDialog.setProductName("Менеджер секретов");
    aboutDialog.setVersion("1.0.0");
    aboutDialog.setDescription(
        "Простой и безопасный менеджер паролей для Linux.\n"
        "Управляйте секретами, хранящимися в системном хранилище ключей через libsecret.");
    aboutDialog.setWebsiteName("linuxaiworks");
    aboutDialog.setWebsiteLink("https://github.com/linuxaiworks/lib-secret-manager");
    aboutDialog.setLicense("GPLv3");

    app.setAboutDialog(&aboutDialog);
    app.setApplicationAcknowledgementPage(
        "https://github.com/linuxaiworks/lib-secret-manager/blob/main/AUTHORS.md");

    app.setApplicationDisplayName("Менеджер секретов");
    app.setApplicationDescription(
        "<span style='font-size:8pt; font-weight:600;'>Простой и безопасный менеджер паролей для Linux.</span><br/>"
        "Управляйте секретами, хранящимися в системном хранилище ключей через libsecret.<br/>"
        "<a href='https://github.com/linuxaiworks/lib-secret-manager'>https://github.com/linuxaiworks/lib-secret-manager</a>");

    // Явный перевод меток (если .qm не подхватился или неполный)
    translateAboutDialogLabels(&aboutDialog);

    registerMetaTypes();

    MainWindow w;
    w.setMinimumSize(900, 650);
    w.show();

    Dtk::Widget::moveToCenter(&w);

    return app.exec();
}