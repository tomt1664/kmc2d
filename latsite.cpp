/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#include "latsite.h"
#include "trans.h"

#include <QStyleOption>
#include <QtWidgets>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

Site::Site(int stat, QMenu *contextMenu, QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    myContextMenu = contextMenu;

    state = stat;

    QColor color(215,215,215,255);
    this->color = color;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
}

void Site::removeTransition(Transition *transition)
{
    int index = transitions.indexOf(transition);

    if (index != -1)
        transitions.removeAt(index);
}

void Site::removeTransitions()
{
    foreach (Transition *transition, transitions) {
        transition->startItem()->removeTransition(transition);
        transition->endItem()->removeTransition(transition);
        scene()->removeItem(transition);
        delete transition;
    }
}

void Site::addTransition(Transition *transition)
{
    transitions.append(transition);
}

//define atom bounding rect
QRectF Site::boundingRect() const
{
    return QRectF(-35, -35, 70, 70);
}

QPainterPath Site::shape() const
{
    QPainterPath path;
    path.addEllipse(-25, -25, 50, 50);
    return path;
}

//paint atom
void Site::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

//    QColor fillColor = (option->state & QStyle::State_Selected) ? color.dark(150) : color;
//    if (option->state & QStyle::State_MouseOver)
//        fillColor = fillColor.light(125);

    QPen myPen;
    myPen.setColor(Qt::gray);

    painter->setPen(QPen(Qt::gray, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

//    QPen oldPen = painter->pen();
//    QPen pen = oldPen;
//    int width = 0;
//    if (option->state & QStyle::State_Selected)
//        width += 2;

//    pen.setWidth(8);
    QBrush b = painter->brush();
    painter->setBrush(QBrush(Qt::white));

    painter->drawEllipse(-25, -25, 50, 50);
    painter->setBrush(b);






//    QPen myPen = pen();
//    myPen.setColor(Qt::gray);

//    painter->setPen(myPen);
//    painter->setBrush(myColor);


//    painter->drawLine(line());
    if (isSelected()) {
//        myPen.setColor(Qt::blue);
//        painter->setBrush(Qt::blue);
        painter->setPen(QPen(Qt::blue, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawEllipse(-25, -25, 50, 50);
    }

}

void Site::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->exec(event->screenPos());
}

QVariant Site::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        foreach (Transition *transition, transitions) {
            transition->updatePosition();
        }
        QPointF newPos = value.toPointF();
        QRectF rect = scene()->sceneRect();
        if (!rect.contains(newPos)) {
               // Keep the item inside the scene rect.
               newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
               newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
               return newPos;
        }
    }

    return value;
}

