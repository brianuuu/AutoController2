#include "keyboardmanager.h"
#include "Helpers/jsonhelper.h"

void KeyboardManager::Initialize(Ui::MainWindow *ui)
{
    connect(ui->PB_KeyboardSettings, &QPushButton::clicked, this, &KeyboardManager::OnShow);

    // Setup layout
    qApp->installEventFilter(this);
    this->setWindowTitle("Keyboard Controls");
    this->setFixedSize(668,504);

    QLabel* image = new QLabel(this);
    image->setScaledContents(true);
    image->setPixmap(QPixmap("../Resources/UI/Mapping.png"));
    image->resize(668,504);

    QPushButton* btnReset = new QPushButton(this);
    btnReset->move(278,286);
    btnReset->setFixedSize(120,28);
    btnReset->setText("Reset All to Default");
    connect(btnReset, &QPushButton::clicked, this, &KeyboardManager::OnResetDefault);

    m_labelReset = new QLabel(this);
    m_labelReset->move(247,310);
    m_labelReset->setFixedSize(182,52);
    m_labelReset->setText("Press any key to map:\n");
    m_labelReset->setStyleSheet("color: red; font-size: 18px");
    m_labelReset->setAlignment(Qt::AlignCenter);
    m_labelReset->hide();

    for (int i = 1; i < BTN_COUNT - 1; i++)
    {
        m_btnButton[i] = new QPushButton(this);
        m_btnButton[i]->setFixedSize(70,28);
        m_btnButton[i]->setText("A");
        m_btnButton[i]->setStyleSheet("background-color: rgb(255,255,255); font-size: 18px;");

        QFont font = m_btnButton[i]->font();
        font.setPointSize(12);
        m_btnButton[i]->setFont(font);

        connect(m_btnButton[i], &QPushButton::clicked, this, &KeyboardManager::OnButtonClicked);
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

    OnResetDefault();
    LoadSettings();
}

void KeyboardManager::closeEvent(QCloseEvent *event)
{
    // triggers when user closes this window
    m_defaultShow = false;
    SaveSettings();
    QWidget::closeEvent(event);
}

bool KeyboardManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ActivationChange && m_btnRemap && !this->isActiveWindow())
    {
        // cancel remap
        OnButtonClicked();
    }

    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        if (e->isAutoRepeat())
        {
            return false;
        }

        int const key = e->key();
        if (key == Qt::Key_Meta) // Windows key
        {
            return false;
        }

        if (event->type() == QEvent::KeyPress && m_btnRemap && this->isActiveWindow())
        {
            // Remapping key
            UpdateButtonMap(m_btnRemap, key);
            OnButtonClicked();
        }

        return true;
    }

    return QWidget::eventFilter(watched, event);
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

void KeyboardManager::OnResetDefault()
{
    m_typeToKeyMap.clear();

    m_typeToKeyMap[BTN_A] = Qt::Key_Z;
    m_typeToKeyMap[BTN_B] = Qt::Key_X;
    m_typeToKeyMap[BTN_X] = Qt::Key_C;
    m_typeToKeyMap[BTN_Y] = Qt::Key_V;

    m_typeToKeyMap[BTN_L] = Qt::Key_U;
    m_typeToKeyMap[BTN_R] = Qt::Key_O;
    m_typeToKeyMap[BTN_ZL] = Qt::Key_Y;
    m_typeToKeyMap[BTN_ZR] = Qt::Key_P;

    m_typeToKeyMap[BTN_Plus] = Qt::Key_Equal;
    m_typeToKeyMap[BTN_Minus] = Qt::Key_Minus;
    m_typeToKeyMap[BTN_Home] = Qt::Key_Return;
    m_typeToKeyMap[BTN_Capture] = Qt::Key_Backspace;

    m_typeToKeyMap[BTN_LClick] = Qt::Key_Q;
    m_typeToKeyMap[BTN_LUp] = Qt::Key_I;
    m_typeToKeyMap[BTN_LDown] = Qt::Key_K;
    m_typeToKeyMap[BTN_LLeft] = Qt::Key_J;
    m_typeToKeyMap[BTN_LRight] = Qt::Key_L;

    m_typeToKeyMap[BTN_RClick] = Qt::Key_E;
    m_typeToKeyMap[BTN_RUp] = Qt::Key_W;
    m_typeToKeyMap[BTN_RDown] = Qt::Key_S;
    m_typeToKeyMap[BTN_RLeft] = Qt::Key_A;
    m_typeToKeyMap[BTN_RRight] = Qt::Key_D;

    m_typeToKeyMap[BTN_DUp] = Qt::Key_Up;
    m_typeToKeyMap[BTN_DDown] = Qt::Key_Down;
    m_typeToKeyMap[BTN_DLeft] = Qt::Key_Left;
    m_typeToKeyMap[BTN_DRight] = Qt::Key_Right;

    for (int i = 1; i < BTN_COUNT - 1; i++)
    {
        ButtonType const type = (ButtonType)i;
        int const key = m_typeToKeyMap[type];
        m_keyToTypeMap[key] = type;
        SetButtonText(type);
    }

    // cancel remap
    OnButtonClicked();
}

void KeyboardManager::OnButtonClicked()
{
    QObject* object = sender();
    QPushButton* button = object ? qobject_cast<QPushButton*>(sender()) : m_btnRemap;

    bool found = false;
    ButtonType type = BTN_None;
    for (int i = 1; i < BTN_COUNT - 1; i++)
    {
        if (m_btnButton[i] == button)
        {
            found = true;
            type = (ButtonType)i;
            break;
        }
    }

    if (!found)
    {
        button = m_btnRemap;
        if (!button) return;
    }

    if (m_btnRemap == button)
    {
        // Cancel/Finish
        m_btnRemap->setStyleSheet("background-color: rgb(255,255,255); font-size: 18px;");
        m_btnRemap = Q_NULLPTR;
    }
    else
    {
        // Previous button
        if (m_btnRemap != Q_NULLPTR)
        {
            m_btnRemap->setStyleSheet("background-color: rgb(255,255,255); font-size: 18px;");
        }

        // Start mapping
        m_btnRemap = button;
        m_btnRemap->setStyleSheet("background-color: rgb(255,220,0); font-size: 18px;");
    }

    m_labelReset->setVisible(m_btnRemap != Q_NULLPTR);
    m_labelReset->setText("Press any key to map:\n" + ButtonToFullString(type));
}

void KeyboardManager::SetButtonText(ButtonType type)
{
    QPushButton* button = m_btnButton[type];
    int const key = m_typeToKeyMap[type];
    QString keyString = QKeySequence(key).toString();
    if (key == Qt::Key_Alt)
    {
        keyString = "Alt";
    }
    else if (key == Qt::Key_Shift)
    {
        keyString = "Shift";
    }
    else if (key == Qt::Key_Backspace)
    {
        keyString = "BSpace";
    }
    else if (key == Qt::Key_Control)
    {
        keyString = "Ctrl";
    }

    button->setText(keyString);
}

void KeyboardManager::UpdateButtonMap(QPushButton *button, int key)
{
    bool found = false;
    ButtonType type = BTN_None;
    for (int i = 1; i < BTN_COUNT - 1; i++)
    {
        if (m_btnButton[i] == button)
        {
            found = true;
            type = (ButtonType)i;
            break;
        }
    }

    if (!found) return;

    // Remove previous key
    if (m_typeToKeyMap.contains(type))
    {
        int const keyPrev = m_typeToKeyMap[type];
        m_keyToTypeMap.remove(keyPrev);
        m_typeToKeyMap.remove(type);
    }

    // Duplicated key
    if (m_keyToTypeMap.contains(key))
    {
        ButtonType const t = m_keyToTypeMap[key];
        m_btnButton[t]->setText("");
        m_typeToKeyMap.remove(t);
        m_keyToTypeMap.remove(key);
    }

    // Set current key mapping if key is valid
    if (key != 0)
    {
        m_typeToKeyMap[type] = key;
        m_keyToTypeMap[key] = type;
    }

    SetButtonText(type);
}

void KeyboardManager::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("KeyboardSettings");
    {
        QJsonObject buttonMapping = JsonHelper::ReadObject(settings, "ButtonMapping");

        QVariant key;
        for (int i = 1; i < BTN_COUNT - 1; i++)
        {
            ButtonType const type = (ButtonType)i;
            if (JsonHelper::ReadValue(buttonMapping, ButtonToString(type), key))
            {
                UpdateButtonMap(m_btnButton[i], key.toInt());
            }
        }
    }
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
    QJsonObject buttonMapping;
    for (int i = 1; i < BTN_COUNT - 1; i++)
    {
        ButtonType const type = (ButtonType)i;
        buttonMapping.insert(ButtonToString(type), m_typeToKeyMap[type]);
    }

    QJsonObject windowSize;
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject settings;
    settings.insert("ButtonMapping", buttonMapping);
    settings.insert("WindowSize", windowSize);
    settings.insert("DefaultShow", m_defaultShow);

    JsonHelper::WriteSetting("KeyboardSettings", settings);
}
