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
    void backgroundButtonGroupClicked(QAbstractButton *button);
    void deleteItem();
    void sceneGroupClicked(int id);
    void toggleImages(bool on);
    void toggleSnap(bool on);
    void changeCellSize();
    void expandSystem();
    void setupMatrix();
    void about();

    void min1Changed();
    void min2Changed();
    void barChanged();

private:
    void createToolBox();
    void createActions();
    void createMenus();
    void drawCells();
    void redrawCells();
    QWidget *createBackgroundCellWidget(const QString &text,
                                        const QString &image);
    QWidget *createCellWidget(const QString &text);

    ConfigScene *scene;
    QGraphicsView *view;
    QGraphicsRectItem *cell;
    QGraphicsRectItem *perarea;

    CurveDisplay *curveDisplay;

    QAction *exitAction;
    QAction *addAction;
    QAction *deleteAction;

    QAction *aboutAction;

    QMenu *fileMenu;
    QMenu *itemMenu;
    QMenu *aboutMenu;

    QMenu *siteMenu;
    QMenu *transMenu;

    QToolBox *toolBox;
    QButtonGroup *buttonGroup;
    QButtonGroup *pointerTypeGroup;
    QButtonGroup *backgroundButtonGroup;

    QButtonGroup *sceneGroup;
    QToolButton *selectButton;
    QToolButton *addUsiteButton;
    QToolButton *addSiteButton;
    QToolButton *addTransButton;
    QToolButton *snapButton;
    QToolButton *imageButton;
    QToolButton *cellSizeButton;
    QToolButton *expandButton;

    QSlider *zoomSlider;

    QDoubleSpinBox *barSpinBox;
    QDoubleSpinBox *min1SpinBox;
    QDoubleSpinBox *min2SpinBox;

    int xcell;
    int ycell;
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
