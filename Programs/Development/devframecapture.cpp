#include "devframecapture.h"

namespace Program::Development
{

DevFrameCapture::DevFrameCapture(QObject *parent) : ProgramBase(parent)
{
}

void DevFrameCapture::PopulateSettings(QBoxLayout *layout)
{

}

void DevFrameCapture::Start()
{
    ProgramBase::Start();
}

void DevFrameCapture::Stop()
{
    ProgramBase::Stop();
}

}
