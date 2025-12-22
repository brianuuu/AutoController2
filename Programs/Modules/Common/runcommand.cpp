#include "runcommand.h"

#include "Managers/keyboardmanager.h"
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
        // TODO: read .ini
    }
    else
    {
        m_command = nameOrCommand;
    }

    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(this, &RunCommand::notifyButton, keyboardManager, &KeyboardManager::OnDisplayButton);

    m_serialManager = ManagerCollection::GetManager<SerialManager>();
    connect(this, &RunCommand::notifyButton, m_serialManager, &SerialManager::OnSendButton);

    if (!SerialManager::VerifyCommand(m_command, m_error))
    {
        m_result = -1;
    }
}

void RunCommand::run()
{
    if (m_result < 0)
    {
        return;
    }

    m_commandTimer.start();
    while(!m_terminate && !m_finished)
    {
        OnSendCurrentCommand();
    }

    // final stop command
    emit notifyButton(0);
}

void RunCommand::OnSendCurrentCommand(bool isLoopCount)
{
    if (!m_serialManager->IsConnected())
    {
        m_finished = true;
        m_result = -1;
        m_error = "Serial not connected";
        return;
    }

    if (m_commandIndex == -1 || m_commandIndex >= m_command.size())
    {
        m_finished = true;
        return;
    }

    // wait for current command delay
    if (m_commandTimer.elapsed() < m_commandDelay)
    {
        return;
    }

    qsizetype endIndex = m_command.indexOf(',', m_commandIndex + 1);
    QString str = m_command.mid(m_commandIndex, endIndex == -1 ? -1 : endIndex - m_commandIndex);

    // look for loop start
    qsizetype const loopStartIndex = str.indexOf('(');
    if (loopStartIndex == 0)
    {
        m_commandIndex++;
        m_commandLoopCounts.push_back(-1);
        OnSendCurrentCommand();
        return;
    }

    // look for loop end
    qsizetype const loopEndIndex = str.indexOf(')');
    if (loopEndIndex >= 0)
    {
        if (loopEndIndex == 0)
        {
            // first index is ')' expecting loop count next
            m_commandIndex++;
            OnSendCurrentCommand(true);
            return;
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
            m_commandLoopCounts.pop_back();
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
                        OnSendCurrentCommand();
                        return;
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

        m_commandDelay = duration;
        m_commandTimer.restart();
    }

    if (endIndex == -1)
    {
        m_commandIndex = -1;
    }
    else
    {
        m_commandIndex = endIndex + 1;
    }
}

}
