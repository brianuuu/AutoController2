#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QDateTime>
#include <QDir>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>

#include "../ui_mainwindow.h"
#include "Enums/system.h"

class LogManager : public QWidget
{
    Q_OBJECT
public:
    LogManager(QWidget* parent = nullptr) {}
    static QString GetTypeID() { return "Log"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();
    void OnInitShow();

    void SetCurrentLogFile(QString const& file);
    void SetClearLogEnabled(bool enable);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void PrintLog(QString const& category, QString const& log, LogType type = LOG_Normal);
    void ClearLog();

    void OnShow();

private:
    void LoadSettings();
    void SaveSettings() const;

private:
    // UI
    QTextBrowser*   m_browser = Q_NULLPTR;
    QPushButton*    m_btnClear = Q_NULLPTR;

    // Members
    bool        m_defaultShow = false;
    int         m_logCount = 0;
    QString     m_logFile;
};

#endif // LOGMANAGER_H
