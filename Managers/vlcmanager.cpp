#include "vlcmanager.h"

#define SCREENSHOT_PATH "../Screenshots/"

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

VlcManager::VlcManager
(
    LogManager* logManager,
    QComboBox *listCamera,
    QComboBox *listResolution,
    QComboBox *listAudioInput,
    QComboBox *listAudioOutput,
    QComboBox *listAudioDisplay,
    QSlider *volumeSlider,
    QPushButton *btnCameraRefresh,
    QPushButton *btnCameraStart,
    QPushButton* btnScreenshot,
    QWidget *parent
)
    : QWidget(parent)
    , m_logManager(logManager)
    , m_btnCameraStart(btnCameraStart)
    , m_btnScreenshot(btnScreenshot)
{
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

    // Video
    int constexpr MAX_WIDTH = 3840;
    int constexpr MAX_HEIGHT = 2160;
    ctxVideo.m_manager = new VideoManager(listCamera, listResolution, btnCameraRefresh, btnCameraStart, this);
    ctxVideo.m_pixels = new uchar[MAX_WIDTH * MAX_HEIGHT * 4];
    memset(ctxVideo.m_pixels, 0, MAX_WIDTH * MAX_HEIGHT * 4);

    // Audio
    ctxAudio.m_manager = new AudioManager(listAudioInput, listAudioOutput, listAudioDisplay, volumeSlider, this);

    // connections
    connect(m_btnCameraStart, &QPushButton::clicked, this, &VlcManager::OnCameraClicked);
    connect(m_btnScreenshot, &QPushButton::clicked, this, &VlcManager::OnScreenshot);
    connect(&m_startVerifyTimer, &QTimer::timeout, this, &VlcManager::OnCameraStartTimeout);

    // Setup layout
    this->setWindowTitle("Media View");
    this->resize(1280,720);
    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    //vBoxLayout->addWidget(ctxAudio.m_manager);
    vBoxLayout->addWidget(ctxVideo.m_manager);
    vBoxLayout->addItem(new QSpacerItem(20, 440, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vBoxLayout->setContentsMargins(0,0,0,0);
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

    return true;
}

void VlcManager::closeEvent(QCloseEvent *event)
{
    if (m_started)
    {
        Stop();
    }

    QWidget::closeEvent(event);
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
    if (!m_started) return;

    libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
    switch (state)
    {
    case libvlc_Opening:
    {
        // still opening, wait
        m_startVerifyTimer.start(3000);
        break;
    }
    default:
    {
        if (state == libvlc_Playing && ctxVideo.m_manager->HasFrameData())
        {
            // success
            m_btnCameraStart->setText("Stop Camera");
            m_btnCameraStart->setEnabled(true);
            m_btnScreenshot->setEnabled(true);
            m_logManager->PrintLog("System", "Camera ON confirmed", LOG_Success);
            break;
        }

        // state not expected
        m_logManager->PrintLog("System", "Failed to start camera", LOG_Error);
        QMessageBox::critical(this, "Error",
            "Failed to start camera!\n"
            "1. Chroma may not be supported for current resolution\n"
            "2. Resolution not matching source (required for OBS Virtual Camera etc.)",
            QMessageBox::Ok
        );
        Stop();
        break;
    }
    }
}

void VlcManager::OnScreenshot()
{
    QString const nameWithTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + "_screenshot.png";
    QImage const frame = ctxVideo.m_manager->GetFrameData();
    frame.save(SCREENSHOT_PATH + nameWithTime);

    m_logManager->PrintLog("System", QDir(SCREENSHOT_PATH).absolutePath() + nameWithTime);
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
        m_logManager->PrintLog("System", "Failed to start camera", LOG_Error);
        QMessageBox::critical(this, "Error", "Unable to start VLC media player!", QMessageBox::Ok);
    }
    else
    {
        m_started = true;
        m_startVerifyTimer.start(3000);

        m_btnCameraStart->setText("Starting...");
        m_btnCameraStart->setEnabled(false);

        ctxVideo.m_manager->Start();
        ctxAudio.m_manager->Start();

        libvlc_video_set_adjust_int(m_mediaPlayer, libvlc_video_adjust_option_t::libvlc_adjust_Enable, true);
        m_logManager->PrintLog("System", "Starting camera...");

        this->show();
    }
}

void VlcManager::Stop()
{
    m_started = false;
    m_startVerifyTimer.stop();

    libvlc_media_player_stop(m_mediaPlayer);
    m_logManager->PrintLog("System", "Camera OFF");

    m_btnCameraStart->setText("Start Camera");
    m_btnCameraStart->setEnabled(true);
    m_btnScreenshot->setEnabled(false);

    ctxVideo.m_manager->Stop();
    ctxAudio.m_manager->Stop();

    this->hide();
}
