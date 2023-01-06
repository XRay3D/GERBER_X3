// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_datasolid.h"

#include "file.h"
#include "graphicsview.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

GiDataSolid::GiDataSolid(Paths& paths, FileInterface* file)
    : GraphicsItem(file)
    , paths_ {paths} {
    for (Path path : paths) {
        if (path.size() && path.back() != path.front())
            path.push_back(path.front());
        shape_.addPolygon(path);
    }
    boundingRect_ = shape_.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

GiDataSolid::~GiDataSolid() { }

void GiDataSolid::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    // FIXME   if (App::graphicsView()->scene()->drawPdf()) {
    //        painter->setBrush(Qt::black);
    //        painter->setPen(Qt::NoPen);
    //        painter->drawPath(shape_);
    //        return;
    //    }

    painter->setBrush(bodyColor_);
    painter->setPen(Qt::NoPen);
    //    for (auto&& poly : shape_.toFillPolygons())
    //        painter->drawPolygon(poly);
    painter->drawPath(shape_);

    //    pen_.setWidthF(option->state & QStyle::State_Selected
    //                || option->state & QStyle::State_MouseOver
    //            ? 2.0 * scaleFactor()
    //            : 0);
    pen_.setColor(pathColor_);
    painter->strokePath(shape_, pen_);
}

int GiDataSolid::type() const { return GiType::DataSolid; }

void GiDataSolid::redraw() {
    //    shape_ = QPainterPath();
    //    for (Path path : qAsConst(paths_)) {
    //        path.push_back(path.front());
    //        shape_.addPolygon(path);
    //    }
    setPos({1, 1}); // костыли
    setPos({0, 0});
    // update();
}

void GiDataSolid::setPaths(Paths paths, int alternate) {
    auto t {transform()};
    auto a {qRadiansToDegrees(asin(t.m12()))};
    t = t.rotateRadians(-t.m12());
    auto x {t.dx()};
    auto y {t.dy()};
    shape_ = {};

    // reverse transform
    t = {};
    t.rotate(-a);
    t.translate(-x, -y);
    for (auto&& path : paths) {
        path = t.map(path);
        shape_.addPolygon(path);
    }
    paths_ = std::move(paths);
    redraw();
}

void GiDataSolid::changeColor() {
    //    auto animation = new QPropertyAnimation(this, "bodyColor");
    //    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    animation.setDuration(100);

    //    animation.setStartValue(bodyColor_);

    bodyColor_ = colorPtr_ ? *colorPtr_ : color_;

    switch (colorState) {
    case Default:
        break;
    case Hovered: // FIXME V1037. Two or more case-branches perform the same actions.
        bodyColor_.setAlpha(255);
        break;
    case Selected:
        bodyColor_.setAlpha(255);
        break;
    case Hovered | Selected:
        bodyColor_.setAlpha(255);
        bodyColor_ = bodyColor_.lighter(150);
        break;
    }

    pathColor_ = colorPtr_ ? *colorPtr_ : color_;
    pathColor_.setAlpha(100);
    switch (colorState) {
    case Default:
        //        pathColor_.setAlpha(100);
        break;
    case Hovered:
        pathColor_.setAlpha(255);
        //        pathColor_ = pathColor_.darker(125);
        break;
    case Selected:
        pathColor_.setAlpha(150);
        break;
    case Hovered | Selected:
        pathColor_.setAlpha(255);
        pathColor_ = pathColor_.lighter(150);
        break;
    }

    //    animation.setEndValue(bodyColor_);
    //    animation.start();
}
