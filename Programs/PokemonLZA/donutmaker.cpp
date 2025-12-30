#include "donutmaker.h"
#include "Programs/Modules/Common/framecapture.h"
#include "Programs/Modules/Common/runcommand.h"

namespace Program::PokemonLZA
{

#define DONUT_MAKER_RUN_COMMAND(...) AddModule<Module::Common::RunCommand>(&DonutMaker::OnCommandFinished, __VA_ARGS__)

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
    case GameLoadStart:
    case FlyToHotelZ:
    case EnterHotelZ:
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
    case Restart:
    case GameLoadStart:
    case FlyToHotelZ:
    case EnterHotelZ:
    {
        // wait for black screen
        if (matched)
        {
            m_elapsedTimer.start();
            m_state = SetState((State)(m_state + 1));
        }
        break;
    }
    case TitleScreen:
    case GameLoadWait:
    case FlyToHotelZLoadWait:
    case EnterHotelZLoadWait:
    {
        // wait for black screen to be not black anymore + buffer from black detection
        if (!matched && m_elapsedTimer.elapsed() > 300)
        {
            ClearModule(sender());
            if (m_state == TitleScreen)
            {
                m_state = SetState(State::GameLoadStart, "Title screen detected, starting game");
                DONUT_MAKER_RUN_COMMAND("PLZA_LoadBackupSave", 500);
            }
            else if (m_state == GameLoadWait)
            {
                m_state = SetState(State::FlyToHotelZ, "Flying to Hotel Z");
                DONUT_MAKER_RUN_COMMAND("PLZA_FlyToHotelZ", 1000);
            }
            else if (m_state == FlyToHotelZLoadWait)
            {
                m_state = SetState(State::EnterHotelZ, "Entering Hotel Z");
                DONUT_MAKER_RUN_COMMAND("PLZA_EnterHotelZ", 1000);
            }
            else if (m_state == EnterHotelZLoadWait)
            {
                m_state = SetState(State::TalkToAnsha, "Going forward to talk to Ansha");
                DONUT_MAKER_RUN_COMMAND("PLZA_TalkToAnsha", 1000);
            }
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
    DONUT_MAKER_RUN_COMMAND("PLZA_MakeBackupSave", 0);
}

void DonutMaker::StateRestart()
{
    m_state = SetState(State::Restart, "Restarting game");
    DONUT_MAKER_RUN_COMMAND("System_RestartGame", 0);
}

void DonutMaker::AddBlackScreenModule()
{
    Module::Common::FrameCapture* module = new Module::Common::FrameCapture("PLZA_LoadingBlackScreen");
    connect(module, &QThread::finished, this, &ProgramBase::OnModuleErrorQuit);
    connect(module, &Module::Common::FrameCapture::notifyResultMatched, this, &DonutMaker::OnFrameCaptureMatched);
    AddModule(module);
}

}
