/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "latsite.h"
#include "curvedisplay.h"

#include <QMainWindow>
#include <QWidget>
#include <QtWidgets>

class ConfigScene;

QT_BEGIN_NAMESPACE
class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QFontComboBox;
class QButtonGroup;
class QLineEdit;
class QGraphicsTextItem;
class QFont;
class QToolButton;
class QAbstractButton;
class QGraphicsView;
class QSlider;
QT_END_NAMESPACE
class CurveDisplay;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
   MainWindow();

private slots:
    void openfile();
    void savefile();
    void exportSVG();
    void print();
    void deleteItem();
    void clearCell();
    void sceneGroupClicked(int id);
    void toggleImages(bool on);
    void toggleSnap(bool on);
    void changeCellSize();
    void expandSystem();
    void setupMatrix();
    void about();
    void occupied();
    void unoccupied();
    void itemSelected(QGraphicsItem *item);
    void itemdeSelected(QGraphicsItem *item);

    void min1Changed();
    void min2Changed();
    void barChanged();
    void startModChanged();
    void endModChanged();
    void startPreFacChanged();
    void endPreFacChanged();
    void startModCBChanged();
    void endModCBChanged();
    void setTemp(int tmp);
    void setSeed(int isd);

    void startKMC();
    void stopKMC();
    void stepForward();
    void stepBack();

    void closeEvent(QCloseEvent *event);

private:
    void createToolBox();
    void createActions();
    void createMenus();
    void drawCells();
    void redrawCells();

    ConfigScene *scene;
    QGraphicsView *view;
    QGraphicsRectItem *cell;
    QGraphicsRectItem *perarea;

    CurveDisplay *curveDisplay;

    QAction *exitAction;
    QAction *addAction;
    QAction *deleteAction;
    QAction *setOccupied;
    QAction *setUnoccupied;
    QAction *printAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *clearAction;
    QAction *exportAction;

    QAction *aboutAction;

    QMenu *fileMenu;
    QMenu *itemMenu;
    QMenu *aboutMenu;

    QMenu *siteMenu;
    QMenu *transMenu;

    QToolBox *toolBox;
    QButtonGroup *buttonGroup;
    QButtonGroup *pointerTypeGroup;

    QButtonGroup *sceneGroup;
    QToolButton *selectButton;
    QToolButton *addUsiteButton;
    QToolButton *addSiteButton;
    QToolButton *addTransButton;
    QToolButton *deleteButton;
    QToolButton *snapButton;
    QToolButton *imageButton;
    QToolButton *cellSizeButton;
    QToolButton *expandButton;
    QToolButton *colorPES;

    QSlider *zoomSlider;

    QDoubleSpinBox *barSpinBox;
    QDoubleSpinBox *min1SpinBox;
    QDoubleSpinBox *min2SpinBox;

    QComboBox *startModifier;
    QComboBox *endModifier;
    QDoubleSpinBox *startModSpinBox;
    QDoubleSpinBox *endModSpinBox;
    QLabel *modimage;

    QDoubleSpinBox *startPreFactor;
    QDoubleSpinBox *endPreFactor;
    QLabel *pfimage;

    QSpinBox *temperature;
    QSpinBox *seed;

    QAction *startAction;
    QAction *stopAction;
    QToolButton *startStopButton;
    QToolButton *rewindButton;
    QToolButton *forwardButton;
    QToolButton *backButton;
    QToolButton *graphButton;

    QDoubleSpinBox *delaySpinBox;
    QToolButton *recordButton;
    QComboBox *detailComboBox;
    QLCDNumber *simulationTime;
    QTextEdit *simulationStatus;

    long nstep; // KMC step
    int pstep; // detail step
    int kmcDetail; // detail printing
    double m_temp; // simulation temperature
    double m_beta; // Boltzman factor
    double m_time; // simulation time

    QList<QPointF> barPFList; // active barrier and PF list
    QList<double> rateList; // list of all the exit rates
    QList<QGraphicsItem *> transList; //list of transition pointers
    double rateTotal; // the sum of all the exit pathway rates

    int xcell; // x cell dimension
    int ycell; // y cell dimension

    QGraphicsRectItem *pcell1;
    QGraphicsRectItem *pcell2;
    QGraphicsRectItem *pcell3;
    QGraphicsRectItem *pcell4;
    QGraphicsRectItem *pcell5;
    QGraphicsRectItem *pcell6;
    QGraphicsRectItem *pcell7;
    QGraphicsRectItem *pcell8;

    QGraphicsRectItem *pcellc1;
    QGraphicsRectItem *pcellc2;
    QGraphicsRectItem *pcellc3;
    QGraphicsRectItem *pcellc4;
    QGraphicsRectItem *pcellc5;
    QGraphicsRectItem *pcellc6;
    QGraphicsRectItem *pcellc7;
    QGraphicsRectItem *pcellc8;
};

#endif // MAINWINDOW_H
