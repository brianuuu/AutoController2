#include "donutmaker.h"
#include "Programs/Modules/Common/framecapture.h"
#include "Programs/Modules/Common/runcommand.h"

namespace Program::PokemonLZA
{

DonutMaker::DonutMaker(QObject *parent) : ProgramBase(parent)
{

}

void DonutMaker::PopulateSettings(QBoxLayout *layout)
{
    m_count = new Setting::SettingSpinBox("Count", 1, 999);
    m_savedSettings.insert(m_count);
    AddSetting(layout, "Target Amount:", "No. of the same donut to make", m_count, true);

    m_status = AddText(layout, "Invalid", true);
    m_status->setFixedWidth(50);
    m_status->setAlignment(Qt::AlignRight);
    m_order = new Setting::SettingLineEdit("BerryOrder");
    m_order->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9,\-]*")));
    m_savedSettings.insert(m_order);
    AddSettings(layout, "Hyper Berry Order:", "Up/Down commands to select berries, separated by commas. Example: 2,-1,0,0 (Four berries, down twice, up once, then same berry twice)", {m_status, m_order}, true);
    connect(m_order, &QLineEdit::textChanged, this, &DonutMaker::OnOrderChanged);

    AddSpacer(layout);
    VerifyOrder();
}

bool DonutMaker::CanRun() const
{
    return ProgramBase::CanRun() && m_validOrder;
}

void DonutMaker::Start()
{
    ProgramBase::Start();
    StateBackupSave();
}

void DonutMaker::Stop()
{
    ProgramBase::Stop();
}

void DonutMaker::OnOrderChanged(const QString &str)
{
    VerifyOrder();
}

void DonutMaker::OnCommandFinished()
{
    if (OnModuleErrorQuit()) return;
    ClearModule(sender());

    switch (m_state)
    {
    case BackupSave:
    {
        StateRestart();
        break;
    }
    case Restart:
    {
        m_state = SetState(State::LaunchGame, "Waiting for title screen");
        AddBlackScreenModule();
        break;
    }
    case TitleLoading:
    {
        AddBlackScreenModule();
        break;
    }
    default:
    {
        PrintLog("Unhandled state after command is finished", LOG_Error);
        emit notifyFinished(-1);
        return;
    }
    }
}

void DonutMaker::OnFrameCaptureMatched(bool matched)
{
    switch (m_state)
    {
    case LaunchGame:
    case TitleLoading:
    {
        if (matched)
        {
            m_state = SetState((State)(m_state + 1));
        }
        break;
    }
    case TitleScreen:
    {
        if (!matched)
        {
            ClearModule(sender());
            m_state = SetState(State::TitleLoading, "Title screen detected, starting game");
            AddModule<Module::Common::RunCommand>(&DonutMaker::OnCommandFinished, "PLZA_LoadBackupSave", true);
        }
        break;
    }
    case WaitLoadFinish:
    {
        if (!matched)
        {
            ClearModule(sender());
            emit notifyFinished(0);
        }
        break;
    }
    default:
    {
        PrintLog("Unhandled state after frame capture has result", LOG_Error);
        emit notifyFinished(-1);
        return;
    }
    }
}

void DonutMaker::VerifyOrder()
{
    m_validOrder = true;

    QStringList const orders = m_order->text().split(',');
    for (QString const& order : orders)
    {
        order.toInt(&m_validOrder);
        if (!m_validOrder) break;
    }

    QPalette palette = m_status->palette();
    palette.setColor(QPalette::WindowText, LogTypeToColor(m_validOrder ? LOG_Success : LOG_Error));
    m_status->setPalette(palette);
    m_status->setText(m_validOrder ? "Valid" : "Invalid");

    OnCanRunChanged();
}

void DonutMaker::StateBackupSave()
{
    m_state = SetState(State::BackupSave, "Putting down backup save");
    AddModule<Module::Common::RunCommand>(&DonutMaker::OnCommandFinished, "PLZA_MakeBackupSave", true);
}

void DonutMaker::StateRestart()
{
    m_state = SetState(State::Restart, "Restarting game");
    AddModule<Module::Common::RunCommand>(&DonutMaker::OnCommandFinished, "System_RestartGame", true);
}

void DonutMaker::AddBlackScreenModule()
{
    Module::Common::FrameCapture* module = new Module::Common::FrameCapture("PLZA_LoadingBlackScreen");
    connect(module, &QThread::finished, this, &ProgramBase::OnModuleErrorQuit);
    connect(module, &Module::Common::FrameCapture::notifyResultMatched, this, &DonutMaker::OnFrameCaptureMatched);
    AddModule(module);
}

}
