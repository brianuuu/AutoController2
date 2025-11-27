#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QCamera>
#include <QCameraDevice>
#include <QComboBox>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMetaEnum>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVideoSink>

class VideoManager : public QWidget
{
    Q_OBJECT

public:
    VideoManager
    (
        QComboBox* listCamera,
        QComboBox* listResolution,
        QPushButton* btnCameraRefresh,
        QPushButton* btnCameraStart,
        QWidget* parent = nullptr
    );

    QString GetDeviceName() const;
    QSize GetResolution() const;

    void Start();
    void Stop();

    void PushFrameData(unsigned char const* data);

protected:
    void paintEvent(QPaintEvent* event) override;

signals:
    void notifyDraw();

private slots:
    // UI
    void OnRefreshList();
    void OnDiscovereFinish(QStringList const& list);
    void OnDraw();

private:
    // UI
    void PopulateResolution();

private:
    // UI
    QComboBox*      m_listCamera = Q_NULLPTR;
    QComboBox*      m_listResolution = Q_NULLPTR;
    QPushButton*    m_btnCameraRefresh = Q_NULLPTR;
    QPushButton*    m_btnCameraStart = Q_NULLPTR;

    // Frame data
    std::atomic<bool>   m_useBackBuffer = false;
    QImage              m_frame[2];
};

#endif // VIDEOMANAGER_H
