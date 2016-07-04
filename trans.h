/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#ifndef TRANS_H
#define TRANS_H

#include <QGraphicsLineItem>

#include "latsite.h"

QT_BEGIN_NAMESPACE
class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QMenu;
class QRectF;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneMouseEvent;
class QPainterPath;
QT_END_NAMESPACE

class Transition : public QGraphicsLineItem
{
public:
    enum { Type = UserType + 4 };

    Transition(QMenu *contextMenu, Site *startItem, Site *endItem,
      QGraphicsItem *parent = 0);

    int type() const Q_DECL_OVERRIDE { return Type; }
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void setColor(const QColor &color) { myColor = color; }
    Site *startItem() const { return myStartItem; }
    Site *endItem() const { return myEndItem; }

    void updatePosition();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;

private:
    Site *myStartItem;
    Site *myEndItem;
    QColor myColor;
    QMenu *myContextMenu;
};

#endif // TRANS_H
