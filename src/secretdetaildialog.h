#pragma once

#include <DDialog>
#include <DLineEdit>
#include <QFormLayout>

DWIDGET_USE_NAMESPACE

class SecretDetailDialog : public DDialog {
    Q_OBJECT
public:
    explicit SecretDetailDialog(const QString &label,
                                const QString &username,
                                const QString &service,
                                const QString &password,
                                QWidget *parent = nullptr);
};