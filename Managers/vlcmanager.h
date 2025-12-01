#ifndef VLCMANAGER_H
#define VLCMANAGER_H

#include <vlc/vlc.h>

#include <QtConcurrent>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QPushButton>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "logmanager.h"
#include "audiomanager.h"
#include "videomanager.h"

struct contextVideo
{
    QMutex m_mutex;
    uchar *m_pixels;

    VideoManager* m_manager;
};

struct contextAudio
{
    AudioManager* m_manager;
};

class VlcManager : public QWidget
{
    Q_OBJECT
public:
    VlcManager
    (
        LogManager* logManager,
        QComboBox* listCamera,
        QComboBox* listResolution,
        QComboBox* listAudioInput,
        QComboBox* listAudioOutput,
        QComboBox* listAudioDisplay,
        QSlider* volumeSlider,
        QPushButton* btnCameraRefresh,
        QPushButton* btnCameraStart,
        QPushButton* btnScreenshot,
        QWidget *parent = nullptr
    );
    ~VlcManager();

    bool OnCloseEvent();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void OnCameraClicked();
    void OnCameraStartTimeout();
    void OnScreenshot();

private:
    void Start();
    void Stop();

private:
    // LibVLC
    libvlc_instance_t*      m_instance = nullptr;
    libvlc_media_player_t*  m_mediaPlayer = nullptr;
    libvlc_media_t*         m_media = nullptr;
    libvlc_state_t          m_state = libvlc_NothingSpecial;

    // Custon video/audio context
    struct contextVideo ctxVideo;
    struct contextAudio ctxAudio;

    // UI
    LogManager*     m_logManager = Q_NULLPTR;
    QPushButton*    m_btnCameraStart = Q_NULLPTR;
    QPushButton*    m_btnScreenshot = Q_NULLPTR;

    // Members
    bool    m_started = false;
    QTimer  m_startVerifyTimer;
};

#endif // VLCMANAGER_H
