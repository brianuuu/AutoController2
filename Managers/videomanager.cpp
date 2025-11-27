#include "videomanager.h"

#include "Helpers/mediadiscoverer.h"

VideoManager::VideoManager
(
    QComboBox* listCamera,
    QComboBox* listResolution,
    QPushButton* btnCameraRefresh,
    QPushButton* btnCameraStart,
    QWidget* parent
)
    : QWidget(parent)
    , m_listCamera(listCamera)
    , m_listResolution(listResolution)
    , m_btnCameraRefresh(btnCameraRefresh)
    , m_btnCameraStart(btnCameraStart)
{
    connect(m_btnCameraRefresh, &QPushButton::clicked, this, &VideoManager::OnRefreshList);
    connect(this, &VideoManager::notifyDraw, this, &VideoManager::OnDraw);

    OnRefreshList();
    PopulateResolution();
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

    QSize const resolution = GetResolution();
    m_frame[0] = QImage(resolution, QImage::Format_ARGB32);
    m_frame[1] = QImage(resolution, QImage::Format_ARGB32);
    m_frame[0].fill(Qt::black);
    m_frame[1].fill(Qt::black);
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
    QSize const resolution = GetResolution();
    QImage& freeFrame = m_useBackBuffer.load() ? m_frame[0] : m_frame[1];
    freeFrame = QImage(data, resolution.width(), resolution.height(), QImage::Format_ARGB32);

    emit notifyDraw();
}

void VideoManager::paintEvent(QPaintEvent *event)
{
    // swap buffer
    m_useBackBuffer.store(!m_useBackBuffer.load());

    QPainter painter(this);
    painter.drawImage(this->rect(), m_useBackBuffer.load() ? m_frame[1] : m_frame[0]);
}

void VideoManager::OnRefreshList()
{
    MediaDiscoverer* discoverer = new MediaDiscoverer(false, this);
    connect(discoverer, &MediaDiscoverer::finished, this, &VideoManager::OnDiscovereFinish);
    connect(discoverer, &MediaDiscoverer::finished, discoverer, &MediaDiscoverer::deleteLater);
    discoverer->start();
}

void VideoManager::OnDiscovereFinish(const QStringList &list)
{
    bool foundPreviousCamera = false;
    QString const previousCamera = m_listCamera->currentText();

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
