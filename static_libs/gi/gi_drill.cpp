/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_drill.h"

#include "geo/boolean.h"
#include "geo/util.h"

#include "app.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Gi {

Drill::Drill(const QPolygonF& path, double diameter, AbstractFile* file, Tool::ID toolId)
    : Item{file}
    , diameter_{diameter}
    , path_{path}
    , toolId_{toolId} {
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setToolId(toolId_);
    create();
    changeColor();
}

void Drill::paintGeometry(QPainter* painter, const RenderState& st) {

    // FIXME   if (App::drawPdf()) {
    // painter->setBrush(Qt::black);
    // painter->setPen(Qt::NoPen);
    // painter->drawPath(shape_);
    // return;
    // }

    painter->setBrush(brushColor_);
    painter->setPen(Qt::NoPen);
    painter->drawPath(shape_);

    // Раньше обводка рисовалась БЕЗУСЛОВНО, а changeColor() в состоянии Default
    // кладёт penColor_ = brushColor_ -- то есть каждый кадр каждое отверстие
    // обводилось цветом собственной заливки. Белым перо становится только на
    // наведении и выделении, там обводка и нужна.
    if(!st.plain()) [[unlikely]]
        painter->strokePath(shape_, QPen{penColor_, 0.0});
}

bool Drill::isSlot() { return path_.size() > 1; }

void Drill::setDiameter(double diameter) {
    if(diameter_ == diameter)
        return;
    diameter_ = diameter;

    create();
    update();
}

void Drill::updatePath(const QPolygonF& path, double diameter) {
    diameter_ = diameter;
    path_     = path;
    create();
    update();
}

void Drill::setToolId(Tool::ID newToolId) {
    toolId_ = newToolId;
    setToolTip(QObject::tr("Tool %1, Ø%2mm")
            .arg(App::toolHolder().tool(toolId_).name())
            .arg(diameter_));
}

// Paths Drill::paths(int /*alternate*/) const {
//     auto path{shape_.toFillPolygon(QTransform::fromScale(100, 100))};
//     path = QTransform::fromScale(0.01, 0.01).map(path);
//     return {~transform().map(path)};
// }

void Drill::changeColor() {
    // animation.setStartValue(bodyColor_);

    switch(colorState) {
    case Default           : brushColor_ = QColor{100, 100, 100}; break;
    case Hovered           : brushColor_ = QColor{150, 150, 150}; break;
    case Selected          : brushColor_ = QColor{255, 0x0, 255}; break;
    case Hovered | Selected: brushColor_ = QColor{255, 150, 255}; break;
    }

    penColor_ = brushColor_;
    switch(colorState) {
    case Default           : break;
    case Hovered           : break;
    case Selected          : penColor_ = Qt::white; break; // FIXME V1037. Two or more case-branches perform the same actions.
    case Hovered | Selected: penColor_ = Qt::white; break;
    }

    // animation.setEndValue(bodyColor_);
    // animation.start();
}

void Drill::create() {
    if(path_.empty()) return;

    shape_.clear();
    curves_.clear();

    if(path_.size() == 1) {
        shape_.addEllipse(path_.front(), diameter_ * 0.5, diameter_ * 0.5);
        curves_.emplace_back(Geo::circle(diameter_, path_.front()));
    } else {
        // Паз: полоса вдоль ломаной прохода шириной ровно в диаметр сверла --
        // delta у Geo::Inflate это ПОЛНАЯ ширина, а не радиус офсета.
        curves_ = Geo::Inflate(Geo::Polylines{Geo::Polyline{path_}}, diameter_).contours();
        shape_ = Geo::toPath(curves_);
    }

    geometryChanged();
}

} // namespace Gi
