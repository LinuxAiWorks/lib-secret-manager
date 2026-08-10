#pragma once

#include <DMainWindow>
#include <QTableWidget>
#include <QMenu>
#include "secretworker.h"

DWIDGET_USE_NAMESPACE

class MainWindow : public DMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void onAddClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onItemStored(bool success, const QString &error);
    void onItemsListed(const QList<SecretItemData> &items, const QString &error);
    void onItemDeleted(bool success, const QString &error);
    void onTableCellDoubleClicked(int row, int column);
    void showContextMenu(const QPoint &pos);

private:
    void setupUI();
    void startWorker();

    QTableWidget *m_table = nullptr;
    QMenu        *m_contextMenu = nullptr;
    SecretWorker *m_worker = nullptr;
    QThread      *m_workerThread = nullptr;
    QList<SecretItemData> m_items;
};