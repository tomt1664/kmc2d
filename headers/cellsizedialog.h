/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef CELLSIZEDIALOG
#define CELLSIZEDIALOG

#include <QDialog>
#include <QtWidgets>
#include <QLineEdit>

class CellSizeDialog : public QDialog   //simulation cell size change dialog box
{
    Q_OBJECT

public:
    CellSizeDialog(int xcell, int ycell);
    int getx() { return m_x; }
    int gety() { return m_y; }
    int cancel() { return cncl; }

private slots:
    void okButtonPress();
    void cancelButtonPress();

private:
    QLineEdit *xEdit;
    QLineEdit *yEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
    int m_x;
    int m_y;
    int cncl;
};

#endif // CELLSIZEDIALOG
