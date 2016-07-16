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

Site::Site(int stat, int img, QMenu *contextMenu, QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    myContextMenu = contextMenu;

    //initialise the state and occupation
    state = stat;
    m_img = img;

    QColor color(215,215,215,255);
    this->color = color;

    if(m_img == 0) // set selectable and moveable if in the main cell area
    {
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    setZValue(-10);

    energy = 0.0;
}

void Site::removeTransition(Transition *transition)
{
    int index = transitions.indexOf(transition);

    if (index != -1)
        transitions.removeAt(index);
}

void Site::removeTransitions()
{
    if(transitions.size() > 0)
    {
    foreach (Transition *transition, transitions) {
        transition->startItem()->removeTransition(transition);
        transition->endItem()->removeTransition(transition);
        scene()->removeItem(transition);
        delete transition;
    }
    }
}

void Site::addTransition(Transition *transition)
{
    transitions.append(transition);
}

//define the site bounding rect
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

//paint the site
void Site::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    QPen myPen;
    myPen.setColor(Qt::darkGray);

    painter->setPen(QPen(Qt::darkGray, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));


    QBrush b = painter->brush();

    // change shading based on state/occupation
    if(m_img == 0) {
        if(state == 0) {
            painter->setBrush(QBrush(Qt::white));
        } else if(state == 1) {
            painter->setBrush(QBrush(Qt::gray));
        }
    }
    else {
        if(state == 0) {
            painter->setBrush(QBrush(QColor(218, 218, 218, 255)));
        } else if(state == 1) {
            painter->setBrush(QBrush(Qt::gray));
        }
    }

    painter->drawEllipse(-25, -25, 50, 50);
    painter->setBrush(b);

    if (isSelected()) {
        painter->setPen(QPen(QColor(80, 80, 255, 255), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawEllipse(-25, -25, 50, 50);
    }

}

void Site::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->exec(event->screenPos());
}

void Site::updateTrans()
{
    foreach (QGraphicsItem *item, childItems()) {
        foreach(Transition *transition, qgraphicsitem_cast<Site *>(item)->transitions) {
            transition->updatePosition();
        }
    }
}

QVariant Site::itemChange(GraphicsItemChange change, const QVariant &value)
{
    //move all the child (periodic image) items
    if (change == QGraphicsItem::ItemPositionChange) {
        foreach (Transition *transition, transitions) {
               transition->updatePosition();
        }
        foreach (QGraphicsItem *item, childItems()) {
            foreach(Transition *transition, qgraphicsitem_cast<Site *>(item)->transitions) {
                transition->updatePosition();
            }
        }

        if(m_img == 0)
        {
            QPointF newPos = value.toPointF();
            QRectF rect = scene()->sceneRect();

            // do snap to grid
            ConfigScene* customScene = qobject_cast<ConfigScene*> (scene());
            bool doSnap = customScene->getSnap();

            if(doSnap) {
                int gridSize = customScene->getGridSize();

                qreal xV = round(newPos.x()/gridSize)*gridSize;
                qreal yV = round(newPos.y()/gridSize)*gridSize;

                newPos.setX(xV);
                newPos.setY(yV);
            }
            if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
                newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
                newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
                return newPos;
            }

            if(doSnap) {
                return newPos;
            }
        }
    }

    return value;
}

