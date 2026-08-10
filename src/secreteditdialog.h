#pragma once

#include <DDialog>
#include <DLineEdit>
#include <DPasswordEdit> // Используем специализированный виджет
#include <QFormLayout>

DWIDGET_USE_NAMESPACE

/**
 * @brief Диалог добавления/редактирования секрета.
 * 
 * Особенности:
 * - Поле пароля использует DPasswordEdit (кнопка "глаз").
 * - Встроенный генератор надежных паролей.
 * - Валидация обязательных полей.
 */
class SecretEditDialog : public DDialog {
    Q_OBJECT
public:
    explicit SecretEditDialog(QWidget *parent = nullptr);

    QString label() const;
    QString username() const;
    QString service() const;
    QString password() const;

private:
    DLineEdit     *m_labelEdit;
    DLineEdit     *m_userEdit;
    DLineEdit     *m_serviceEdit;
    DPasswordEdit *m_passEdit;
    
    /// Генерация случайной строки заданной длины
    QString generatePassword(int length = 16) const;
};