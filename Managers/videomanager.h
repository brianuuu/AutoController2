#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QCameraDevice>
#include <QComboBox>
#include <QMediaDevices>
#include <QPushButton>

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
    QComboBox* m_cameraList = Q_NULLPTR;
    QComboBox* m_resolutionList = Q_NULLPTR;
    QPushButton* m_btnCameraStart = Q_NULLPTR;
    QMediaDevices m_devices;
};

#endif // VIDEOMANAGER_H
