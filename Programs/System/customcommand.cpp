#include "customcommand.h"

void System::CustomCommand::PopulateSettings(QBoxLayout *layout)
{
    m_list = AddComboBox(layout,
        "Command Select:",
        "Select a pre-made command to run",
        "CommandType",
        {} // TODO:
    );
}
