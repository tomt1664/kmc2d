/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef CURVEDISPLAY_H
#define CURVEDISPLAY_H

#include <QBrush>
#include <QPen>
#include <QWidget>

class CurveDisplay : public QWidget  // QWidget class to draw the selected transition energy surface
{
    Q_OBJECT

public:

    CurveDisplay(QWidget *parent = 0);

    QSize size() const Q_DECL_OVERRIDE;

public slots:
    void setMin1(double min1);  //set start site minimum
    void setMin2(double min2);  //set end site minimum
    void setBar(double bar);  //set the transition point height
    void setMin(double min);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QPen pen;
    QBrush brush;
    double m_min;
    double m_min1;
    double m_min2;
    double m_bar;
};

#endif // CURVEDISPLAY_H

