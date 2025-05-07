/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_datasolid.h"

#include "abstract_file.h"
#include "graphicsview.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

namespace Gi {

DataFill::DataFill(Paths& paths, AbstractFile* file)
    : Item{file}
    , paths_{paths} {
    for(Path path: paths) {
        if(path.size() && path.back() != path.front())
            path.push_back(path.front());
        shape_.addPolygon(~path);
    }
    boundingRect_ = shape_.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

void DataFill::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    // FIXME   if (App::drawPdf()) {
    //        painter->setBrush(Qt::black);
    //        painter->setPen(Qt::NoPen);
    //        painter->drawPath(shape_);
    //        return;
    //    }
    // pen_.setWidth(penWidth());

    painter->setBrush(brushColor_);
    painter->setPen(Qt::NoPen);
    //    for (auto&& poly : shape_.toFillPolygons())
    //        painter->drawPolygon(poly);
    painter->drawPath(shape_);
    bool fl = option->state & (QStyle::State_Selected | QStyle::State_MouseOver);
    if(fl) {
        pen_.setWidthF(1.0 * scaleFactor());
        pen_.setColor(penColor_);
        painter->strokePath(shape_, pen_);
    }
}

int DataFill::type() const { return Type::DataSolid; }

void DataFill::redraw() {
    //    shape_ = QPainterPath();
    //    for (Path path : qAsConst(paths_)) {
    //        path.push_back(path.front());
    //        shape_.addPolygon(path);
    //    }
    setPos({1, 1}); // костыли
    setPos({0, 0});
    // update();
}

Paths& DataFill::getPaths() {
    return paths_;
}

void DataFill::setPaths(Paths paths, int /*alternate*/) {
    auto t{transform()};
    auto a{qRadiansToDegrees(asin(t.m12()))};
    t = t.rotateRadians(-t.m12());
    auto x{t.dx()};
    auto y{t.dy()};

    // reverse transform
    t = {};
    t.rotate(-a);
    t.translate(-x, -y);

    shape_ = {};
    for(auto&& path: paths)
        shape_.addPolygon(t.map(~path));
    paths_ = std::move(paths);

    redraw();
}

void DataFill::changeColor() {
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
