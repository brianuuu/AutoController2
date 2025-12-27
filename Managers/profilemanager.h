#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QGroupBox>
#include <QMessageBox>
#include <QWidget>

#include "Programs/Settings/settinglanguage.h"
#include "Types/languagetype.h"

namespace Ui { class MainWindow; }

class ProfileManager : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileManager(QWidget *parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Profile"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();

public:
    LanguageType GetLanguageType() const;
    bool OcrTrainedDataExist() const;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void OnShow();

    // System
    void OnLanguageChanged(int index);

private:
    void LoadSettings();
    void SaveSettings() const;

private:
    // System
    Setting::SettingLanguage* m_language = Q_NULLPTR;
};

#endif // PROFILEMANAGER_H
