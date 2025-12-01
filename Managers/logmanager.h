#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QDateTime>
#include <QDir>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>

#include "Enums/system.h"

class LogManager : public QWidget
{
    Q_OBJECT
public:
    LogManager
    (
        QPushButton* btnOpen,
        QWidget* parent = nullptr
    );

    bool OnCloseEvent();

    void SetCurrentLogFile(QString const& file);
    void SetClearLogEnabled(bool enable);

public slots:
    void PrintLog(QString const& category, QString const& log, LogType type = LOG_Normal);
    void ClearLog();

private:
    // UI
    QTextBrowser*   m_browser = Q_NULLPTR;
    QPushButton*    m_btnClear = Q_NULLPTR;

    // Members
    int         m_logCount = 0;
    QString     m_logFile;
};

#endif // LOGMANAGER_H
