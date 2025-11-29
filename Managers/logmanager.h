#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QDateTime>
#include <QDir>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>

class LogManager : public QWidget
{
    Q_OBJECT
public:
    LogManager(QWidget* parent = nullptr);

    bool OnCloseEvent();

    void SetCurrentLogFile(QString const& file);
    void SetClearLogEnabled(bool enable);

    void PrintLog(QString const& log);
    void ClearLog();

private:
    // UI
    QTextBrowser* m_browser = Q_NULLPTR;
    QPushButton* m_btnClear = Q_NULLPTR;

    // Members
    int m_logCount = 0;
    QString m_logFile;
};

#endif // LOGMANAGER_H
