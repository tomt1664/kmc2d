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

class PlotWindow : public QWidget   // plotting graph for simulation output
{
    Q_OBJECT

public:
    PlotWindow(QVector<double> *e1, QVector<double> *t1,
               QVector<double> *x1, QVector<double> *y1, QVector<double> *s1);

private slots:
    void saveButtonPress();
    void exportButtonPress();
    void cancelButtonPress();
    void setPlotType();

private:
    QPushButton *saveButton;
    QPushButton *cancelButton;
    QPushButton *exportButton;
    QComboBox *plotType;
    QCustomPlot *customPlot;

    QVector<double> *time;
    QVector<double> *energy;
    QVector<double> *xDisp;
    QVector<double> *yDisp;
    QVector<double> *sDisp;
};

#endif // PLOTWINDOW_H

