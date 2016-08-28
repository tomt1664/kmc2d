/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#include <QtWidgets>

#include <QDebug>

#include "plotwindow.h"
#include "qcustomplot.h"

PlotWindow::PlotWindow(QVector<double>& energy, QVector<double>& time, QVector<double>& xDisp,
                       QVector<double>& yDisp, QVector<double>& sDisp)
{
    customPlot = new QCustomPlot(this);
    // add two new graphs and set their look:
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::red)); // line color blue for first graph

    customPlot->xAxis2->setVisible(true);
    customPlot->xAxis2->setTickLabels(false);
    customPlot->yAxis2->setVisible(true);
    customPlot->yAxis2->setTickLabels(false);

    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
    // pass data points to graphs:
    customPlot->graph(0)->setData(time, energy);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    customPlot->setMinimumWidth(600);
    customPlot->setMinimumHeight(400);

    saveButton = new QPushButton(tr("Save Image"));
    exportButton = new QPushButton(tr("Export Data"));
    cancelButton = new QPushButton(tr("Cancel"));

    plotType = new QComboBox;
    plotType->addItem("Energy");
    plotType->addItem("x-Displacement");
    plotType->addItem("y-Displacement");
    plotType->addItem("Sq. Displacement");
    plotType->setToolTip("Plot type");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(plotType);
    buttonLayout->addStretch(0);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch(0);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addStretch(0);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addWidget(customPlot);
    plotLayout->addSpacing(30);
    plotLayout->addLayout(buttonLayout);
    plotLayout->addSpacing(10);

    setLayout(plotLayout);

    connect(saveButton, SIGNAL(clicked()), this, SLOT(okButtonPress()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPress()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportButtonPress()));
    connect(plotType, SIGNAL(currentIndexChanged(int)), this,
            SLOT(setPlotType(energy,time,xDisp, yDisp, sDisp)));

    setWindowTitle(tr("Trajectory Plot"));
}

void PlotWindow::saveButtonPress()
{
    close();
}

void PlotWindow::cancelButtonPress()
{
    close();
}

void PlotWindow::exportButtonPress()
{
    close();
}

void PlotWindow::setPlotType(QVector<double>& energy, QVector<double>& time,
                             QVector<double>& xDisp, QVector<double>& yDisp, QVector<double>& sDisp)
{
    customPlot->clearGraphs();
    int ptype = plotType->currentIndex();

    customPlot->addGraph();
    if(ptype == 0) {
        customPlot->graph(0)->setPen(QPen(Qt::red));
        customPlot->graph(0)->setData(time, energy);
    } else if(ptype == 1) {
        customPlot->graph(0)->setPen(QPen(Qt::black));
        customPlot->graph(0)->setData(time, xDisp);
    } else if(ptype == 2) {
        customPlot->graph(0)->setPen(QPen(Qt::black));
        customPlot->graph(0)->setData(time, yDisp);
    } else if(ptype == 3) {
        customPlot->graph(0)->setPen(QPen(Qt::blue));
        customPlot->graph(0)->setData(time, sDisp);
    }
}


