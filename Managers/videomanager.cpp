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
    connect(m_cameraList, &QComboBox::currentTextChanged, this, &VideoManager::OnCameraChanged);
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &VideoManager::OnRefreshList);

    OnRefreshList();
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
