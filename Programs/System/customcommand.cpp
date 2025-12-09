#include "customcommand.h"

#include "Enums/system.h"
#include "Helpers/jsonhelper.h"
#include "Managers/serialmanager.h"

#define CUSTOM_COMMAND_DIRECTORY "../Resources/System/CustomCommand/"
#define CUSTOM_COMMAND_FORMAT QString(".customcommand")

System::CustomCommand::CustomCommand(QObject *parent) : ProgramBase(parent)
{
    connect(m_serialManager, &SerialManager::notifyCommandFinished, this, &CustomCommand::OnCommandFinished);
}

void System::CustomCommand::PopulateSettings(QBoxLayout *layout)
{
    QDir const directory(CUSTOM_COMMAND_DIRECTORY);
    QStringList const files = directory.entryList({"*" + CUSTOM_COMMAND_FORMAT}, QDir::Files);

    QStringList names;
    for (QString const& file : files)
    {
        names << file.mid(0, file.size() - CUSTOM_COMMAND_FORMAT.size());
    }

    m_list = AddComboBox(layout,"Command Select:", "Select a pre-made command to run", "CommandType", names);
    connect(m_list, &QComboBox::currentTextChanged, this, &CustomCommand::OnListChanged);

    m_command = AddLineEdit(layout, "Current Command:", "", "CommandEdit");
    m_command->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9()|,\-\.]*")));
    m_command->m_shouldSave = false;
    connect(m_command, &QLineEdit::textChanged, this, &CustomCommand::OnCommandEdit);

    // add error message label and move it to the layout above, horribly
    m_labelStatus = AddText(layout, "", true);
    layout->itemAt(layout->count() - 3)->widget()->layout()->addWidget(m_labelStatus);

    m_description = AddTextEdit(layout, "Description:", "", "Description");
    m_description->m_shouldSave = false;
    m_description->setMaximumHeight(100);

    m_btnSave = new QPushButton("Save As...");
    m_btnDelete = new QPushButton("Delete");
    AddSettings(layout, "", "", {m_btnSave, m_btnDelete}, true);
    connect(m_btnSave, &QPushButton::clicked, this, &CustomCommand::OnCommandSave);
    connect(m_btnDelete, &QPushButton::clicked, this, &CustomCommand::OnCommandDelete);

    // set initial text
    OnListChanged(m_list->currentText());
    OnCommandEdit(m_command->text());
}

bool System::CustomCommand::CanRun() const
{
    return ProgramBase::CanRun() && m_validCommand;
}

void System::CustomCommand::Start()
{
    ProgramBase::Start();

    PrintLog("Running command \"" + m_command->text() + "\"");
    m_serialManager->SendCommand(m_command->text());
}

void System::CustomCommand::Stop()
{
    ProgramBase::Stop();
}

void System::CustomCommand::OnListChanged(const QString &str)
{
    QString const name = CUSTOM_COMMAND_DIRECTORY + str + CUSTOM_COMMAND_FORMAT;
    QJsonObject const object = JsonHelper::ReadJson(name);

    QVariant command;
    if (JsonHelper::ReadValue(object, "Command", command))
    {
        m_command->setText(command.toString());
    }
    else
    {
        m_command->clear();
    }

    QVariant description;
    if (JsonHelper::ReadValue(object, "Description", description))
    {
        m_description->setText(description.toString());
    }
    else
    {
        m_description->clear();
    }
}

void System::CustomCommand::OnCommandEdit(const QString &command)
{
    QString errorMsg;
    if (SerialManager::VerifyCommand(command, errorMsg))
    {
        m_labelStatus->setText("Valid!");
        m_validCommand = true;
    }
    else
    {
        m_labelStatus->setText(errorMsg);
        m_validCommand = false;
    }

    QPalette palette = m_labelStatus->palette();
    palette.setColor(QPalette::WindowText, LogTypeToColor(m_validCommand ? LOG_Success : LOG_Error));
    m_labelStatus->setPalette(palette);

    m_btnSave->setEnabled(m_validCommand);
    OnCanRunChanged();
}

void System::CustomCommand::OnCommandFinished()
{
    if (!m_started) return;
    emit notifyFinished();
}

void System::CustomCommand::OnCommandSave()
{
    QString const file = QFileDialog::getSaveFileName(m_btnSave, tr("Save Command As"), CUSTOM_COMMAND_DIRECTORY, "Custom Command (*" + CUSTOM_COMMAND_FORMAT + ")");
    if (file == Q_NULLPTR) return;

    QJsonObject object;
    object.insert("Command", m_command->text());
    object.insert("Description", m_description->toPlainText());
    JsonHelper::WriteJson(file, object);

    QFileInfo const info(file);
    QString name = info.fileName();
    name = name.mid(0, name.size() - CUSTOM_COMMAND_FORMAT.size());

    if (m_list->findText(name) == -1)
    {
        m_list->addItem(name);
        m_list->model()->sort(0);
        m_list->setCurrentText(name);
    }
}

void System::CustomCommand::OnCommandDelete()
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;
    resBtn = QMessageBox::warning(m_btnDelete, "Warning", "Are you sure you want to delete current command?\nThis cannot be undone.", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        QFile::remove(CUSTOM_COMMAND_DIRECTORY + m_list->currentText() + CUSTOM_COMMAND_FORMAT);
        m_list->removeItem(m_list->currentIndex());
    }
}
