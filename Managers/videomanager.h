#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QCamera>
#include <QCameraDevice>
#include <QComboBox>
#include <QElapsedTimer>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMetaEnum>
#include <QMessageBox>
#include <QMutex>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QShortcut>
#include <QTimer>
#include <QVideoSink>

namespace Ui { class MainWindow; }

class VideoManager : public QWidget
{
    Q_OBJECT

public:
    explicit VideoManager(QWidget* parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Video"; }
    void Initialize(Ui::MainWindow* ui);

    QString GetDeviceName() const;
    QSize GetResolution() const;

    void Start();
    void Stop();

    void PushFrameData(unsigned char const* data);
    QImage GetFrameData();

    void LoadSettings();
    void SaveSettings() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

    int heightForWidth(int width) const override;
    bool hasHeightForWidth() const override { return true; }

signals:
    void notifyDraw();

private slots:
    // UI
    void OnRefreshList();
    void OnDiscoverFinish(QStringList const& list);
    void OnDraw();
    void OnResize();

private:
    // UI
    void PopulateResolution();

private:
    // UI
    QComboBox*      m_listCamera = Q_NULLPTR;
    QComboBox*      m_listResolution = Q_NULLPTR;
    QPushButton*    m_btnCameraRefresh = Q_NULLPTR;
    QPushButton*    m_btnCameraStart = Q_NULLPTR;
    QString         m_defaultCamera;

    // Frame data
    QMutex  m_mutex;
    QImage  m_frame;

    // Overlays
    QTimer          m_resolutionTimer;
    bool            m_showFps = false;
    int             m_frameCount = 0;
    qreal           m_fps = 0.0;
    QElapsedTimer   m_fpsTimer;
};

#endif // VIDEOMANAGER_H
