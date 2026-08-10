#include "secreteditdialog.h"
#include <QAbstractButton>

SecretEditDialog::SecretEditDialog(QWidget *parent) : DDialog(parent) {
    setTitle("Добавить секрет");
    setMinimumSize(420, 260);

    m_labelEdit   = new DLineEdit(this);
    m_userEdit    = new DLineEdit(this);
    m_serviceEdit = new DLineEdit(this);
    m_passEdit    = new DLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);

    auto *form = new QFormLayout();
    form->addRow("Метка:",    m_labelEdit);
    form->addRow("Имя пользователя:", m_userEdit);
    form->addRow("Сервис:",  m_serviceEdit);
    form->addRow("Пароль:", m_passEdit);

    auto *content = new QWidget(this);
    content->setLayout(form);
    addContent(content);

    addButton("Отмена", false, DDialog::ButtonNormal);
    addButton("Сохранить",   true,  DDialog::ButtonRecommend);

    connect(getButton(0), &QAbstractButton::clicked, this, &SecretEditDialog::reject);
    connect(getButton(1), &QAbstractButton::clicked, this, &SecretEditDialog::accept);
}

QString SecretEditDialog::label()    const { return m_labelEdit->text(); }
QString SecretEditDialog::username() const { return m_userEdit->text(); }
QString SecretEditDialog::service()  const { return m_serviceEdit->text(); }
QString SecretEditDialog::password() const { return m_passEdit->text(); }