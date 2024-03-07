// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_drill.h"

#include "myclipper.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Gi {

Drill::Drill(const Path& path, double diameter, AbstractFile* file, int toolId)
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

void Drill::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {

    // FIXME   if (App::drawPdf()) {
    //        painter->setBrush(Qt::black);
    //        painter->setPen(Qt::NoPen);
    //        painter->drawPath(shape_);
    //        return;
    //    }

    painter->setBrush(bodyColor_);
    painter->setPen(Qt::NoPen);
    painter->drawPath(shape_);

    pen_.setColor(pathColor_);
    painter->strokePath(shape_, pen_);
}

bool Drill::isSlot() { return path_.size() > 1; }

void Drill::setDiameter(double diameter) {
    if(diameter_ == diameter)
        return;
    diameter_ = diameter;

    create();
    update();
}

void Drill::update(const Path& path, double diameter) {
    diameter_ = diameter;
    path_ = path;
    create();
    update();
}

Paths Drill::paths(int alternate) const {
    auto path{shape_.toFillPolygon(QTransform::fromScale(100, 100))};
    path = QTransform::fromScale(0.01, 0.01).map(path);
    return {~transform().map(path)};
}

void Drill::changeColor() {
    //    animation.setStartValue(bodyColor_);

    switch(colorState) {
    case Default:
        bodyColor_ = QColor(100, 100, 100);
        break;
    case Hovered:
        bodyColor_ = QColor(150, 0x0, 150);
        break;
    case Selected:
        bodyColor_ = QColor(255, 0x0, 255);
        break;
    case Hovered | Selected:
        bodyColor_ = QColor(127, 0x0, 255);
        break;
    }

    pathColor_ = bodyColor_;
    switch(colorState) {
    case Default:
        break;
    case Hovered:
        break;
    case Selected: // FIXME V1037. Two or more case-branches perform the same actions.
        pathColor_ = Qt::white;
        break;
    case Hovered | Selected:
        pathColor_ = Qt::white;
        break;
    }

    //    animation.setEndValue(bodyColor_);
    //    animation.start();
}

void Drill::create() {
    shape_ = QPainterPath();

    if(!path_.size()) {
        return;
    } else if(path_.size() == 1) {
        //        path_ = CirclePath(double(diameter_ ? diameter_ * uScale : uScale), path_.front());
        //        ReversePath(path_);
        //        path_.push_back(path_.front());
        // shape_.addPolygon(path_);
        shape_.addEllipse(~path_.front(), diameter_ * 0.5, diameter_ * 0.5);
        //        path_ = shape_.toFillPolygon();
    } else {
        boundingRect_ = shape_.boundingRect();
        for(auto&& path: InflatePaths(Paths{path_}, diameter_ * uScale, JoinType::Round, EndType::Round, uScale)) {
            path.push_back(path.front());
            shape_.addPolygon(~path);
        }
    }

    boundingRect_ = shape_.boundingRect();
    fillPolygon = shape_.toFillPolygon();
}

} // namespace Gi
