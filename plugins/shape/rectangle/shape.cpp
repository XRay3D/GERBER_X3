/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shape.h"
#include "graphicsview.h"

using Shapes::Handle;

namespace ShRect {

Shape::Shape(Shapes::Plugin* plugin, QPointF pt1, QPointF pt2)
    : AbstractShape{plugin} {
    if(!std::isnan(pt1.x())) {
        handles = {
            Handle{{}, Handle::Center},
            Handle{pt1},
            Handle{},
            Handle{pt2},
            Handle{}
        };
        curHandle = handles.data() + Point3;
        redraw();
    }
    App::grView().addItem(this);
}

void Shape::rebuild() {
    if(handles.size() < PtCount) return;
    auto updCenter = [this] {
        handles[Center] = QLineF{handles[Point1], handles[Point3]}.center();
    };

    auto updCorner = [this](int src, int p1, int p2) {
        handles[p1] = {handles[src].x(), curHandle->y()};
        handles[p2] = {curHandle->x(), handles[src].y()};
    };

    switch(curHandle ? std::distance(handles.data(), curHandle) : PtCount) {
    case Center: {
        QRectF rect{handles[Point1], handles[Point3]};
        rect.moveCenter(handles[Center]);
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

    shape_.clear();
    // shape_.addPolygon({
    //     handles[Point1],
    //     handles[Point2],
    //     handles[Point3],
    //     handles[Point4],
    //     handles[Point1],
    // });shape_.

    curves_ = {
        {
         {handles[Point1]},
         {handles[Point2]},
         {handles[Point3]},
         {handles[Point4]},
         }
    };
    curves_.front().close();
    closed = true;
    if(curves_.front().area() < 0) curves_.front().reverse();
    shape_ = Geo::toPath(curves_);
    assert(handles.size() == PtCount);
}

QString Shape::name() const { return QObject::tr("Rectangle"); }

QIcon Shape::icon() const { return QIcon::fromTheme(u"draw-rectangle"_s); }

void Shape::setPt(const QPointF& pt) {
    *curHandle = pt;
    redraw();
}

void Shape::setSize(double w, double h) {
    const QRectF r = QRectF{handles[Point1], handles[Point3]}.normalized();
    double x1 = r.left(), x2 = r.right(), y1 = r.top(), y2 = r.bottom();
    if(!std::isnan(w) && w > 0) {
        if(anchor & Qt::AlignLeft) x2 = x1 + w;
        else if(anchor & Qt::AlignRight) x1 = x2 - w;
        else x1 = r.center().x() - w / 2, x2 = r.center().x() + w / 2;
    }
    if(!std::isnan(h) && h > 0) {
        // сцена растёт по Y вверх: низ на экране — меньший Y
        if(anchor & Qt::AlignBottom) y2 = y1 + h;
        else if(anchor & Qt::AlignTop) y1 = y2 - h;
        else y1 = r.center().y() - h / 2, y2 = r.center().y() + h / 2;
    }
    handles[Point1] = {x1, y1};
    handles[Point2] = {x2, y1};
    handles[Point3] = {x2, y2};
    handles[Point4] = {x1, y2};
    handles[Center] = {(x1 + x2) / 2, (y1 + y2) / 2};
    curHandle = {};
    redraw();
}

} // namespace ShRect

#include "moc_shape.cpp"
