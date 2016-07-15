/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef CONFIGSCENE_H
#define CONFIGSCENE_H

#include "latsite.h"

#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QGraphicsItem;
class QColor;
class Site;
QT_END_NAMESPACE

class ConfigScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { InsertUSite, InsertSite, InsertTrans, MoveItem };

    explicit ConfigScene(QMenu *siteMenu, QMenu *transMenu,int xc, int yc, QObject *parent = 0);
    void addSite(bool ostate, double xc, double yc, int sindx, int xrep, int yrep);
    void addTrans(Site *myStartItem, Site *myEndItem);
    void addTransPair(Site *myStartItem1, Site *myEndItem1,Site *myStartItem2, Site *myEndItem2);

    int getGridSize() const { return this->gridSize; }
    bool getSnap() { return this->snap; }
    void setSnap(bool dosnap) { snap = dosnap; }
    void changeCell(int xcelln, int ycelln) { xcell = xcelln; ycell = ycelln; }

    void setTransMin1(double energy);
    void setTransMin2(double energy);
    void setTransBar(double energy);

public slots:
    void setMode(Mode mode);

signals:
    void itemSelected(QGraphicsItem *item);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;

private:
    bool isItemChange(int type);

    QMenu *mySiteMenu;
    QMenu *myTransMenu;
    Mode myMode;
    bool leftButtonDown;
    QPointF startPoint;
    QGraphicsLineItem *line;
    QColor myItemColor;
    QColor myLineColor;
    int xcell; // cell x dimension
    int ycell; // cell y dimension
    int indx; // index for linking periodic transitions
    int gridSize;
    bool snap;
};

#endif // CONFIGSCENE_H
