#include "videomanager.h"

#include "Helpers/jsonhelper.h"
#include "Helpers/mediadiscoverer.h"

void VideoManager::Initialize(Ui::MainWindow *ui)
{
    m_listCamera = ui->CB_CameraDevice;
    m_listResolution = ui->CB_Resolution;
    m_btnCameraRefresh = ui->PB_CameraRefresh;
    m_btnCameraStart = ui->PB_CameraStart;

    connect(m_btnCameraRefresh, &QPushButton::clicked, this, &VideoManager::OnRefreshList);
    connect(this, &VideoManager::notifyDraw, this, &VideoManager::OnDraw);

    m_resolutionTimer.setSingleShot(true);
    QShortcut *shortcut = new QShortcut(QKeySequence("F1"), this);
    connect(shortcut, &QShortcut::activated, this, [this]{ m_showFps = !m_showFps; });

    OnRefreshList();
    PopulateResolution();

    this->resize(1280, 720);
}

QString VideoManager::GetDeviceName() const
{
    return m_listCamera->currentText();
}

QSize VideoManager::GetResolution() const
{
    return m_listResolution->currentData().toSize();
}

void VideoManager::Start()
{
    m_listCamera->setEnabled(false);
    m_listResolution->setEnabled(false);
    m_btnCameraRefresh->setEnabled(false);

    OnResize();
    m_fpsTimer.start();

    QSize const resolution = GetResolution();
    m_frame = QImage(resolution, QImage::Format_ARGB32);
    m_frame.fill(Qt::black);
    this->update();
}

void VideoManager::Stop()
{
    m_listCamera->setEnabled(true);
    m_listResolution->setEnabled(true);
    m_btnCameraRefresh->setEnabled(true);
}

void VideoManager::PushFrameData(const unsigned char *data)
{
    // this is called from LibVLC thread, not thread safe
    QSize const resolution = GetResolution();

    QMutexLocker locker(&m_mutex);
    m_frame = QImage(data, resolution.width(), resolution.height(), QImage::Format_ARGB32);

    emit notifyDraw();
}

QImage VideoManager::GetFrameData()
{
    QMutexLocker locker(&m_mutex);
    return m_frame.copy();
}

void VideoManager::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("VideoSettings");
    {
        QVariant cameraName;
        if (JsonHelper::ReadValue(settings, "CameraName", cameraName))
        {
            // cannot set until MediaDiscoverer finished
            m_defaultCamera = cameraName.toString();
        }

        QVariant resolution;
        if (JsonHelper::ReadValue(settings, "Resolution", resolution) && !resolution.toString().isEmpty())
        {
            m_listResolution->setCurrentText(resolution.toString());
        }

        QVariant showFPS;
        if (JsonHelper::ReadValue(settings, "ShowFPS", showFPS))
        {
            m_showFps = showFPS.toBool();
        }
    }
}

void VideoManager::SaveSettings() const
{
    QJsonObject settings;
    settings.insert("CameraName", m_listCamera->currentText());
    settings.insert("Resolution", m_listResolution->currentText());
    settings.insert("ShowFPS", m_showFps);

    JsonHelper::WriteSetting("VideoSettings", settings);
}

void VideoManager::paintEvent(QPaintEvent *event)
{
    m_frameCount++;
    if (m_fpsTimer.elapsed() >= 500)
    {
        m_fps = m_frameCount * 1000.0 / (qreal)m_fpsTimer.restart();
        m_frameCount = 0;
    }

    QRect const rect = this->rect();
    QPainter painter(this);
    painter.drawImage(rect, GetFrameData());

    // draw fps
    if (m_showFps)
    {
        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);

        painter.fillRect(QRect(20,20,80,15), Qt::black);
        painter.setPen(Qt::white);
        painter.drawText(QPoint(24,33), "FPS: " + QString::number(m_fps, 'f', 2));
    }

    // draw display size
    if (m_resolutionTimer.isActive())
    {
        QFont font = painter.font();
        font.setPointSize(20);
        painter.setFont(font);

        QString const display = "Display Size: " + QString::number(rect.width()) + "x" + QString::number(rect.height());
        painter.setPen(Qt::black);
        painter.drawText(QPoint(6,31), display);
        painter.setPen(Qt::white);
        painter.drawText(QPoint(5,30), display);
    }
}

void VideoManager::resizeEvent(QResizeEvent *event)
{
    OnResize();
    QWidget::resizeEvent(event);
}

int VideoManager::heightForWidth(int width) const
{
    return width * 9 / 16;
}

void VideoManager::OnRefreshList()
{
    MediaDiscoverer* discoverer = new MediaDiscoverer(false, this);
    connect(discoverer, &MediaDiscoverer::finished, this, &VideoManager::OnDiscoverFinish);
    connect(discoverer, &MediaDiscoverer::finished, discoverer, &MediaDiscoverer::deleteLater);
    discoverer->start();
}

void VideoManager::OnDiscoverFinish(const QStringList &list)
{
    bool foundPreviousCamera = false;
    QString const previousCamera = m_defaultCamera.isEmpty() ? m_listCamera->currentText() : m_defaultCamera;
    m_defaultCamera.clear();

    m_listCamera->clear();
    for (QString const& str : list)
    {
        m_listCamera->addItem(str);
        if (!foundPreviousCamera && str == previousCamera)
        {
            foundPreviousCamera = true;
        }
    }

    // keep the previous camera and resolution
    if (foundPreviousCamera)
    {
        m_listCamera->setCurrentText(previousCamera);
    }

    // only allow camera start if there are available cameras
    m_btnCameraStart->setEnabled(m_listCamera->count());
}

void VideoManager::OnDraw()
{
    this->update();
}

void VideoManager::OnResize()
{
    m_resolutionTimer.start(2000);
}

void VideoManager::PopulateResolution()
{
    QVector<QSize> const list = {
        {1280, 720},
        {1920, 1080},
        {2560, 1440},
        {3840, 2160},
    };

    m_listResolution->clear();
    for (QSize const& size : list)
    {
        QString const str = QString::number(size.width()) + "x" + QString::number(size.height());
        m_listResolution->addItem(str, size);
    }

    // default 1920x1080
    m_listResolution->setCurrentIndex(1);
}
