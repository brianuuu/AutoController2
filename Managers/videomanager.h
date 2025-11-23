#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QCamera>
#include <QCameraDevice>
#include <QComboBox>
#include <QMediaCaptureSession>
#include <QMediaDevices>
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
        QWidget* parent,
        QComboBox* cameraList,
        QComboBox* resolutionList,
        QPushButton* btnCameraStart
    );

    bool OnCloseEvent();

protected:
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    // UI
    void OnRefreshList();
    void OnCameraChanged(QString const& str);
    void OnCameraClicked();
    void OnFrameChanged(QVideoFrame const& frame);

private:
    // UI
    void PopulateCameraList();
    void PopulateResolution(QCameraDevice const& device);

    // Camera
    void Start();
    void Stop();

private:
    // UI
    QComboBox*      m_cameraList = Q_NULLPTR;
    QComboBox*      m_resolutionList = Q_NULLPTR;
    QPushButton*    m_btnCameraStart = Q_NULLPTR;

    // Camera
    QMediaDevices           m_devices;
    QMediaCaptureSession    m_capture;
    QCamera                 m_camera;
    QVideoSink              m_sink;
    QImage                  m_frame;
    QSize                   m_displaySize;
};

#endif // VIDEOMANAGER_H
