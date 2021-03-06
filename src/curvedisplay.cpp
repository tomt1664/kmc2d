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
    setMinimumHeight(190);

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
    float fbar = m_bar - m_min1;
    float bbar = m_bar - m_min2;
    if(fbar < 0.0) fbar = 0.0;
    if(bbar < 0.0) bbar = 0.0;
    setToolTip(QString::number(fbar)+" ->  <- "+QString::number(bbar));
}

void CurveDisplay::setMin2(double min2)
{
    m_min2 = min2;
    update();
    float fbar = m_bar - m_min1;
    float bbar = m_bar - m_min2;
    if(fbar < 0.0) fbar = 0.0;
    if(bbar < 0.0) bbar = 0.0;
    setToolTip(QString::number(fbar)+" ->  <- "+QString::number(bbar));
}

void CurveDisplay::setMin(double min)
{
    m_min = min;
    update();
    float fbar = m_bar - m_min1;
    float bbar = m_bar - m_min2;
    if(fbar < 0.0) fbar = 0.0;
    if(bbar < 0.0) bbar = 0.0;
    setToolTip(QString::number(fbar)+" ->  <- "+QString::number(bbar));
}

void CurveDisplay::setBar(double bar)
{
    m_bar = bar;
    update();
}

void CurveDisplay::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    //if everything is zero (no item selected) return
    if(m_bar == 0.0 && m_min1 == 0.0 && m_min2 == 0.0) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(Qt::lightGray, 2, Qt::SolidLine ));
    painter.drawLine(30,160,30,124-m_min1*20);
    painter.drawLine(176,160,176,124-m_min2*20);
    painter.drawLine(103,168,103,124-m_bar*20);

    painter.drawLine(30,184,30,190);
    painter.drawLine(176,184,176,190);
    painter.drawLine(103,176,103,190);

    painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine));
    painter.setBrush(brush);

    //draw representation of transition path with 2 Bezier curves
    int iwidth = 30;
    QPainterPath path;
    path.moveTo(20, 120-m_min1*20);
    path.cubicTo(20+iwidth+15, 120-m_min1*20, 103-iwidth, 120-m_bar*20, 103, 120-m_bar*20);
    path.cubicTo(103+iwidth, 120-m_bar*20, 186-iwidth-15, 120-m_min2*20, 186, 120-m_min2*20);
    painter.drawPath(path);

    //draw the direction indicator
    QPointF startpoint(30,172);
    QPointF endpoint(176,172);
    painter.setPen(QPen(QColor(80, 80, 255, 255), 2));
    painter.drawLine(startpoint,endpoint);
    painter.setBrush(QColor(255, 255, 255, 255));
    painter.drawEllipse(startpoint,8,8);
    painter.setBrush(QColor(80, 80, 255, 255));
    painter.drawEllipse(endpoint,8,8);

    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);

}
