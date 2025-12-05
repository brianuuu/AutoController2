#include "keyboardmanager.h"
#include "Helpers/jsonhelper.h"

void KeyboardManager::Initialize(Ui::MainWindow *ui)
{
    connect(ui->PB_KeyboardSettings, &QPushButton::clicked, this, &KeyboardManager::OnShow);

    // Setup layout
    this->setWindowTitle("Keyboard Controls");
    this->setFixedSize(668,504);

    QLabel* image = new QLabel(this);
    image->setScaledContents(true);
    image->setPixmap(QPixmap("../Resources/UI/Mapping.png"));
    image->resize(668,504);

    for (int i = 1; i < BTN_COUNT - 1; i++)
    {
        m_btnButton[i] = new QPushButton(this);
        m_btnButton[i]->setFixedSize(70,28);
        m_btnButton[i]->setText("A");

        QFont font = m_btnButton[i]->font();
        font.setPointSize(12);
        m_btnButton[i]->setFont(font);
    }

    for (int i = 1; i < BTN_COUNT - 1; i++)
    {
        m_labelButton[i] = new QLabel(this);
        m_labelButton[i]->setFixedSize(90,26);
        m_labelButton[i]->setStyleSheet("color: white; font-size: 16px");
    }

    int x = 14;
    int y = 25;
    m_btnButton[BTN_ZL]->move(x+96,y);
    m_labelButton[BTN_ZL]->move(x,y+3);
    m_labelButton[BTN_ZL]->setText("ZL");
    m_labelButton[BTN_ZL]->setAlignment(Qt::AlignRight);

    y += 37;
    m_btnButton[BTN_L]->move(x+96,y);
    m_labelButton[BTN_L]->move(x,y+3);
    m_labelButton[BTN_L]->setText("L");
    m_labelButton[BTN_L]->setAlignment(Qt::AlignRight);

    y += 39;
    m_btnButton[BTN_Minus]->move(x+96,y);
    m_labelButton[BTN_Minus]->move(x,y+3);
    m_labelButton[BTN_Minus]->setText("Minus");
    m_labelButton[BTN_Minus]->setAlignment(Qt::AlignRight);

    y += 38;
    m_btnButton[BTN_Capture]->move(x+96,y);
    m_labelButton[BTN_Capture]->move(x,y+3);
    m_labelButton[BTN_Capture]->setText("Capture");
    m_labelButton[BTN_Capture]->setAlignment(Qt::AlignRight);

    y += 38;
    m_btnButton[BTN_LClick]->move(x+96,y);
    m_labelButton[BTN_LClick]->move(x,y+3);
    m_labelButton[BTN_LClick]->setText("L-Click");
    m_labelButton[BTN_LClick]->setAlignment(Qt::AlignRight);

    y += 38;
    m_btnButton[BTN_DUp]->move(x+96,y);
    m_labelButton[BTN_DUp]->move(x,y+3);
    m_labelButton[BTN_DUp]->setText("D-Pad Up");
    m_labelButton[BTN_DUp]->setAlignment(Qt::AlignRight);

    y += 37;
    m_btnButton[BTN_DLeft]->move(x+96,y);
    m_labelButton[BTN_DLeft]->move(x,y+3);
    m_labelButton[BTN_DLeft]->setText("D-Pad Left");
    m_labelButton[BTN_DLeft]->setAlignment(Qt::AlignRight);

    y += 38;
    m_btnButton[BTN_DDown]->move(x+96,y);
    m_labelButton[BTN_DDown]->move(x,y+3);
    m_labelButton[BTN_DDown]->setText("D-Pad Down");
    m_labelButton[BTN_DDown]->setAlignment(Qt::AlignRight);

    y += 37;
    m_btnButton[BTN_DRight]->move(x+96,y);
    m_labelButton[BTN_DRight]->move(x,y+3);
    m_labelButton[BTN_DRight]->setText("D-Pad Right");
    m_labelButton[BTN_DRight]->setAlignment(Qt::AlignRight);

    x = 496;
    y = 25;
    m_btnButton[BTN_ZR]->move(x,y);
    m_labelButton[BTN_ZR]->move(x+76,y+3);
    m_labelButton[BTN_ZR]->setText("ZR");
    m_labelButton[BTN_ZR]->setAlignment(Qt::AlignLeft);

    y += 37;
    m_btnButton[BTN_R]->move(x,y);
    m_labelButton[BTN_R]->move(x+76,y+3);
    m_labelButton[BTN_R]->setText("R");
    m_labelButton[BTN_R]->setAlignment(Qt::AlignLeft);

    y += 39;
    m_btnButton[BTN_Plus]->move(x,y);
    m_labelButton[BTN_Plus]->move(x+76,y+3);
    m_labelButton[BTN_Plus]->setText("Plus");
    m_labelButton[BTN_Plus]->setAlignment(Qt::AlignLeft);

    y += 38;
    m_btnButton[BTN_Home]->move(x,y);
    m_labelButton[BTN_Home]->move(x+76,y+3);
    m_labelButton[BTN_Home]->setText("Home");
    m_labelButton[BTN_Home]->setAlignment(Qt::AlignLeft);

    y += 38;
    m_btnButton[BTN_X]->move(x,y);
    m_labelButton[BTN_X]->move(x+76,y+3);
    m_labelButton[BTN_X]->setText("X");
    m_labelButton[BTN_X]->setAlignment(Qt::AlignLeft);

    y += 38;
    m_btnButton[BTN_A]->move(x,y);
    m_labelButton[BTN_A]->move(x+76,y+3);
    m_labelButton[BTN_A]->setText("A");
    m_labelButton[BTN_A]->setAlignment(Qt::AlignLeft);

    y += 37;
    m_btnButton[BTN_B]->move(x,y);
    m_labelButton[BTN_B]->move(x+76,y+3);
    m_labelButton[BTN_B]->setText("B");
    m_labelButton[BTN_B]->setAlignment(Qt::AlignLeft);

    y += 38;
    m_btnButton[BTN_Y]->move(x,y);
    m_labelButton[BTN_Y]->move(x+76,y+3);
    m_labelButton[BTN_Y]->setText("Y");
    m_labelButton[BTN_Y]->setAlignment(Qt::AlignLeft);

    y += 37;
    m_btnButton[BTN_RClick]->move(x,y);
    m_labelButton[BTN_RClick]->move(x+76,y+3);
    m_labelButton[BTN_RClick]->setText("R-Click");
    m_labelButton[BTN_RClick]->setAlignment(Qt::AlignLeft);

    x = 197;
    y = 376;
    m_btnButton[BTN_LUp]->move(x,y);
    m_btnButton[BTN_LDown]->move(x,y+72);
    m_btnButton[BTN_LLeft]->move(x-55,y+36);
    m_btnButton[BTN_LRight]->move(x+55,y+36);
    m_labelButton[BTN_LRight]->move(x-10,y+100);
    m_labelButton[BTN_LRight]->setText("L-Stick");
    m_labelButton[BTN_LRight]->setAlignment(Qt::AlignCenter);

    x = 407;
    m_btnButton[BTN_RUp]->move(x,y);
    m_btnButton[BTN_RDown]->move(x,y+72);
    m_btnButton[BTN_RLeft]->move(x-55,y+36);
    m_btnButton[BTN_RRight]->move(x+55,y+36);
    m_labelButton[BTN_RRight]->move(x-10,y+100);
    m_labelButton[BTN_RRight]->setText("R-Stick");
    m_labelButton[BTN_RRight]->setAlignment(Qt::AlignCenter);

    LoadSettings();
}

void KeyboardManager::closeEvent(QCloseEvent *event)
{
    // triggers when user closes this window
    m_defaultShow = false;
    SaveSettings();
    QWidget::closeEvent(event);
}

bool KeyboardManager::OnCloseEvent()
{
    // triggers when main window closes, don't change m_defaultShow
    this->hide();
    SaveSettings();
    return true;
}

bool KeyboardManager::OnInitShow()
{
    if (m_defaultShow && this->isHidden())
    {
        OnShow();
        return true;
    }

    return false;
}

void KeyboardManager::OnShow()
{
    m_defaultShow = true;
    this->show();
    if (this->isMinimized())
    {
        this->showNormal();
    }
    this->activateWindow();
}

void KeyboardManager::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("KeyboardSettings");
    {
        QJsonObject windowSize = JsonHelper::ReadObject(settings, "WindowSize");

        QVariant x, y;
        if (JsonHelper::ReadValue(windowSize, "X", x) && JsonHelper::ReadValue(windowSize, "Y", y))
        {
            this->move(x.toInt(), y.toInt());
        }
    }
    {
        QVariant defaultShow;
        if (JsonHelper::ReadValue(settings, "DefaultShow", defaultShow))
        {
            m_defaultShow = defaultShow.toBool();
        }
    }
}

void KeyboardManager::SaveSettings() const
{
    QJsonObject windowSize;
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject settings;
    settings.insert("WindowSize", windowSize);
    settings.insert("DefaultShow", m_defaultShow);

    JsonHelper::WriteSetting("KeyboardSettings", settings);
}
