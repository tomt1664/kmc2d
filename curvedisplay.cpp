/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#include "curvedisplay.h"

#include <QPainter>

CurveDisplay::CurveDisplay(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(true);
    setMinimumHeight(160);

    //initial energies
    m_min1 = 0.0;
    m_min2 = 0.0;
    m_bar = 0.0;
}

QSize CurveDisplay::size() const
{
    return QSize(100, 100);
}

void CurveDisplay::setMin1(double min1)
{
    m_min1 = min1;
    update();
}

void CurveDisplay::setMin2(double min2)
{
    m_min2 = min2;
    update();
}

void CurveDisplay::setMin(double min)
{
    m_min = min;
    update();
}

void CurveDisplay::setBar(double bar)
{
    m_bar = bar;
    update();
}

void CurveDisplay::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    if(m_bar == 0.0 && m_min1 == 0.0 && m_min2 == 0.0) {
        return;
    }

    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(brush);

    int iwidth = 30;
    QPainterPath path;
    path.moveTo(20, 120-m_min1*20);
    path.cubicTo(20+iwidth+15, 120-m_min1*20, 100-iwidth, 120-m_bar*20, 100, 120-m_bar*20);
    path.cubicTo(100+iwidth, 120-m_bar*20, 180-iwidth-15, 120-m_min2*20, 180, 120-m_min2*20);


    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.drawPath(path);

    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);

}
