#include "donutmaker.h"

#include "Modules/Common/framecapture.h"
#include "Modules/Common/ocr.h"
#include "Modules/Common/runcommand.h"

namespace Program::PokemonLZA
{

#define DONUT_MAKER_RUN_COMMAND(...) AddModule<Module::Common::RunCommand>(&DonutMaker::OnCommandFinished, __VA_ARGS__)

DonutMaker::DonutMaker(QObject *parent) : ProgramBase(parent)
{

}

void DonutMaker::PopulateSettings(QBoxLayout *layout)
{
    AddText(layout, "Make sure correct language is set in Global Settings", true, LogTypeToColor(LOG_State));

    m_count = new Setting::SettingSpinBox("Count", 1, 999);
    m_savedSettings.insert(m_count);
    AddSetting(layout, "Target Amount:", "No. of the same donut to make", m_count, true);

    m_status = AddText(layout, "Invalid", true);
    m_status->setFixedWidth(50);
    m_status->setAlignment(Qt::AlignRight);
    m_order = new Setting::SettingLineEdit("BerryOrder");
    m_order->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9,\-]*")));
    m_savedSettings.insert(m_order);
    AddSettings(layout, "Hyper Berry Order:", "Up/Down commands to select berries, separated by commas. Positive = Down, Negative = Up. Example: 2,-1,0,0 (Four berries, down twice, up once, then same berry twice)", {m_status, m_order}, true);
    connect(m_order, &QLineEdit::textChanged, this, &DonutMaker::OnOrderChanged);

    m_power = new FlavorPower("FlavorPower");
    m_savedSettings.insert(m_power);
    AddSetting(layout, "Flavor Power Selection:", "Drag & drop desired flavor power into each slot (order does not matter), target donut will match at least one of the flavor powers in each slot (having all 3 levels in a slot will pick any level etc.). You can also double-click to remove power from a slot", m_power, false);
    m_power->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    VerifyOrder();
}

void DonutMaker::RegisterStats()
{
    RegisterStat(m_statMade, "Made");
    RegisterStat(m_statFound, "Found");
}

bool DonutMaker::CanRun() const
{
    return ProgramBase::CanRun() && m_validOrder;
}

void DonutMaker::Start()
{
    ProgramBase::Start();

    m_donutCount = 0;
    m_cachedSlots = m_power->GetPowerSlots();

    if (!EnsureOCRDatabase(FlavorPower::GetDatabase()))
    {
        return;
    }

    StateFlyToVertPC();
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
        m_state = SetState(State::FlyToHotelZ, "Flying to Hotel Z");
        DONUT_MAKER_RUN_COMMAND("PLZA_FlyToHotelZ", 0);
        break;
    }
    case Restart:
    case GameLoadStart:
    case FlyToHotelZ:
    case EnterHotelZ:
    case FlyToVertPC:
    {
        AddBlackScreenModule();
        break;
    }
    case TalkToAnsha:
    {
        QString command = "(Minus|50,None|50)2";
        QStringList const orders = m_order->text().split(',');
        for (QString const& order : orders)
        {
            int const number = order.toInt();
            if (number > 0)
            {
                command += ",(LDown|50,None|100)" + QString::number(number);
            }
            else if (number < 0)
            {
                command += ",(LUp|50,None|100)" + QString::number(-number);
            }
            command += ",A|50,None|50";
        }

        m_state = SetState(State::SelectBerries, "Selecting berries");
        DONUT_MAKER_RUN_COMMAND(command);
        break;
    }
    case SelectBerries:
    {
        m_state = SetState(State::MakeDonut, "Making donut");
        DONUT_MAKER_RUN_COMMAND("PLZA_MakeDonut", 0);
        IncrementStat(m_statMade);
        break;
    }
    case MakeDonut:
    {
        m_state = SetState(State::PowerCapture, "Performing OCR on flavor powers");

        CleanOCRFiles();
        m_powerEntries.clear();
        for (int i = 0; i < 3; i++)
        {
            Module::Common::OCR* ocr = new Module::Common::OCR("PLZA_FlavorPowerSlot" + QString::number(i+1), FlavorPower::GetDatabase());
            connect(ocr, &QThread::finished, this, &DonutMaker::OnOCRFinished);
            AddModule(ocr);
        }
        break;
    }
    case QuitDonut:
    {
        StateFlyToVertPC();
        break;
    }
    case WalkToNurseJoy:
    {
        StateBackupSave();
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
    if (!sender()) return;

    switch (m_state)
    {
    case Restart:
    case GameLoadStart:
    case FlyToHotelZ:
    case EnterHotelZ:
    case FlyToVertPC:
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
    case FlyToVertPCLoadWait:
    {
        // wait for black screen to be not black anymore + buffer from black detection
        if (!matched && m_elapsedTimer.elapsed() > 300)
        {
            ClearModule(sender());
            if (m_state == TitleScreen)
            {
                m_state = SetState(State::GameLoadStart, "Title screen detected, loading backup save");
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
            else if (m_state == FlyToVertPCLoadWait)
            {
                m_state = SetState(State::WalkToNurseJoy, "Walking up to Nurse Joy");
                DONUT_MAKER_RUN_COMMAND("None|1000,B|LUp|50,LUp|1200");
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

void DonutMaker::OnOCRFinished()
{
    // only expecting State::PowerCapture
    if (OnModuleErrorQuit()) return;

    // wait until there are three OCR results
    Module::Common::OCR* ocr = qobject_cast<Module::Common::OCR*>(sender());
    m_powerEntries.push_back(ocr->GetResultEntry());

    if (m_powerEntries.size() == 3)
    {
        // clear all modules
        ClearModules();

        // do matching with cached powers
        bool allFound = true;
        for (int i = 0; i < 3; i++)
        {
            bool found = false;
            QSet<QString> const& powers = m_cachedSlots[i];
            if (powers.isEmpty())
            {
                // empty slot
                continue;
            }

            for (QString const& power : powers)
            {
                if (m_powerEntries.contains(power))
                {
                    PrintLog(power + " has matched Slot " + QString::number(i+1));
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                // current slot cannot be found, failed donut
                PrintLog("Cannot find any flavor power in Slot " + QString::number(i+1), LOG_Warning);
                allFound = false;
                break;
            }
        }

        if (allFound)
        {
            m_donutCount++;
            PrintLog("Correct donut found! " + QString::number(m_count->value() - m_donutCount) + " remaining", LOG_Important);
            IncrementStat(m_statFound);

            if (m_donutCount == m_count->value())
            {
                // finished!
                emit notifyFinished(0);
            }
            else
            {
                // correct donut, put down a backup save
                m_state = SetState(State::QuitDonut, "Quitting donut menu");
                DONUT_MAKER_RUN_COMMAND("B|Spam|4000,None|50");
            }
        }
        else
        {
            // wrong donut, restart game load backup save
            m_state = SetState(State::Restart, "Restarting game");
            DONUT_MAKER_RUN_COMMAND("System_RestartGame", 0);
        }
    }
}

void DonutMaker::VerifyOrder()
{
    m_validOrder = true;

    QStringList const orders = m_order->text().split(',');
    if (orders.size() < 3 || orders.size() > 8)
    {
        m_validOrder = false;
    }
    else
    {
        for (QString const& order : orders)
        {
            order.toInt(&m_validOrder);
            if (!m_validOrder) break;
        }
    }

    QPalette palette = m_status->palette();
    palette.setColor(QPalette::WindowText, LogTypeToColor(m_validOrder ? LOG_Success : LOG_Error));
    m_status->setPalette(palette);
    m_status->setText(m_validOrder ? "Valid" : "Invalid");

    OnCanRunChanged();
}

void DonutMaker::StateFlyToVertPC()
{
    m_state = SetState(State::FlyToVertPC, "Flying to Vert Pokemon Center");
    DONUT_MAKER_RUN_COMMAND("PLZA_FlyToVertPC", 0);
}

void DonutMaker::StateBackupSave()
{
    m_state = SetState(State::BackupSave, "Putting down backup save");
    DONUT_MAKER_RUN_COMMAND("PLZA_MakeBackupSave", 0);
}

void DonutMaker::AddBlackScreenModule()
{
    Module::Common::FrameCapture* module = new Module::Common::FrameCapture("PLZA_LoadingBlackScreen");
    connect(module, &QThread::finished, this, &ProgramBase::OnModuleErrorQuit);
    connect(module, &Module::Common::FrameCapture::notifyResultMatched, this, &DonutMaker::OnFrameCaptureMatched);
    AddModule(module);
}

}
