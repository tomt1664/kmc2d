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
#include <QCloseEvent>
#include <QSvgGenerator>
#include <QPainter>

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

    //connect to configscene for selection/deselection signals
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
    zoomSlider->setValue(200);

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

    //initialise simulation
    qsrand(123);
    nstep = 0;
    pstep = 1;
    kmcDetail = 2;
    setTemp(300);
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

    simulationStatus->clear();
    simulationTime->setText(QString::number(0.0));
    m_time = 0.0;
    nstep = 0;
    pstep = 1;
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
                             double m1 = qgraphicsitem_cast<Site *>(item)->nnMod(1);
                             double m2 = qgraphicsitem_cast<Site *>(item)->nnMod(2);
                             double m3 = qgraphicsitem_cast<Site *>(item)->nnMod(3);
                             double m4 = qgraphicsitem_cast<Site *>(item)->nnMod(4);
                             double m5 = qgraphicsitem_cast<Site *>(item)->nnMod(5);
                             double m6 = qgraphicsitem_cast<Site *>(item)->nnMod(6);
                             scene->addSite(nsite,nen,xadd,yadd,indx,i,j,m1,m2,m3,m4,m5,m6);
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
                            double startPF = itransition->startPrefac();
                            double endPF = itransition->endPrefac();
                            scene->addTrans(myStartItem,myEndItem,nbar,0,startPF,endPF);
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
                    double startPF = itransition->startPrefac();
                    double endPF = itransition->endPrefac();
                    scene->addTrans(myStartItem1,myEndItem1,nbar,0,startPF,endPF);
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
                    double startPF = itransition->startPrefac();
                    double endPF = itransition->endPrefac();
                    scene->addTrans(myStartItem1,myEndItem1,nbar,0,startPF,endPF);
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
    modimage->setDisabled(false);
    pfimage->setDisabled(false);

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
    modimage->setDisabled(true);
    startPreFactor->setDisabled(true);
    endPreFactor->setDisabled(true);
    pfimage->setDisabled(true);
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
    scene->setStartMod(nn, energy);
}

//update the nn site energy modifier (start item)
void MainWindow::endModChanged()
{
    double energy = endModSpinBox->value();
    int nn = endModifier->currentIndex() + 1;
    scene->setEndMod(nn, energy);
}

//update the start-direction pre-factor
void MainWindow::startPreFacChanged()
{
    double pf = startPreFactor->value();
    scene->setStartPreFac(pf);
}

//update the start-direction pre-factor
void MainWindow::endPreFacChanged()
{
    double pf = endPreFactor->value();
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

//update output detail
void MainWindow::simDetailChanged()
{
    kmcDetail = detailComboBox->currentIndex() + 1;
}

//set site as occupied
void MainWindow::occupied()
{
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (item->type() == Site::Type) {
            Site *site = qgraphicsitem_cast<Site *>(item);
            site->on();
            foreach (QGraphicsItem *child, item->childItems() )
            {
                qgraphicsitem_cast<Site *>(child)->on();
            }
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
            foreach (QGraphicsItem *child, item->childItems() )
            {
                qgraphicsitem_cast<Site *>(child)->off();
            }
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
    addUsiteButton->setIconSize(QSize(24, 24));
    addUsiteButton->setCheckable(true);
    addUsiteButton->setChecked(true);
    addUsiteButton->setToolTip("Add unoccupied site");

    addSiteButton = new QToolButton;
    addSiteButton->setIcon(QIcon(QPixmap(":/icons/osite.png")));
    addSiteButton->setIconSize(QSize(24, 24));
    addSiteButton->setCheckable(true);
    addSiteButton->setChecked(false);
    addSiteButton->setToolTip("Add occupied site");

    addTransButton = new QToolButton;
    addTransButton->setIcon(QIcon(QPixmap(":/icons/trans.png")));
    addTransButton->setIconSize(QSize(24, 24));
    addTransButton->setCheckable(true);
    addTransButton->setChecked(false);
    addTransButton->setToolTip("Add transition");

    selectButton = new QToolButton;
    selectButton->setIcon(QIcon(QPixmap(":/icons/point.png")));
    selectButton->setIconSize(QSize(24, 24));
    selectButton->setCheckable(true);
    selectButton->setChecked(false);
    selectButton->setToolTip("Select item");

    deleteButton = new QToolButton;
    deleteButton->setIcon(QIcon(QPixmap(":/icons/delete.png")));
    deleteButton->setIconSize(QSize(24, 24));
    deleteButton->setToolTip("Delete Item");

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
    sceneButtonLayout->addWidget(deleteButton, 0, 4);

    connect(sceneGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(sceneGroupClicked(int)));

    imageButton = new QToolButton;
    imageButton->setIcon(QIcon(QPixmap(":/icons/image.png")));
    imageButton->setIconSize(QSize(24, 24));
    imageButton->setCheckable(true);
    imageButton->setChecked(false);
    imageButton->setToolTip("Display periodic images");

    snapButton = new QToolButton;
    snapButton->setIcon(QIcon(QPixmap(":/icons/snap.png")));
    snapButton->setIconSize(QSize(24, 24));
    snapButton->setCheckable(true);
    snapButton->setChecked(false);
    snapButton->setToolTip("Snap to grid");

    cellSizeButton = new QToolButton;
    cellSizeButton->setIcon(QIcon(QPixmap(":/icons/csize.png")));
    cellSizeButton->setIconSize(QSize(24, 24));
    cellSizeButton->setToolTip("Set cell dimensions");

    expandButton = new QToolButton;
    expandButton->setIcon(QIcon(QPixmap(":/icons/expand.png")));
    expandButton->setIconSize(QSize(24, 24));
    expandButton->setToolTip("Expand system");

    colorPES = new QToolButton;
    colorPES->setIcon(QIcon(QPixmap(":/icons/color.png")));
    colorPES->setIconSize(QSize(24, 24));
    colorPES->setToolTip("Colour bonds to energy");

    sceneButtonLayout->setVerticalSpacing(6);

    sceneButtonLayout->addWidget(imageButton, 1, 0);
    sceneButtonLayout->addWidget(snapButton, 1, 1);
    sceneButtonLayout->addWidget(cellSizeButton, 1, 2);
    sceneButtonLayout->addWidget(expandButton, 1, 3);
    sceneButtonLayout->addWidget(colorPES, 1, 4);

    connect(imageButton, SIGNAL(toggled(bool)),
            this, SLOT(toggleImages(bool)));
    connect(snapButton, SIGNAL(toggled(bool)),this,SLOT(toggleSnap(bool)));
    connect(cellSizeButton, SIGNAL(clicked()),this,SLOT(changeCellSize()));
    connect(expandButton, SIGNAL(clicked()),this,SLOT(expandSystem()));
    connect(deleteButton, SIGNAL(clicked()),this,SLOT(deleteItem()));

    curveDisplay = new CurveDisplay;

    min1SpinBox = new QDoubleSpinBox;
    min1SpinBox->setRange(-5, 5);
    min1SpinBox->setSingleStep(0.1);
    min1SpinBox->setValue(0.0);
    min1SpinBox->setToolTip("Start state energy (eV)");
    min1SpinBox->setDisabled(true);

    barSpinBox = new QDoubleSpinBox;
    barSpinBox->setRange(-5, 9);
    barSpinBox->setSingleStep(0.1);
    barSpinBox->setValue(0.0);
    barSpinBox->setToolTip("Transition point energy (eV)");
    barSpinBox->setDisabled(true);

    min2SpinBox = new QDoubleSpinBox;
    min2SpinBox->setRange(-5, 5);
    min2SpinBox->setSingleStep(0.1);
    min2SpinBox->setValue(0.0);
    min2SpinBox->setToolTip("End state energy (eV)");
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
    startModifier->addItem("5");
    startModifier->addItem("6");
    startModifier->setToolTip("Coordination number");
    startModifier->setDisabled(true);

    endModifier = new QComboBox;
    endModifier->addItem("1");
    endModifier->addItem("2");
    endModifier->addItem("3");
    endModifier->addItem("4");
    endModifier->addItem("5");
    endModifier->addItem("6");
    endModifier->setToolTip("Coordination number");
    endModifier->setDisabled(true);

    startModSpinBox = new QDoubleSpinBox;
    startModSpinBox->setRange(-5,5);
    startModSpinBox->setSingleStep(0.1);
    startModSpinBox->setValue(0.0);
    startModSpinBox->setToolTip("Start state modifier (eV)");
    startModSpinBox->setDisabled(true);

    endModSpinBox = new QDoubleSpinBox;
    endModSpinBox->setRange(-5,5);
    endModSpinBox->setSingleStep(0.1);
    endModSpinBox->setValue(0.0);
    endModSpinBox->setToolTip("End state modifier (eV)");
    endModSpinBox->setDisabled(true);

    startPreFactor = new QDoubleSpinBox;
    startPreFactor->setRange(0,99);
    startPreFactor->setSingleStep(1.0);
    startPreFactor->setValue(0.0);
    startPreFactor->setToolTip("Forward prefactor (THz)");
    startPreFactor->setDisabled(true);

    endPreFactor = new QDoubleSpinBox;
    endPreFactor->setRange(0,99);
    endPreFactor->setSingleStep(1.0);
    endPreFactor->setValue(0.0);
    endPreFactor->setToolTip("Backward prefactor (THz)");
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
    modimage = new QLabel();
    modimage->setPixmap(QPixmap(":/icons/modify.png"));
    modimage->setToolTip("Coordination modifier");
    modimage->setDisabled(true);


    QVBoxLayout *startModEnLayout = new QVBoxLayout;
    startModEnLayout->addWidget(startModifier);
    startModEnLayout->addWidget(startModSpinBox);

    QVBoxLayout *endModEnLayout = new QVBoxLayout;
    endModEnLayout->addWidget(endModifier);
    endModEnLayout->addWidget(endModSpinBox);

    modifierLayout->addLayout(startModEnLayout);
    modifierLayout->addStretch(0);
    modifierLayout->addWidget(modimage);
    modifierLayout->addStretch(0);
    modifierLayout->addLayout(endModEnLayout);

    QHBoxLayout *prefactorLayout = new QHBoxLayout;
    pfimage = new QLabel();
    pfimage->setPixmap(QPixmap(":/icons/prefac.png"));
    pfimage->setToolTip("Rate prefactor (THz)");
    pfimage->setDisabled(true);
    prefactorLayout->addWidget(startPreFactor);
    prefactorLayout->addStretch(0);
    prefactorLayout->addWidget(pfimage);
    prefactorLayout->addStretch(0);
    prefactorLayout->addWidget(endPreFactor);

    QVBoxLayout *createBox = new QVBoxLayout;
    createBox->addLayout(sceneButtonLayout);
    createBox->addWidget(curveDisplay);
    createBox->addLayout(energiesLayout);
    createBox->addSpacing(25);
    createBox->addLayout(modifierLayout);
    createBox->addSpacing(25);
    createBox->addLayout(prefactorLayout);
    createBox->addStretch(0);

    QWidget *systemWidget = new QWidget;
    systemWidget->setLayout(createBox);

    QVBoxLayout *simulationLayout = new QVBoxLayout;

    QHBoxLayout *topControls = new QHBoxLayout;

    temperature = new QSpinBox;
    temperature->setRange(0,999);
    temperature->setValue(300);
    temperature->setToolTip("Simulation temperature (K)");

    seed = new QSpinBox;
    seed->setRange(1,999);
    seed->setValue(123);
    seed->setToolTip("Random number generator seed");

    QLabel *tempicon = new QLabel();
    tempicon->setPixmap(QPixmap(":/icons/temp.png"));
    tempicon->setToolTip("Temperature");

    QLabel *diceicon = new QLabel();
    diceicon->setPixmap(QPixmap(":/icons/dice.png"));
    diceicon->setToolTip("Random number generator seed");

    connect(temperature, SIGNAL(valueChanged(int)),this,SLOT(setTemp(int)));
    connect(seed, SIGNAL(valueChanged(int)),this,SLOT(setSeed(int)));

    topControls->addWidget(tempicon);
    topControls->addWidget(temperature);
    topControls->addStretch(0);
    topControls->addWidget(diceicon);
    topControls->addWidget(seed);

    QHBoxLayout *simulationControls = new QHBoxLayout;

    startStopButton = new QToolButton;
    startStopButton->setIconSize(QSize(24, 24));
    startStopButton->setDefaultAction(startAction);

    rewindButton = new QToolButton;
    rewindButton->setIcon(QIcon(QPixmap(":/icons/rewind.png")));
    rewindButton->setIconSize(QSize(24,24));
    rewindButton->setToolTip("Rewind to beginning");

    backButton = new QToolButton;
    backButton->setIcon(QIcon(QPixmap(":/icons/back.png")));
    backButton->setIconSize(QSize(24,24));
    backButton->setToolTip("Step back");

    forwardButton = new QToolButton;
    forwardButton->setIcon(QIcon(QPixmap(":/icons/forward.png")));
    forwardButton->setIconSize(QSize(24,24));
    forwardButton->setToolTip("Step forward");

    recordButton = new QToolButton;
    recordButton->setIcon(QIcon(QPixmap(":/icons/record.png")));
    recordButton->setIconSize(QSize(24,24));
    recordButton->setCheckable(true);
    recordButton->setChecked(false);
    recordButton->setToolTip("Save trajectory");

    connect(forwardButton, SIGNAL(clicked()),this,SLOT(stepForward()));
    connect(backButton, SIGNAL(clicked()),this,SLOT(stepBack()));

    simulationControls->addWidget(startStopButton);
    simulationControls->addStretch(0);
    simulationControls->addWidget(rewindButton);
    simulationControls->addStretch(0);
    simulationControls->addWidget(backButton);
    simulationControls->addStretch(0);
    simulationControls->addWidget(forwardButton);
    simulationControls->addStretch(0);
    simulationControls->addWidget(recordButton);

    QHBoxLayout *infoLayout = new QHBoxLayout;

    delaySpinBox = new QDoubleSpinBox;
    delaySpinBox->setRange(0,99);
    delaySpinBox->setSingleStep(1.0);
    delaySpinBox->setValue(1.0);
    delaySpinBox->setToolTip("Step delay (s)");

    detailComboBox = new QComboBox;
    detailComboBox->addItem("1");
    detailComboBox->addItem("2");
    detailComboBox->addItem("3");
    detailComboBox->setToolTip("Output detail");

    connect(detailComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(simDetailChanged()));

    graphButton = new QToolButton;
    graphButton->setIcon(QIcon(QPixmap(":/icons/plot.png")));
    graphButton->setIconSize(QSize(24,24));
    graphButton->setToolTip("Data plot");

    QLabel *listicon = new QLabel();
    listicon->setPixmap(QPixmap(":/icons/list.png"));
    listicon->setToolTip("Output detail");

    infoLayout->addWidget(delaySpinBox);
    infoLayout->addStretch(0);
    infoLayout->addWidget(listicon);
    infoLayout->addWidget(detailComboBox);
    infoLayout->addStretch(0);
    infoLayout->addWidget(graphButton);

    QHBoxLayout *timeLayout = new QHBoxLayout;

    QLabel *timeicon = new QLabel();
    timeicon->setPixmap(QPixmap(":/icons/time.png"));
    timeicon->setToolTip("Simulation time (s)");

    simulationTime = new QLabel;
    simulationTime->setMaximumWidth(160);
    QPalette* palette = new QPalette();
    palette->setColor(QPalette::Base,QColor(238,238,238,255));
    simulationTime->setPalette(*palette);
    simulationTime->setText(QString::number(0.0));

    timeLayout->addWidget(timeicon);
    timeLayout->addSpacing(10);
    timeLayout->addWidget(simulationTime);

    simulationStatus = new QTextEdit;
    simulationStatus->setReadOnly(true);
    simulationStatus->setBackgroundRole(QPalette::NoRole);
    simulationStatus->setMaximumWidth(210);
    simulationStatus->setPalette(*palette);

    simulationLayout->addLayout(topControls);
    simulationLayout->addSpacing(15);
    simulationLayout->addLayout(simulationControls);
    simulationLayout->addSpacing(15);
    simulationLayout->addLayout(infoLayout);
    simulationLayout->addSpacing(15);
    simulationLayout->addLayout(timeLayout);
    simulationLayout->addSpacing(15);
    simulationLayout->addWidget(simulationStatus);
    simulationLayout->addStretch(0);

    QWidget *simulationWidget = new QWidget;
    simulationWidget->setLayout(simulationLayout);

    toolBox = new QToolBox;
    toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
    toolBox->setMinimumWidth(systemWidget->sizeHint().width());
    toolBox->addItem(systemWidget, tr("System"));
    toolBox->addItem(simulationWidget, tr("Simulation"));
}

void MainWindow::createActions()
{
    deleteAction = new QAction(("&Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip(tr("Delete object from system"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

    clearAction = new QAction(("&Clear"), this);
    clearAction->setShortcut(QKeySequence::Cut);
    clearAction->setStatusTip(tr("Clear simulation cell"));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearCell()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Quit KMC2D"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    openAction = new QAction(tr("&Open"), this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open configuration file");
    connect(openAction, SIGNAL(triggered()), this, SLOT(openfile()));

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("Save configuration file");
    connect(saveAction, SIGNAL(triggered()), this, SLOT(savefile()));

    exportAction = new QAction(tr("&Export"), this);
    exportAction->setShortcut(Qt::CTRL + Qt::Key_E);
    exportAction->setStatusTip("Export system as SVG");
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportSVG()));

    printAction = new QAction(tr("&Print"), this);
    printAction->setShortcut(QKeySequence::Print);
    printAction->setStatusTip(tr("Print the system view"));
    connect(printAction, SIGNAL(triggered()), this, SLOT(print()));

    aboutAction = new QAction(tr("A&bout"), this);
    aboutAction->setShortcut(Qt::CTRL + Qt::Key_A);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    setOccupied = new QAction(tr("O&ccupied"),this);
    setOccupied->setShortcut(Qt::Key_O);
    setOccupied->setStatusTip(tr("Set site as occupied"));
    connect(setOccupied, SIGNAL(triggered()), this, SLOT(occupied()));

    setUnoccupied = new QAction(tr("&Unoccupied"),this);
    setUnoccupied->setShortcut(Qt::Key_U);
    setUnoccupied->setStatusTip(tr("Set site as unoccupied"));
    connect(setUnoccupied, SIGNAL(triggered()), this, SLOT(unoccupied()));

    startAction = new QAction(QIcon(":/icons/play.png"), "Start", this);
    startAction->setShortcut(Qt::Key_P);
    startAction->setToolTip(tr("Run KMC simulation"));
    connect(startAction, SIGNAL(triggered()), this, SLOT(startKMC()));

    stopAction = new QAction(QIcon(":/icons/pause.png"), "Stop", this);
    stopAction->setShortcut(Qt::Key_S);
    stopAction->setToolTip(tr("Stop KMC simulation"));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopKMC()));
}


void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(exportAction);
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

    if (!inputfile.isNull()) {

        clearCell();

        QFile file(inputfile);

        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox msgbox;
            msgbox.setText("Error reading XML file");
            msgbox.exec();
            return;
        }

        QXmlStreamReader xmlReader(&file);



        xmlReader.readNext();


        while(!xmlReader.atEnd()) {
            xmlReader.readNext();
            if(xmlReader.isStartElement()) {
                QString name = xmlReader.name().toString();
                if(name == "Cell") {
                    QXmlStreamAttributes attributes = xmlReader.attributes();
                    foreach(QXmlStreamAttribute attribute, attributes) {
                        QString aname = attribute.name().toString();
                        QString avalue = attribute.value().toString();
                        if(aname == "xDim") {
                            xcell = avalue.toInt();
                        }
                        if(aname == "yDim") {
                            ycell = avalue.toInt();
                        }
                    }
                    //re-draw the system cell
                    scene->changeCell(xcell,ycell);
                    scene->setSceneRect(QRectF(0, 0, xcell, ycell));
                    cell->setRect(0, 0, xcell, ycell);
                    perarea->setRect(-xcell-10, -ycell-10, 3*xcell+20, 3*ycell+20);
                    redrawCells();
                }
                if(name == "Site") {
                    QXmlStreamAttributes attributes = xmlReader.attributes();
                    int xcrd,ycrd,occt;
                    double ent,m1,m2,m3,m4,m5,m6 = 0.0;
                    int sum = 0;
                    foreach(QXmlStreamAttribute attribute, attributes) {
                        QString aname = attribute.name().toString();
                        QString avalue = attribute.value().toString();
                        if(aname == "xCoord") {
                            xcrd = avalue.toInt();
                            sum++;
                        }
                        if(aname == "yCoord") {
                            ycrd = avalue.toInt();
                            sum++;
                        }
                        if(aname == "Occ") {
                            occt = avalue.toInt();
                            sum++;
                        }
                        if(aname == "En") {
                            ent = avalue.toDouble();
                            sum++;
                        }
                        if(aname == "Mod1") m1 = avalue.toDouble();
                        if(aname == "Mod2") m2 = avalue.toDouble();
                        if(aname == "Mod3") m3 = avalue.toDouble();
                        if(aname == "Mod4") m4 = avalue.toDouble();
                        if(aname == "Mod5") m5 = avalue.toDouble();
                        if(aname == "Mod6") m6 = avalue.toDouble();
                        }
                        if(sum != 4) {
                            QMessageBox msgbox;
                            msgbox.setText("Error. Malformed system file: Site attributes missing");
                            msgbox.exec();
                            return;
                    }
                    scene->addSite(occt,ent,xcrd,ycrd,0,0,0,m1,m2,m3,m4,m5,m6);
                }
                if(name == "Transition") {
                    QXmlStreamAttributes attributes = xmlReader.attributes();
                    int sxcrd,sycrd,excrd,eycrd,idt;
                    double ent,fpf,bpf;
                    int sum = 0;
                    foreach(QXmlStreamAttribute attribute, attributes) {
                        QString aname = attribute.name().toString();
                        QString avalue = attribute.value().toString();
                        if(aname == "xStart") {
                            sxcrd = avalue.toInt();
                            sum++;
                        }
                        if(aname == "yStart") {
                            sycrd = avalue.toInt();
                            sum++;
                        }
                        if(aname == "xEnd") {
                            excrd = avalue.toInt();
                            sum++;
                        }
                        if(aname == "yEnd") {
                            eycrd = avalue.toInt();
                            sum++;
                        }
                        if(aname == "En") {
                            ent = avalue.toDouble();
                            sum++;
                        }
                        if(aname == "startPF") {
                            fpf = avalue.toDouble();
                            sum++;
                        }
                        if(aname == "endPF") {
                            bpf = avalue.toDouble();
                            sum++;
                        }
                        if(aname == "ID") {
                            idt = avalue.toInt();
                            sum++;
                        }
                    }
                    if(sum != 8) {
                        QMessageBox msgbox;
                        msgbox.setText("Error. Malformed system file: Trans attributes missing");
                        msgbox.exec();
                        return;
                    }
                    Site *startItem;
                    Site *endItem;
                    bool foundStart = false;
                    bool foundEnd = false;
                    foreach(QGraphicsItem *item, scene->items()) {
                        if(item->type() == Site::Type ) {
                            Site *sItem = qgraphicsitem_cast<Site *>(item);
                            int xsp = sItem->scenePos().x();
                            int ysp = sItem->scenePos().y();
                            if(xsp == sxcrd && ysp == sycrd) {
                                startItem = sItem;
                                foundStart = true;
                            }
                            if(xsp == excrd && ysp == eycrd) {
                                foundEnd = true;
                                endItem = sItem;
                            }
                        }
                    }
                    if(!foundStart || !foundEnd) {
                        QMessageBox msgbox;
                        msgbox.setText("Error. Malformed system file: hanging transition");
                        msgbox.exec();
                        return;
                    }
                    scene->addTrans(startItem,endItem,ent,idt,fpf,bpf);
                }
            }
        }

    }
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
            xmlWriter.writeStartElement("Cell");
            xmlWriter.writeAttribute("xDim", QString::number(xcell));
            xmlWriter.writeAttribute("yDim", QString::number(ycell));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("ItemList");
            foreach( QGraphicsItem* item, scene->items())
            {
                if(item->type() == Site::Type )
                {
                    if(item->childItems().size() > 0) {
                        Site *mySite = qgraphicsitem_cast<Site *>(item);
                        xmlWriter.writeStartElement("Site");
                        xmlWriter.writeAttribute("xCoord", QString::number(mySite->x()));
                        xmlWriter.writeAttribute("yCoord", QString::number(mySite->y()));
                        xmlWriter.writeAttribute("Occ", QString::number(mySite->stat()));
                        xmlWriter.writeAttribute("En", QString::number(mySite->en()));
                        xmlWriter.writeAttribute("Mod1", QString::number(mySite->nnMod(1)));
                        xmlWriter.writeAttribute("Mod2", QString::number(mySite->nnMod(2)));
                        xmlWriter.writeAttribute("Mod3", QString::number(mySite->nnMod(3)));
                        xmlWriter.writeAttribute("Mod4", QString::number(mySite->nnMod(4)));
                        xmlWriter.writeAttribute("Mod5", QString::number(mySite->nnMod(5)));
                        xmlWriter.writeAttribute("Mod6", QString::number(mySite->nnMod(6)));
                        foreach(QGraphicsItem* child, item->childItems()) {
                            Site *myChild = qgraphicsitem_cast<Site *>(child);
                            xmlWriter.writeStartElement("Image");
                            xmlWriter.writeAttribute("xCoord", QString::number(myChild->scenePos().x()));
                            xmlWriter.writeAttribute("yCoord", QString::number(myChild->scenePos().y()));
                            xmlWriter.writeAttribute("ImgNo", QString::number(myChild->img()));
                            xmlWriter.writeEndElement();
                        }
                        xmlWriter.writeEndElement();
                    }
                }
            }
            foreach( QGraphicsItem* item, scene->items())
            {
                if(item->type() == Transition::Type )
                {
                    Transition *transition = qgraphicsitem_cast<Transition *>(item);
                    xmlWriter.writeStartElement("Transition");
                    xmlWriter.writeAttribute("xStart", QString::number(transition->startItem()->scenePos().x()));
                    xmlWriter.writeAttribute("yStart", QString::number(transition->startItem()->scenePos().y()));
                    xmlWriter.writeAttribute("xEnd", QString::number(transition->endItem()->scenePos().x()));
                    xmlWriter.writeAttribute("yEnd", QString::number(transition->endItem()->scenePos().y()));
                    xmlWriter.writeAttribute("En", QString::number(transition->en()));
                    xmlWriter.writeAttribute("startPF", QString::number(transition->startPrefac()));
                    xmlWriter.writeAttribute("endPF", QString::number(transition->endPrefac()));
                    xmlWriter.writeAttribute("ID", QString::number(transition->id()));
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

//ask to save on exit
void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Quit KMC2D",
                                                                tr("Save system configuration?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if(resBtn == QMessageBox::Cancel) {
        event->ignore();
    } else if(resBtn == QMessageBox::Yes) {
        savefile();
        event->accept();
    } else if(resBtn == QMessageBox::No) {
        event->accept();
    }
}

//expot qgraphicsscene as an SVG file
void MainWindow::exportSVG()
{
    QString exportfile = QFileDialog::getSaveFileName(this, tr("Export as SVG"),
        QString(), tr("SVG files (*.svg)"));

    if (!exportfile.isNull()) {

    QSvgGenerator *svggenerator = new QSvgGenerator();
    svggenerator->setFileName(exportfile);
    svggenerator->setSize(QSize(xcell*3, ycell*3));
    svggenerator->setViewBox(QRect(-xcell, -ycell, xcell*3, ycell*3));
    svggenerator->setTitle(tr("KMC2D Exported Scene"));

    QPainter *svgpainter = new QPainter(svggenerator);
    svgpainter->begin(svggenerator);
    scene->render(svgpainter);
    svgpainter->end();
    }
}

//start the KMC simulation
void MainWindow::startKMC()
{
    startStopButton->setDefaultAction(stopAction);

    simulationStatus->clear();
    simulationStatus->setTextBackgroundColor(Qt::white);
    simulationStatus->setTextColor(Qt::blue);

}

//stop the KMC simulation
void MainWindow::stopKMC()
{
    startStopButton->setDefaultAction(startAction);

//    simulationStatus->setAlignment(Qt::AlignRight);


}

//move the KMC simulation forward 1 step
void MainWindow::stepForward()
{
    // create energy and rate list
    if(pstep == 1) {
        barPFList.clear();
        rateList.clear();
        transList.clear();
        rateTotal = 0.0;
        bool sceneEmpty = true;
        foreach (QGraphicsItem *item, scene->items()) {
            if (item->type() == Site::Type && qgraphicsitem_cast<Site *>(item)->img() == 0) {
            Site *site = qgraphicsitem_cast<Site *>(item);
            site->stopHighlight();
            if(site->stat()) {
                double site_en = site->en();
                //apply coordination modifier to minima
                int coordination = 0;
                double mod_en = 0.0;
                foreach(QGraphicsItem *citem, site->transList()) {
                    if(qgraphicsitem_cast<Transition *>(citem)->startItem()->stat() &&
                            qgraphicsitem_cast<Transition *>(citem)->endItem()->stat()) {
                        coordination++;
                    }
                }
                if(coordination > 0) {
                    mod_en = site->nnMod(coordination);
                }
                foreach(QGraphicsItem *titem, site->transList()) {
                    sceneEmpty = false;
                    Transition *trans = qgraphicsitem_cast<Transition *>(titem);
                    trans->stopHighlight();
                    double trans_en = trans->en();
                    double barrier = trans_en - site_en - mod_en;
                    double prefac;
                    if(trans->startItem() == site) {
                        prefac = trans->startPrefac();
                    }
                    else {
                        prefac = trans->endPrefac();
                    }
                    if(barrier < 0.0) barrier = 0;
                    if(!(trans->startItem()->stat() && trans->endItem()->stat())) {
                        QPointF barPF(barrier,prefac);
                        barPFList.append(barPF);
                        double pathRate = (prefac*1.0e12)*qExp(-barrier*m_beta);
                        rateList.append(pathRate);
                        rateTotal += pathRate;
                        transList.append(titem);
                    }
                    trans->stopHighlight();
                    trans->update();
                }
            }
            }
        }
        if(sceneEmpty) return;
        simulationStatus->clear();
    }

    //highlight exit barriers
    if(pstep == 1) {
        foreach (QGraphicsItem *item, scene->items()) {
            if (item->type() == Site::Type) {
                Site *site = qgraphicsitem_cast<Site *>(item);
                if(site->stat()) {
                    foreach(QGraphicsItem *titem, site->transList()) {
                        Transition *trans = qgraphicsitem_cast<Transition *>(titem);
                        if(!(trans->startItem()->stat() && trans->endItem()->stat())) {
                            trans->highlight();
                            trans->update();
                        }
                    }
                }
            }
        }

        //print barriers
        simulationStatus->setTextColor(Qt::blue);
        simulationStatus->append("Bar (eV) \t Pre-fac (THz)");
        simulationStatus->setTextColor(Qt::black);
        simulationStatus->append(" ");
        foreach(QPointF barPF, barPFList) {
            QString en_val = QString::number(barPF.x());
            QString pf_val = QString::number(barPF.y());
            simulationStatus->setAlignment(Qt::AlignLeft);
            simulationStatus->append(en_val + "\t" + pf_val);
        }
    }

    if(pstep == 2) {
        //print rates
        simulationStatus->clear();
        simulationStatus->setTextColor(Qt::blue);
        simulationStatus->append("Rates (Hz):");
        simulationStatus->append(" ");
        simulationStatus->setTextColor(Qt::black);
        foreach(double irate, rateList) {
            QString prate = QString::number(irate);
            simulationStatus->setAlignment(Qt::AlignRight);
            simulationStatus->append(prate);
        }
    }

    // select transition pathway
    if(pstep == 3) {
        foreach (QGraphicsItem *item, scene->items()) {
            if (item->type() == Site::Type) {
            Site *site = qgraphicsitem_cast<Site *>(item);
            foreach(QGraphicsItem *titem, site->transList()) {
                Transition *trans = qgraphicsitem_cast<Transition *>(titem);
                trans->stopHighlight();
                trans->update();
            }
            }
        }
        double transProb = 0.0;
        int icount = 0;
        double ran1 = qrand()*1.0/RAND_MAX;
        foreach(double pathRate, rateList) {
            if(ran1 > transProb && ran1 <= (transProb + pathRate/rateTotal)) {
                transPath = transList[icount];
                break;
            }
            transProb += pathRate/rateTotal;
            icount++;
        }
        qDebug() << icount;
        simulationStatus->clear();
        simulationStatus->setAlignment(Qt::AlignLeft);
        simulationStatus->setTextColor(Qt::red);
        simulationStatus->append("Rand: "+QString::number(ran1));
        simulationStatus->setTextColor(Qt::black);
        simulationStatus->append(" ");
        int pcount = 0;
        foreach(double irate, rateList) {
            QString prate = QString::number(irate);
            simulationStatus->setAlignment(Qt::AlignRight);
            if(icount == pcount) {
               simulationStatus->setTextBackgroundColor(Qt::red);
            } else {
               simulationStatus->setTextBackgroundColor(QColor(238,238,238,255));
            }
            simulationStatus->append(prate);
            pcount++;
        }
        simulationStatus->setTextBackgroundColor(QColor(238,238,238,255));


        qgraphicsitem_cast<Transition *>(transPath)->highlight();
        qgraphicsitem_cast<Transition *>(transPath)->update();


    }

    //perform the transition
    if(pstep == 4) {
        if(qgraphicsitem_cast<Transition *>(transPath)->startItem()->stat()) {
            qgraphicsitem_cast<Transition *>(transPath)->startItem()->off();
            qgraphicsitem_cast<Transition *>(transPath)->startItem()->update();
            foreach (QGraphicsItem *child, qgraphicsitem_cast<Transition *>(transPath)->startItem()->childItems()) {
                qgraphicsitem_cast<Site *>(child)->off();
                qgraphicsitem_cast<Site *>(child)->update();
            }
            if(qgraphicsitem_cast<Transition *>(transPath)->endItem()->childItems().size() > 0) {
                qgraphicsitem_cast<Transition *>(transPath)->endItem()->on();
                qgraphicsitem_cast<Transition *>(transPath)->endItem()->highlight();
                qgraphicsitem_cast<Transition *>(transPath)->endItem()->update();
                foreach (QGraphicsItem *child, qgraphicsitem_cast<Transition *>(transPath)->endItem()->childItems()) {
                    qgraphicsitem_cast<Site *>(child)->on();
                    qgraphicsitem_cast<Site *>(child)->update();
                }
            } else {
                QGraphicsItem *parent = qgraphicsitem_cast<Transition *>(transPath)->endItem()->parentItem();
                qgraphicsitem_cast<Site *>(parent)->on();
                qgraphicsitem_cast<Site *>(parent)->highlight();
                qgraphicsitem_cast<Site *>(parent)->update();
                foreach (QGraphicsItem *child, parent->childItems()) {
                    qgraphicsitem_cast<Site *>(child)->on();
                    qgraphicsitem_cast<Site *>(child)->update();
                }
            }

        } else {
            qgraphicsitem_cast<Transition *>(transPath)->endItem()->off();
            qgraphicsitem_cast<Transition *>(transPath)->endItem()->update();
            foreach (QGraphicsItem *child, qgraphicsitem_cast<Transition *>(transPath)->endItem()->childItems()) {
                qgraphicsitem_cast<Site *>(child)->off();
                qgraphicsitem_cast<Site *>(child)->update();
            }
            if(qgraphicsitem_cast<Transition *>(transPath)->startItem()->childItems().size() > 0) {
                qgraphicsitem_cast<Transition *>(transPath)->startItem()->on();
                qgraphicsitem_cast<Transition *>(transPath)->startItem()->highlight();
                qgraphicsitem_cast<Transition *>(transPath)->startItem()->update();
                foreach (QGraphicsItem *child, qgraphicsitem_cast<Transition *>(transPath)->startItem()->childItems()) {
                    qgraphicsitem_cast<Site *>(child)->on();
                    qgraphicsitem_cast<Site *>(child)->update();
                }
            } else {
                QGraphicsItem *parent = qgraphicsitem_cast<Transition *>(transPath)->startItem()->parentItem();
                qgraphicsitem_cast<Site *>(parent)->on();
                qgraphicsitem_cast<Site *>(parent)->highlight();
                qgraphicsitem_cast<Site *>(parent)->update();
                foreach (QGraphicsItem *child, parent->childItems()) {
                    qgraphicsitem_cast<Site *>(child)->on();
                    qgraphicsitem_cast<Site *>(child)->update();
                }
            }
        }
    }

    //update time
    if(pstep == 5) {
        qgraphicsitem_cast<Transition *>(transPath)->stopHighlight();
        qgraphicsitem_cast<Transition *>(transPath)->update();

        double timeInt;
        double ran2 = qrand()*1.0/RAND_MAX;
        timeInt = -qLn(ran2)/rateTotal;
        simulationStatus->clear();
        simulationStatus->setTextBackgroundColor(QColor(238,238,238,255));
        simulationStatus->setAlignment(Qt::AlignLeft);
        simulationStatus->setTextColor(Qt::blue);
        simulationStatus->append("Rand: "+QString::number(ran2));
        simulationStatus->append(" ");
        simulationStatus->append("Residence time (s):");
        simulationStatus->append(" ");
        simulationStatus->setTextColor(Qt::black);
        simulationStatus->setAlignment(Qt::AlignLeft);
        simulationStatus->append(QString::number(timeInt));
        simulationStatus->setAlignment(Qt::AlignRight);
        m_time += timeInt;
        simulationTime->clear();
        simulationTime->setText(QString::number(m_time));
    }

    pstep++;
    if(pstep > 5) pstep = 1;
}

//move the simulation back one step
void MainWindow::stepBack()
{

}

//set the temperature
void MainWindow::setTemp(int tmp)
{
    m_temp = tmp*1.0;
    m_beta = 1.60217662e-19/(m_temp*1.38064852e-23);
}

//set the seed of the Mersenne Twister
void MainWindow::setSeed(int isd)
{
    qsrand(isd);
    nstep = 0;
    pstep = 1;
}
