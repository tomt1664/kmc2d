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

class Transition : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

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

    void setID(int id) { m_id = id; }
    int id() { return m_id; }
    double en() { return m_en; }
    void setEn(double en) { m_en = en; }

    void updatePosition();

signals:
    void selectedChange(QGraphicsItem *item);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;

//    void mousePressEvent(QGraphicsSceneMouseEvent *event)
//    {
//        emit selectedChange(this);
//                qDebug() << "Bar" << m_en << " " << this->startItem()->en() << " " << this->endItem()->en();
//    }

private:
    int m_id;
    double m_en;
    Site *myStartItem;
    Site *myEndItem;
    QColor myColor;
    QMenu *myContextMenu;
};

#endif // TRANS_H
