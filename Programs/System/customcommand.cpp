#include "customcommand.h"

void System::CustomCommand::PopulateSettings(QBoxLayout *layout)
{
    m_list = AddComboBox(layout,
        "Command Select:",
        "Select a pre-made command to run",
        "CommandType",
        {} // TODO:
    );

    m_command = AddLineEdit(layout,
        "Current Command:",
        "",
        "CommandEdit"
    );
    m_command->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9()|,\-\.]*")));
    connect(m_command, &QLineEdit::textChanged, this, &CustomCommand::OnCommandEdit);

    // add error message label and move it to the layout above, horribly
    m_labelStatus = AddSingleText(layout, "", true);
    layout->itemAt(layout->count() - 3)->widget()->layout()->addWidget(m_labelStatus);

    // set initial error text
    OnCommandEdit(m_labelStatus->text());
}

bool System::CustomCommand::CanRun() const
{
    return ProgramBase::CanRun() && m_validCommand;
}

void System::CustomCommand::Start()
{
    ProgramBase::Start();
}

void System::CustomCommand::Stop()
{
    ProgramBase::Stop();
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

    OnCanRunChanged();
}
