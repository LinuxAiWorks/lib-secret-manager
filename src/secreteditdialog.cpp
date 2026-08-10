#include "secreteditdialog.h"
#include <QAbstractButton>
#include <QRandomGenerator>
#include <QAction>

SecretEditDialog::SecretEditDialog(QWidget *parent) : DDialog(parent) {
    setTitle("Добавить секрет");
    setMinimumSize(450, 280);

    m_labelEdit   = new DLineEdit(this);
    m_userEdit    = new DLineEdit(this);
    m_serviceEdit = new DLineEdit(this);
    m_passEdit    = new DPasswordEdit(this);

    // Добавляем кнопку генерации пароля внутрь поля ввода
    auto *genAction = new QAction(QIcon::fromTheme("view-refresh"), "Сгенерировать", this);
    genAction->setToolTip("Сгенерировать надёжный пароль");
    m_passEdit->lineEdit()->addAction(genAction, QLineEdit::LeadingPosition);

    connect(genAction, &QAction::triggered, this, [this]() {
        m_passEdit->lineEdit()->setText(generatePassword());
    });

    auto *form = new QFormLayout();
    form->addRow("Метка:",           m_labelEdit);
    form->addRow("Имя пользователя:", m_userEdit);
    form->addRow("Сервис:",          m_serviceEdit);
    form->addRow("Пароль:",          m_passEdit);

    auto *content = new QWidget(this);
    content->setLayout(form);
    addContent(content);

    addButton("Отмена",   false, DDialog::ButtonNormal);
    addButton("Сохранить", true,  DDialog::ButtonRecommend);

    auto *saveBtn = getButton(1);
    saveBtn->setEnabled(false); // Блокируем пока поля пустые

    // Лямбда для валидации полей в реальном времени
    auto validateFields = [this, saveBtn]() {
        bool valid = !m_labelEdit->text().trimmed().isEmpty() &&
                     !m_passEdit->lineEdit()->text().isEmpty();
        saveBtn->setEnabled(valid);
    };

    connect(m_labelEdit, &DLineEdit::textChanged, this, validateFields);
    connect(m_passEdit->lineEdit(), &QLineEdit::textChanged, this, validateFields);

    connect(getButton(0), &QAbstractButton::clicked, this, &SecretEditDialog::reject);
    connect(getButton(1), &QAbstractButton::clicked, this, &SecretEditDialog::accept);
}

QString SecretEditDialog::label()    const { return m_labelEdit->text(); }
QString SecretEditDialog::username() const { return m_userEdit->text(); }
QString SecretEditDialog::service()  const { return m_serviceEdit->text(); }
// Получаем текст из внутреннего lineEdit виджета DPasswordEdit
QString SecretEditDialog::password() const { return m_passEdit->lineEdit()->text(); }

QString SecretEditDialog::generatePassword(int length) const {
    // Набор символов: буквы, цифры, спецсимволы
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+";
    QString pass;
    pass.reserve(length);
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        pass.append(chars[index]);
    }
    return pass;
}