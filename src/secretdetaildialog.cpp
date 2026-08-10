#include "secretdetaildialog.h"
#include <DPasswordEdit>
#include <QClipboard>
#include <QGuiApplication>
#include <QPushButton>
#include <QHBoxLayout>

SecretDetailDialog::SecretDetailDialog(const QString &label,
                                       const QString &username,
                                       const QString &service,
                                       const QString &password,
                                       QWidget *parent)
    : DDialog(parent)
{
    setTitle("Детали секрета");
    setMinimumSize(500, 300);

    auto *labelEdit    = new DLineEdit(this);
    auto *userEdit     = new DLineEdit(this);
    auto *serviceEdit  = new DLineEdit(this);
    auto *passEdit     = new DPasswordEdit(this);

    labelEdit->setText(label);
    userEdit->setText(username);
    serviceEdit->setText(service);
    passEdit->lineEdit()->setText(password);

    labelEdit->lineEdit()->setReadOnly(true);
    userEdit->lineEdit()->setReadOnly(true);
    serviceEdit->lineEdit()->setReadOnly(true);
    passEdit->lineEdit()->setReadOnly(true);

    auto *form = new QFormLayout();
    form->addRow("Метка:",    labelEdit);
    form->addRow("Имя пользователя:", userEdit);
    form->addRow("Сервис:",  serviceEdit);
    form->addRow("Пароль:", passEdit);

    auto *content = new QWidget(this);
    content->setLayout(form);
    addContent(content);

    auto *copyBtn = new QPushButton("Копировать пароль", this);
    connect(copyBtn, &QPushButton::clicked, this, [password]() {
        QGuiApplication::clipboard()->setText(password);
    });

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(copyBtn);
    btnLayout->addStretch();

    auto *btnWidget = new QWidget(this);
    btnWidget->setLayout(btnLayout);
    addContent(btnWidget);

    addButton("Закрыть", true, DDialog::ButtonNormal);
    connect(getButton(0), &QAbstractButton::clicked, this, &SecretDetailDialog::accept);
}