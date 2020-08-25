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



bool Constructor::snap() { return m_snap; }

void Constructor::setSnap(bool snap) { m_snap = snap; }

void Constructor::addShapePoint(const QPointF& value)
{
    point = value;
    switch (type) {
    case Rect:
        if (!counter) {
            item = new Rectangle(point, point + QPointF { 1, 1 });
        } else {
            finalizeShape();
        }
        break;
    case PolyLine:
        if (!counter) {
            item = new class PolyLine(point, point + QPointF { 1, 1 });
        } else {
            if (static_cast<class PolyLine*>(item)->closed())
                finalizeShape();
            else
                static_cast<class PolyLine*>(item)->addPt(point);
        };
        break;
    case Elipse:
        if (!counter) {
            item = new Circle(point, point + QPointF { 1, 1 });
        } else {
            finalizeShape();
        }
        break;
    case ArcPT:
        if (!counter) {
            item = new Arc(point, point + QPointF { 0, 1 }, point + QPointF { 0, 1 } + QPointF { 1, 0 });
        } else if (counter > 1) {
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
    if (item == nullptr)
        return;
    switch (type) {
    case Rect:
        static_cast<Rectangle*>(item)->setPt(point);
        break;
    case PolyLine:
        static_cast<class PolyLine*>(item)->setPt(point);
        break;
    case Elipse:
        static_cast<Circle*>(item)->setPt(point);
        break;
    case ArcPT:
        if (counter == 1)
            static_cast<Arc*>(item)->setPt(point);
        else
            static_cast<Arc*>(item)->setPt2(point);
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
        type = NullShape;
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

void Constructor::setType(const ShapeType& value, QAction* act)
{
    type = value;
    counter = 0;
    if (action)
        action->setChecked(false);
    action = act;
}

} // namespace ShapePr
