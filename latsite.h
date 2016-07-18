/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef LATSITE_H
#define LATSITE_H

#include <QGraphicsPixmapItem>
#include <QList>
#include "configscene.h"

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
QT_END_NAMESPACE

class Transition;

class Site : public QGraphicsItem
{
public:
    enum { Type = UserType + 15 };

    Site(int stat, int img, QMenu *contextMenu, QGraphicsItem *parent = 0);

    int type() const Q_DECL_OVERRIDE { return Type;}
    void removeTransition(Transition *transition);
    void removeTransitions();
    void addTransition(Transition *transition);
    void updateTrans();

    //physical state
    void setEn(double en) { energy = en; }
    float en() { return energy; }
    void on() { state = 1; }  //turn occupation on and off
    void off() { state = 0; }
    int stat() { return state; } // return occupation
    void setNNMod(int nn, double men) { nnmod[nn] = men; }
    void setNNMod(double men1, double men2, double men3, double men4);
    double nnMod(int nn) { return nnmod[nn]; }

    //periodic cell information
    int img() { return m_img; }
    void setID(int id) { m_id = id; }
    int id() { return m_id; }
    void setRep(int xrep, int yrep) { m_xrep = xrep; m_yrep = yrep; }
    int xr() { return m_xrep; }
    int yr() { return m_yrep; }

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;

private:
    QColor color;
    double energy;  // the potential energy level of the state
    int state;  // the occupation: 0 = unoccupied, 1 = occupied
    double nnmod [5]; // the change in energy for coordination

    int m_img; // set to 1 if the object is a periodic image
    QMenu *myContextMenu;
    QList<Transition *> transitions; // list of site transitions
    int m_id; // indexing
    int m_xrep,m_yrep; // replication indexing
};

#endif // LATSITE_H
