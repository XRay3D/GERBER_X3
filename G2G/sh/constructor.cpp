#include "constructor.h"
#include "circle.h"
#include "rectangle.h"

#include <scene.h>

namespace ShapePr {

PrType Constructor::type = NullPT;
int Constructor::counter = 0;
QPointF Constructor::point;
QGraphicsItem* Constructor::item = nullptr;
bool Constructor::m_snap = false;

bool Constructor::snap()
{
    return m_snap;
}

void Constructor::setSnap(bool snap)
{
    m_snap = snap;
}

void Constructor::addItem(const QPointF& value)
{
    point = value;
    switch (type) {
    case Rect:
        qDebug() << type << counter << item << point;
        switch (counter) {
        case 0:
            item = new Rectangle(point, point + QPointF{ 1, 1 });
            break;
        default:
            type = NullPT;
            item = nullptr;
        }
        break;
    case Line:
        break;
    case Elipse:
        qDebug() << type << counter << item << point;
        switch (counter) {
        case 0:
            item = new Circle(point, point + QPointF{ 1, 1 });
            break;
        default:
            type = NullPT;
            item = nullptr;
        }
        break;
    case ArcPT:
        break;
    case Text:
        break;
    default:
        break;
    }
    ++counter;
}

void Constructor::updateItem(const QPointF& value)
{
    point = value;
    switch (type) {
    case Rect:
        if (item != nullptr)
            static_cast<Rectangle*>(item)->setRh(point);
        break;
    case Line:
        break;
    case Elipse:
        if (item != nullptr)
            static_cast<Circle*>(item)->setRh(point);
        break;
    case ArcPT:
        break;
    case Text:
        break;
    default:
        break;
    }
}

PrType Constructor::getType()
{
    return type;
}

void Constructor::setType(const PrType& value)
{
    type = value;
    counter = 0;
}

void Constructor::update(bool click)
{
    if (click)
        ++counter;
    switch (type) {
    case Rect:
        qDebug() << type << counter << item << point;
        switch (counter) {
        case 1:
            item = new Rectangle(point, point + QPointF{ 1, 1 });
            ++counter;
            break;
        case 2:
            static_cast<Rectangle*>(item)->setRh(point);
            break;
        default:
            type = NullPT;
            item = nullptr;
        }
        break;
    case Line: //{
        //        QGraphicsLineItem *item = scene()->addLine(QLineF(pt1, pt2), QPen(Qt::red, 0.0));
        //        item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable
        //                       | QGraphicsItem::ItemIsFocusable);
        //        m_pt = NullPT;
        //    } break;
    case Elipse:
        qDebug() << type << counter << item << point;
        switch (counter) {
        case 1:
            item = new Circle(point, point + QPointF{ 1, 1 });
            ++counter;
            break;
        case 2:
            static_cast<Circle*>(item)->setRh(point);
            break;
        default:
            type = NullPT;
            item = nullptr;
        }
        break;
    case ArcPT: //{
    //        QGraphicsEllipseItem *item = scene()->addEllipse(std::min(pt1.x(), pt2.x()),
    //                                                         std::min(pt1.y(), pt2.y()),
    //                                                         std::max(pt1.x(), pt2.x())
    //                                                             - std::min(pt1.x(), pt2.x()),
    //                                                         std::max(pt1.y(), pt2.y())
    //                                                             - std::min(pt1.y(), pt2.y()),
    //                                                         QPen(Qt::red, 0.0),
    //                                                         QColor(255, 255, 255, 100));
    //        item->setStartAngle(0);
    //        item->setSpanAngle(123);
    //        item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable
    //                       | QGraphicsItem::ItemIsFocusable);
    //        m_pt = NullPT;
    //    } break;
    case Text: //{
        //        QGraphicsTextItem *item = scene()->addText("QGraphicsTextItem");
        //        // QPen(Qt::red, 0.0), QColor(255, 255, 255, 100));
        //        item->setDefaultTextColor(QColor(255, 255, 255, 100));
        //        // item->setMatrix(QMatrix().scale(1, -1));
        //        item->setPos(pt1);
        //        item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable
        //                       | QGraphicsItem::ItemIsFocusable);
        //        m_pt = NullPT;
        //    } break;
    default:
        break;
    }
}

int Constructor::getCounter()
{
    return counter;
}

void Constructor::setCounter(int value)
{
    counter = value;
}

//void Constructor::addItem(const QPointF& value, bool click)
//{
//    point = value;
//    update(click);
//}

} // namespace ShapePr
//Shapes::Shapes()
//{
//    pt2 = mapToScene(event->pos());

//}
