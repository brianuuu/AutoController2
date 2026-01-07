#include "mainwindow.h"

#include <QApplication>
#include <QThread>

#include "Helpers/jsonhelper.h"
#include "Programs/Settings/settingthreadpriority.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // set main thread priority
    {
        QJsonObject profileSettings = JsonHelper::ReadSetting("ProfileSettings");
        QJsonObject performance = JsonHelper::ReadObject(profileSettings, "Performance");
        QVariant text;
        if (JsonHelper::ReadValue(performance, "MainPriority", text))
        {
            QThread::currentThread()->setPriority(Setting::SettingThreadPriority::StringToPriority(text.toString()));
        }
        else
        {
            // default
            QThread::currentThread()->setPriority(QThread::HighestPriority);
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
