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
    QCustomPlot *customPlot;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // PLOTWINDOW_H

