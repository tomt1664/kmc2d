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

#include "cellsizedialog.h"

CellSizeDialog::CellSizeDialog(int xcell, int ycell)
{
    int xred = xcell/10;
    int yred = ycell/10;

    QLabel *xLabel = new QLabel(tr("x Dimension:"));
    xEdit = new QLineEdit;
    xEdit->setText(QString::number(xred));
    xEdit->setValidator(new QIntValidator(10,1000, xEdit));

    QLabel *yLabel = new QLabel(tr("y Dimension:"));
    yEdit = new QLineEdit;
    yEdit->setText(QString::number(yred));
    yEdit->setValidator(new QIntValidator(10,1000, yEdit));

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));

    QGridLayout *cellSizeLayout = new QGridLayout;
    cellSizeLayout->addWidget(xLabel, 0, 0);
    cellSizeLayout->addWidget(xEdit, 0, 1);
    cellSizeLayout->addWidget(yLabel, 1, 0);
    cellSizeLayout->addWidget(yEdit, 1, 1);
    cellSizeLayout->addWidget(okButton, 2, 0);
    cellSizeLayout->addWidget(cancelButton, 2, 1);
    setLayout(cellSizeLayout);

    connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonPress()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPress()));

    setWindowTitle(tr("Cell Dimensions"));
}

void CellSizeDialog::okButtonPress()
{
    m_x = xEdit->text().toInt()*10;
    m_y = yEdit->text().toInt()*10;

    close();
}

void CellSizeDialog::cancelButtonPress()
{
    close();
}

