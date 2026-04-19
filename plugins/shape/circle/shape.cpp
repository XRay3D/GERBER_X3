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
#include "math.h"

using Shapes::Handle;

namespace ShCirc {

Shape::Shape(Shapes::Plugin* plugin, QPointF center, QPointF pt)
    : AbstractShape{plugin}
    , radius_(geo::Length(center, pt)) {
    // paths_.resize(1);

    if(!std::isnan(center.x())) {
        handles = {
            Handle{center, Handle::Center},
            Handle{pt}
        };
        curHandle = handles.data() + Point1;
        redraw();
    }

    App::grView().addItem(this);
}

void Shape::redraw() {
    switch(std::distance(handles.data(), curHandle)) {
    case Center: {
        auto radLine = QLineF::fromPolar(radius_, 0);
        radLine.translate(handles[Center]);
        handles[Point1] = radLine.p2();
    } break;
    case Point1:
    default    : radius_ = geo::Length(handles[Center], handles[Point1]);
    }
    closed = true;
    curves_ = {CircleCurve(radius_ * 2., handles[Center])};
    shape_ = toPPath(curves_);
    assert(handles.size() == PtCount);
}

QString Shape::name() const { return QObject::tr("Circle"); }

QIcon Shape::icon() const { return QIcon::fromTheme(u"draw-ellipse"_s); }

void Shape::setPt(const QPointF& pt) {
    if(curHandle) *curHandle = pt;
    redraw();
}

double Shape::radius() const { return radius_; }

void Shape::setRadius(double radius) {
    if(qFuzzyIsNull(radius) || qFuzzyCompare(radius_, radius))
        return;
    QLineF line{handles[Center], handles[Point1]};
    line.setLength(radius);
    handles[Point1] = line.p2();
    curHandle = handles.data() + Point1;
    AbstractShape::redraw();
}

void Shape::readAndInit(QDataStream& /*stream*/) {
    radius_ = geo::Length(handles.front(), handles.back());
    curHandle = handles.data();
    AbstractShape::redraw();
}

} // namespace ShCirc

#include "moc_shape.cpp"
