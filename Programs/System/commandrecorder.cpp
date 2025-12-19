#include "commandrecorder.h"
#include "Managers/keyboardmanager.h"

namespace System
{

CommandRecorder::CommandRecorder(QObject *parent)
    : ProgramBase{parent}
{
    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(keyboardManager, &KeyboardManager::notifyUserInput, this, &CommandRecorder::OnUserInput);
    connect(this, &ProgramBase::notifyStarted, keyboardManager, &KeyboardManager::OnShow);
}

void CommandRecorder::PopulateSettings(QBoxLayout *layout)
{
    m_nothing = new SettingComboBox("NothingType", {"Disabled", "Add Nothing at the Start", "Add Nothing at the End", "Add Nothing at both Start/End"});
    m_savedSettings.insert(m_nothing);
    AddSetting(layout, "\"Nothing\" Setting:", "Prefix/Suffix \"Nothing\" command, useful for commands that require long waits at the beginning/end", m_nothing, true);

    m_browser = new SettingTextBrowser("ResultCommand");
    m_savedSettings.insert(m_browser);
    AddSetting(layout, "Result Command:", "Note: Starting program will clear previous command", m_browser, false);
    m_browser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CommandRecorder::Start()
{
    ProgramBase::Start();
    m_nothing->setEnabled(false);

    m_timer.invalidate();
    m_buttonFlags = 0;
    m_lStick = QPointF();
    m_rStick = QPointF();

    int const nothingIndex = m_nothing->currentIndex();
    if (nothingIndex == 1 || nothingIndex == 3)
    {
        // immedately start timer so it records Nothing
        m_timer.start();
    }

    m_browser->clear();
}

void CommandRecorder::Stop()
{
    // record final Nothing
    int const nothingIndex = m_nothing->currentIndex();
    if ((nothingIndex == 2 || nothingIndex == 3) && m_timer.isValid())
    {
        quint64 const elapsed = m_timer.restart();
        QString const command = "Nothing|" + QString::number(elapsed);
        AppendCommand(command);
    }

    m_nothing->setEnabled(true);
    ProgramBase::Stop();
}

void CommandRecorder::OnUserInput(quint32 buttonFlag, QPointF lStick, QPointF rStick)
{
    if (!IsRunning()) return;

    if (!m_timer.isValid())
    {
        m_timer.start();
        m_buttonFlags = buttonFlag;
        m_lStick = lStick;
        m_rStick = rStick;
        return;
    }

    // write previous command
    QString command;
    if (m_buttonFlags == 0 && m_lStick == QPointF() && m_rStick == QPointF())
    {
        command = "Nothing|";
    }
    else
    {
        for (int i = 1; i < BTN_COUNT; i++)
        {
            ButtonType type = (ButtonType)i;
            if (m_buttonFlags & ButtonToFlag(type))
            {
                command += ButtonToString(type) + "|";
            }
        }

        if (m_lStick.x() != 0.0)
        {
            command += "LX" + QString::number(m_lStick.x(), 'g', 3) + "|";
        }
        if (m_lStick.y() != 0.0)
        {
            command += "LY" + QString::number(m_lStick.y(), 'g', 3) + "|";
        }
        if (m_rStick.x() != 0.0)
        {
            command += "RX" + QString::number(m_rStick.x(), 'g', 3) + "|";
        }
        if (m_rStick.y() != 0.0)
        {
            command += "RY" + QString::number(m_rStick.y(), 'g', 3) + "|";
        }
    }

    quint64 const elapsed = m_timer.restart();
    command += QString::number(elapsed);
    AppendCommand(command);

    // remember current command
    m_buttonFlags = buttonFlag;
    m_lStick = lStick;
    m_rStick = rStick;
}

void CommandRecorder::AppendCommand(const QString &command)
{
    if (!m_browser->toPlainText().isEmpty())
    {
        m_browser->insertPlainText(",");
    }
    m_browser->insertPlainText(command);
}

}
