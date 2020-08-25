// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "constructor.h"
#include "arc.h"
#include "circle.h"
#include "pline.h"
#include "rectangle.h"
#include <QAction>
#include <project.h>
#include <scene.h>

namespace Shapes {

PrType Constructor::type = NullPT;
int Constructor::counter = 0;
QPointF Constructor::point;
Shape* Constructor::item = nullptr;
bool Constructor::m_snap = false;
QAction* Constructor::action = nullptr;

bool Constructor::snap() { return m_snap; }

void Constructor::setSnap(bool snap) { m_snap = snap; }

void Constructor::addShapePoint(const QPointF& value)
{
    point = value;
    switch (type) {
    case Rect:
        switch (counter) {
        case 0:
            item = new Rectangle(point, point + QPointF { 1, 1 });
            break;
        default:
            finalizeShape();
        }
        break;
    case PolyLine:
        switch (counter) {
        case 0:
        default:
            if (item == nullptr) {
                item = new class PolyLine(point, point + QPointF { 1, 1 });
            } else {
                if (static_cast<class PolyLine*>(item)->closed())
                    finalizeShape();
                else
                    static_cast<class PolyLine*>(item)->addPt(point);
            }
            //            break;
            //            type = NullPT;
            //            item->setSelected(true);
            //            item = nullptr;
            //            action->setChecked(false);
            //            action = nullptr;
        }
        break;
    case Elipse:
        switch (counter) {
        case 0:
            item = new Circle(point, point + QPointF { 1, 1 });
            break;
        default:
            finalizeShape();
        }
        break;
    case ArcPT:
        switch (counter) {
        case 0:
            item = new Arc(point, point + QPointF { 0, 1 }, point + QPointF { 0, 1 } + QPointF { 1, 0 });
            break;
        case 1:
            break;
        default:
            finalizeShape();
        }
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
    case PolyLine:
        if (item != nullptr)
            static_cast<class PolyLine*>(item)->setPt(point);
        break;
    case Elipse:
        if (item != nullptr)
            static_cast<Circle*>(item)->setPt(point);
        break;
    case ArcPT:
        if (item != nullptr) {
            if (counter == 1)
                static_cast<Arc*>(item)->setPt(point);
            else
                static_cast<Arc*>(item)->setPt2(point);
        }
        break;
    case Text:
        break;
    default:
        break;
    }
}

void Constructor::finalizeShape(/*const QPointF& value*/)
{
    if (item == nullptr)
        return;

    App::project()->addShape(item);

    switch (type) {
    case Rect:
    case PolyLine:
    case Elipse:
    case ArcPT:
        type = NullPT;
        item->setSelected(true);
        item = nullptr;
        action->setChecked(false);
        action = nullptr;
        break;
    case Text:
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
