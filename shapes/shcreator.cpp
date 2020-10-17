// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "shcreator.h"
#include "shheaders.h"
#include <QAction>

#include "leakdetector.h"

namespace Shapes {

void Constructor::addShapePoint(const QPointF& value)
{
    point = value;
    switch (static_cast<GiType>(type)) {
    case GiType::ShapeR:
        if (!counter) {
            item = new Rectangle(point, point + QPointF { 1, 1 });
        } else {
            finalizeShape();
        }
        break;
    case GiType::ShapeL:
        if (!counter) {
            item = new class PolyLine(point, point + QPointF { 1, 1 });
        } else {
            if (static_cast<class PolyLine*>(item)->closed())
                finalizeShape();
            else
                static_cast<class PolyLine*>(item)->addPt(point);
        };
        break;
    case GiType::ShapeC:
        if (!counter) {
            item = new Circle(point, point + QPointF { 1, 1 });
        } else {
            finalizeShape();
        }
        break;
    case GiType::ShapeA:
        if (!counter) {
            item = new Arc(point, point + QPointF { 0, 1 }, point + QPointF { 0, 1 } + QPointF { 1, 0 });
        } else if (counter > 1) {
            finalizeShape();
        }
        break;
    case GiType::ShapeT:
        item = new Text(point);
        finalizeShape();
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
    switch (static_cast<GiType>(type)) {
    case GiType::ShapeR:
        static_cast<Rectangle*>(item)->setPt(point);
        break;
    case GiType::ShapeL:
        static_cast<class PolyLine*>(item)->setPt(point);
        break;
    case GiType::ShapeC:
        static_cast<Circle*>(item)->setPt(point);
        break;
    case GiType::ShapeA:
        if (counter == 1)
            static_cast<Arc*>(item)->setPt(point);
        else
            static_cast<Arc*>(item)->setPt2(point);
        break;
    case GiType::ShapeT:
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

    switch (static_cast<GiType>(type)) {
    case GiType::ShapeL:
        if (item->handlers.size() > 4 && !static_cast<class PolyLine*>(item)->closed()) {
            delete item->handlers.last();
            item->handlers.removeLast();
            delete item->handlers.last();
            item->handlers.removeLast();
            item->redraw();
        }
    case GiType::ShapeR:
    case GiType::ShapeC:
    case GiType::ShapeA:
    case GiType::ShapeT:
        type = 0;
        item->setSelected(true);
        item = nullptr;
        action->setChecked(false);
        action = nullptr;
        break;
    default:
        break;
    }
}

void Constructor::setType(const int value, QAction* act)
{
    type = value;
    counter = 0;
    if (action)
        action->setChecked(false);
    action = act;
}

} // namespace ShapePr
