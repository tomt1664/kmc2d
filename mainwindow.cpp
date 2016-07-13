/****************************************************************************
** KMC2D: A 2D lattice kinetic Monte Carlo model constructor and simulator
**
** Built using Qt 5.6
**
** Tom Trevethan 2016
** tptrevethan@googlemail.com
****************************************************************************/

#include "trans.h"
#include "latsite.h"
#include "configscene.h"
#include "mainwindow.h"
#include "cellsizedialog.h"
#include "expanddialog.h"

#include <QtWidgets>
#include <QDebug>

const int InsertTextButton = 10;

MainWindow::MainWindow()
{
    createActions();
    createToolBox();
    createMenus();

    xcell = 400;
    ycell = 400;

    scene = new ConfigScene(siteMenu, transMenu, xcell, ycell, this);
    scene->setSceneRect(QRectF(0, 0, xcell, ycell));
    scene->setBackgroundBrush(Qt::lightGray);

    //draw the simulation cell white on the gray background
    cell = new QGraphicsRectItem;
    cell->setRect(0, 0, xcell, ycell);
    cell->setBrush(Qt::white);
    cell->setZValue(-2000);
    scene->addItem(cell);

    //draw the periodic area
    perarea = new QGraphicsRectItem;
    perarea->setRect(-xcell-10, -ycell-10, 3*xcell+20, 3*ycell+20);
    perarea->setBrush(Qt::lightGray);
    perarea->setPen(QPen(Qt::lightGray));
    perarea->setZValue(-3000);
    scene->addItem(perarea);

    //draw the periodic cells
    drawCells();

    //add slider to the right of qgraphicsview for zooming
    zoomSlider = new QSlider;
    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(500);
    zoomSlider->setValue(250);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(toolBox);
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(view);
    layout->addWidget(zoomSlider);

    connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    setupMatrix();

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    setCentralWidget(widget);
    setWindowTitle(tr("KMC2D"));
}

void MainWindow::backgroundButtonGroupClicked(QAbstractButton *button)
{
    QList<QAbstractButton *> buttons = backgroundButtonGroup->buttons();
    foreach (QAbstractButton *myButton, buttons) {
        if (myButton != button)
            button->setChecked(false);
    }
    QString text = button->text();
    if (text == tr("Blue Grid"))
        scene->setBackgroundBrush(QPixmap(":/images/background1.png"));
    else if (text == tr("White Grid"))
        scene->setBackgroundBrush(QPixmap(":/images/background2.png"));
    else if (text == tr("Gray Grid"))
        scene->setBackgroundBrush(QPixmap(":/images/background3.png"));
    else
        scene->setBackgroundBrush(QPixmap(":/images/background4.png"));

    scene->update();
    view->update();
}

//delete item
void MainWindow::deleteItem()
{
    int irem = 0;
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (item->type() == Transition::Type) {
            scene->removeItem(item);
            Transition *transition = qgraphicsitem_cast<Transition *>(item);
            irem = transition->id();
            transition->startItem()->removeTransition(transition);
            transition->endItem()->removeTransition(transition);
            delete item;
        }
    }

    //if transition has a periodic mirror (id > 0) then delete that too
    if(irem > 0)
    {
        foreach (QGraphicsItem *item, scene->items() ) {
            if (item->type() == Transition::Type) {
                Transition *mtransition = qgraphicsitem_cast<Transition *>(item);
                if(mtransition->id() == irem)
                {
                    mtransition->startItem()->removeTransition(mtransition);
                    mtransition->endItem()->removeTransition(mtransition);
                    delete mtransition;
                    break;
                }
            }
        }
    }

    //if delete site, delete all child items as well (periodic images)
    foreach (QGraphicsItem *item, scene->selectedItems()) {
         if (item->type() == Site::Type)
         {
             qgraphicsitem_cast<Site *>(item)->removeTransitions();

             foreach (QGraphicsItem *child, item->childItems() )
             {
                 qgraphicsitem_cast<Site *>(child)->removeTransitions();
             }
             scene->removeItem(item);
             delete item;
         }
    }
}

void MainWindow::sceneGroupClicked(int)
{
    scene->setMode(ConfigScene::Mode(sceneGroup->checkedId()));
}

//toggle the mask to hide the periodic images
void MainWindow::toggleImages(bool on)
{
    if(on)
    {
        pcellc1->setZValue(-4000);
        pcellc2->setZValue(-4000);
        pcellc3->setZValue(-4000);
        pcellc4->setZValue(-4000);
        pcellc5->setZValue(-4000);
        pcellc6->setZValue(-4000);
        pcellc7->setZValue(-4000);
        pcellc8->setZValue(-4000);
    } else
    {
        pcellc1->setZValue(0);
        pcellc2->setZValue(0);
        pcellc3->setZValue(0);
        pcellc4->setZValue(0);
        pcellc5->setZValue(0);
        pcellc6->setZValue(0);
        pcellc7->setZValue(0);
        pcellc8->setZValue(0);
    }
}

//toggle the snap-to-grid function
void MainWindow::toggleSnap(bool on)
{
    if(on)
    {
        scene->setSnap(true);
    } else
    {
        scene->setSnap(false);
    }
}

//change the simulation cell size: launch dialog box
void MainWindow::changeCellSize()
{
    int xcellOld = xcell;
    int ycellOld = ycell;
    CellSizeDialog cellsizedialog(xcell,ycell);
    cellsizedialog.exec();
    if(cellsizedialog.cancel()) return;
    //get the new dimensions
    xcell = cellsizedialog.getx();
    ycell = cellsizedialog.gety();

    //re-draw the system
    scene->changeCell(xcell,ycell);
    scene->setSceneRect(QRectF(0, 0, xcell, ycell));
    cell->setRect(0, 0, xcell, ycell);
    perarea->setRect(-xcell-10, -ycell-10, 3*xcell+20, 3*ycell+20);
    redrawCells();

    //move the image items
    foreach (QGraphicsItem *item, scene->items() ) {
         if (item->type() == Site::Type)
         {
             foreach (QGraphicsItem *child, item->childItems() )
             {
                 qreal ximg = child->scenePos().x();
                 qreal yimg = child->scenePos().y();
                 qreal ximgp = child->x();
                 qreal yimgp = child->y();
                 if(ximg > 0 && ximg < xcellOld && yimg < 0) {
                     child->setY(yimgp - (ycell - ycellOld));
                 } else if(ximg > xcellOld && yimg < 0) {
                     child->setY(yimgp - (ycell - ycellOld));
                     child->setX(ximgp + (xcell - xcellOld));
                 } else if(ximg > xcellOld && yimg > 0 && yimg < ycellOld) {
                     child->setX(ximgp + (xcell - xcellOld));
                 } else if(ximg > xcellOld && yimg > ycellOld) {
                     child->setY(yimgp + (ycell - ycellOld));
                     child->setX(ximgp + (xcell - xcellOld));
                 } else if(ximg > 0 && ximg < xcellOld && yimg > ycellOld) {
                     child->setY(yimgp + (ycell - ycellOld));
                 } else if(ximg < 0 && yimg > ycellOld) {
                     child->setY(yimgp + (ycell - ycellOld));
                     child->setX(ximgp - (xcell - xcellOld));
                 } else if(ximg < 0 && yimg > 0 && yimg < ycellOld) {
                     child->setX(ximgp - (xcell - xcellOld));
                 } else if(ximg < 0 && yimg < 0) {
                     child->setY(yimgp - (ycell - ycellOld));
                     child->setX(ximgp - (xcell - xcellOld));
                 }
             }
             qgraphicsitem_cast<Site *>(item)->updateTrans();
         }
    }
}

//function to multiply out the system
//this performs three main functions:
// 1. Increase the cell size
// 2. Replicate the sites within the initial cell in x and y directions
// 3. Create and replace transitions based on the initial and final periodicity
void MainWindow::expandSystem()
{
    int xcellOld = xcell;
    int ycellOld = ycell;
    ExpandDialog expanddialog;
    expanddialog.exec();
    if(expanddialog.cancel()) return;
    //get the expansion multiples
    int xexp = expanddialog.getx();
    int yexp = expanddialog.gety();

    xcell = xcell*xexp;
    ycell = ycell*yexp;

    //re-draw the system
    scene->changeCell(xcell,ycell);
    scene->setSceneRect(QRectF(0, 0, xcell, ycell));
    cell->setRect(0, 0, xcell, ycell);
    perarea->setRect(-xcell-10, -ycell-10, 3*xcell+20, 3*ycell+20);
    redrawCells();

    //move the image items
    foreach (QGraphicsItem *item, scene->items() ) {
         if (item->type() == Site::Type)
         {
             foreach (QGraphicsItem *child, item->childItems() )
             {
                 qreal ximg = child->scenePos().x();
                 qreal yimg = child->scenePos().y();
                 qreal ximgp = child->x();
                 qreal yimgp = child->y();
                 if(ximg > 0 && ximg < xcellOld && yimg < 0) {
                     child->setY(yimgp - (ycell - ycellOld));
                 } else if(ximg > xcellOld && yimg < 0) {
                     child->setY(yimgp - (ycell - ycellOld));
                     child->setX(ximgp + (xcell - xcellOld));
                 } else if(ximg > xcellOld && yimg > 0 && yimg < ycellOld) {
                     child->setX(ximgp + (xcell - xcellOld));
                 } else if(ximg > xcellOld && yimg > ycellOld) {
                     child->setY(yimgp + (ycell - ycellOld));
                     child->setX(ximgp + (xcell - xcellOld));
                 } else if(ximg > 0 && ximg < xcellOld && yimg > ycellOld) {
                     child->setY(yimgp + (ycell - ycellOld));
                 } else if(ximg < 0 && yimg > ycellOld) {
                     child->setY(yimgp + (ycell - ycellOld));
                     child->setX(ximgp - (xcell - xcellOld));
                 } else if(ximg < 0 && yimg > 0 && yimg < ycellOld) {
                     child->setX(ximgp - (xcell - xcellOld));
                 } else if(ximg < 0 && yimg < 0) {
                     child->setY(yimgp - (ycell - ycellOld));
                     child->setX(ximgp - (xcell - xcellOld));
                 }
             }
             qgraphicsitem_cast<Site *>(item)->updateTrans();
         }
    }

    //put transition start and end items into a temporary vector
    QList<Site*> startSites;
    QList<Site*> endSites;
    QList<Transition*> stransition;
    QList<Transition*> btransition;
    foreach (QGraphicsItem *item, scene->items()) {
        if (item->type() == Transition::Type) {
            Transition *itransition = qgraphicsitem_cast<Transition *>(item);
            if(itransition->id() == 0)
            {
                stransition.append(itransition);
                startSites.append(itransition->startItem());
                endSites.append(itransition->endItem());
            } else {
                btransition.append(itransition);
            }
        }
    }

    int indx = 0;
    //replicate the sites
    foreach (QGraphicsItem *item, scene->items() ) {
         if (item->type() == Site::Type)
         {
             if(item->childItems().size() > 0)
             {
                 indx++;
                 qgraphicsitem_cast<Site *>(item)->setID(indx);
                 qgraphicsitem_cast<Site *>(item)->setRep(0,0);
                 foreach(QGraphicsItem *citem, item->childItems())
                 {
                     qgraphicsitem_cast<Site *>(citem)->setID(indx);
                     qgraphicsitem_cast<Site *>(citem)->setRep(0,0);
                 }

                 for(int i=0; i < xexp; i++)
                 {
                     for(int j=0; j < yexp; j++)
                     {
                         if((i+j) > 0) {
                             double xadd = item->x() + i*xcellOld;
                             double yadd = item->y() + j*ycellOld;
                             scene->addSite(qgraphicsitem_cast<Site *>(item)->stat(),xadd,yadd,indx,i,j);
                         }
                     }
                 }
             }
         }
    }

    int istart;
    int iend;
    //replicate transitions
    for(int i=0; i < xexp; i++)
    {
        for(int j=0; j < yexp; j++)
        {
            if((i+j) > 0) {
                foreach(Transition *itransition, stransition) {
                    if(itransition->id() == 0)
                        {
                            istart = itransition->startItem()->id();
                            iend = itransition->endItem()->id();
                            Site *myStartItem;
                            Site *myEndItem;
                            foreach (QGraphicsItem *sitem, scene->items()) {
                                if (sitem->type() == Site::Type) {
                                    if(qgraphicsitem_cast<Site *>(sitem)->xr() == i && qgraphicsitem_cast<Site *>(sitem)->yr() == j)
                                    {
                                        if(qgraphicsitem_cast<Site *>(sitem)->id() == istart)
                                        {
                                            myStartItem = qgraphicsitem_cast<Site *>(sitem);
                                        }
                                        if(qgraphicsitem_cast<Site *>(sitem)->id() == iend)
                                        {
                                            myEndItem = qgraphicsitem_cast<Site *>(sitem);
                                        }
                                    }
                                }
                            }
                            scene->addTrans(myStartItem,myEndItem);
                        }
                    }
                }
            }
        }

//  rearrange boundary transitions
//  and add inter-replica transitions

    int imgStart;
    int imgEnd;
    int sindx = 0;
    int eindx = 0;
    foreach(Transition *itransition, btransition) {
        imgStart = itransition->startItem()->img();
        imgEnd = itransition->endItem()->img();
        Site *myStartItem1;
        Site *myEndItem1;
        Site *myStartItem2;
        Site *myEndItem2;

        //do the x-direction replication
        if(imgEnd == 7) {
            sindx = itransition->startItem()->id();
            eindx = itransition->endItem()->id();
            //loop over y-replicas
            for(int iy = 0; iy < yexp; iy++) {
                //add new 0,0 boundary transition
                foreach (QGraphicsItem *sitem, scene->items()) {
                    if (sitem->type() == Site::Type) {
                        if(qgraphicsitem_cast<Site *>(sitem)->xr() == (xexp-1) && qgraphicsitem_cast<Site *>(sitem)->yr() == iy)
                            {
                                if(qgraphicsitem_cast<Site *>(sitem)->id() == eindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                                {
                                    myStartItem1 = qgraphicsitem_cast<Site *>(sitem);
                                }
                            }
                        if(qgraphicsitem_cast<Site *>(sitem)->xr() == 0 && qgraphicsitem_cast<Site *>(sitem)->yr() == iy)
                        {
                            if(qgraphicsitem_cast<Site *>(sitem)->id() == sindx && qgraphicsitem_cast<Site *>(sitem)->img() == 3)
                            {
                                myEndItem1 = qgraphicsitem_cast<Site *>(sitem);
                            }
                         }
                    }
                }
                //add new xexp,0 boundary transition
                foreach (QGraphicsItem *sitem, scene->items()) {
                    if (sitem->type() == Site::Type) {
                        if(qgraphicsitem_cast<Site *>(sitem)->xr() == 0 && qgraphicsitem_cast<Site *>(sitem)->yr() == iy)
                        {
                            if(qgraphicsitem_cast<Site *>(sitem)->id() == sindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                            {
                                myStartItem2 = qgraphicsitem_cast<Site *>(sitem);
                            }
                        }
                        if(qgraphicsitem_cast<Site *>(sitem)->xr() == (xexp-1) && qgraphicsitem_cast<Site *>(sitem)->yr() == iy)
                        {
                            if(qgraphicsitem_cast<Site *>(sitem)->id() == eindx && qgraphicsitem_cast<Site *>(sitem)->img() == 7)
                             {
                                 myEndItem2 = qgraphicsitem_cast<Site *>(sitem);
                             }
                        }
                    }
                }
                scene->addTransPair(myStartItem1,myEndItem1,myStartItem2,myEndItem2);

                //add inter-replica transitions
                for(int ix = 0; ix < (xexp-1); ix++) {
                    foreach (QGraphicsItem *sitem, scene->items()) {
                        if (sitem->type() == Site::Type) {
                            if(qgraphicsitem_cast<Site *>(sitem)->xr() == ix && qgraphicsitem_cast<Site *>(sitem)->yr() == iy)
                            {
                                if(qgraphicsitem_cast<Site *>(sitem)->id() == eindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                                {
                                    myStartItem1 = qgraphicsitem_cast<Site *>(sitem);
                                }
                            }
                            if(qgraphicsitem_cast<Site *>(sitem)->xr() == (ix+1) && qgraphicsitem_cast<Site *>(sitem)->yr() == iy)
                            {
                                if(qgraphicsitem_cast<Site *>(sitem)->id() == sindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                                {
                                    myEndItem1 = qgraphicsitem_cast<Site *>(sitem);
                                }
                            }
                        }
                    }
                    scene->addTrans(myStartItem1,myEndItem1);
                }
            }
        }

        //do the y-direction replication
        if(imgEnd == 5) {
            sindx = itransition->startItem()->id();
            eindx = itransition->endItem()->id();
            //loop over y-replicas
            for(int ix = 0; ix < xexp; ix++) {
                //add new 0 boundary transition
                foreach (QGraphicsItem *sitem, scene->items()) {
                    if (sitem->type() == Site::Type) {
                        if(qgraphicsitem_cast<Site *>(sitem)->yr() == (yexp-1) && qgraphicsitem_cast<Site *>(sitem)->xr() == ix)
                            {
                                if(qgraphicsitem_cast<Site *>(sitem)->id() == eindx && qgraphicsitem_cast<Site *>(sitem)->img() == 5)
                                {
                                    myStartItem1 = qgraphicsitem_cast<Site *>(sitem);
                                }
                            }
                        if(qgraphicsitem_cast<Site *>(sitem)->yr() == 0 && qgraphicsitem_cast<Site *>(sitem)->xr() == ix)
                        {
                            if(qgraphicsitem_cast<Site *>(sitem)->id() == sindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                            {
                                myEndItem1 = qgraphicsitem_cast<Site *>(sitem);
                            }
                         }
                    }
                }
                //add new yexp boundary transition
                foreach (QGraphicsItem *sitem, scene->items()) {
                    if (sitem->type() == Site::Type) {
                        if(qgraphicsitem_cast<Site *>(sitem)->yr() == 0 && qgraphicsitem_cast<Site *>(sitem)->xr() == ix)
                        {
                            if(qgraphicsitem_cast<Site *>(sitem)->id() == sindx && qgraphicsitem_cast<Site *>(sitem)->img() == 1)
                            {
                                myStartItem2 = qgraphicsitem_cast<Site *>(sitem);
                            }
                        }
                        if(qgraphicsitem_cast<Site *>(sitem)->yr() == (yexp-1) && qgraphicsitem_cast<Site *>(sitem)->xr() == ix)
                        {
                            if(qgraphicsitem_cast<Site *>(sitem)->id() == eindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                             {
                                 myEndItem2 = qgraphicsitem_cast<Site *>(sitem);
                             }
                        }
                    }
                }
                scene->addTransPair(myStartItem1,myEndItem1,myStartItem2,myEndItem2);

                //add inter-replica transitions
                for(int iy = 0; iy < (yexp-1); iy++) {
                    foreach (QGraphicsItem *sitem, scene->items()) {
                        if (sitem->type() == Site::Type) {
                            if(qgraphicsitem_cast<Site *>(sitem)->yr() == iy && qgraphicsitem_cast<Site *>(sitem)->xr() == ix)
                            {
                                if(qgraphicsitem_cast<Site *>(sitem)->id() == eindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                                {
                                    myStartItem1 = qgraphicsitem_cast<Site *>(sitem);
                                }
                            }
                            if(qgraphicsitem_cast<Site *>(sitem)->yr() == (iy+1) && qgraphicsitem_cast<Site *>(sitem)->xr() == ix)
                            {
                                if(qgraphicsitem_cast<Site *>(sitem)->id() == sindx && qgraphicsitem_cast<Site *>(sitem)->img() == 0)
                                {
                                    myEndItem1 = qgraphicsitem_cast<Site *>(sitem);
                                }
                            }
                        }
                    }
                    scene->addTrans(myStartItem1,myEndItem1);
                }
            }
        }
    }

    //delete the old boundary transitions
    foreach (QGraphicsItem *item, scene->items()) {
        if (item->type() == Transition::Type) {
            Transition *itransition = qgraphicsitem_cast<Transition *>(item);
            imgStart = itransition->startItem()->img();
            imgEnd = itransition->endItem()->img();
            int isx = itransition->startItem()->xr();
            int isy = itransition->startItem()->yr();
            int iex = itransition->endItem()->xr();
            int iey = itransition->endItem()->yr();
            if(isx == 0 && isy == 0 && iex == 0 && iey == 0 && imgStart != 0 && imgEnd == 0) {
                scene->removeItem(item);
                itransition->startItem()->removeTransition(itransition);
                itransition->endItem()->removeTransition(itransition);
                delete item;
            } else if(iex == 0 && iey == 0 && isx == 0 && isy == 0 && imgStart == 0 && imgEnd != 0) {
                scene->removeItem(item);
                itransition->startItem()->removeTransition(itransition);
                itransition->endItem()->removeTransition(itransition);
                delete item;
            }

        }
    }
}

//launch about program dialog box
void MainWindow::about()
{
    QMessageBox::about(this, tr("About "),
            tr("<p><b>KMC2D version 0.22</b></p>"
               "<br> "
               "<p>Build date: %1"
               "<br> "
               "<br>This program is built using Qt 5.6"
               "<br> "
               "<br>Tom Trevethan"
               "<br>Email: <a href=\"mailto:tptrevethan@googlemail.com>\">tptrevethan@googlemail.com</a>"
               "<br>2016"
               "</p>").arg(__TIMESTAMP__));

}

//create the left hand side toolbox
void MainWindow::createToolBox()
{
    QGridLayout *sceneButtonLayout = new QGridLayout;

    addUsiteButton = new QToolButton;
    addUsiteButton->setIcon(QIcon(QPixmap(":/icons/uosite.png")));
    addUsiteButton->setIconSize(QSize(32, 32));
    addUsiteButton->setCheckable(true);
    addUsiteButton->setChecked(true);
    addUsiteButton->setToolTip("Add unoccupied site");

    addSiteButton = new QToolButton;
    addSiteButton->setIcon(QIcon(QPixmap(":/icons/osite.png")));
    addSiteButton->setIconSize(QSize(32, 32));
    addSiteButton->setCheckable(true);
    addSiteButton->setChecked(false);
    addSiteButton->setToolTip("Add occupied site");

    addTransButton = new QToolButton;
    addTransButton->setIcon(QIcon(QPixmap(":/icons/trans.png")));
    addTransButton->setIconSize(QSize(32, 32));
    addTransButton->setCheckable(true);
    addTransButton->setChecked(false);
    addTransButton->setToolTip("Add transition");

    selectButton = new QToolButton;
    selectButton->setIcon(QIcon(QPixmap(":/icons/point.png")));
    selectButton->setIconSize(QSize(32, 32));
    selectButton->setCheckable(true);
    selectButton->setChecked(false);
    selectButton->setToolTip("Select item");

    sceneGroup = new QButtonGroup;
    sceneGroup->setExclusive(true);
    sceneGroup->addButton(addUsiteButton, int(ConfigScene::InsertUSite));
    sceneGroup->addButton(addSiteButton, int(ConfigScene::InsertSite));
    sceneGroup->addButton(addTransButton, int(ConfigScene::InsertTrans));
    sceneGroup->addButton(selectButton, int(ConfigScene::MoveItem));

    sceneButtonLayout->addWidget(addUsiteButton, 0, 0);
    sceneButtonLayout->addWidget(addSiteButton, 0, 1);
    sceneButtonLayout->addWidget(addTransButton, 0, 2);
    sceneButtonLayout->addWidget(selectButton, 0, 3);

    connect(sceneGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(sceneGroupClicked(int)));

    imageButton = new QToolButton;
    imageButton->setIcon(QIcon(QPixmap(":/icons/image.png")));
    imageButton->setIconSize(QSize(32, 32));
    imageButton->setCheckable(true);
    imageButton->setChecked(false);
    imageButton->setToolTip("Display periodic images");

    snapButton = new QToolButton;
    snapButton->setIcon(QIcon(QPixmap(":/icons/snap.png")));
    snapButton->setIconSize(QSize(32, 32));
    snapButton->setCheckable(true);
    snapButton->setChecked(false);
    snapButton->setToolTip("Snap to grid");

    cellSizeButton = new QToolButton;
    cellSizeButton->setIcon(QIcon(QPixmap(":/icons/csize.png")));
    cellSizeButton->setIconSize(QSize(32, 32));
    cellSizeButton->setToolTip("Set cell dimensions");

    expandButton = new QToolButton;
    expandButton->setIcon(QIcon(QPixmap(":/icons/expand.png")));
    expandButton->setIconSize(QSize(32, 32));
    expandButton->setToolTip("Expand system");

    sceneButtonLayout->addWidget(imageButton, 1, 0);
    sceneButtonLayout->addWidget(snapButton, 1, 1);
    sceneButtonLayout->addWidget(cellSizeButton, 1, 2);
    sceneButtonLayout->addWidget(expandButton, 1, 3);

    connect(imageButton, SIGNAL(toggled(bool)),
            this, SLOT(toggleImages(bool)));
    connect(snapButton, SIGNAL(toggled(bool)),this,SLOT(toggleSnap(bool)));
    connect(cellSizeButton, SIGNAL(clicked()),this,SLOT(changeCellSize()));
    connect(expandButton, SIGNAL(clicked()),this,SLOT(expandSystem()));

//    QGridLayout *layout = new QGridLayout;
//    layout->addWidget(createCellWidget(tr("Site")), 0, 0);

//    layout->setRowStretch(3, 10);
//    layout->setColumnStretch(2, 10);



    QWidget *itemWidget = new QWidget;
    itemWidget->setLayout(sceneButtonLayout);

    backgroundButtonGroup = new QButtonGroup(this);
    connect(backgroundButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(backgroundButtonGroupClicked(QAbstractButton*)));

    QGridLayout *backgroundLayout = new QGridLayout;
    backgroundLayout->addWidget(createBackgroundCellWidget(tr("Blue Grid"),
                                                           ":/images/background1.png"), 0, 0);
    backgroundLayout->addWidget(createBackgroundCellWidget(tr("White Grid"),
                                                           ":/images/background2.png"), 0, 1);
    backgroundLayout->addWidget(createBackgroundCellWidget(tr("Gray Grid"),
                                                           ":/images/background3.png"), 1, 0);
    backgroundLayout->addWidget(createBackgroundCellWidget(tr("No Grid"),
                                                           ":/images/background4.png"), 1, 1);

    backgroundLayout->setRowStretch(2, 10);
    backgroundLayout->setColumnStretch(2, 10);

    QWidget *backgroundWidget = new QWidget;
    backgroundWidget->setLayout(backgroundLayout);


    toolBox = new QToolBox;
    toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
    toolBox->setMinimumWidth(itemWidget->sizeHint().width());
    toolBox->addItem(itemWidget, tr("System"));
    toolBox->addItem(backgroundWidget, tr("Simulation"));
}

void MainWindow::createActions()
{
    deleteAction = new QAction(QIcon(":/images/delete.png"), tr("&Delete"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("Delete item from diagram"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Quit Scenediagram example"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    aboutAction = new QAction(tr("A&bout"), this);
    aboutAction->setShortcut(tr("Ctrl+B"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}


void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAction);

    itemMenu = menuBar()->addMenu(tr("&Item"));
    itemMenu->addAction(deleteAction);

    aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutAction);

    siteMenu = menuBar()->addMenu(tr("&Site"));
    siteMenu->addAction(aboutAction);

    transMenu = menuBar()->addMenu(tr("&Transition"));
    transMenu->addAction(deleteAction);

}

QWidget *MainWindow::createBackgroundCellWidget(const QString &text, const QString &image)
{
    QToolButton *button = new QToolButton;
    button->setText(text);
    button->setIcon(QIcon(image));
    button->setIconSize(QSize(50, 50));
    button->setCheckable(true);
    backgroundButtonGroup->addButton(button);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    return widget;
}

QWidget *MainWindow::createCellWidget(const QString &text)
{

    QToolButton *button = new QToolButton;
    button->setIcon(QIcon(QPixmap(":/icons/uosite.png")));
    button->setIconSize(QSize(32, 32));
    button->setCheckable(true);
    button->setToolTip("Unoccupied site");
    buttonGroup->addButton(button);



    QGridLayout *layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    return widget;
}

void MainWindow::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QMatrix matrix;
    matrix.scale(scale, scale);

    view->setMatrix(matrix);
}

//draw the periodic cells and the mask
void MainWindow::drawCells()
{
    pcell1 = new QGraphicsRectItem;
    pcell1->setRect(0,ycell, xcell, ycell);
    pcell1->setBrush(QColor(218, 218, 218, 255));
    pcell1->setPen(QPen(Qt::gray));
    pcell1->setZValue(-2900);
    scene->addItem(pcell1);

    pcell2 = new QGraphicsRectItem;
    pcell2->setRect(xcell,ycell, xcell, ycell);
    pcell2->setBrush(QColor(218, 218, 218, 255));
    pcell2->setPen(QPen(Qt::gray));
    pcell2->setZValue(-2900);
    scene->addItem(pcell2);

    pcell3 = new QGraphicsRectItem;
    pcell3->setRect(xcell,0, xcell, ycell);
    pcell3->setBrush(QColor(218, 218, 218, 255));
    pcell3->setPen(QPen(Qt::gray));
    pcell3->setZValue(-2900);
    scene->addItem(pcell3);

    pcell4 = new QGraphicsRectItem;
    pcell4->setRect(xcell,-ycell, xcell, ycell);
    pcell4->setBrush(QColor(218, 218, 218, 255));
    pcell4->setPen(QPen(Qt::gray));
    pcell4->setZValue(-2900);
    scene->addItem(pcell4);

    pcell5 = new QGraphicsRectItem;
    pcell5->setRect(0,-ycell, xcell, ycell);
    pcell5->setBrush(QColor(218, 218, 218, 255));
    pcell5->setPen(QPen(Qt::gray));
    pcell5->setZValue(-2900);
    scene->addItem(pcell5);

    pcell6 = new QGraphicsRectItem;
    pcell6->setRect(-xcell,-ycell, xcell, ycell);
    pcell6->setBrush(QColor(218, 218, 218, 255));
    pcell6->setPen(QPen(Qt::gray));
    pcell6->setZValue(-2900);
    scene->addItem(pcell6);

    pcell7 = new QGraphicsRectItem;
    pcell7->setRect(-xcell,0, xcell, ycell);
    pcell7->setBrush(QColor(218, 218, 218, 255));
    pcell7->setPen(QPen(Qt::gray));
    pcell7->setZValue(-2900);
    scene->addItem(pcell7);

    pcell8 = new QGraphicsRectItem;
    pcell8->setRect(-xcell,ycell, xcell, ycell);
    pcell8->setBrush(QColor(218, 218, 218, 255));
    pcell8->setPen(QPen(Qt::gray));
    pcell8->setZValue(-2900);
    scene->addItem(pcell8);

    pcellc1 = new QGraphicsRectItem;
    pcellc1->setRect(0,ycell, xcell, ycell+40);
    pcellc1->setBrush(Qt::lightGray);
    pcellc1->setPen(QPen(Qt::lightGray));
    pcellc1->setZValue(0);
    scene->addItem(pcellc1);

    pcellc2 = new QGraphicsRectItem;
    pcellc2->setRect(xcell,ycell, xcell+40, ycell+40);
    pcellc2->setBrush(Qt::lightGray);
    pcellc2->setPen(QPen(Qt::lightGray));
    pcellc2->setZValue(0);
    scene->addItem(pcellc2);

    pcellc3 = new QGraphicsRectItem;
    pcellc3->setRect(xcell,0, xcell+40, ycell);
    pcellc3->setBrush(Qt::lightGray);
    pcellc3->setPen(QPen(Qt::lightGray));
    pcellc3->setZValue(0);
    scene->addItem(pcellc3);

    pcellc4 = new QGraphicsRectItem;
    pcellc4->setRect(xcell,-ycell-40, xcell+40, ycell+40);
    pcellc4->setBrush(Qt::lightGray);
    pcellc4->setPen(QPen(Qt::lightGray));
    pcellc4->setZValue(0);
    scene->addItem(pcellc4);

    pcellc5 = new QGraphicsRectItem;
    pcellc5->setRect(0,-ycell-40, xcell, ycell+40);
    pcellc5->setBrush(Qt::lightGray);
    pcellc5->setPen(QPen(Qt::lightGray));
    pcellc5->setZValue(0);
    scene->addItem(pcellc5);

    pcellc6 = new QGraphicsRectItem;
    pcellc6->setRect(-xcell-40,-ycell-40, xcell+40, ycell+40);
    pcellc6->setBrush(Qt::lightGray);
    pcellc6->setPen(QPen(Qt::lightGray));
    pcellc6->setZValue(0);
    scene->addItem(pcellc6);

    pcellc7 = new QGraphicsRectItem;
    pcellc7->setRect(-xcell-40,0, xcell+40, ycell);
    pcellc7->setBrush(Qt::lightGray);
    pcellc7->setPen(QPen(Qt::lightGray));
    pcellc7->setZValue(0);
    scene->addItem(pcellc7);

    pcellc8 = new QGraphicsRectItem;
    pcellc8->setRect(-xcell-40,ycell, xcell+40, ycell+40);
    pcellc8->setBrush(Qt::lightGray);
    pcellc8->setPen(QPen(Qt::lightGray));
    pcellc8->setZValue(0);
    scene->addItem(pcellc8);
}

//replot the periodic cells and mask for the toggle signal
void MainWindow::redrawCells()
{
    pcell1->setRect(0,ycell, xcell, ycell);
    pcell2->setRect(xcell,ycell, xcell, ycell);
    pcell3->setRect(xcell,0, xcell, ycell);
    pcell4->setRect(xcell,-ycell, xcell, ycell);
    pcell5->setRect(0,-ycell, xcell, ycell);
    pcell6->setRect(-xcell,-ycell, xcell, ycell);
    pcell7->setRect(-xcell,0, xcell, ycell);
    pcell8->setRect(-xcell,ycell, xcell, ycell);
    pcellc1->setRect(0,ycell, xcell, ycell+40);
    pcellc2->setRect(xcell,ycell, xcell+40, ycell+40);
    pcellc3->setRect(xcell,0, xcell+40, ycell);
    pcellc4->setRect(xcell,-ycell-40, xcell+40, ycell+40);
    pcellc5->setRect(0,-ycell-40, xcell, ycell+40);
    pcellc6->setRect(-xcell-40,-ycell-40, xcell+40, ycell+40);
    pcellc7->setRect(-xcell-40,0, xcell+40, ycell);
    pcellc8->setRect(-xcell-40,ycell, xcell+40, ycell+40);
}
