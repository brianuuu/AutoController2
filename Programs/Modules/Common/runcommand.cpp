#include "runcommand.h"

#include "Helpers/jsonhelper.h"
#include "Managers/keyboardmanager.h"
#include "Managers/managercollection.h"
#include "Managers/profilemanager.h"
#include "Managers/serialmanager.h"

namespace Module::Common
{

RunCommand::RunCommand
(
    const QString &nameOrCommand,
    bool isName,
    QObject *parent
)
    : ModuleBase(parent)
{
    if (isName)
    {
        m_name = nameOrCommand;

        ProfileManager* profileManager = ManagerCollection::GetManager<ProfileManager>();
        m_systemType = profileManager->GetSystemType();

        QString const name = Module::Common::RunCommand::GetDirectory() + m_name + Module::Common::RunCommand::GetExtension();
        QJsonObject const object = JsonHelper::ReadJson(name);

        QVariant command;
        if (JsonHelper::ReadValue(object, SystemToString(m_systemType), command) && !command.toString().isEmpty())
        {
            m_command = command.toString();
        }
        else if (JsonHelper::ReadValue(object, "Default", command))
        {
            m_systemType = ST_COUNT;
            m_command = command.toString();
        }
        else
        {
            m_error = "Command \"" + m_name + "\" not found";
            m_result = -1;
            return;
        }
    }
    else
    {
        m_command = nameOrCommand;
    }

    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(this, &RunCommand::notifyButton, keyboardManager, &KeyboardManager::OnDisplayButton);

    SerialManager* serialManager = ManagerCollection::GetManager<SerialManager>();
    connect(this, &RunCommand::notifyButton, serialManager->GetHolder(), &SerialHolder::OnSendButton);

    if (!SerialManager::VerifyCommand(m_command, m_error))
    {
        m_result = -1;
    }

    if (!serialManager->IsConnected())
    {
        m_result = -1;
        m_error = "Serial not connected";
    }
}

void RunCommand::run()
{
    if (m_result < 0 || m_terminate) return;

    if (m_name.isEmpty())
    {
        PrintLog("Running command = " + m_command);
    }
    else
    {
        QString const type = m_systemType == ST_COUNT ? "Default" : SystemToString(m_systemType);
        PrintLog("Running command \"" + m_name + "\" for \"" + type + "\" = " + m_command);
    }

    while(!m_terminate)
    {
        // check completion
        if (m_commandIndex == -1 || m_commandIndex >= m_command.size())
        {
            break;
        }

        if (SendCurrentCommand())
        {
            // wait for QTimer event
            exec();
        }
    }

    // final stop command
    emit notifyButton(0);
}

bool RunCommand::SendCurrentCommand(bool isLoopCount)
{
    bool shouldExec = false;

    qsizetype endIndex = m_command.indexOf(',', m_commandIndex + 1);
    QString str = m_command.mid(m_commandIndex, endIndex == -1 ? -1 : endIndex - m_commandIndex);

    // look for loop start
    qsizetype const loopStartIndex = str.indexOf('(');
    if (loopStartIndex == 0)
    {
        m_commandIndex++;
        m_commandLoopCounts.push_back(-1);
        return SendCurrentCommand();
    }

    // look for loop end
    qsizetype const loopEndIndex = str.indexOf(')');
    if (loopEndIndex >= 0)
    {
        if (loopEndIndex == 0)
        {
            // first index is ')' expecting loop count next
            m_commandIndex++;
            return SendCurrentCommand(true);
        }
        else
        {
            // remove all char after ')' so number remains
            str = str.mid(0, loopEndIndex);
            endIndex = m_commandIndex + loopEndIndex - 1;
        }
    }

    quint32 buttonFlag = 0;
    QPointF lStick(0,0);
    QPointF rStick(0,0);

    QStringList const buttons = str.split('|');
    for (int b = 0; b < buttons.size() - 1; b++)
    {
        QString const& button = buttons[b].toLower();
        if (button.startsWith("lx") || button.startsWith("ly") || button.startsWith("rx") || button.startsWith("ry"))
        {
            qreal const stickPos = button.mid(2).toDouble();
            if (button.startsWith("lx"))
            {
                lStick.setX(stickPos);
            }
            else if (button.startsWith("ly"))
            {
                lStick.setY(stickPos);
            }
            else if (button.startsWith("rx"))
            {
                rStick.setX(stickPos);
            }
            else if (button.startsWith("ry"))
            {
                rStick.setY(stickPos);
            }
        }
        else
        {
            buttonFlag |= StringToButtonFlag(button);
        }
    }

    int duration = buttons.back().toInt();
    if (isLoopCount)
    {
        // found a loop count
        int& loopLeft = m_commandLoopCounts.back();
        if (loopLeft == -1)
        {
            loopLeft = duration;
        }

        if (loopLeft == 1)
        {
            // immediately run next command if loop finished
            m_commandLoopCounts.pop_back();
            if (endIndex != -1)
            {
                m_commandIndex = endIndex + 1;
                return SendCurrentCommand();
            }
        }
        else
        {
            m_commandIndex--;
            if (loopLeft > 1)
            {
                // if loopCount is 0 it loops forever
                loopLeft--;
            }

            // roll back to '('
            int loopEndCount = 0;
            while (m_commandIndex > 0)
            {
                m_commandIndex--;
                if (m_command[m_commandIndex] == ')')
                {
                    loopEndCount++;
                }
                else if (m_command[m_commandIndex] == '(')
                {
                    if (loopEndCount == 0)
                    {
                        m_commandIndex++;
                        return SendCurrentCommand();
                    }
                    else
                    {
                        loopEndCount--;
                    }
                }
            }
        }
    }
    else
    {
        //PrintLog("Button: \"" + str + "\"");
        emit notifyButton(buttonFlag, lStick, rStick);
        QTimer::singleShot(duration, this, [this]{ quit(); } );
        shouldExec = true;
    }

    if (endIndex == -1)
    {
        m_commandIndex = -1;
    }
    else
    {
        m_commandIndex = endIndex + 1;
    }

    return shouldExec;
}

}
