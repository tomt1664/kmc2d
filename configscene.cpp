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
#include <QGraphicsItemGroup>
#include <QDebug>


ConfigScene::ConfigScene(QMenu *siteMenu, QMenu *transMenu, int xc, int yc, QObject *parent)
    : QGraphicsScene(parent), gridSize(20)
{
    mySiteMenu = siteMenu;
    myTransMenu = transMenu;
    myMode = MoveItem;
    line = 0;
    myItemColor = Qt::white;
    myLineColor = Qt::black;
    xcell = xc;
    ycell = yc;
    indx = 1;
    snap = false;
}

void ConfigScene::addSite(bool ostate,double en, double xc, double yc, int sindx, int xrep, int yrep)
{
    Site *item;
    //periodic images
    Site *image1, *image2, *image3, *image4;
    Site *image5, *image6, *image7, *image8;

    //insert objects into scene
    item = new Site(0,0,mySiteMenu);
    if(ostate){
        item->on();
    } else {
        item->off();
    }
    item->setEn(en);
    item->setID(sindx);
    item->setRep(xrep,yrep);
    QPointF xper1(0,ycell), xper2(xcell,ycell), xper3(xcell,0), xper4(xcell,-ycell);
    QPointF xper5(0,-ycell), xper6(-xcell,-ycell), xper7(-xcell,0), xper8(-xcell,ycell);
    image1 = new Site(0,1,mySiteMenu);
    image2 = new Site(0,2,mySiteMenu);
    image3 = new Site(0,3,mySiteMenu);
    image4 = new Site(0,4,mySiteMenu);
    image5 = new Site(0,5,mySiteMenu);
    image6 = new Site(0,6,mySiteMenu);
    image7 = new Site(0,7,mySiteMenu);
    image8 = new Site(0,8,mySiteMenu);
    if(ostate){
        image1->on();
        image2->on();
        image3->on();
        image4->on();
        image5->on();
        image6->on();
        image7->on();
        image8->on();
    } else {
        image1->off();
        image2->off();
        image3->off();
        image4->off();
        image5->off();
        image6->off();
        image7->off();
        image8->off();
    }
    image1->setID(sindx);
    image2->setID(sindx);
    image3->setID(sindx);
    image4->setID(sindx);
    image5->setID(sindx);
    image6->setID(sindx);
    image7->setID(sindx);
    image8->setID(sindx);
    image1->setEn(en);
    image2->setEn(en);
    image3->setEn(en);
    image4->setEn(en);
    image5->setEn(en);
    image6->setEn(en);
    image7->setEn(en);
    image8->setEn(en);
    image1->setRep(xrep,yrep);
    image2->setRep(xrep,yrep);
    image3->setRep(xrep,yrep);
    image4->setRep(xrep,yrep);
    image5->setRep(xrep,yrep);
    image6->setRep(xrep,yrep);
    image7->setRep(xrep,yrep);
    image8->setRep(xrep,yrep);
    image1->setParentItem(item);
    image2->setParentItem(item);
    image3->setParentItem(item);
    image4->setParentItem(item);
    image5->setParentItem(item);
    image6->setParentItem(item);
    image7->setParentItem(item);
    image8->setParentItem(item);
    addItem(item);
    item->setPos(xc,yc);
    image1->setPos(xper1);
    image2->setPos(xper2);
    image3->setPos(xper3);
    image4->setPos(xper4);
    image5->setPos(xper5);
    image6->setPos(xper6);
    image7->setPos(xper7);
    image8->setPos(xper8);
}

void ConfigScene::addTrans(Site *myStartItem, Site *myEndItem, double nbar)
{
    Transition *transition = new Transition(myTransMenu, myStartItem, myEndItem);
    transition->setColor(myLineColor);
    myStartItem->addTransition(transition);
    myEndItem->addTransition(transition);
    transition->setZValue(-1000.0);
    transition->setID(0);
    transition->setEn(nbar);
    connect(transition, SIGNAL(selectedChange(QGraphicsItem*)),
            this, SIGNAL(itemSelected(QGraphicsItem*)));
    connect(transition, SIGNAL(deselectedChange(QGraphicsItem*)),
            this, SIGNAL(itemdeSelected(QGraphicsItem*)));
    addItem(transition);
    transition->updatePosition();
}

void ConfigScene::addTransPair(Site *myStartItem1, Site *myEndItem1,Site *myStartItem2, Site *myEndItem2, double nbar)
{
    Transition *transition1 = new Transition(myTransMenu, myStartItem1, myEndItem1);
    transition1->setColor(myLineColor);
    myStartItem1->addTransition(transition1);
    myEndItem1->addTransition(transition1);
    transition1->setZValue(-1000.0);
    transition1->setID(indx);
    transition1->setEn(nbar);
    connect(transition1, SIGNAL(selectedChange(QGraphicsItem*)),
            this, SIGNAL(itemSelected(QGraphicsItem*)));
    connect(transition1, SIGNAL(deselectedChange(QGraphicsItem*)),
            this, SIGNAL(itemdeSelected(QGraphicsItem*)));
    addItem(transition1);
    transition1->updatePosition();

    Transition *transition2 = new Transition(myTransMenu, myStartItem2, myEndItem2);
    transition2->setColor(myLineColor);
    myStartItem2->addTransition(transition2);
    myEndItem2->addTransition(transition2);
    transition2->setZValue(-1000.0);
    transition2->setID(indx);
    transition2->setEn(nbar);
    connect(transition2, SIGNAL(selectedChange(QGraphicsItem*)),
            this, SIGNAL(itemSelected(QGraphicsItem*)));
    connect(transition2, SIGNAL(deselectedChange(QGraphicsItem*)),
            this, SIGNAL(itemdeSelected(QGraphicsItem*)));
    addItem(transition2);
    transition2->updatePosition();

    indx++;
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

    //periodic images
    Site *image1, *image2, *image3, *image4;
    Site *image5, *image6, *image7, *image8;

    //insert objects into scene
    switch (myMode) {
        case InsertUSite:
            if(mouseEvent->scenePos().x() > sceneRect().left() &&
                    mouseEvent->scenePos().x() < sceneRect().right() &&
                    mouseEvent->scenePos().y() > sceneRect().top() &&
                    mouseEvent->scenePos().y() < sceneRect().bottom())
            {
                item = new Site(0,0,mySiteMenu);
                item->off(); // set to unoccupied
                QPointF xper1(0,ycell), xper2(xcell,ycell), xper3(xcell,0), xper4(xcell,-ycell);
                QPointF xper5(0,-ycell), xper6(-xcell,-ycell), xper7(-xcell,0), xper8(-xcell,ycell);
                // add the periodic images
                image1 = new Site(0,1,mySiteMenu);
                image1->off();
                image1->setParentItem(item);
                image2 = new Site(0,2,mySiteMenu);
                image2->off();
                image2->setParentItem(item);
                image3 = new Site(0,3,mySiteMenu);
                image3->off();
                image3->setParentItem(item);
                image4 = new Site(0,4,mySiteMenu);
                image4->off();
                image4->setParentItem(item);
                image5 = new Site(0,5,mySiteMenu);
                image5->off();
                image5->setParentItem(item);
                image6 = new Site(0,6,mySiteMenu);
                image6->off();
                image6->setParentItem(item);
                image7 = new Site(0,7,mySiteMenu);
                image7->off();
                image7->setParentItem(item);
                image8 = new Site(0,8,mySiteMenu);
                image8->off();
                image8->setParentItem(item);
                addItem(item);
                item->setPos(mouseEvent->scenePos());
                image1->setPos(xper1);
                image2->setPos(xper2);
                image3->setPos(xper3);
                image4->setPos(xper4);
                image5->setPos(xper5);
                image6->setPos(xper6);
                image7->setPos(xper7);
                image8->setPos(xper8);
            }
            break;

    case InsertSite:
        if(mouseEvent->scenePos().x() > sceneRect().left() &&
                mouseEvent->scenePos().x() < sceneRect().right() &&
                mouseEvent->scenePos().y() > sceneRect().top() &&
                mouseEvent->scenePos().y() < sceneRect().bottom())
        {

            item = new Site(0,0,mySiteMenu);
            item->on(); //set to occupied
            QPointF xper1(0,ycell), xper2(xcell,ycell), xper3(xcell,0), xper4(xcell,-ycell);
            QPointF xper5(0,-ycell), xper6(-xcell,-ycell), xper7(-xcell,0), xper8(-xcell,ycell);
            // add the periodic images
            image1 = new Site(0,1,mySiteMenu);
            image1->on();
            image1->setParentItem(item);
            image2 = new Site(0,2,mySiteMenu);
            image2->on();
            image2->setParentItem(item);
            image3 = new Site(0,3,mySiteMenu);
            image3->on();
            image3->setParentItem(item);
            image4 = new Site(0,4,mySiteMenu);
            image4->on();
            image4->setParentItem(item);
            image5 = new Site(0,5,mySiteMenu);
            image5->on();
            image5->setParentItem(item);
            image6 = new Site(0,6,mySiteMenu);
            image6->on();
            image6->setParentItem(item);
            image7 = new Site(0,7,mySiteMenu);
            image7->on();
            image7->setParentItem(item);
            image8 = new Site(0,8,mySiteMenu);
            image8->on();
            image8->setParentItem(item);
            addItem(item);
            item->setPos(mouseEvent->scenePos());
            image1->setPos(xper1);
            image2->setPos(xper2);
            image3->setPos(xper3);
            image4->setPos(xper4);
            image5->setPos(xper5);
            image6->setPos(xper6);
            image7->setPos(xper7);
            image8->setPos(xper8);
        }
        break;

        case InsertTrans:
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
    if (myMode == InsertTrans && line != 0) {
        QLineF newLine(line->line().p1(), mouseEvent->scenePos());
        line->setLine(newLine);
    } else if (myMode == MoveItem) {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void ConfigScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (line != 0 && myMode == InsertTrans) {
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
            startItems.first() != endItems.first())
        {
            Site *startItem = qgraphicsitem_cast<Site *>(startItems.first());
            Site *endItem = qgraphicsitem_cast<Site *>(endItems.first());

            //normal transition in cell
            if(startItem->img() == 0 && endItem->img() == 0)
            {
                Transition *transition = new Transition(myTransMenu, startItem, endItem);
                transition->setColor(myLineColor);
                startItem->addTransition(transition);
                endItem->addTransition(transition);
                transition->setZValue(-1000.0);
                transition->setID(0);
                connect(transition, SIGNAL(selectedChange(QGraphicsItem*)),
                        this, SIGNAL(itemSelected(QGraphicsItem*)));
                connect(transition, SIGNAL(deselectedChange(QGraphicsItem*)),
                        this, SIGNAL(itemdeSelected(QGraphicsItem*)));
                addItem(transition);
                transition->updatePosition();
            }
            //transition from cell site to image site
            else if(startItem->img() == 0 && endItem->img() > 0)
            {
                //main transition
                Transition *transition = new Transition(myTransMenu, startItem, endItem);
                transition->setColor(myLineColor);
                startItem->addTransition(transition);
                endItem->addTransition(transition);
                transition->setZValue(-1000.0);
                transition->setID(indx);
                connect(transition, SIGNAL(selectedChange(QGraphicsItem*)),
                        this, SIGNAL(itemSelected(QGraphicsItem*)));
                connect(transition, SIGNAL(deselectedChange(QGraphicsItem*)),
                        this, SIGNAL(itemdeSelected(QGraphicsItem*)));
                addItem(transition);
                transition->updatePosition();

                //get the image and opposite image
                int image = endItem->img();
                int opimage;
                if(image == 1) opimage = 5;
                if(image == 2) opimage = 6;
                if(image == 3) opimage = 7;
                if(image == 4) opimage = 8;
                if(image == 5) opimage = 1;
                if(image == 6) opimage = 2;
                if(image == 7) opimage = 3;
                if(image == 8) opimage = 4;

                //mirror transition
                Site *startImage;
                Site *endImage;
                foreach (QGraphicsItem *item, startItem->childItems()) {
                    startImage = qgraphicsitem_cast<Site *>(item);
                    if(startImage->img() == opimage) break;
                }
                endImage = qgraphicsitem_cast<Site *>(endItem->parentItem());

                //mirror transition
                Transition *mtransition = new Transition(myTransMenu, startImage, endImage);
                mtransition->setColor(myLineColor);
                startImage->addTransition(mtransition);
                endImage->addTransition(mtransition);
                mtransition->setZValue(-1000.0);
                mtransition->setID(indx);
                connect(mtransition, SIGNAL(selectedChange(QGraphicsItem*)),
                        this, SIGNAL(itemSelected(QGraphicsItem*)));
                connect(mtransition, SIGNAL(deselectedChange(QGraphicsItem*)),
                        this, SIGNAL(itemdeSelected(QGraphicsItem*)));
                addItem(mtransition);
                mtransition->updatePosition();
                indx++;
            }
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

//set the scene item physical properties
void ConfigScene::setTransMin1(double energy)
{
    int tid;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->startItem()->setEn(energy);

        tid = item->id();
        //set energy of image boundary transition
        if(tid > 0) {
            foreach(QGraphicsItem *titem, this->items()) {
                if (titem->type() == Transition::Type) {
                    Transition *itransition = qgraphicsitem_cast<Transition *>(titem);
                    if(itransition->id() == tid)
                    {
                        itransition->startItem()->setEn(energy);
                    }
                }
            }
        }
    }
}

void ConfigScene::setTransMin2(double energy)
{
    int tid;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->endItem()->setEn(energy);

        tid = item->id();
        //set energy of image boundary transition
        if(tid > 0) {
            foreach(QGraphicsItem *titem, this->items()) {
                if (titem->type() == Transition::Type) {
                    Transition *itransition = qgraphicsitem_cast<Transition *>(titem);
                    if(itransition->id() == tid)
                    {
                        itransition->endItem()->setEn(energy);
                    }
                }
            }
        }
    }
}

void ConfigScene::setTransBar(double energy)
{
    int tid;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->setEn(energy);
        tid = item->id();
        //set energy of image boundary transition
        if(tid > 0) {
            foreach(QGraphicsItem *item, this->items()) {
                if (item->type() == Transition::Type) {
                    Transition *itransition = qgraphicsitem_cast<Transition *>(item);
                    if(itransition->id() == tid)
                    {
                        itransition->setEn(energy);
                    }
                }
            }
        }
    }
}

void ConfigScene::setStartMod(int nn, double energy)
{
    int tid;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->startItem()->setNNMod(nn, energy);

        tid = item->id();
        //set energy of image boundary transition
        if(tid > 0) {
            foreach(QGraphicsItem *titem, this->items()) {
                if (titem->type() == Transition::Type) {
                    Transition *itransition = qgraphicsitem_cast<Transition *>(titem);
                    if(itransition->id() == tid)
                    {
                        itransition->startItem()->setNNMod(nn, energy);
                    }
                }
            }
        }
    }
}

void ConfigScene::setEndMod(int nn, double energy)
{
    int tid;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->endItem()->setNNMod(nn, energy);

        tid = item->id();
        //set energy of image boundary transition
        if(tid > 0) {
            foreach(QGraphicsItem *titem, this->items()) {
                if (titem->type() == Transition::Type) {
                    Transition *itransition = qgraphicsitem_cast<Transition *>(titem);
                    if(itransition->id() == tid)
                    {
                        itransition->endItem()->setNNMod(nn, energy);
                    }
                }
            }
        }
    }
}

void ConfigScene::setStartPreFac(double pf)
{
    int tid;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->setStartPrefac(pf);
        tid = item->id();
        //set energy of image boundary transition
        if(tid > 0) {
            foreach(QGraphicsItem *item, this->items()) {
                if (item->type() == Transition::Type) {
                    Transition *itransition = qgraphicsitem_cast<Transition *>(item);
                    if(itransition->id() == tid)
                    {
                        itransition->setStartPrefac(pf);
                    }
                }
            }
        }
    }
}

void ConfigScene::setEndPreFac(double pf)
{
    int tid;
    if (isItemChange(Transition::Type)) {
        Transition *item = qgraphicsitem_cast<Transition *>(selectedItems().first());
        item->setEndPrefac(pf);
        tid = item->id();
        //set energy of image boundary transition
        if(tid > 0) {
            foreach(QGraphicsItem *item, this->items()) {
                if (item->type() == Transition::Type) {
                    Transition *itransition = qgraphicsitem_cast<Transition *>(item);
                    if(itransition->id() == tid)
                    {
                        itransition->setEndPrefac(pf);
                    }
                }
            }
        }
    }
}
