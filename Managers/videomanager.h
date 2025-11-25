#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QCamera>
#include <QCameraDevice>
#include <QComboBox>
#include <QMediaCaptureSession>
#include <QMediaDevices>
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
        QPushButton* btnCameraStart,
        QWidget* parent = nullptr
    );

    QString GetDeviceName() const;
    QStringList GetResolutionData() const;

    void Start();
    void Stop();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    // UI
    void OnRefreshList();
    void OnCameraChanged(QString const& str);

private:
    // UI
    void PopulateCameraList();
    void PopulateResolution(QCameraDevice const& device);

private:
    // UI
    QComboBox*      m_listCamera = Q_NULLPTR;
    QComboBox*      m_listResolution = Q_NULLPTR;
    QPushButton*    m_btnCameraStart = Q_NULLPTR;

    // Members
    QMediaDevices   m_devices;
};

#endif // VIDEOMANAGER_H
