/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#include "trans.h"
#include "configscene.h"

#include <math.h>

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include <QPen>
#include <QPainter>

Transition::Transition(QMenu *contextMenu, Site *startItem, Site *endItem, QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
{
    myContextMenu = contextMenu;

    myStartItem = startItem;
    myEndItem = endItem;
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    QColor color(170,170,170,255);
    this->myColor = color;

    setPen(QPen(Qt::darkGray, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setZValue(-50);
    m_en = 1.0;
    m_startprefac = 10.0;
    m_endprefac = 10.0;
    m_highlight = 0;
}

QRectF Transition::boundingRect() const
{
    qreal extra = (pen().width() + 50) / 2.0;

    return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
                                      line().p2().y() - line().p1().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

QPainterPath Transition::shape() const
{
    QPainterPath path = QGraphicsLineItem::shape();
    return path;
}

void Transition::updatePosition()
{
    QLineF line(mapFromItem(myStartItem, 0, 0), mapFromItem(myEndItem, 0, 0));

    setLine(line);
}

void Transition::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
          QWidget *)
{
    if (myStartItem->collidesWithItem(myEndItem))
        return;

    QPen myPen = pen();

    painter->setBrush(myColor);

    if(m_highlight) {
        myPen.setColor(QColor(235, 0, 0, 255));
        myPen.setWidth(10);
        painter->setPen(myPen);
    } else {
        myPen.setColor(Qt::darkGray);
        myPen.setWidth(10);
        painter->setPen(myPen);
    }
    painter->drawLine(line());

    this->setZValue(-50);
    if (isSelected() && !m_highlight) {
        myPen.setColor(QColor(80, 80, 255, 255));
        painter->setBrush(QColor(80, 80, 255, 255));
        painter->setPen(myPen);
        painter->drawLine(line());
        myPen.setWidth(7);
        painter->setPen(myPen);
        painter->drawEllipse(myEndItem->scenePos(),24,24);
        painter->setBrush(QColor(255, 255, 255, 255));
        painter->drawEllipse(myStartItem->scenePos(),24,24);
        this->setZValue(1000);
        myPen.setWidth(10);
        painter->setPen(myPen);
    }
}


void Transition::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->exec(event->screenPos());
}

QVariant Transition::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
    if(value == true)
    {
        emit selectedChange(this);
    } else
    {
        emit deselectedChange(this);
    }
   }
    return value;
}
