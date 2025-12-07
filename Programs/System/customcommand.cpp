#include "customcommand.h"

void System::CustomCommand::PopulateSettings(QBoxLayout *layout)
{
    m_list = AddComboBox(layout,
        "Command Select:",
        "Select pre-made commands to run",
        "CommandType",
        {} // TODO:
    );
}
