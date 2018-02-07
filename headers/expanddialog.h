/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef EXPANDDIALOG
#define EXPANDDIALOG

#include <QDialog>
#include <QtWidgets>
#include <QLineEdit>

class ExpandDialog : public QDialog   //expand the simulation cell dialog box
{
    Q_OBJECT

public:
    ExpandDialog(void);
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

#endif // EXPANDDIALOG

