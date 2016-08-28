/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QDialog>
#include <QtWidgets>
#include <QLineEdit>
#include <qcustomplot.h>

class PlotWindow : public QWidget   //plotting
{
    Q_OBJECT

public:
    PlotWindow(QVector<double>& energy, QVector<double>& time);

private slots:
    void okButtonPress();
    void cancelButtonPress();

private:
    QPushButton *okButton;
    QPushButton *cancelButton;
    QCustomPlot *customPlot;
};

#endif // PLOTWINDOW_H

