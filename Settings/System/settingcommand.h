#ifndef SETTINGCOMMAND_H
#define SETTINGCOMMAND_H

#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpressionValidator>

#include "../settingbase.h"

namespace Setting::System {

class SettingCommand : public QWidget, public SettingBase
{
    Q_OBJECT
public:
    explicit SettingCommand(QString const& name, bool allowEmpty);

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

    bool IsValid() const { return m_valid; }
    QString GetText() const { return m_command->text(); }
    void SetText(QString const& text) { return m_command->setText(text); }

signals:
    void notifyValid(bool valid);
    void notifyEdited();

private slots:
    void VerifyCommand();

private:
    QLineEdit* m_command = Q_NULLPTR;
    QLabel* m_status = Q_NULLPTR;
    bool m_valid = true;
    bool m_allowEmpty = false;
};

}

#endif // SETTINGCOMMAND_H
