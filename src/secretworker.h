#pragma once

#include <libsecret/secret.h>
#include <QObject>
#include <QThread>
#include <QString>
#include <QList>

/**
 * @brief Структура данных для хранения информации о секрете.
 * 
 * ВАЖНО: Поле password заполняется только при явном запросе (ленивая загрузка),
 * чтобы не держать все пароли в оперативной памяти одновременно.
 */
struct SecretItemData {
    QString label;      ///< Отображаемое имя секрета
    QString username;   ///< Имя пользователя / логин
    QString service;    ///< Название сервиса / приложения
    QString password;   ///< Пароль (пуст при listItems, заполняется при loadSecret)
    QString objectPath; ///< Уникальный D-Bus путь объекта в keyring
};
Q_DECLARE_METATYPE(SecretItemData)

/**
 * @brief Рабочий поток для взаимодействия с libsecret.
 * 
 * Все блокирующие операции GLib/GIO выполняются здесь, чтобы не замораживать GUI.
 * Использует отдельный GMainContext для корректной работы синхронных вызовов в QThread.
 */
class SecretWorker : public QObject {
    Q_OBJECT
public:
    explicit SecretWorker(QObject *parent = nullptr);
    ~SecretWorker();

public Q_SLOTS:
    /// Инициализация GLib контекста потока (вызывается при старте QThread)
    void init();
    /// Очистка ресурсов GLib (вызывается перед остановкой потока)
    void cleanup();
    
    /// Сохранение нового секрета в системное хранилище
    void storeItem(const QString &label, const QString &username,
                   const QString &service, const QString &password);
    
    /// Удаление секрета по его D-Bus пути
    void deleteItemByPath(const QString &objectPath);
    
    /// Получение списка всех секретов (без паролей!)
    void listItems();
    
    /// Загрузка конкретного пароля по запросу (ленивая загрузка)
    void loadSecret(const QString &objectPath);

Q_SIGNALS:
    void itemStored(bool success, const QString &error);
    void itemDeleted(bool success, const QString &error);
    void itemsListed(const QList<SecretItemData> &items, const QString &error);
    void secretLoaded(const QString &objectPath, const QString &password, const QString &error);

private:
    GMainContext *m_glibContext = nullptr; ///< Контекст GLib для этого потока
};