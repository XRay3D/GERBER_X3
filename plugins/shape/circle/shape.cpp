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
#include "math.h"

#include <QIcon>

using Shapes::Handle;

namespace ShCirc {

Shape::Shape(Shapes::Plugin* plugin, QPointF center, QPointF pt)
    : AbstractShape{plugin}
    , radius_(QLineF{center, pt}.length()) {
    paths_.resize(1);

    if(!std::isnan(center.x())) {
        handles = {
            Handle{center, Handle::Center},
            Handle{pt}
        };
        curHandle = handles.begin() + Point1;
        redraw();
    }

    App::grView().addItem(this);
}

void Shape::redraw() {

    switch(std::distance(handles.begin(), curHandle)) {
    case Center: {
        auto radLine = QLineF::fromPolar(radius_, 0);
        radLine.translate(handles[Center]);
        handles[Point1] = radLine.p2();
    } break;
    case Point1: {
        QLineF radLine{handles[Center], handles[Point1]};
        radius_ = radLine.length();
    } break;
    }

    const int intSteps = App::settings().clpCircleSegments(radius_);
    const /*Point::Type*/ int32_t radius = static_cast</*Point::Type*/ int32_t>(radius_ * uScale);
    const Point center = ~handles[Center];
    const double delta_angle = (2.0 * pi) / intSteps;
    Path& path = paths_.front();
    path.clear();
    for(int i = 0; i <= intSteps; i++) {
        const double theta = delta_angle * i;
        path.emplace_back(Point{
            radius * cos(theta) + center.x,
            radius * sin(theta) + center.y,
        });
    }

    shape_ = QPainterPath();
    shape_.addPolygon(~path);

    assert(handles.size() == PtCount);
    assert(paths_.size() == 1);
}

QString Shape::name() const { return QObject::tr("Circle"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-ellipse"); }

void Shape::setPt(const QPointF& pt) {
    if(curHandle.base()) *curHandle = pt;
    redraw();
}

double Shape::radius() const { return radius_; }

void Shape::setRadius(double radius) {
    if(qFuzzyIsNull(radius) || qFuzzyCompare(radius_, radius))
        return;
    QLineF line{handles[Center], handles[Point1]};
    line.setLength(radius);
    handles[Point1] = line.p2();
    curHandle = handles.begin() + Point1;
    AbstractShape::redraw();
}

void Shape::readAndInit(QDataStream& /*stream*/) {
    radius_ = QLineF{handles.front(), handles.back()}.length();
    curHandle = handles.begin();
    AbstractShape::redraw();
}

} // namespace ShCirc

#include "moc_shape.cpp"
