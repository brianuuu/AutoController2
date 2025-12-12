#ifndef STICKPAINTER_H
#define STICKPAINTER_H

#include <QPainter>
#include <QVector2D>
#include <QWidget>

class StickPainter : public QWidget
{
    Q_OBJECT
public:
    explicit StickPainter(QPoint center, QWidget *parent = nullptr);
    void SetStickPos(QPointF pos);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPointF m_pos = QPointF();
};

#endif // STICKPAINTER_H
