#include "devframecapture.h"
#include "Helpers/captureholder.h"

namespace Program::Development
{

DevFrameCapture::DevFrameCapture(QObject *parent) : ProgramBase(parent)
{
}

void DevFrameCapture::PopulateSettings(QBoxLayout *layout)
{
    static QList<QString> modes =
    {
        "Point Color Match",
        "Point Range Match",
        "Area Color Match",
        "Area Range Match",
    };
    m_mode = new Setting::SettingComboBox("Mode", modes);
    m_savedSettings.insert(m_mode);
    AddSetting(layout, "Mode:", "", m_mode, true);
    connect(m_mode, &QComboBox::currentIndexChanged, this, &DevFrameCapture::OnModeChanged);

    QSize const captureRes = CaptureHolder::GetCaptureResolution();
    m_left = new Setting::SettingSpinBox("Left", 0, captureRes.width() - 1);
    m_savedSettings.insert(m_left);
    connect(m_left, &QSpinBox::valueChanged, this, &DevFrameCapture::OnLeftChanged);
    m_top = new Setting::SettingSpinBox("Top", 0, captureRes.height() - 1);
    m_savedSettings.insert(m_top);
    connect(m_top, &QSpinBox::valueChanged, this, &DevFrameCapture::OnTopChanged);
    m_width = new Setting::SettingSpinBox("Width", 1, captureRes.width(), 100);
    m_width->setEnabled(false);
    m_savedSettings.insert(m_width);
    connect(m_width, &QSpinBox::valueChanged, this, &DevFrameCapture::OnWidthChanged);
    m_height = new Setting::SettingSpinBox("Height", 1, captureRes.height(), 100);
    m_height->setEnabled(false);
    m_savedSettings.insert(m_height);
    connect(m_height, &QSpinBox::valueChanged, this, &DevFrameCapture::OnHeightChanged);
    AddSettings(layout, "Capture Point/Area:", "", {m_left, m_top, m_width, m_height}, true);

    m_minH = new Setting::SettingSpinBox("MinH", 0, 359);
    m_savedSettings.insert(m_minH);
    connect(m_minH, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_minS = new Setting::SettingSpinBox("MinS", 0, 255);
    m_savedSettings.insert(m_minS);
    connect(m_minS, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_minV = new Setting::SettingSpinBox("MinV", 0, 255);
    m_savedSettings.insert(m_minV);
    connect(m_minV, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    AddSettings(layout, "Min HSV:", "", {m_minH, m_minS, m_minV}, true);
    m_maxH = new Setting::SettingSpinBox("MaxH", 0, 359, 359);
    m_savedSettings.insert(m_maxH);
    connect(m_maxH, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_maxS = new Setting::SettingSpinBox("MaxS", 0, 255, 255);
    m_savedSettings.insert(m_maxS);
    connect(m_maxS, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_maxV = new Setting::SettingSpinBox("MaxV", 0, 255, 255);
    m_savedSettings.insert(m_maxV);
    connect(m_maxV, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    AddSettings(layout, "Max HSV:", "", {m_maxH, m_maxS, m_maxV}, true);

    m_color = new Setting::SettingColor("Color");
    m_savedSettings.insert(m_color);
    connect(m_color, &Setting::SettingColor::notifyColorChanged, this, &DevFrameCapture::OnColorChanged);
    AddSetting(layout, "Target Color:", "", m_color, true);

    AddSpacer(layout);
}

void DevFrameCapture::Start()
{
    ProgramBase::Start();
    m_mode->setEnabled(false);

    CaptureHolder::Mode const mode = CaptureHolder::Mode(m_mode->currentIndex());
    switch (mode)
    {
    case CaptureHolder::Mode::PointColorMatch:
        m_moduleCapture = new Module::Common::FrameCapture(GetPoint(), m_color->GetColor());
        break;
    case CaptureHolder::Mode::PointRangeMatch:
        m_moduleCapture = new Module::Common::FrameCapture(GetPoint(), GetRange());
        break;
    case CaptureHolder::Mode::AreaColorMatch:
        m_moduleCapture = new Module::Common::FrameCapture(GetRect(), m_color->GetColor());
        break;
    case CaptureHolder::Mode::AreaRangeMatch:
        m_moduleCapture = new Module::Common::FrameCapture(GetRect(), GetRange());
        break;
    }

    AddModule(m_moduleCapture);
}

void DevFrameCapture::Stop()
{
    ClearModule((Module::ModuleBase**)&m_moduleCapture);

    m_mode->setEnabled(true);
    ProgramBase::Stop();
}

void DevFrameCapture::OnModeChanged(int mode)
{
    bool const isArea = (mode == 2 || mode == 3);
    m_width->setEnabled(isArea);
    m_height->setEnabled(isArea);

    bool const isRange = (mode == 1 || mode == 3);
    m_minH->setEnabled(isRange);
    m_minS->setEnabled(isRange);
    m_minV->setEnabled(isRange);
    m_maxH->setEnabled(isRange);
    m_maxS->setEnabled(isRange);
    m_maxV->setEnabled(isRange);

    bool const isColor = (mode == 0 || mode == 2);
    m_color->SetEnabled(isColor);
}

void DevFrameCapture::OnLeftChanged(int value)
{
    QSize const captureRes = CaptureHolder::GetCaptureResolution();
    m_width->setMaximum(captureRes.width() - value);
    UpdateRect();
}

void DevFrameCapture::OnTopChanged(int value)
{
    QSize const captureRes = CaptureHolder::GetCaptureResolution();
    m_height->setMaximum(captureRes.height() - value);
    UpdateRect();
}

void DevFrameCapture::OnWidthChanged(int value)
{
    UpdateRect();
}

void DevFrameCapture::OnHeightChanged(int value)
{
    UpdateRect();
}

void DevFrameCapture::OnRangeChanged()
{
    UpdateRange();
}

void DevFrameCapture::OnColorChanged(QColor color)
{
    if (m_moduleCapture)
    {
        m_moduleCapture->SetTargetColor(color);
    }
}

QPoint DevFrameCapture::GetPoint() const
{
    return QPoint(m_left->value(), m_top->value());
}

QRect DevFrameCapture::GetRect() const
{
    return QRect(m_left->value(), m_top->value(), m_width->value(), m_height->value());
}

HsvRange DevFrameCapture::GetRange() const
{
    return HsvRange(m_minH->value(), m_minS->value(), m_minV->value(), m_maxH->value(), m_maxS->value(), m_maxV->value());
}

void DevFrameCapture::UpdateRect()
{
    if (m_moduleCapture)
    {
        m_moduleCapture->SetPoint(GetPoint());
        m_moduleCapture->SetArea(GetRect());
    }
}

void DevFrameCapture::UpdateRange()
{
    if (m_moduleCapture)
    {
        m_moduleCapture->SetHsvRange(GetRange());
    }
}

}
