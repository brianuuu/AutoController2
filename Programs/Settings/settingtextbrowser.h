#ifndef SETTINGTEXTBROWSER_H
#define SETTINGTEXTBROWSER_H

#include <QTextBrowser>
#include "settingbase.h"

class SettingTextBrowser : public QTextBrowser, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingTextBrowser(QString const& name, QString const& defaultText = "");

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

private:
    QString m_defaultText;
};

#endif // SETTINGTEXTBROWSER_H
