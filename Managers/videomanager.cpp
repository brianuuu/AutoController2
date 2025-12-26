#include "videomanager.h"

#include "../ui_mainwindow.h"
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
    new QShortcut(QKeySequence("F1"), this, [this]{ m_showFps = !m_showFps; }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence("F2"), this, [this]{ m_showCaptureResult = !m_showCaptureResult; }, Qt::ApplicationShortcut);

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

    QMutexLocker captureLocker(&m_captureMutex);
    if (m_captureHolders.empty())
    {
        // we don't need m_frame anymore
        locker.unlock();
    }
    else
    {
        QSize const captrueRes = CaptureHolder::GetCaptureResolution();
        QImage const fram720p = (resolution == captrueRes) ? m_frame.copy() : m_frame.scaled(captrueRes);

        // we don't need m_frame anymore
        locker.unlock();

        // distribute frame data to captures
        for (CaptureHolder* holder : std::as_const(m_captureHolders))
        {
            holder->PushFrameData(fram720p);
        }
    }

    emit notifyDraw();
}

QImage VideoManager::GetFrameData() const
{
    QMutexLocker locker(&m_mutex);
    return m_frame.copy();
}

void VideoManager::RegisterCapture(CaptureHolder *holder)
{
    QMutexLocker locker(&m_captureMutex);
    m_captureHolders.insert(holder);
}

void VideoManager::UnregisterCapture(CaptureHolder *holder)
{
    QMutexLocker locker(&m_captureMutex);
    m_captureHolders.remove(holder);
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

        QVariant showCaptureResult;
        if (JsonHelper::ReadValue(settings, "ShowCaptureResult", showCaptureResult))
        {
            m_showCaptureResult = showCaptureResult.toBool();
        }
    }
}

void VideoManager::SaveSettings() const
{
    QJsonObject settings;
    settings.insert("CameraName", m_listCamera->currentText());
    settings.insert("Resolution", m_listResolution->currentText());
    settings.insert("ShowFPS", m_showFps);
    settings.insert("ShowCaptureResult", m_showCaptureResult);

    JsonHelper::WriteSetting("VideoSettings", settings);
}

void VideoManager::paintEvent(QPaintEvent *event)
{
    m_frameCount++;
    if (m_fpsTimer.elapsed() >= 1000)
    {
        m_fps = m_frameCount * 1000.0 / (qreal)m_fpsTimer.restart();
        m_frameCount = 0;
    }

    QRect const rect = this->rect();
    QPainter painter(this);
    painter.drawImage(rect, GetFrameData());

    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);

    // draw captures
    {
        qreal const scale = (qreal)width() / (qreal)CaptureHolder::GetCaptureResolution().width();

        QPen pen;
        pen.setWidth(2);

        QMutexLocker captureLocker(&m_captureMutex);
        for (CaptureHolder* holder : std::as_const(m_captureHolders))
        {
            pen.setColor(holder->GetDisplayColor());
            painter.setPen(pen);

            CaptureHolder::Mode const mode = holder->GetMode();
            if (mode == CaptureHolder::Mode::AreaColorMatch || mode == CaptureHolder::Mode::AreaRangeMatch)
            {
                QRect rect = holder->GetRect();
                rect = QRect(rect.topLeft() * scale, rect.size() * scale);

                if (m_showCaptureResult)
                {
                    QPoint const topLeft = rect.top() < 17 ? rect.bottomLeft() + QPoint(0,1) : rect.topLeft() - QPoint(0,17);
                    painter.fillRect(QRect(topLeft,QSize(55,16)), Qt::black);
                    if (mode == CaptureHolder::Mode::AreaRangeMatch)
                    {
                        painter.drawText(topLeft + QPoint(4,14), QString::number(holder->GetResultMean(), 'f', 4));
                        painter.drawImage(rect, holder->GetResultMasked());
                    }
                    else
                    {
                        painter.drawText(topLeft + QPoint(4,14), holder->GetResultMatched() ? "True" : "False");
                    }
                }

                painter.drawRect(rect);
            }
            else
            {
                // TODO: point test result
                QPoint const point = holder->GetPoint();
                painter.drawLine(point * scale + QPoint(7,7), point * scale + QPoint(-7,-7));
                painter.drawLine(point * scale + QPoint(-7,7), point * scale + QPoint(7,-7));
            }
        }
    }

    // draw fps
    if (m_showFps)
    {
        painter.fillRect(QRect(20,20,80,16), Qt::black);
        painter.setPen(Qt::white);
        painter.drawText(QPoint(24,34), "FPS: " + QString::number(m_fps, 'f', 2));
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
