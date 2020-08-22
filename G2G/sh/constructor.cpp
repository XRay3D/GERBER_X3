// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "constructor.h"
#include "circle.h"
#include "pline.h"
#include "rectangle.h"

#include <QAction>
#include <project.h>
#include <scene.h>

namespace ShapePr {

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
    qDebug() << type << counter << item << point;
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
    case Pline:
        switch (counter) {
        case 0:
        default:
            if (item == nullptr) {
                item = new class Pline(point, point + QPointF { 1, 1 });
            } else {
                if (static_cast<class Pline*>(item)->closed())
                    finalizeShape();
                else
                    static_cast<class Pline*>(item)->addPt(point);
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
    case Pline:
        if (item != nullptr)
            static_cast<class Pline*>(item)->setPt(point);
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

void Constructor::finalizeShape(/*const QPointF& value*/)
{
    //    point = value;
    qDebug() << type << counter << item << point;
    if (item == nullptr)
        return;

    App::project()->addShape(item);

    switch (type) {
    case Rect:
        type = NullPT;
        item->setSelected(true);
        item = nullptr;
        action->setChecked(false);
        action = nullptr;
        break;
    case Pline:
        type = NullPT;
        item->setSelected(true);
        item = nullptr;
        action->setChecked(false);
        action = nullptr;
        break;
    case Elipse:
        type = NullPT;
        item->setSelected(true);
        item = nullptr;
        action->setChecked(false);
        action = nullptr;
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
