#ifndef VLCMANAGER_H
#define VLCMANAGER_H

#include <vlc/vlc.h>

#include <QtConcurrent>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QMouseEvent>
#include <QMutex>
#include <QPushButton>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>

#include "../ui_mainwindow.h"
#include "Managers/managercollection.h"

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
    explicit VlcManager(QWidget* parent = nullptr) {};
    ~VlcManager();
    static QString GetTypeID() { return "VLC"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();
    bool HasVideo() const { return m_startVerified; }

protected:
    void closeEvent(QCloseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void notifyStateChanged();
    void notifyHasVideo();

private slots:
    void OnCameraClicked();
    void OnCameraStartTimeout();
    void OnAudioDisplayChanged(int index);
    void OnEventCallback();
    void OnScreenshot();

private:
    void LoadSettings();
    void SaveSettings() const;

    void Start();
    void Stop();

private:
    // LibVLC
    libvlc_instance_t*      m_instance = nullptr;
    libvlc_media_player_t*  m_mediaPlayer = nullptr;
    libvlc_media_t*         m_media = nullptr;

    // Custon video/audio context
    contextVideo ctxVideo;
    contextAudio ctxAudio;

    // Managers
    LogManager*     m_logManager = Q_NULLPTR;

    // UI
    QPushButton*    m_btnCameraStart = Q_NULLPTR;
    QPushButton*    m_btnScreenshot = Q_NULLPTR;
    QComboBox*      m_audioDisplay = Q_NULLPTR;

    // Members
    bool    m_started = false;
    bool    m_startVerified = false;
    QTimer  m_startVerifyTimer;
};

#endif // VLCMANAGER_H
