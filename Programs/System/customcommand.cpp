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
        "Modify Command:",
        "",
        "CommandEdit"
    );
    m_command->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9()|,\-\.]*")));
}
