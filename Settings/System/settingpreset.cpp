#include "settingpreset.h"
#include "defines.h"

namespace Setting::System
{

SettingPreset::SettingPreset(const QString &name, const QString &directory, const QString &extension, bool addCustom)
    : SettingComboBox(name, {})
    , m_directory(directory)
    , m_extension(extension)
{
    QDir const dir(directory);
    QStringList const files = dir.entryList({"*" + extension}, QDir::Files);

    QStringList names = {};
    if (addCustom)
    {
        names << CUSTOM_SELECTION;
    }
    else
    {
        names << "None";
    }

    for (QString const& file : files)
    {
        names << file.mid(0, file.size() - extension.size());
    }

    this->addItems(names);
}

void SettingPreset::OnEdited()
{
    // user input only
    this->setCurrentIndex(0);
}

void SettingPreset::OnDelete()
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;
    resBtn = QMessageBox::warning(this, "Warning", "Are you sure you want to delete current preset?\nThis cannot be undone.", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        QFile::remove(m_directory + this->currentText() + m_extension);
        this->removeItem(this->currentIndex());
    }
}

void SettingPreset::OnOpenDirectory()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_directory));
}

}
