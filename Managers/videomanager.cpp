#include "videomanager.h"

VideoManager::VideoManager
(
    QWidget* parent,
    QComboBox* cameraList,
    QComboBox* resolutionList,
    QPushButton* btnCameraStart
)
    : m_cameraList(cameraList)
    , m_resolutionList(resolutionList)
    , m_btnCameraStart(btnCameraStart)
{
    m_displaySize = QSize(1280,720);
    this->resize(m_displaySize);
    this->setWindowTitle("Camera View");

    m_capture.setCamera(&m_camera);
    m_capture.setVideoSink(&m_sink);

    connect(m_cameraList, &QComboBox::currentTextChanged, this, &VideoManager::OnCameraChanged);
    connect(m_btnCameraStart, &QPushButton::clicked, this, &VideoManager::OnCameraClicked);
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &VideoManager::OnRefreshList);
    connect(&m_sink, &QVideoSink::videoFrameChanged, this, &VideoManager::OnFrameChanged);

    OnRefreshList();
}

bool VideoManager::OnCloseEvent()
{
    // main window is closing
    if (m_camera.isActive())
    {
        Stop();
    }
    return true;
}

void VideoManager::closeEvent(QCloseEvent *event)
{
    if (m_camera.isActive())
    {
        Stop();
    }

    QWidget::closeEvent(event);
}

void VideoManager::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Keep aspect ratio of the camera tp 16:9
    QRect drawRect = this->rect();
    if (this->width() * 9 / 16 > this->height())
    {
        int const expectedWidth = this->height() * 16 / 9;
        drawRect.setLeft((this->width() - expectedWidth) / 2);
        drawRect.setWidth(expectedWidth);
    }
    else if (this->height() * 16 / 9 > this->width())
    {
        int const expectedHeight = this->width() * 9 / 16;
        drawRect.setTop((this->height() - expectedHeight) / 2);
        drawRect.setHeight(expectedHeight);
    }

    // Draw current frame
    painter.drawImage(drawRect, m_frame);
    m_displaySize = drawRect.size();
}

void VideoManager::mousePressEvent(QMouseEvent *event)
{
    static QVector<QSize> fixedSizes =
    {
        { 640, 360 },
        { 960, 540 },
        { 1280, 720 },
        { 1920, 1080 },
        { 2560, 1440 },
    };

    if (event->button() == Qt::LeftButton)
    {
        // scale up
        for (QSize const size : fixedSizes)
        {
            if (size.width() > m_displaySize.width())
            {
                resize(size);
                break;
            }
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        // scale down
        for (int i = fixedSizes.size() - 1; i >= 0 ; i--)
        {
            QSize const& size = fixedSizes.at(i);
            if (size.width() < m_displaySize.width())
            {
                resize(size);
                break;
            }
        }
    }

    QWidget::mousePressEvent(event);
}

void VideoManager::OnRefreshList()
{
    PopulateCameraList();
}

void VideoManager::OnCameraChanged(const QString &str)
{
    // a different camera is selected, populate resolution
    for (const QCameraDevice &device : QMediaDevices::videoInputs())
    {
        if (device.description() == str)
        {
            PopulateResolution(device);
            return;
        }
    }

    // if we are here there's no camera available
    m_btnCameraStart->setEnabled(false);
}

void VideoManager::OnCameraClicked()
{
    if (m_camera.isActive())
    {
        Stop();
    }
    else
    {
        Start();
    }
}

void VideoManager::OnFrameChanged(const QVideoFrame &frame)
{
    m_frame = frame.toImage();
    this->update();
}

void VideoManager::PopulateCameraList()
{
    QString const previousCamera = m_cameraList->currentText();
    QString const previousResolution = m_resolutionList->currentText();

    m_cameraList->clear();

    bool foundPreviousCamera = false;
    for (const QCameraDevice &device : QMediaDevices::videoInputs())
    {
        m_cameraList->addItem(device.description());
        if (!foundPreviousCamera && device.description() == previousCamera)
        {
            foundPreviousCamera = true;
        }
    }

    // keep the previous camera and resolution
    if (foundPreviousCamera)
    {
        m_cameraList->setCurrentText(previousCamera);
        m_resolutionList->setCurrentText(previousResolution);
    }
}

void VideoManager::PopulateResolution(const QCameraDevice &device)
{
    m_resolutionList->clear();

    // sort by resolution then framerate
    QList<QCameraFormat> formats = device.videoFormats();
    std::sort(formats.begin(), formats.end(),
        [] (QCameraFormat const& a, QCameraFormat const& b)
        {
            if (a.resolution().width() > b.resolution().width()) return true;
            if (a.resolution().width() < b.resolution().width()) return false;
            if (a.resolution().height() > b.resolution().height()) return true;
            if (a.resolution().height() < b.resolution().height()) return false;
            return a.maxFrameRate() > b.maxFrameRate();
        }
    );

    QSet<QString> resDataList;
    for (auto const& format : std::as_const(formats))
    {
        QSize const resolution = format.resolution();
        qreal const aspectRatioInverse = (qreal)resolution.height() / (qreal)resolution.width();

        QString const resItem = QString::number(resolution.width()) + "x" + QString::number(resolution.height()) + " (" + QString::number(format.maxFrameRate()) + " fps)";
        QString const resData = QString::number(resolution.width()) + "|" + QString::number(resolution.height()) + "|" + QString::number(format.maxFrameRate());

        // only allow 16:9, ignore pixel format choice which has duplicated resData
        if (aspectRatioInverse == 0.5625 && !resDataList.contains(resData))
        {
            resDataList.insert(resData);
            m_resolutionList->addItem(resItem, resData);
        }
    }

    // only allow camera start if there are available resolutions
    m_btnCameraStart->setEnabled(m_resolutionList->count());
}

void VideoManager::Start()
{
    // find QCameraDevice
    QString const currentCamera = m_cameraList->currentText();
    for (const QCameraDevice &device : QMediaDevices::videoInputs())
    {
        if (device.description() == currentCamera)
        {
            // find QCameraFormat
            QStringList const resData = m_resolutionList->currentData().toString().split('|');
            QSize const size(resData[0].toInt(),resData[1].toInt());
            int const fps = resData[2].toInt();

            for (auto const& format : device.videoFormats())
            {
                if (format.resolution() == size && format.maxFrameRate() == fps)
                {
                    m_cameraList->setEnabled(false);
                    m_resolutionList->setEnabled(false);
                    m_btnCameraStart->setText("Stop Camera");

                    m_camera.setCameraDevice(device);
                    m_camera.setCameraFormat(format);
                    m_camera.start();

                    m_frame = QImage(format.resolution(), QImage::Format_RGBA8888_Premultiplied);
                    m_frame.fill(Qt::black);

                    this->show();
                    return;
                }
            }
        }
    }

    QMessageBox::critical(this, "Error", "Unable to start camera!", QMessageBox::Ok);
}

void VideoManager::Stop()
{
    m_cameraList->setEnabled(true);
    m_resolutionList->setEnabled(true);
    m_btnCameraStart->setText("Start Camera");
    m_camera.stop();
    m_frame.fill(Qt::black);

    this->update();
    this->hide();
}
