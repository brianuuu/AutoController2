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
    this->resize(1280,720);
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
}

void VideoManager::paintEvent(QPaintEvent *event)
{
    // Draw current frame
    QPainter painter(this);
    painter.drawImage(this->rect(), m_frame);
}

void VideoManager::OnRefreshList()
{
    PopulateCameraList();
}

void VideoManager::OnCameraChanged(const QString &str)
{
    // a different camera is selected, populate resolution
    const QList<QCameraDevice> videoDevices = QMediaDevices::videoInputs();
    for (const QCameraDevice &device : videoDevices)
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
    const QList<QCameraDevice> videoDevices = QMediaDevices::videoInputs();
    for (const QCameraDevice &device : videoDevices)
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
                    m_btnCameraStart->setText("Stop Camera");
                    m_camera.setCameraDevice(device);
                    m_camera.setCameraFormat(format);
                    m_camera.start();

                    this->show();
                    return;
                }
            }
        }
    }
}

void VideoManager::Stop()
{
    m_btnCameraStart->setText("Start Camera");
    m_camera.stop();
    m_frame.fill(Qt::black);

    this->update();
    this->hide();
}
