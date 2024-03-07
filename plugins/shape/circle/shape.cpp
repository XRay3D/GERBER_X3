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
#include "math.h"
#include "shhandler.h"
#include <QIcon>

using Shapes::Handle;

namespace ShCirc {

Shape::Shape(QPointF center, QPointF pt)
    : radius_(QLineF(center, pt).length()) {
    paths_.resize(1);

    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.emplace_back(std::make_unique<Handle>(this));

    handlers[Center]->setPos(center);
    handlers[Point1]->setPos(pt);

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
    radius_ = (QLineF(handlers[Center]->pos(), handlers[Point1]->pos()).length());
    const int intSteps = App::settings().clpCircleSegments(radius_);
    const /*Point::Type*/ int32_t radius = static_cast</*Point::Type*/ int32_t>(radius_ * uScale);
    const Point center{~handlers[Center]->pos()};
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
    setPos({1, 1}); // костыли    //update();
    setPos({0, 0});

    //    if(model)
    //        model->dataChanged(model->index(Center, 0), model->index(Diameter, 0));
}

QString Shape::name() const { return QObject::tr("Shape"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-ellipse"); }

void Shape::setPt(const QPointF& pt) {
    handlers[Point1]->setPos(pt);
    redraw();
}

double Shape::radius() const { return radius_; }

void Shape::setRadius(double radius) {
    if(qFuzzyIsNull(radius) || qFuzzyCompare(radius_, radius))
        return;
    QLineF line(handlers[Center]->pos(), handlers[Point1]->pos());
    line.setLength(radius);
    handlers[Point1]->setPos(line.p2());
    currentHandler = handlers[Point1].get();
    redraw();
}

} // namespace ShCirc

#include "moc_shape.cpp"
