#include "stickpainter.h"

#define STICK_WIDTH 4
#define STICK_MAX_RADIUS 70
#define STICK_CENTER QPoint(STICK_MAX_RADIUS, STICK_MAX_RADIUS)

StickPainter::StickPainter(QPoint center, QWidget *parent)
    : QWidget{parent}
{
    this->move(center - STICK_CENTER);
    this->resize(STICK_MAX_RADIUS * 2, STICK_MAX_RADIUS * 2);
}

void StickPainter::SetStickPos(QPointF pos)
{
    QVector2D point(pos.x(),-pos.y());
    m_pos = point.normalized().toPointF();
    this->update();
}

void StickPainter::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QPen pen;
    pen.setWidth(STICK_WIDTH);
    pen.setColor(QColor(255,0,0));
    painter.setPen(pen);

    if (m_pos == QPointF(0,0)) return;

    QPointF endPoint = m_pos * (STICK_MAX_RADIUS - 10) + STICK_CENTER;
    painter.drawLine(STICK_CENTER, endPoint);
    painter.drawEllipse(endPoint, 3, 3);
    painter.drawEllipse(endPoint, 6, 6);
}
