#include "mainwindow.h"
#include "secreteditdialog.h"
#include "secretdetaildialog.h"
#include <DApplication>
#include <DTitlebar>
#include <DMessageManager>
#include <DFloatingMessage>
#include <DDialog>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) : DMainWindow(parent) {
    setupUI();
    startWorker();

    connect(m_worker, &SecretWorker::itemStored,  this, &MainWindow::onItemStored);
    connect(m_worker, &SecretWorker::itemsListed, this, &MainWindow::onItemsListed);
    connect(m_worker, &SecretWorker::itemDeleted, this, &MainWindow::onItemDeleted);

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableCellDoubleClicked);
    connect(m_table, &QTableWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    onRefreshClicked();
}

MainWindow::~MainWindow() {
    QMetaObject::invokeMethod(m_worker, "cleanup", Qt::BlockingQueuedConnection);
    m_workerThread->quit();
    m_workerThread->wait();
}

void MainWindow::setupUI() {
    titlebar()->setTitle("Менеджер секретов");
    titlebar()->setIcon(QIcon(":/lib-secret-manager.svg"));

    DApplication *app = dynamic_cast<DApplication*>(qApp);
    if (app) {
        app->setProductIcon(QIcon(":/lib-secret-manager.svg"));
        app->setProductName("Менеджер секретов");
        app->setApplicationVersion("1.0.0");
        app->setApplicationDescription(
            "Простой и безопасный менеджер паролей для Linux.\n"
            "Управляйте секретами, хранящимися в системном хранилище ключей через libsecret.");
    }

    auto *cw = new QWidget(this);
    auto *vlay = new QVBoxLayout(cw);
    vlay->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({ "Метка", "Имя пользователя", "Сервис", "Пароль" });

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 350);

    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);

    m_contextMenu = new QMenu(this);
    QAction *addAction = m_contextMenu->addAction(QIcon::fromTheme("list-add"), "Добавить секрет");
    QAction *delAction = m_contextMenu->addAction(QIcon::fromTheme("edit-delete"), "Удалить секрет");
    m_contextMenu->addSeparator();
    QAction *refreshAction = m_contextMenu->addAction(QIcon::fromTheme("view-refresh"), "Обновить");

    connect(addAction, &QAction::triggered, this, &MainWindow::onAddClicked);
    connect(delAction, &QAction::triggered, this, &MainWindow::onDeleteClicked);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshClicked);

    vlay->addWidget(m_table);
    setCentralWidget(cw);
    resize(1200, 700);
}

void MainWindow::showContextMenu(const QPoint &pos) {
    m_contextMenu->exec(m_table->viewport()->mapToGlobal(pos));
}

void MainWindow::startWorker() {
    m_worker = new SecretWorker();
    m_workerThread = new QThread(this);

    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, m_worker, &SecretWorker::init);
    m_workerThread->start();
}

void MainWindow::onRefreshClicked() {
    QMetaObject::invokeMethod(m_worker, "listItems", Qt::QueuedConnection);
}

void MainWindow::onAddClicked() {
    SecretEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QMetaObject::invokeMethod(m_worker, "storeItem", Qt::QueuedConnection,
                                  Q_ARG(QString, dlg.label()),
                                  Q_ARG(QString, dlg.username()),
                                  Q_ARG(QString, dlg.service()),
                                  Q_ARG(QString, dlg.password()));
    }
}

void MainWindow::onDeleteClicked() {
    int row = m_table->currentRow();
    if (row < 0 || row >= m_items.size()) {
        auto *msg = new DFloatingMessage(DFloatingMessage::ResidentType);
        msg->setMessage("Выберите строку для удаления");
        DMessageManager::instance()->sendMessage(this, msg);
        return;
    }

    const SecretItemData &item = m_items[row];

    DDialog confirm(this);
    confirm.setTitle("Подтверждение удаления");
    confirm.setMessage(QString("Вы уверены, что хотите удалить \"%1\"?").arg(item.label));
    confirm.setIcon(QIcon::fromTheme("dialog-warning"));
    confirm.addButton("Отмена", false, DDialog::ButtonNormal);
    confirm.addButton("Удалить", true,  DDialog::ButtonWarning);

    if (confirm.exec() != QDialog::Accepted) {
        return;
    }

    QString objectPath = item.objectPath;
    if (objectPath.isEmpty()) return;

    QMetaObject::invokeMethod(m_worker, "deleteItemByPath", Qt::QueuedConnection,
                              Q_ARG(QString, objectPath));
}

void MainWindow::onTableCellDoubleClicked(int row, int /*column*/) {
    if (row < 0 || row >= m_items.size()) return;

    const SecretItemData &item = m_items[row];
    SecretDetailDialog dlg(item.label, item.username, item.service, item.password, this);
    dlg.exec();
}

void MainWindow::onItemStored(bool success, const QString &error) {
    auto *msg = new DFloatingMessage(DFloatingMessage::ResidentType);
    if (success) {
        msg->setMessage("Секрет сохранён");
        onRefreshClicked();
    } else {
        msg->setMessage(QString("Ошибка сохранения: %1").arg(error));
    }
    DMessageManager::instance()->sendMessage(this, msg);
}

void MainWindow::onItemsListed(const QList<SecretItemData> &items, const QString &error) {
    if (!error.isEmpty()) {
        auto *msg = new DFloatingMessage(DFloatingMessage::ResidentType);
        msg->setMessage(QString("Ошибка загрузки: %1").arg(error));
        DMessageManager::instance()->sendMessage(this, msg);
        return;
    }

    m_items = items;

    m_table->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(items[i].label));
        m_table->setItem(i, 1, new QTableWidgetItem(items[i].username));
        m_table->setItem(i, 2, new QTableWidgetItem(items[i].service));

        QString masked = items[i].password.isEmpty() ? "" : QString(8, QChar(0x2022));
        m_table->setItem(i, 3, new QTableWidgetItem(masked));
    }
}

void MainWindow::onItemDeleted(bool success, const QString &error) {
    auto *msg = new DFloatingMessage(DFloatingMessage::ResidentType);
    if (success) {
        msg->setMessage("Удалено");
        onRefreshClicked();
    } else {
        msg->setMessage(QString("Ошибка удаления: %1").arg(error));
    }
    DMessageManager::instance()->sendMessage(this, msg);
}