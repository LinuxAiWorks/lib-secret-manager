#pragma once

#include <libsecret/secret.h>
#include <QObject>
#include <QThread>
#include <QString>
#include <QList>

struct SecretItemData {
    QString label;
    QString username;
    QString service;
    QString password;
    QString objectPath;
};

Q_DECLARE_METATYPE(SecretItemData)

class SecretWorker : public QObject {
    Q_OBJECT
public:
    explicit SecretWorker(QObject *parent = nullptr);
    ~SecretWorker();

public Q_SLOTS:
    void init();
    void cleanup();
    void storeItem(const QString &label, const QString &username,
                   const QString &service, const QString &password);
    void deleteItemByPath(const QString &objectPath);
    void listItems();

Q_SIGNALS:
    void itemStored(bool success, const QString &error);
    void itemDeleted(bool success, const QString &error);
    void itemsListed(const QList<SecretItemData> &items, const QString &error);

private:
    GMainContext *m_glibContext = nullptr;
};