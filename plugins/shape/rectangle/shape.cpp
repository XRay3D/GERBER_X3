// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shape.h"
#include "graphicsview.h"
#include "shhandler.h"
#include <QIcon>
#include <QTableView>
#include <array>
#include <ranges>

using Shapes::Handle;

namespace ShRect {

Shape::Shape(QPointF pt1, QPointF pt2) {
    paths_.resize(1);
    paths_.front().resize(5);
    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));

    handlers[Point1]->setPos(pt1);
    handlers[Point3]->setPos(pt2);
    currentHandler = handlers[Point1].get();
    redraw();

    App::grView().addItem(this);
}

Shape::~Shape() {
    std::erase(model->shapes, this);
    qobject_cast<QTableView*>(model->parent())->reset();
}

QVariant Shape::itemChange(GraphicsItemChange change, const QVariant& value) {
    if(change == GraphicsItemChange::ItemSelectedChange)
        qobject_cast<QTableView*>(model->parent())->reset();
    return Shapes::AbstractShape::itemChange(change, value);
}

void Shape::redraw() {

    auto updCenter = [this] { handlers[Center]->setPos(QLineF(handlers[Point1]->pos(), handlers[Point3]->pos()).center()); };
    auto updCorner = [this](int src, int p1, int p2) {
        handlers[p1]->setPos(handlers[src]->x(), currentHandler->y());
        handlers[p2]->setPos(currentHandler->x(), handlers[src]->y());
    };

    switch(handlers.indexOf(currentHandler)) {
    case Center: {
        QRectF rect(handlers[Point1]->pos(), handlers[Point3]->pos());
        rect.moveCenter(handlers[Center]->pos());
        auto center{handlers[Center]->pos()};
        handlers[Point1]->setPos(rect.topLeft());
        handlers[Point2]->setPos(rect.topRight());
        handlers[Point3]->setPos(rect.bottomRight());
        handlers[Point4]->setPos(rect.bottomLeft());
    } break;
    case Point1:
        updCorner(Point3, Point2, Point4), updCenter();
        break;
    case Point2:
        updCorner(Point4, Point1, Point3), updCenter();
        break;
    case Point3:
        updCorner(Point1, Point2, Point4), updCenter();
        break;
    case Point4:
        updCorner(Point2, Point1, Point3), updCenter();
        break;
    }

    paths_.front() = Path{
        handlers[Point1]->pos(),
        handlers[Point2]->pos(),
        handlers[Point3]->pos(),
        handlers[Point4]->pos(),
        handlers[Point1]->pos(),
    };

    if(Area(paths_.front()) < 0)
        ReversePath(paths_.front());
    shape_ = QPainterPath();
    shape_.addPolygon(paths_.front());
    setPos({1, 1}); // костыли    //update();
    setPos({0, 0});

    if(model)
        model->dataChanged(model->index(Center, 0), model->index(Height, 0));
}

QString Shape::name() const { return QObject::tr("Rectangle"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-rectangle"); }

void Shape::setPt(const QPointF& pt) {
    handlers[Point3]->setPos(pt);
    updateOtherHandlers(handlers[Point3].get());
}

} // namespace ShRect

#include "moc_shape.cpp"
