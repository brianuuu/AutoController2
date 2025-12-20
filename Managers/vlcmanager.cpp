#include "vlcmanager.h"

#include "Helpers/jsonhelper.h"
#include "Managers/logmanager.h"
#include "Managers/audiomanager.h"
#include "Managers/videomanager.h"

#define SCREENSHOT_PATH "../Screenshots/"
#define ERROR_TEXT \
"1. Camera may be occupied by another application\n" \
"2. Chroma may not be supported for current resolution\n" \
"3. Resolution not matching source (required for OBS Virtual Camera etc.)"

static void* cbVideoLock(void *opaque, void **planes)
{
    struct contextVideo *ctx = (contextVideo *)opaque;
    ctx->m_mutex.lock();

    // tell VLC to put the decoded data in the buffer
    *planes = ctx->m_pixels;
    return nullptr;
}

// get the argb image and save it to a file
static void cbVideoUnlock(void *opaque, void *picture, void *const *planes)
{
    struct contextVideo *ctx = (contextVideo *)opaque;
    unsigned char const* data = (unsigned char const*)*planes;

    ctx->m_manager->PushFrameData(data);
    ctx->m_mutex.unlock();
}

static void cbAudioPlay(void* p_audio_data, const void *samples, unsigned int count, int64_t pts)
{
    struct contextAudio *ctx = (contextAudio *)p_audio_data;
    if (!ctx->m_manager) return;

    // Pass new raw data to manager
    ctx->m_manager->PushAudioData(samples, count, pts);
}

static void eventCallbacks(const libvlc_event_t* event, void* ptr)
{
    VlcManager* parent = (VlcManager*)ptr;
    emit parent->notifyStateChanged();
}

void VlcManager::Initialize(Ui::MainWindow *ui)
{
    m_logManager = ManagerCollection::GetManager<LogManager>();
    m_btnCameraStart = ui->PB_CameraStart;
    m_btnScreenshot = ui->PB_Screenshot;

    if (!QDir(SCREENSHOT_PATH).exists())
    {
        QDir().mkdir(SCREENSHOT_PATH);
    }

    const char* const vlc_args[] = {
        "--intf", "dummy",
        "--vout", "dummy",
    };
    m_instance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    m_mediaPlayer = libvlc_media_player_new(m_instance);

    // Event callbacks
    libvlc_event_manager_t* em = libvlc_media_player_event_manager(m_mediaPlayer);
    if (em)
    {
        libvlc_event_attach(em, libvlc_MediaPlayerPlaying, eventCallbacks, this);
        libvlc_event_attach(em, libvlc_MediaPlayerEncounteredError, eventCallbacks, this);
    }

    // Video
    int constexpr MAX_WIDTH = 3840;
    int constexpr MAX_HEIGHT = 2160;
    ctxVideo.m_manager = ManagerCollection::AddManager<VideoManager>(this);
    ctxVideo.m_manager->Initialize(ui);
    ctxVideo.m_pixels = new uchar[MAX_WIDTH * MAX_HEIGHT * 4];
    memset(ctxVideo.m_pixels, 0, MAX_WIDTH * MAX_HEIGHT * 4);

    // Audio
    ctxAudio.m_manager = ManagerCollection::AddManager<AudioManager>(this);
    ctxAudio.m_manager->Initialize(ui);

    // connections
    connect(m_btnCameraStart, &QPushButton::clicked, this, &VlcManager::OnCameraClicked);
    connect(m_btnScreenshot, &QPushButton::clicked, this, &VlcManager::OnScreenshot);
    connect(&m_startVerifyTimer, &QTimer::timeout, this, &VlcManager::OnCameraStartTimeout);
    connect(ctxVideo.m_manager, &VideoManager::notifyDraw, this, &VlcManager::OnCameraStartTimeout);
    connect(this, &VlcManager::notifyStateChanged, this, &VlcManager::OnEventCallback);

    // Setup layout
    this->setWindowTitle("Media View");
    this->resize(1280,720);
    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    //vBoxLayout->addWidget(ctxAudio.m_manager);
    vBoxLayout->addWidget(ctxVideo.m_manager);
    vBoxLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vBoxLayout->setContentsMargins(0,0,0,0);

    LoadSettings();
}

VlcManager::~VlcManager()
{
    delete[] ctxVideo.m_pixels;

    libvlc_media_player_release(m_mediaPlayer);
    libvlc_release(m_instance);
}

bool VlcManager::OnCloseEvent()
{
    if (m_started)
    {
        Stop();
    }

    SaveSettings();
    return true;
}

void VlcManager::LoadSettings()
{
    ctxVideo.m_manager->LoadSettings();
    ctxAudio.m_manager->LoadSettings();

    QJsonObject settings = JsonHelper::ReadSetting("MediaView");
    {
        QJsonObject windowSize = JsonHelper::ReadObject(settings, "WindowSize");

        QVariant x, y;
        if (JsonHelper::ReadValue(windowSize, "X", x) && JsonHelper::ReadValue(windowSize, "Y", y))
        {
            this->move(x.toInt(), y.toInt());
        }

        QVariant width, height;
        if (JsonHelper::ReadValue(windowSize, "Width", width) && JsonHelper::ReadValue(windowSize, "Height", height))
        {
            this->resize(width.toInt(), height.toInt());
        }
    }
}

void VlcManager::SaveSettings() const
{
    ctxVideo.m_manager->SaveSettings();
    ctxAudio.m_manager->SaveSettings();
    QJsonObject windowSize;
    windowSize.insert("Width", this->width());
    windowSize.insert("Height", this->height());
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject settings;
    settings.insert("WindowSize", windowSize);

    JsonHelper::WriteSetting("MediaView", settings);
}

void VlcManager::closeEvent(QCloseEvent *event)
{
    if (m_started)
    {
        Stop();
    }

    QWidget::closeEvent(event);
}

void VlcManager::wheelEvent(QWheelEvent *event)
{
    QVector<QSize> const list = {
        {640, 360},
        {960, 540},
        {1280, 720},
        {1920, 1080},
        {2560, 1440},
        {3840, 2160},
    };

    int const width = this->size().width();
    int const amount = event->angleDelta().y();
    if (amount > 0)
    {
        for (int i = 0; i < list.size(); i++)
        {
            QSize const& size = list.at(i);
            if (width < size.width())
            {
                this->resize(size);
                break;
            }
        }
    }
    else if (amount < 0)
    {
        for (int i = list.size() - 1; i >= 0; i--)
        {
            QSize const& size = list.at(i);
            if (width > size.width())
            {
                this->resize(size);
                break;
            }
        }
    }

    QWidget::wheelEvent(event);
}

void VlcManager::OnCameraClicked()
{
    if (m_started)
    {
        Stop();
    }
    else
    {
        Start();
    }
}

void VlcManager::OnCameraStartTimeout()
{
    if (!m_started || m_startVerified) return;

    if (m_startVerifyTimer.isActive())
    {
        // got camera data before timeout
        m_startVerified = true;
        m_startVerifyTimer.stop();
        emit notifyHasVideo();

        m_btnCameraStart->setText("Stop Camera");
        m_btnCameraStart->setEnabled(true);
        m_btnScreenshot->setEnabled(true);

        m_logManager->PrintLog("Global", "Camera ON", LOG_Success);
    }
    else
    {
        // no feedback detected
        m_logManager->PrintLog("Global", "No video feedback detected", LOG_Error);
        QMessageBox::critical(this, "Error", QString("No video feedback detected!\n") + ERROR_TEXT, QMessageBox::Ok);
        Stop();
    }
}

void VlcManager::OnEventCallback()
{
    if (!m_started) return;

    libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
    switch (state)
    {
    case libvlc_Playing:
    {
        // give some time to check if there are any video/audio feedback
        m_startVerifyTimer.setSingleShot(true);
        m_startVerifyTimer.start(2000);
        break;
    }
    default:
    {
        // state not expected
        m_logManager->PrintLog("Global", "Failed to start camera", LOG_Error);
        QMessageBox::critical(this, "Error", QString("Failed to start camera!\n") + ERROR_TEXT, QMessageBox::Ok);
        Stop();
        break;
    }
    }
}

void VlcManager::OnScreenshot()
{
    QString const nameWithTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + "_screenshot.png";

    auto future = QtConcurrent::run([this, nameWithTime]{
        ctxVideo.m_manager->GetFrameData().save(SCREENSHOT_PATH + nameWithTime);
    });

    m_logManager->PrintLog("Global", "Screenshot saved: " + QDir(SCREENSHOT_PATH).absolutePath() + nameWithTime);
}

void VlcManager::Start()
{
    m_media = libvlc_media_new_location(m_instance, "dshow://");

    // Video
    QString vdevOption = ":dshow-vdev=";
    QString const vdev = ctxVideo.m_manager->GetDeviceName();
    vdevOption += vdev.isEmpty() ? "none" : vdev;
    libvlc_media_add_option(m_media, vdevOption.toStdString().c_str());

    // Audio
    QString adevOption = ":dshow-adev=";
    QString const adev = ctxAudio.m_manager->GetDeviceName();
    adevOption += adev.isEmpty() ? "none" : adev;
    libvlc_media_add_option(m_media, adevOption.toStdString().c_str());

    // Aspect ratio, resolution
    QSize const resolution = ctxVideo.m_manager->GetResolution();
    QString const dshowSize = ":dshow-size=" + QString::number(resolution.width()) + "x" + QString::number(resolution.height());
    libvlc_media_add_option(m_media, dshowSize.toStdString().c_str());
    libvlc_media_add_option(m_media, ":dshow-aspect-ratio=16:9");

    // Frame rate
    //QString const frameRate = ":dshow-fps=" + QString::number(fps);
    //libvlc_media_add_option(m_media, frameRate.toStdString().c_str());

    // Audio samples
    libvlc_media_add_option(m_media, ":dshow-audio-samplerate=48000");
    libvlc_media_add_option(m_media, ":dshow-audio-bitspersample=16");

    // Caching
    libvlc_media_add_option(m_media, ":live-caching=0");

    // Pass to player and release media
    libvlc_media_player_set_media(m_mediaPlayer, m_media);
    libvlc_media_release(m_media);

    // Set the callback to extract the frame or display it on the screen
    libvlc_video_set_callbacks(m_mediaPlayer, cbVideoLock, cbVideoUnlock, nullptr, &ctxVideo);
    libvlc_video_set_format(m_mediaPlayer, "BGRA", resolution.width(), resolution.height(), resolution.width() * 4);

    // Set callback to extract raw PCM data
    QAudioFormat const format = ctxAudio.m_manager->GetAudioFormat();
    libvlc_audio_set_callbacks(m_mediaPlayer, cbAudioPlay, nullptr, nullptr, nullptr, nullptr, &ctxAudio);
    libvlc_audio_set_format(m_mediaPlayer, "S16N", format.sampleRate(), format.channelCount());

    // Play media
    int result = libvlc_media_player_play(m_mediaPlayer);
    if (result == -1)
    {
        m_logManager->PrintLog("Global", "Failed to start camera", LOG_Error);
        QMessageBox::critical(this, "Error", "Unable to start VLC media player!", QMessageBox::Ok);
    }
    else
    {
        m_started = true;
        m_startVerified = false;
        emit notifyHasVideo();

        m_btnCameraStart->setText("Starting...");
        m_btnCameraStart->setEnabled(false);

        ctxVideo.m_manager->Start();
        ctxAudio.m_manager->Start();

        libvlc_video_set_adjust_int(m_mediaPlayer, libvlc_video_adjust_option_t::libvlc_adjust_Enable, true);
        m_logManager->PrintLog("Global", "Starting camera...");

        this->show();
    }
}

void VlcManager::Stop()
{
    m_started = false;
    m_startVerified = false;
    m_startVerifyTimer.stop();
    emit notifyHasVideo();

    libvlc_media_player_stop(m_mediaPlayer);
    m_logManager->PrintLog("Global", "Camera OFF", LOG_Warning);

    m_btnCameraStart->setText("Start Camera");
    m_btnCameraStart->setEnabled(true);
    m_btnScreenshot->setEnabled(false);

    ctxVideo.m_manager->Stop();
    ctxAudio.m_manager->Stop();

    this->hide();
}
