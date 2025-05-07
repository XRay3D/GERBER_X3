/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shape.h"
#include "graphicsview.h"

#include <QIcon>
#include <QTableView>
#include <array>
#include <ranges>

using Shapes::Handle;

namespace ShRect {

Shape::Shape(QPointF pt1, QPointF pt2) {
    paths_.resize(1);
    paths_.front().resize(5);
    if(!std::isnan(pt1.x())) {
        handles = {
            Handle{{}, Handle::Center},
            Handle{pt1},
            Handle{},
            Handle{pt2},
            Handle{}
        };
        curHandle = handles.begin() + Point1;
        redraw();
    }

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

    auto updCenter = [this] { handles[Center].setPos(QLineF{handles[Point1], handles[Point3]}.center()); };
    auto updCorner = [this](int src, int p1, int p2) {
        handles[p1].setPos({handles[src].x(), curHandle->y()});
        handles[p2].setPos({curHandle->x(), handles[src].y()});
    };

    switch(std::distance(handles.begin(), curHandle)) {
    case Center: {
        QRectF rect(handles[Point1].pos(), handles[Point3].pos());
        rect.moveCenter(handles[Center].pos());
        // auto center{handles[Center].pos()};
        handles[Point1].setPos(rect.topLeft());
        handles[Point2].setPos(rect.topRight());
        handles[Point3].setPos(rect.bottomRight());
        handles[Point4].setPos(rect.bottomLeft());
    } break;
    case Point1: updCorner(Point3, Point2, Point4), updCenter(); break;
    case Point2: updCorner(Point4, Point1, Point3), updCenter(); break;
    case Point3: updCorner(Point1, Point2, Point4), updCenter(); break;
    case Point4: updCorner(Point2, Point1, Point3), updCenter(); break;
    }

    paths_.front() = {
        ~handles[Point1].pos(),
        ~handles[Point2].pos(),
        ~handles[Point3].pos(),
        ~handles[Point4].pos(),
        ~handles[Point1].pos(),
    };

    if(Area(paths_.front()) < 0)
        ReversePath(paths_.front());
    shape_ = QPainterPath();
    shape_.addPolygon(~paths_.front());

    if(model)
        model->dataChanged(model->index(Center, 0), model->index(Height, 0));

    assert(handles.size() == PtCount);
    assert(paths_.size() == 1);
    assert(paths_.front().size() == 5);
}

QString Shape::name() const { return QObject::tr("Rectangle"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-rectangle"); }

void Shape::setPt(const QPointF& pt) {
    handles[Point3].setPos(pt);
    updateOtherhandles(handles.data() + Point3);
}

} // namespace ShRect

#include "moc_shape.cpp"
