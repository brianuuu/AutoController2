#include "devframecapture.h"
#include "Helpers/captureholder.h"
#include "Helpers/jsonhelper.h"
#include "Managers/videomanager.h"

#define FRAME_CAPTURE_CUSTOM QString("(Custom)")

namespace Program::Development
{

DevFrameCapture::DevFrameCapture(QObject *parent) : ProgramBase(parent)
{
    VideoManager* videoManager = ManagerCollection::GetManager<VideoManager>();
    connect(videoManager, &VideoManager::notifyMousePressed, this, &DevFrameCapture::OnMousePressed);
    connect(videoManager, &VideoManager::notifyMouseMoved, this, &DevFrameCapture::OnMouseMoved);
}

void DevFrameCapture::PopulateSettings(QBoxLayout *layout)
{
    QDir const directory(CaptureHolder::GetDirectory());
    QStringList const files = directory.entryList({"*" + CaptureHolder::GetFormat()}, QDir::Files);

    QStringList names = { FRAME_CAPTURE_CUSTOM };
    for (QString const& file : files)
    {
        names << file.mid(0, file.size() - CaptureHolder::GetFormat().size());
    }

    m_list = new Setting::SettingComboBox("CaptureType", names);
    m_savedSettings.insert(m_list);
    AddSetting(layout, "Capture Preset:", "", m_list, true);
    connect(m_list, &QComboBox::currentTextChanged, this, &DevFrameCapture::OnListChanged);

    static QList<QString> modes =
    {
        "Point Color Match",
        "Point Range Match",
        "Area Color Match",
        "Area Range Match",
    };
    m_mode = new Setting::SettingComboBox("Mode", modes);
    AddSetting(layout, "Mode:", "", m_mode, true);
    connect(m_mode, &QComboBox::currentIndexChanged, this, &DevFrameCapture::OnModeChanged);

    QSize const captureRes = CaptureHolder::GetCaptureResolution();
    m_left = new Setting::SettingSpinBox("Left", 0, captureRes.width() - 1);
    connect(m_left, &QSpinBox::valueChanged, this, &DevFrameCapture::OnLeftChanged);
    m_top = new Setting::SettingSpinBox("Top", 0, captureRes.height() - 1);
    connect(m_top, &QSpinBox::valueChanged, this, &DevFrameCapture::OnTopChanged);
    m_width = new Setting::SettingSpinBox("Width", 1, captureRes.width(), 100);
    m_width->setEnabled(false);
    connect(m_width, &QSpinBox::valueChanged, this, &DevFrameCapture::OnWidthChanged);
    m_height = new Setting::SettingSpinBox("Height", 1, captureRes.height(), 100);
    m_height->setEnabled(false);
    connect(m_height, &QSpinBox::valueChanged, this, &DevFrameCapture::OnHeightChanged);
    AddSettings(layout, "Capture Point/Area:", "", {m_left, m_top, m_width, m_height}, true);

    m_minH = new Setting::SettingSpinBox("MinH", 0, 359);
    connect(m_minH, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_minS = new Setting::SettingSpinBox("MinS", 0, 255);
    connect(m_minS, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_minV = new Setting::SettingSpinBox("MinV", 0, 255);
    connect(m_minV, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    AddSettings(layout, "Min HSV:", "", {m_minH, m_minS, m_minV}, true);
    m_maxH = new Setting::SettingSpinBox("MaxH", 0, 359, 359);
    connect(m_maxH, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_maxS = new Setting::SettingSpinBox("MaxS", 0, 255, 255);
    connect(m_maxS, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    m_maxV = new Setting::SettingSpinBox("MaxV", 0, 255, 255);
    connect(m_maxV, &QSpinBox::valueChanged, this, &DevFrameCapture::OnRangeChanged);
    AddSettings(layout, "Max HSV:", "", {m_maxH, m_maxS, m_maxV}, true);

    m_color = new Setting::SettingColor("Color");
    connect(m_color, &Setting::SettingColor::notifyColorChanged, this, &DevFrameCapture::OnColorChanged);
    AddSetting(layout, "Target Color:", "", m_color, true);

    m_mean = new Setting::SettingDoubleSpinBox("Mean", 0.0, 1.0);
    connect(m_mean, &Setting::SettingDoubleSpinBox::valueChanged, this, &DevFrameCapture::OnMeanChanged);
    AddSetting(layout, "Target Mean:", "", m_mean, true);

    m_btnSave = new QPushButton("Save As...");
    m_btnDelete = new QPushButton("Delete");
    m_btnDirectory = new QPushButton("Open Directory");
    AddSettings(layout, "", "", {m_btnSave, m_btnDelete, m_btnDirectory}, true);
    connect(m_btnSave, &QPushButton::clicked, this, &DevFrameCapture::OnSave);
    connect(m_btnDelete, &QPushButton::clicked, this, &DevFrameCapture::OnDelete);
    connect(m_btnDirectory, &QPushButton::clicked, this, &DevFrameCapture::OnOpenDirectory);

    AddSpacer(layout);

    // set initial text
    OnListChanged(m_list->currentText());
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

void DevFrameCapture::OnListChanged(const QString &str)
{
    if (str == FRAME_CAPTURE_CUSTOM)
    {
        // should save settings
        m_savedSettings.insert(m_mode);
        m_savedSettings.insert(m_left);
        m_savedSettings.insert(m_top);
        m_savedSettings.insert(m_width);
        m_savedSettings.insert(m_height);
        m_savedSettings.insert(m_minH);
        m_savedSettings.insert(m_minS);
        m_savedSettings.insert(m_minV);
        m_savedSettings.insert(m_maxH);
        m_savedSettings.insert(m_maxS);
        m_savedSettings.insert(m_maxV);
        m_savedSettings.insert(m_color);
        m_savedSettings.insert(m_mean);
        m_btnDelete->setEnabled(false);
        return;
    }
    else
    {
        // don't save
        m_savedSettings.remove(m_mode);
        m_savedSettings.remove(m_left);
        m_savedSettings.remove(m_top);
        m_savedSettings.remove(m_width);
        m_savedSettings.remove(m_height);
        m_savedSettings.remove(m_minH);
        m_savedSettings.remove(m_minS);
        m_savedSettings.remove(m_minV);
        m_savedSettings.remove(m_maxH);
        m_savedSettings.remove(m_maxS);
        m_savedSettings.remove(m_maxV);
        m_savedSettings.remove(m_color);
        m_savedSettings.remove(m_mean);
        m_btnDelete->setEnabled(true);
    }

    QString const name = CaptureHolder::GetDirectory() + str + CaptureHolder::GetFormat();
    QJsonObject const object = JsonHelper::ReadJson(name);

    QVariant value;
    if (JsonHelper::ReadValue(object, "Mode", value))
    {
        m_mode->blockSignals(true);
        m_mode->setCurrentIndex(value.toInt());
        m_mode->blockSignals(false);
        UpdateSettingEnabled();
    }

    int const mode = m_mode->currentIndex();
    bool const isArea = (mode == 2 || mode == 3);
    bool const isRange = (mode == 1 || mode == 3);
    bool const isColor = (mode == 0 || mode == 2);

    if (JsonHelper::ReadValue(object, "Left", value))
    {
        m_left->blockSignals(true);
        m_left->setValue(value.toInt());
        m_left->blockSignals(false);
    }
    if (JsonHelper::ReadValue(object, "Top", value))
    {
        m_top->blockSignals(true);
        m_top->setValue(value.toInt());
        m_top->blockSignals(false);
    }

    if (isArea)
    {
        if (JsonHelper::ReadValue(object, "Width", value))
        {
            m_width->blockSignals(true);
            m_width->setValue(value.toInt());
            m_width->blockSignals(false);
        }
        if (JsonHelper::ReadValue(object, "Height", value))
        {
            m_height->blockSignals(true);
            m_height->setValue(value.toInt());
            m_height->blockSignals(false);
        }
    }

    if (isRange)
    {
        if (JsonHelper::ReadValue(object, "MinH", value))
        {
            m_minH->blockSignals(true);
            m_minH->setValue(value.toInt());
            m_minH->blockSignals(false);
        }
        if (JsonHelper::ReadValue(object, "MinS", value))
        {
            m_minS->blockSignals(true);
            m_minS->setValue(value.toInt());
            m_minS->blockSignals(false);
        }
        if (JsonHelper::ReadValue(object, "MinS", value))
        {
            m_minV->blockSignals(true);
            m_minV->setValue(value.toInt());
            m_minV->blockSignals(false);
        }
        if (JsonHelper::ReadValue(object, "MaxH", value))
        {
            m_maxH->blockSignals(true);
            m_maxH->setValue(value.toInt());
            m_maxH->blockSignals(false);
        }
        if (JsonHelper::ReadValue(object, "MaxS", value))
        {
            m_maxS->blockSignals(true);
            m_maxS->setValue(value.toInt());
            m_maxS->blockSignals(false);
        }
        if (JsonHelper::ReadValue(object, "MaxS", value))
        {
            m_maxV->blockSignals(true);
            m_maxV->setValue(value.toInt());
            m_maxV->blockSignals(false);
        }
    }

    if (isColor)
    {
        if (JsonHelper::ReadValue(object, "Color", value))
        {
            m_color->blockSignals(true);
            m_color->SetColor(QColor(value.toUInt()));
            m_color->blockSignals(false);
        }
    }

    if (mode == 3)
    {
        if (JsonHelper::ReadValue(object, "Mean", value))
        {
            m_mean->blockSignals(true);
            m_mean->setValue(value.toDouble());
            m_mean->blockSignals(false);
        }
    }
}

void DevFrameCapture::OnModeChanged(int mode)
{
    SwitchToCustom();
    UpdateSettingEnabled();
}

void DevFrameCapture::OnLeftChanged(int value)
{
    QSize const captureRes = CaptureHolder::GetCaptureResolution();
    m_width->setMaximum(captureRes.width() - value);

    SwitchToCustom();
    UpdateRect();
}

void DevFrameCapture::OnTopChanged(int value)
{
    QSize const captureRes = CaptureHolder::GetCaptureResolution();
    m_height->setMaximum(captureRes.height() - value);

    SwitchToCustom();
    UpdateRect();
}

void DevFrameCapture::OnWidthChanged(int value)
{
    SwitchToCustom();
    UpdateRect();
}

void DevFrameCapture::OnHeightChanged(int value)
{
    SwitchToCustom();
    UpdateRect();
}

void DevFrameCapture::OnRangeChanged()
{
    SwitchToCustom();
    UpdateRange();
}

void DevFrameCapture::OnColorChanged(QColor color)
{
    SwitchToCustom();
    if (m_moduleCapture)
    {
        m_moduleCapture->SetTargetColor(color);
    }
}

void DevFrameCapture::OnMeanChanged(double value)
{
    SwitchToCustom();
}

void DevFrameCapture::OnMousePressed(QPoint pos)
{
    if (!m_started) return;

    m_left->setValue(pos.x());
    m_top->setValue(pos.y());
}

void DevFrameCapture::OnMouseMoved(QPoint pos)
{
    if (!m_started) return;

    int const mode = m_mode->currentIndex();
    bool const isArea = (mode == 2 || mode == 3);

    if (isArea)
    {
        m_width->setValue(pos.x() - m_left->value());
        m_height->setValue(pos.y() - m_top->value());
    }
    else
    {
        OnMousePressed(pos);
    }
}

void DevFrameCapture::OnSave()
{
    QString const file = QFileDialog::getSaveFileName(m_btnSave, tr("Save Capture As"), CaptureHolder::GetDirectory(), "Custom Command (*" + CaptureHolder::GetFormat() + ")");
    if (file == Q_NULLPTR) return;

    QFileInfo const info(file);
    QString name = info.fileName();
    name = name.mid(0, name.size() - CaptureHolder::GetFormat().size());

    if (name == FRAME_CAPTURE_CUSTOM)
    {
        QMessageBox::critical(m_list, "Error", "This name is not allowed", QMessageBox::Ok);
        return;
    }

    int const mode = m_mode->currentIndex();
    bool const isArea = (mode == 2 || mode == 3);
    bool const isRange = (mode == 1 || mode == 3);
    bool const isColor = (mode == 0 || mode == 2);

    QJsonObject object;
    object.insert("Mode", m_mode->currentIndex());
    object.insert("Left", m_left->value());
    object.insert("Top", m_top->value());
    if (isArea)
    {
        object.insert("Width", m_width->value());
        object.insert("Height", m_height->value());
    }
    if (isRange)
    {
        object.insert("MinH", m_minH->value());
        object.insert("MinS", m_minS->value());
        object.insert("MinV", m_minV->value());
        object.insert("MaxH", m_maxH->value());
        object.insert("MaxS", m_maxS->value());
        object.insert("MaxV", m_maxV->value());
    }
    if (isColor)
    {
        object.insert("Color", (int)m_color->GetColor().rgb());
    }
    if (mode == 3)
    {
        object.insert("Mean", m_mean->value());
    }
    JsonHelper::WriteJson(file, object);

    if (m_list->findText(name) == -1)
    {
        m_list->addItem(name);
        m_list->model()->sort(0);
        m_list->setCurrentText(name);
    }
}

void DevFrameCapture::OnDelete()
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;
    resBtn = QMessageBox::warning(m_btnDelete, "Warning", "Are you sure you want to delete current capture?\nThis cannot be undone.", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        QFile::remove(CaptureHolder::GetDirectory() + m_list->currentText() + CaptureHolder::GetFormat());
        m_list->removeItem(m_list->currentIndex());
    }
}

void DevFrameCapture::OnOpenDirectory()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(CaptureHolder::GetDirectory()));
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

void DevFrameCapture::SwitchToCustom()
{
    m_list->setCurrentText(FRAME_CAPTURE_CUSTOM);
}

void DevFrameCapture::UpdateSettingEnabled()
{
    int const mode = m_mode->currentIndex();
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
    m_mean->setEnabled(mode == 3);
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
