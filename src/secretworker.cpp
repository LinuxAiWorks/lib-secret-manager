#include "secretworker.h"
#include <QDebug>

static const SecretSchema *my_schema(void) {
    static const SecretSchema schema = {
        "com.example.LibSecretManager", SECRET_SCHEMA_NONE,
        {
            {"label",    SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"username", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"service",  SECRET_SCHEMA_ATTRIBUTE_STRING},
            {nullptr,    (SecretSchemaAttributeType)0}
        }
    };
    return &schema;
}

SecretWorker::SecretWorker(QObject *parent) : QObject(parent) {}

SecretWorker::~SecretWorker() {
    // Освобождение контекста перенесено в cleanup()
}

void SecretWorker::init() {
    m_glibContext = g_main_context_new();
    g_main_context_push_thread_default(m_glibContext);
}

void SecretWorker::cleanup() {
    if (m_glibContext) {
        g_main_context_pop_thread_default(m_glibContext);
        g_main_context_unref(m_glibContext);
        m_glibContext = nullptr;
    }
}

void SecretWorker::storeItem(const QString &label, const QString &username,
                             const QString &service, const QString &password) {
    GError *error = nullptr;
    gboolean ok = secret_password_store_sync(
        my_schema(),
        SECRET_COLLECTION_DEFAULT,
        label.toUtf8().constData(),
        password.toUtf8().constData(),
        nullptr, &error,
        "label",    label.toUtf8().constData(),
        "username", username.toUtf8().constData(),
        "service",  service.toUtf8().constData(),
        nullptr
    );

    if (error) {
        Q_EMIT itemStored(false, QString::fromUtf8(error->message));
        g_error_free(error);
    } else {
        Q_EMIT itemStored(ok, ok ? QString() : QStringLiteral("Failed to store secret"));
    }
}

void SecretWorker::deleteItemByPath(const QString &objectPath) {
    GError *error = nullptr;
    SecretService *service = secret_service_get_sync(SECRET_SERVICE_NONE, nullptr, &error);
    if (error) {
        Q_EMIT itemDeleted(false, QString::fromUtf8(error->message));
        g_error_free(error);
        return;
    }

    SecretCollection *collection = secret_collection_for_alias_sync(
        service,
        SECRET_COLLECTION_DEFAULT,
        SECRET_COLLECTION_LOAD_ITEMS,
        nullptr, &error
    );

    if (error) {
        Q_EMIT itemDeleted(false, QString::fromUtf8(error->message));
        g_error_free(error);
        g_object_unref(service);
        return;
    }

    GList *items = secret_collection_get_items(collection);
    SecretItem *itemToDelete = nullptr;

    for (GList *l = items; l != nullptr; l = l->next) {
        SecretItem *item = SECRET_ITEM(l->data);
        const gchar *path = g_dbus_proxy_get_object_path(G_DBUS_PROXY(item));
        if (path && objectPath == QString::fromUtf8(path)) {
            itemToDelete = item;
            break;
        }
    }

    if (!itemToDelete) {
        Q_EMIT itemDeleted(false, "Item not found in collection");
        g_list_free(items);
        g_object_unref(collection);
        g_object_unref(service);
        return;
    }

    gboolean ok = secret_item_delete_sync(itemToDelete, nullptr, &error);
    if (error) {
        Q_EMIT itemDeleted(false, QString::fromUtf8(error->message));
        g_error_free(error);
    } else {
        Q_EMIT itemDeleted(ok, QString());
    }

    g_list_free(items);
    g_object_unref(collection);
    g_object_unref(service);
}

void SecretWorker::listItems() {
    GError *error = nullptr;
    SecretService *service = secret_service_get_sync(SECRET_SERVICE_NONE, nullptr, &error);
    if (error) {
        Q_EMIT itemsListed({}, QString::fromUtf8(error->message));
        g_error_free(error);
        return;
    }

    SecretCollection *collection = secret_collection_for_alias_sync(
        service,
        SECRET_COLLECTION_DEFAULT,
        SECRET_COLLECTION_LOAD_ITEMS,
        nullptr, &error
    );
    if (error) {
        Q_EMIT itemsListed({}, QString::fromUtf8(error->message));
        g_error_free(error);
        g_object_unref(service);
        return;
    }

    GList *items = secret_collection_get_items(collection);
    QList<SecretItemData> result;

    for (GList *l = items; l != nullptr; l = l->next) {
        SecretItem *item = SECRET_ITEM(l->data);
        if (!item) continue;

        SecretItemData data;
        const gchar *lbl = secret_item_get_label(item);
        data.label = lbl ? QString::fromUtf8(lbl) : QStringLiteral("(no label)");
        data.objectPath = QString::fromUtf8(g_dbus_proxy_get_object_path(G_DBUS_PROXY(item)));

        GHashTable *attrs = secret_item_get_attributes(item);
        if (attrs) {
            const gchar *u = static_cast<const gchar*>(g_hash_table_lookup(attrs, "username"));
            if (!u) u = static_cast<const gchar*>(g_hash_table_lookup(attrs, "user"));
            if (!u) u = static_cast<const gchar*>(g_hash_table_lookup(attrs, "account"));

            const gchar *s = static_cast<const gchar*>(g_hash_table_lookup(attrs, "service"));
            if (!s) s = static_cast<const gchar*>(g_hash_table_lookup(attrs, "application"));
            if (!s) s = static_cast<const gchar*>(g_hash_table_lookup(attrs, "server"));

            data.username = u ? QString::fromUtf8(u) : QString();
            data.service  = s ? QString::fromUtf8(s) : QString();
        }

        SecretValue *value = secret_item_get_secret(item);
        bool needUnrefValue = false;

        if (!value) {
            GError *loadError = nullptr;
            secret_item_load_secret_sync(item, nullptr, &loadError);
            if (loadError) {
                g_error_free(loadError);
            } else {
                value = secret_item_get_secret(item);
                needUnrefValue = true;
            }
        }

        if (value) {
            const gchar *text = secret_value_get_text(value);
            if (text) {
                data.password = QString::fromUtf8(text);
            }
            if (needUnrefValue) {
                secret_value_unref(value);
            }
        }

        result.append(data);
    }

    g_list_free(items);
    g_object_unref(collection);
    g_object_unref(service);

    Q_EMIT itemsListed(result, QString());
}