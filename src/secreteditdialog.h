#pragma once

#include <DDialog>
#include <DLineEdit>
#include <QFormLayout>

DWIDGET_USE_NAMESPACE

class SecretEditDialog : public DDialog {
    Q_OBJECT
public:
    explicit SecretEditDialog(QWidget *parent = nullptr);
    QString label() const;
    QString username() const;
    QString service() const;
    QString password() const;
private:
    DLineEdit *m_labelEdit;
    DLineEdit *m_userEdit;
    DLineEdit *m_serviceEdit;
    DLineEdit *m_passEdit;
};