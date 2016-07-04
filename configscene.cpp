/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#include "configscene.h"
#include "trans.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


ConfigScene::ConfigScene(QMenu *siteMenu, QMenu *transMenu, QObject *parent)
    : QGraphicsScene(parent)
{
    mySiteMenu = siteMenu;
    myTransMenu = transMenu;
    myMode = MoveItem;
    line = 0;
    myItemColor = Qt::white;
    myLineColor = Qt::black;
}

void ConfigScene::setLineColor(const QColor &color)
{
    myLineColor = color;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->setColor(myLineColor);
        update();
    }
}

void ConfigScene::setItemColor(const QColor &color)
{
    myItemColor = color;
    if (isItemChange(Site::Type)) {
        Site *item = qgraphicsitem_cast<Site *>(selectedItems().first());
//        item->setBrush(myItemColor);
    }
}

void ConfigScene::setMode(Mode mode)
{
    myMode = mode;
}

void ConfigScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;

    Site *item;
    Site *item2;
    switch (myMode) {
        case InsertItem:
            if(mouseEvent->scenePos().x() > sceneRect().left() &&
                    mouseEvent->scenePos().x() < sceneRect().right() &&
                    mouseEvent->scenePos().y() > sceneRect().top() &&
                    mouseEvent->scenePos().y() < sceneRect().bottom())
            {
                item = new Site(0,mySiteMenu);
//                item->setBrush(myItemColor);


                QPointF xper(-100,0);

                item2 = new Site(0,mySiteMenu);
                item2->setParentItem(item);


                addItem(item);

                item->setPos(mouseEvent->scenePos());

                item2->setPos(xper);

                qDebug() << item;
                qDebug() << item2;

                emit itemInserted(item);

            }
            break;

        case InsertLine:
            line = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(),
                                        mouseEvent->scenePos()));
            line->setPen(QPen(myLineColor, 2));
            addItem(line);
            break;

    default:
        ;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void ConfigScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (myMode == InsertLine && line != 0) {
        QLineF newLine(line->line().p1(), mouseEvent->scenePos());
        line->setLine(newLine);
    } else if (myMode == MoveItem) {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void ConfigScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (line != 0 && myMode == InsertLine) {
        QList<QGraphicsItem *> startItems = items(line->line().p1());
        if (startItems.count() && startItems.first() == line)
            startItems.removeFirst();
        QList<QGraphicsItem *> endItems = items(line->line().p2());
        if (endItems.count() && endItems.first() == line)
            endItems.removeFirst();

        removeItem(line);
        delete line;


        if (startItems.count() > 0 && endItems.count() > 0 &&
            startItems.first()->type() == Site::Type &&
            endItems.first()->type() == Site::Type &&
            startItems.first() != endItems.first()) {
            Site *startItem = qgraphicsitem_cast<Site *>(startItems.first());
            Site *endItem = qgraphicsitem_cast<Site *>(endItems.first());
            Transition *transition = new Transition(myTransMenu, startItem, endItem);
            transition->setColor(myLineColor);
            startItem->addTransition(transition);
            endItem->addTransition(transition);
            transition->setZValue(-1000.0);
            addItem(transition);
            transition->updatePosition();
        }
    }

    line = 0;
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

bool ConfigScene::isItemChange(int type)
{
    foreach (QGraphicsItem *item, selectedItems()) {
        if (item->type() == type)
            return true;
    }
    return false;
}

