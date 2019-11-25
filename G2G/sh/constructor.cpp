#include "constructor.h"
#include "circle.h"
#include "rectangle.h"

#include <QAction>
#include <scene.h>

namespace ShapePr {

PrType Constructor::type = NullPT;
int Constructor::counter = 0;
QPointF Constructor::point;
QGraphicsItem* Constructor::item = nullptr;
bool Constructor::m_snap = false;
QAction* Constructor::action = nullptr;

bool Constructor::snap() { return m_snap; }

void Constructor::setSnap(bool snap) { m_snap = snap; }

void Constructor::addShapePoint(const QPointF& value)
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
            action->setChecked(false);
            action = nullptr;
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
            action->setChecked(false);
            action = nullptr;
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

void Constructor::updateShape(const QPointF& value)
{
    point = value;
    switch (type) {
    case Rect:
        if (item != nullptr)
            static_cast<Rectangle*>(item)->setPt(point);
        break;
    case Line:
        break;
    case Elipse:
        if (item != nullptr)
            static_cast<Circle*>(item)->setPt(point);
        break;
    case ArcPT:
        break;
    case Text:
        break;
    default:
        break;
    }
}

PrType Constructor::getType() { return type; }

void Constructor::setType(const PrType& value, QAction* act)
{
    type = value;
    counter = 0;
    if (action)
        action->setChecked(false);
    action = act;
}

} // namespace ShapePr
