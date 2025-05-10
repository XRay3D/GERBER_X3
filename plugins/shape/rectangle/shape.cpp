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

Shape::Shape(Shapes::Plugin* plugin, QPointF pt1, QPointF pt2)
    : AbstractShape{plugin} {
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
        curHandle = handles.begin() + Point3;
        redraw();
    }

    App::grView().addItem(this);
}

void Shape::redraw() {

    auto updCenter = [this] { handles[Center] = QLineF{handles[Point1], handles[Point3]}.center(); };
    auto updCorner = [this](int src, int p1, int p2) {
        handles[p1] = {handles[src].x(), curHandle->y()};
        handles[p2] = {curHandle->x(), handles[src].y()};
    };

    switch(std::distance(handles.begin(), curHandle)) {
    case Center: {
        QRectF rect{handles[Point1], handles[Point3]};
        rect.moveCenter(handles[Center]);
        // auto center{handles[Center] };
        handles[Point1] = rect.topLeft();
        handles[Point2] = rect.topRight();
        handles[Point3] = rect.bottomRight();
        handles[Point4] = rect.bottomLeft();
    } break;
    case Point1: updCorner(Point3, Point2, Point4), updCenter(); break;
    case Point2: updCorner(Point4, Point1, Point3), updCenter(); break;
    case Point3: updCorner(Point1, Point2, Point4), updCenter(); break;
    case Point4: updCorner(Point2, Point1, Point3), updCenter(); break;
    }

    paths_.front() = {
        ~handles[Point1],
        ~handles[Point2],
        ~handles[Point3],
        ~handles[Point4],
        ~handles[Point1],
    };

    if(Area(paths_.front()) < 0)
        ReversePath(paths_.front());
    shape_ = QPainterPath();
    shape_.addPolygon(~paths_.front());

    assert(handles.size() == PtCount);
    assert(paths_.size() == 1);
    assert(paths_.front().size() == 5);
}

QString Shape::name() const { return QObject::tr("Rectangle"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-rectangle"); }

void Shape::setPt(const QPointF& pt) {
    *curHandle = pt;
    redraw();
}

} // namespace ShRect

#include "moc_shape.cpp"
