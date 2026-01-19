#include "mainwindow.h"

#include <QApplication>
#include <QThread>

#include <windows.h>
#include <cstdio>

#include "Helpers/jsonhelper.h"
#include "Settings/settingthreadpriority.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // enable debug console
    {
        QJsonObject profileSettings = JsonHelper::ReadSetting("ProfileSettings");
        QJsonObject development = JsonHelper::ReadObject(profileSettings, "Development");
        QVariant enabled;

        if (JsonHelper::ReadValue(development, "DebugConsole", enabled) && enabled.toBool())
        {
            // detach from the current console window
            FreeConsole();

            // create a separate new console window
            AllocConsole();

            // attach the new console to this application's process
            AttachConsole(GetCurrentProcessId());

            // reopen the std I/O streams to redirect I/O to the new console
            freopen("CON", "r", stdin);
            freopen("CON", "w", stdout);
            freopen("CON", "w", stderr);
        }
    }

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
