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
#ifndef QT_NO_PRINTER
#include <QPrinter>
#include <QPrintDialog>
#endif
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

MainWindow::MainWindow()
{
    createActions();
    createToolBox();
    createMenus();

    //set initial cell dimensions
    xcell = 400;
    ycell = 400;

    scene = new ConfigScene(siteMenu, transMenu, xcell, ycell, this);
    scene->setSceneRect(QRectF(0, 0, xcell, ycell));
    scene->setBackgroundBrush(Qt::lightGray);

    //connect to configscene
    connect(scene, SIGNAL(itemSelected(QGraphicsItem*)),
                this, SLOT(itemSelected(QGraphicsItem*)));
    connect(scene, SIGNAL(itemdeSelected(QGraphicsItem*)),
                this, SLOT(itemdeSelected(QGraphicsItem*)));

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

//delete all items in the simulation cell (and images)
void MainWindow::clearCell()
{
    foreach (QGraphicsItem *item, scene->items()) {
        if (item->type() == Transition::Type) {
            scene->removeItem(item);
            Transition *transition = qgraphicsitem_cast<Transition *>(item);
            transition->startItem()->removeTransition(transition);
            transition->endItem()->removeTransition(transition);
            delete item;
        }
    }
    foreach (QGraphicsItem *item, scene->items()) {
        if (item->type() == Site::Type) {
            qgraphicsitem_cast<Site *>(item)->removeTransitions();
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
                             int nsite = qgraphicsitem_cast<Site *>(item)->stat();
                             double nen = qgraphicsitem_cast<Site *>(item)->en();
                             scene->addSite(nsite,nen,xadd,yadd,indx,i,j);
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
                            double nbar = itransition->en();
                            scene->addTrans(myStartItem,myEndItem,nbar);
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
                double nbar = itransition->en();
                scene->addTransPair(myStartItem1,myEndItem1,myStartItem2,myEndItem2,nbar);

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
                    nbar = itransition->en();
                    scene->addTrans(myStartItem1,myEndItem1,nbar);
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
                double nbar = itransition->en();
                scene->addTransPair(myStartItem1,myEndItem1,myStartItem2,myEndItem2,nbar);

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
                    nbar = itransition->en();
                    scene->addTrans(myStartItem1,myEndItem1,nbar);
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

//update spinboxes to selected transition properties
void MainWindow::itemSelected(QGraphicsItem *item)
{
    Transition *transition = qgraphicsitem_cast<Transition *>(item);
    double baren = transition->en();
    double min1 = transition->startItem()->en();
    double min2 = transition->endItem()->en();
    barSpinBox->setDisabled(false);
    min1SpinBox->setDisabled(false);
    min2SpinBox->setDisabled(false);
    barSpinBox->setValue(baren);
    min1SpinBox->setValue(min1);
    min2SpinBox->setValue(min2);
    startModifier->setDisabled(false);
    endModifier->setDisabled(false);
    startModSpinBox->setDisabled(false);
    endModSpinBox->setDisabled(false);
    startPreFactor->setDisabled(false);
    endPreFactor->setDisabled(false);

    double startpf = transition->startPrefac();
    double endpf = transition->endPrefac();
    double startmod = transition->startItem()->nnMod(1);
    double endmod = transition->endItem()->nnMod(1);
    startModSpinBox->setValue(startmod);
    endModSpinBox->setValue(endmod);
    startPreFactor->setValue(startpf);
    endPreFactor->setValue(endpf);
}

//item deselected
void MainWindow::itemdeSelected(QGraphicsItem *item)
{
    barSpinBox->setValue(0.0);
    min1SpinBox->setValue(0.0);
    min2SpinBox->setValue(0.0);
    startModSpinBox->setValue(0.0);
    endModSpinBox->setValue(0.0);
    startPreFactor->setValue(0.0);
    endPreFactor->setValue(0.0);
    startModifier->setCurrentIndex(0);
    endModifier->setCurrentIndex(0);
    barSpinBox->setDisabled(true);
    min1SpinBox->setDisabled(true);
    min2SpinBox->setDisabled(true);
    startModifier->setDisabled(true);
    endModifier->setDisabled(true);
    startModSpinBox->setDisabled(true);
    endModSpinBox->setDisabled(true);
    startPreFactor->setDisabled(true);
    endPreFactor->setDisabled(true);
}

//update the graph view and site properties on spinbox change
void MainWindow::min1Changed()
{
    double energy = min1SpinBox->value();
    curveDisplay->setMin1(energy);
    scene->setTransMin1(energy);
}

//update the graph view and site properties on spinbox change
void MainWindow::min2Changed()
{
    double energy = min2SpinBox->value();
    curveDisplay->setMin2(energy);
    scene->setTransMin2(energy);
}

//update the graph view and transition property on spinbox change
void MainWindow::barChanged()
{
    double energy = barSpinBox->value();
    curveDisplay->setBar(energy);
    scene->setTransBar(energy);
}

//update the nn site energy modifier (start item)
void MainWindow::startModChanged()
{
    double energy = startModSpinBox->value();
    int nn = startModifier->currentIndex() + 1;
    qDebug() << energy << " " << nn;
    scene->setStartMod(nn, energy);
}

//update the nn site energy modifier (start item)
void MainWindow::endModChanged()
{
    double energy = endModSpinBox->value();
    int nn = endModifier->currentIndex() + 1;
    qDebug() << energy << " " << nn;
    scene->setEndMod(nn, energy);
}

//update the start-direction pre-factor
void MainWindow::startPreFacChanged()
{
    double pf = startPreFactor->value();
    qDebug() << pf;
    scene->setStartPreFac(pf);
}

//update the start-direction pre-factor
void MainWindow::endPreFacChanged()
{
    double pf = endPreFactor->value();
    qDebug() << pf;
    scene->setEndPreFac(pf);
}

//update spinbox value dependent on combobox
void MainWindow::startModCBChanged()
{
    int nn = startModifier->currentIndex() + 1;
    if(!scene->selectedItems().isEmpty()) {
    Transition *transition = qgraphicsitem_cast<Transition *>(scene->selectedItems().front());
    double pf = transition->startItem()->nnMod(nn);
    startModSpinBox->setValue(pf);
    }
}

//update spinbox value dependent on combobox
void MainWindow::endModCBChanged()
{
    int nn = endModifier->currentIndex() + 1;
    if(!scene->selectedItems().isEmpty()) {
    Transition *transition = qgraphicsitem_cast<Transition *>(scene->selectedItems().front());
    double pf = transition->endItem()->nnMod(nn);
    endModSpinBox->setValue(pf);
    }
}

//set site as occupied
void MainWindow::occupied()
{
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (item->type() == Site::Type) {
            Site *site = qgraphicsitem_cast<Site *>(item);
            site->on();
        }
    }
    scene->update();
}

//set site as unoccupied
void MainWindow::unoccupied()
{
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (item->type() == Site::Type) {
            Site *site = qgraphicsitem_cast<Site *>(item);
            site->off();
        }
    }
    scene->update();
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

    curveDisplay = new CurveDisplay;

    min1SpinBox = new QDoubleSpinBox;
    min1SpinBox->setRange(-5, 5);
    min1SpinBox->setSingleStep(0.1);
    min1SpinBox->setValue(0.0);
    min1SpinBox->setDisabled(true);

    barSpinBox = new QDoubleSpinBox;
    barSpinBox->setRange(-5, 9);
    barSpinBox->setSingleStep(0.1);
    barSpinBox->setValue(0.0);
    barSpinBox->setDisabled(true);

    min2SpinBox = new QDoubleSpinBox;
    min2SpinBox->setRange(-5, 5);
    min2SpinBox->setSingleStep(0.1);
    min2SpinBox->setValue(0.0);
    min2SpinBox->setDisabled(true);

    connect(min1SpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(min1Changed()));
    connect(min2SpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(min2Changed()));
    connect(barSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(barChanged()));

    QHBoxLayout *energiesLayout = new QHBoxLayout;
    energiesLayout->addWidget(min1SpinBox);
    energiesLayout->addWidget(barSpinBox);
    energiesLayout->addWidget(min2SpinBox);

    startModifier = new QComboBox;
    startModifier->addItem("1");
    startModifier->addItem("2");
    startModifier->addItem("3");
    startModifier->addItem("4");
    startModifier->setDisabled(true);

    endModifier = new QComboBox;
    endModifier->addItem("1");
    endModifier->addItem("2");
    endModifier->addItem("3");
    endModifier->addItem("4");
    endModifier->setDisabled(true);

    startModSpinBox = new QDoubleSpinBox;
    startModSpinBox->setRange(-5,5);
    startModSpinBox->setSingleStep(0.1);
    startModSpinBox->setValue(0.0);
    startModSpinBox->setDisabled(true);

    endModSpinBox = new QDoubleSpinBox;
    endModSpinBox->setRange(-5,5);
    endModSpinBox->setSingleStep(0.1);
    endModSpinBox->setValue(0.0);
    endModSpinBox->setDisabled(true);

    startPreFactor = new QDoubleSpinBox;
    startPreFactor->setRange(0,99);
    startPreFactor->setSingleStep(1.0);
    startPreFactor->setValue(0.0);
    startPreFactor->setDisabled(true);

    endPreFactor = new QDoubleSpinBox;
    endPreFactor->setRange(0,99);
    endPreFactor->setSingleStep(1.0);
    endPreFactor->setValue(0.0);
    endPreFactor->setDisabled(true);

    connect(startModSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(startModChanged()));
    connect(endModSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(endModChanged()));
    connect(startPreFactor, SIGNAL(valueChanged(double)),
            this, SLOT(startPreFacChanged()));
    connect(endPreFactor, SIGNAL(valueChanged(double)),
            this, SLOT(endPreFacChanged()));
    connect(startModifier, SIGNAL(currentIndexChanged(int)),
            this, SLOT(startModCBChanged()));
    connect(endModifier, SIGNAL(currentIndexChanged(int)),
            this, SLOT(endModCBChanged()));

    QHBoxLayout *modifierLayout = new QHBoxLayout;
    modifierLayout->addWidget(startModifier);
    modifierLayout->addStretch(0);
    modifierLayout->addWidget(endModifier);

    QHBoxLayout *modEnLayout = new QHBoxLayout;
    modEnLayout->addWidget(startModSpinBox);
    modEnLayout->addStretch(0);
    modEnLayout->addWidget(endModSpinBox);

    QHBoxLayout *prefactorLayout = new QHBoxLayout;
    prefactorLayout->addWidget(startPreFactor);
    prefactorLayout->addStretch(0);
    prefactorLayout->addWidget(endPreFactor);

    QVBoxLayout *createBox = new QVBoxLayout;
    createBox->addLayout(sceneButtonLayout);
    createBox->addWidget(curveDisplay);
    createBox->addLayout(energiesLayout);
    createBox->addLayout(modifierLayout);
    createBox->addLayout(modEnLayout);
    createBox->addLayout(prefactorLayout);
    createBox->addStretch(0);

    QWidget *itemWidget = new QWidget;
    itemWidget->setLayout(createBox);

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
    deleteAction = new QAction(("&Delete"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("Delete object from system"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

    clearAction = new QAction(("&Clear"), this);
    clearAction->setShortcut(tr("Clear"));
    clearAction->setStatusTip(tr("Clear simulation cell"));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearCell()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Quit KMC2D"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    openAction = new QAction(tr("&Open"), this);
    openAction->setShortcut(tr("Open"));
    openAction->setStatusTip("Open configuration file");
    connect(openAction, SIGNAL(triggered()), this, SLOT(openfile()));

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(tr("Save"));
    saveAction->setStatusTip("Save configuration file");
    connect(saveAction, SIGNAL(triggered()), this, SLOT(savefile()));

    printAction = new QAction(tr("&Print"), this);
    printAction->setShortcut(tr("Print"));
    printAction->setStatusTip(tr("Print the system view"));
    connect(printAction, SIGNAL(triggered()), this, SLOT(print()));

    aboutAction = new QAction(tr("A&bout"), this);
    aboutAction->setShortcut(tr("Ctrl+B"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    setOccupied = new QAction(tr("O&ccupied"),this);
    setOccupied->setShortcut(Qt::Key_O);
    setOccupied->setStatusTip(tr("Set site as occupied"));
    connect(setOccupied, SIGNAL(triggered()), this, SLOT(occupied()));

    setUnoccupied = new QAction(tr("&Unoccupied"),this);
    setUnoccupied->setShortcut(Qt::Key_U);
    setUnoccupied->setStatusTip(tr("Set site as unoccupied"));
    connect(setUnoccupied, SIGNAL(triggered()), this, SLOT(unoccupied()));
}


void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(printAction);
    fileMenu->addAction(exitAction);

    itemMenu = menuBar()->addMenu(tr("&System"));
    itemMenu->addAction(deleteAction);
    itemMenu->addAction(clearAction);

    aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutAction);

    siteMenu = menuBar()->addMenu(tr("&Site"));
    siteMenu->addAction(setOccupied);
    siteMenu->addAction(setUnoccupied);

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


//function to open file dialog and read in the XML configuration file
void MainWindow::openfile()
{
    QString inputfile = QFileDialog::getOpenFileName(this, "Open XML File",
                                                    QString(),
                                                    "XML Files (*.xml)");


    /*
    if (!inputfile.isNull()) {




        QString line;

        atoms.clear(); //clear configuration arrays
        xatmpos.clear();
        yatmpos.clear();

        nitems = 0;

        QFile file(inputfile);
        if (!file.open(QFile::ReadOnly)) {
            throw std::bad_exception();
        }
        QTextStream input(&file);

        line = input.readLine();
        bool ok;
        nitems = line.toInt(&ok,10);

        if(!ok)
        {
            QMessageBox msgbox;
            msgbox.setText("Error reading XYZ file");
            msgbox.exec();
            return;
        }

        line = input.readLine();
        QStringList cell = line.split(" ",QString::SkipEmptyParts);
        if(cell.size() == 3)
        {
            bounds[0] = 0.0;
            bounds[1] = 100*cell[0].toFloat(&ok);
            bounds[2] = 0.0;
            bounds[3] = 100*cell[1].toFloat(&ok);
        }

        for(int i = 0; i < nitems; i++)
        {
            line = input.readLine();
            QStringList coords = line.split(" ",QString::SkipEmptyParts);
            if(coords.size() != 4)
            {
                QMessageBox msgbox;
                msgbox.setText("Error reading XYZ file");
                msgbox.exec();
                return;
            }
            float xap =  100*coords[1].toFloat(&ok);
            float yap =  100*coords[2].toFloat(&ok);

            xatmpos.append(xap);
            yatmpos.append(yap);
        }

        populateScene(); // re-draw scene with new coordinates
        gView->setScene(scene);
    }
    */
}

void MainWindow::savefile() //save scene configuration to an xyz file
{
    QString savefile = QFileDialog::getSaveFileName(this, "Save coordinates",
                                                    QString(),
                                                    "XML Files (*.xml)");

    if (!savefile.isNull()) {

        QFile sfile(savefile);
        if (sfile.open(QFile::WriteOnly | QFile::Truncate))
        {
            QXmlStreamWriter xmlWriter(&sfile);
            xmlWriter.setAutoFormatting(true);
            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("KMC2DData");
            xmlWriter.writeAttribute("version", "v1.0");
            xmlWriter.writeStartElement("GraphicsItemList");
            foreach( QGraphicsItem* item, scene->items())
            {
                if(item->type() == Site::Type )
                {
                    Site *mySite = qgraphicsitem_cast<Site *>(item);
                    xmlWriter.writeStartElement("Site");
                    xmlWriter.writeAttribute("xCoord", QString::number(mySite->x()));
                    xmlWriter.writeAttribute("yCoord", QString::number(mySite->y()));
                    xmlWriter.writeEndElement();
                }
            }
            xmlWriter.writeEndElement();
            xmlWriter.writeEndElement();
            sfile.close();
        } else
        {
            QMessageBox msgbox;
            msgbox.setText("Error writing XML file");
            msgbox.exec();
            return;
        }
    } else
    {
        QMessageBox msgbox;
        msgbox.setText("Error writing XML file");
        msgbox.exec();
        return;
    }
}


void MainWindow::print()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        view->render(&painter);
    }
#endif
}
