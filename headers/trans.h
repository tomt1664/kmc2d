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
#include <QDebug>
class QGraphicsItem;

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
class QObject;
QT_END_NAMESPACE

// class for the transition pathways that connect pairs of states
class Transition : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

public:
    enum { Type = UserType + 4 };

    Transition(QMenu *contextMenu, Site *startItem, Site *endItem,
      QGraphicsItem *parent = 0); // constructor: passed the start and end sites

    int type() const Q_DECL_OVERRIDE { return Type; }
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void setColor(const QColor &color) { myColor = color; }
    Site *startItem() const { return myStartItem; }
    Site *endItem() const { return myEndItem; }

    void setID(int id) { m_id = id; } // set pairing ID
    int id() { return m_id; }
    double en() { return m_en; } // barrier energy
    void setEn(double en) { m_en = en; }
    void setStartPrefac(double pf) { m_startprefac = pf; } // forward prefactor
    double startPrefac() { return m_startprefac; }
    void setEndPrefac(double pf) { m_endprefac = pf; } // backward prefactor
    double endPrefac() { return m_endprefac; }

    void updatePosition();
    void highlight() { m_highlight = 1; } // highlight transition
    void stopHighlight() { m_highlight = 0; } // turn off highlighting

signals:
    void selectedChange(QGraphicsItem *item); // selected signals for the properties toolbox
    void deselectedChange(QGraphicsItem *item);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;

private:
    int m_id; // pairing ID
    int m_highlight; // basic highlight
    int m_drawBars; // paint the barrier values
    double m_en; // barrier energy
    double m_startprefac; // forward prefactor
    double m_endprefac; // backward prefactor
    Site *myStartItem; // start site
    Site *myEndItem; // end site
    QColor myColor; // transition color
    QMenu *myContextMenu; // right-click menu
};

#endif // TRANS_H
