#pragma once

#include <DMainWindow>
#include <QTableWidget>
#include <QMenu>
#include "secretworker.h"

DWIDGET_USE_NAMESPACE

/**
 * @brief Главное окно приложения.
 * 
 * Отображает таблицу секретов (без паролей).
 * Управляет жизненным циклом рабочего потока SecretWorker.
 */
class MainWindow : public DMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void onAddClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    
    // Слоты обработки результатов от Worker
    void onItemStored(bool success, const QString &error);
    void onItemsListed(const QList<SecretItemData> &items, const QString &error);
    void onItemDeleted(bool success, const QString &error);
    
    // Обработка двойного клика и загрузки пароля
    void onTableCellDoubleClicked(int row, int column);
    void showContextMenu(const QPoint &pos);
    void onSecretLoaded(const QString &objectPath, const QString &password, const QString &error);

private:
    void setupUI();
    void startWorker();

    QTableWidget *m_table = nullptr;
    QMenu        *m_contextMenu = nullptr;
    SecretWorker *m_worker = nullptr;
    QThread      *m_workerThread = nullptr;
    QList<SecretItemData> m_items; ///< Локальный кэш метаданных (без паролей)
    QString m_pendingDetailPath;   ///< Путь элемента, для которого ждем пароль
};