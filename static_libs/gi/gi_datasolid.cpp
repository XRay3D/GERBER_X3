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
#include "gi_datasolid.h"
#include "abstract_file.h"
#include "gi_dbg.h"
#include "graphicsview.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

namespace Gi {

DataFill::DataFill(Geo::Polygons curves, AbstractFile* file)
    : Item{file} {
    // Polygons::toPath идёт прямо по точным кривым и выставляет WindingFill,
    // при котором дырки (они канонически обходятся навстречу телу) вычитаются
    // сами, а законный остров внутри дырки остаётся.
    shape_ = curves.toPath();
    // Контуры -- для базового curves(), сам регион -- для region(): собрать
    // его обратно из плоского списка нельзя, острова в дырках теряются.
    curves_ = curves.contours();
    region_ = std::move(curves);
    boundingRect_ = shape_.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

void DataFill::paintGeometry(QPainter* painter, const RenderState& st) {
    // FIXME   if (App::drawPdf()) {
    //        painter->setBrush(Qt::black);
    //        painter->setPen(Qt::NoPen);
    //        painter->drawPath(shape_);
    //        return;
    //    }
    painter->setBrush(brushColor_);
    painter->setPen(Qt::NoPen);
    painter->drawPath(shape_);
    // Обводка -- ПОВЕРХ заливки, поэтому здесь, а не в paintHighlight.
    // Перо локальное: член pen_ принадлежит базе, мутировать его в отрисовке
    // нельзя.
    if(!st.plain()) [[unlikely]]
        painter->strokePath(shape_, QPen{penColor_, 1.0 * st.sf});
}

int DataFill::type() const { return Type::DataSolid; }

void DataFill::redraw() {
    //    shape_ = QPainterPath();
    //    for (Path path :  std::as_const(paths_)) {
    //        path.push_back(path.front());
    //        shape_.addPolygon(path);
    //    }
    setPos({1, 1}); // костыли
    setPos({0, 0});
    // update();
}

void DataFill::setCurves(Geo::Polylines paths, int alternate) // FIXME from setPaths
{
}

Geo::Polygons DataFill::region() const {
    // Хранится в своей системе координат, наружу -- в сценовой, как и curves().
    return Geo::transformed(region_, transform());
}

// Paths& DataFill::getPaths() {
//     return curves_;
// }

// void DataFill::setPaths(Paths paths, int /*alternate*/) {
//     qCritical("setPaths");
//     auto t{transform()};
//     auto a{qRadiansToDegrees(asin(t.m12()))};
//     t = t.rotateRadians(-t.m12());
//     auto x{t.dx()};
//     auto y{t.dy()};

//     // reverse transform
//     t = {};
//     t.rotate(-a);
//     t.translate(-x, -y);

//     shape_ = {};
//     for(auto&& path: paths)
//         shape_.addPolygon(t.map(~path));
//     // paths_ = std::move(paths);

//     redraw();
// }

void DataFill::updateColors() {
    //    auto animation = new QPropertyAnimation{this, "bodyColor"};
    //    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    animation.setDuration(100);
    //    animation.setStartValue(bodyColor_);

    brushColor_ = colorPtr_ ? *colorPtr_ : color_;

    switch(colorState) {
    case Default:
        break;
    case Hovered:
    case Selected:
        brushColor_.setAlpha(255);
        break;
    case Hovered | Selected:
        brushColor_.setAlpha(255);
        brushColor_ = brushColor_.lighter(150);
        break;
    }

    penColor_ = colorPtr_ ? *colorPtr_ : color_;
    penColor_.setAlpha(100);
    switch(colorState) {
    case Default:
        //        pathColor_.setAlpha(100);
        break;
    case Hovered:
        penColor_.setAlpha(255);
        //        pathColor_ = pathColor_.darker(125);
        break;
    case Selected:
        penColor_.setAlpha(150);
        break;
    case Hovered | Selected:
        penColor_.setAlpha(255);
        penColor_ = penColor_.lighter(150);
        break;
    }

    //    animation.setEndValue(bodyColor_);
    //    animation.start();
}

} // namespace Gi
