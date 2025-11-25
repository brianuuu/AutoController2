#include "videomanager.h"

VideoManager::VideoManager
(
    QComboBox* listCamera,
    QComboBox* listResolution,
    QPushButton* btnCameraStart,
    QWidget* parent
)
    : QWidget(parent)
    , m_listCamera(listCamera)
    , m_listResolution(listResolution)
    , m_btnCameraStart(btnCameraStart)
{
    connect(m_listCamera, &QComboBox::currentTextChanged, this, &VideoManager::OnCameraChanged);
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &VideoManager::OnRefreshList);

    OnRefreshList();
}

QString VideoManager::GetDeviceName() const
{
    return m_listCamera->currentText();
}

QStringList VideoManager::GetResolutionData() const
{
    // Width|Height|fps
    return m_listResolution->currentData().toString().split('|');
}

void VideoManager::Start()
{
    m_listCamera->setEnabled(false);
    m_listResolution->setEnabled(false);
}

void VideoManager::Stop()
{
    m_listCamera->setEnabled(true);
    m_listResolution->setEnabled(true);
}

void VideoManager::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
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

void VideoManager::PopulateCameraList()
{
    QString const previousCamera = m_listCamera->currentText();
    QString const previousResolution = m_listResolution->currentText();

    m_listCamera->clear();

    bool foundPreviousCamera = false;
    for (const QCameraDevice &device : QMediaDevices::videoInputs())
    {
        m_listCamera->addItem(device.description());
        if (!foundPreviousCamera && device.description() == previousCamera)
        {
            foundPreviousCamera = true;
        }
    }

    // keep the previous camera and resolution
    if (foundPreviousCamera)
    {
        m_listCamera->setCurrentText(previousCamera);
        m_listResolution->setCurrentText(previousResolution);
    }
}

void VideoManager::PopulateResolution(const QCameraDevice &device)
{
    m_listResolution->clear();

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
        float const fps = format.maxFrameRate();
        if (fps != 30 && fps != 60)
        {
            continue;
        }

        QString const resItem = QString::number(resolution.width()) + "x" + QString::number(resolution.height()) + " (" + QString::number(fps) + " fps)";
        QString const resData = QString::number(resolution.width()) + "|" + QString::number(resolution.height()) + "|" + QString::number(fps);

        // only allow 16:9, ignore pixel format choice which has duplicated resData
        if (aspectRatioInverse == 0.5625 && !resDataList.contains(resData))
        {
            resDataList.insert(resData);
            m_listResolution->addItem(resItem, resData);
        }
    }

    // only allow camera start if there are available resolutions
    m_btnCameraStart->setEnabled(m_listResolution->count());
}
