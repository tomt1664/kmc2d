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

#include "expanddialog.h"

ExpandDialog::ExpandDialog(void)
{
    QLabel *xLabel = new QLabel(tr("Multiply x:"));
    xEdit = new QLineEdit;
    xEdit->setText(QString::number(1));
    xEdit->setValidator(new QIntValidator(1,100, xEdit));

    QLabel *yLabel = new QLabel(tr("Multiply y:"));
    yEdit = new QLineEdit;
    yEdit->setText(QString::number(1));
    yEdit->setValidator(new QIntValidator(1,100, yEdit));

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

    setWindowTitle(tr("Expand system cell"));
}

void ExpandDialog::okButtonPress()
{
    m_x = xEdit->text().toInt();
    m_y = yEdit->text().toInt();
    cncl = 0;
    close();
}

void ExpandDialog::cancelButtonPress()
{
    cncl = 1;
    close();
}


