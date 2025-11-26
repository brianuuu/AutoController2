#ifndef MEDIADISCOVERER_H
#define MEDIADISCOVERER_H

#include <QThread>

class MediaDiscoverer : public QThread
{
    Q_OBJECT
public:
    explicit MediaDiscoverer(
        bool isAudio,
        QObject *parent = nullptr
    )
        : QThread(parent)
        , m_isAudio(isAudio)
    { }

protected:
    void run() override;

signals:
    void finished(QStringList const& list);

private:
    bool m_isAudio = false;
};

#endif // MEDIADISCOVERER_H
