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
#include <QMutex>
#include <QPainter>
#include <QPushButton>
#include <QVideoSink>

#include "../ui_mainwindow.h"

class VideoManager : public QWidget
{
    Q_OBJECT

public:
    VideoManager(QWidget* parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Video"; }
    void Initialize(Ui::MainWindow* ui);

    QString GetDeviceName() const;
    QSize GetResolution() const;
    bool HasFrameData() const { return m_hasData; }

    void Start();
    void Stop();

    void PushFrameData(unsigned char const* data);
    QImage GetFrameData();

protected:
    void paintEvent(QPaintEvent* event) override;

    int heightForWidth(int width) const override;
    bool hasHeightForWidth() const override { return true; }

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
    QMutex              m_mutex;
    QImage              m_frame;
    bool                m_hasData = false;
};

#endif // VIDEOMANAGER_H
