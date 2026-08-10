#include "secretdetaildialog.h"
#include <DPasswordEdit>
#include <QClipboard>
#include <QGuiApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTimer>
#include <QAction>

SecretDetailDialog::SecretDetailDialog(const QString &label,
                                       const QString &username,
                                       const QString &service,
                                       const QString &password,
                                       QWidget *parent)
    : DDialog(parent)
{
    setTitle("Детали секрета");
    setMinimumSize(500, 280);

    auto *labelEdit   = new DLineEdit(this);
    auto *userEdit    = new DLineEdit(this);
    auto *serviceEdit = new DLineEdit(this);
    auto *passEdit    = new DPasswordEdit(this);

    labelEdit->setText(label);
    userEdit->setText(username);
    serviceEdit->setText(service);
    passEdit->lineEdit()->setText(password);

    // Делаем поля только для чтения
    labelEdit->lineEdit()->setReadOnly(true);
    userEdit->lineEdit()->setReadOnly(true);
    serviceEdit->lineEdit()->setReadOnly(true);
    passEdit->lineEdit()->setReadOnly(true);

    // Кнопка копирования как иконка внутри поля пароля (рядом с "глазом")
    auto *copyAction = new QAction(QIcon::fromTheme("edit-copy"), "Копировать", this);
    copyAction->setToolTip("Копировать пароль (автоочистка через 30 сек)");
    passEdit->lineEdit()->addAction(copyAction, QLineEdit::TrailingPosition);

    connect(copyAction, &QAction::triggered, this, [password]() {
        QGuiApplication::clipboard()->setText(password);
        
        // БЕЗОПАСНОСТЬ: Автоочистка буфера обмена через 30 секунд
        QTimer::singleShot(30000, qApp, [password]() {
            // Проверяем, не скопировал ли пользователь что-то другое за это время
            if (QGuiApplication::clipboard()->text() == password) {
                QGuiApplication::clipboard()->clear();
            }
        });
    });

    auto *form = new QFormLayout();
    form->addRow("Метка:",            labelEdit);
    form->addRow("Имя пользователя:", userEdit);
    form->addRow("Сервис:",           serviceEdit);
    form->addRow("Пароль:",           passEdit);

    auto *content = new QWidget(this);
    content->setLayout(form);
    addContent(content);

    addButton("Закрыть", true, DDialog::ButtonNormal);
    connect(getButton(0), &QAbstractButton::clicked, this, &SecretDetailDialog::accept);
}