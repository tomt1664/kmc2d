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

    Site(int stat, QMenu *contextMenu, QGraphicsItem *parent = 0);

    void removeTransition(Transition *transition);
    void removeTransitions();
    void addTransition(Transition *transition);
    int type() const Q_DECL_OVERRIDE { return Type;}

    void setEn(float en) { energy = en; }
    float en() { return energy; }
    void on() { state = 1; }
    void off() { state = 0; }
    int stat() { return state; }

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;

private:
    QColor color;
    float energy;  // the potential energy level of the state
    int state;  // the occupation: 0 = unoccupied, 1 = occupied
    QMenu *myContextMenu;
    QList<Transition *> transitions;
};

#endif // LATSITE_H
