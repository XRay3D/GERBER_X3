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
#include "gi_gcpath.h"
#include <QPainter>
#include <ranges>

namespace Gi {

GcPath::GcPath(const Path& path, AbstractFile* file)
    : GcPath{Paths{path}, file} { }

GcPath::GcPath(const Paths& paths, AbstractFile* file)
    : gcFile_{file} {
    for(const Path& path: paths) shape_.addPolygon(~path);
    double k;
    // if(gcFile_)
    //     k = 0; // FIXME gcFile_->gcp_.getToolDiameter() * 0.5;
    // else
    k = pen_.widthF() * 0.5;
    boundingRect_ = shape_.boundingRect() + QMarginsF{k, k, k, k};

    // setAcceptHoverEvents(true);
}

QRectF GcPath::boundingRect() const { return boundingRect_; }

void GcPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    if(pnColorPrt_) pen_.setColor(*pnColorPrt_);
    if(colorPtr_) color_ = *colorPtr_;

    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::NoPen);

    QPen pen{pen_};
    if(qFuzzyIsNull(pen_.widthF())) {
        pen.setWidthF(1.5 * scaleFactor());
        updateArrows(); // for cut direction
        painter->strokePath(arrows_, pen);
    }
    painter->strokePath(shape_, pen);
}

int GcPath::type() const { return Type::Path_; }

Paths GcPath::paths(int) const { return {} /*paths_*/; }

void GcPath::updateArrows() {
    if(sc_ == scaleFactor()) return;
    sc_ = scaleFactor();
    arrows_.clear();

    const double length = std::clamp(30 * scaleFactor(), 0.0, 0.5);
    for(const QPolygonF& path: shape_.toSubpathPolygons()) {
        for(auto&& r: std::ranges::slide_view(path, 2)) {
            QLineF line{r.back(), r.front()};
            if(line.length() < length) continue;
            const double angle = line.angle();
            line.setLength(length);
            line.setAngle(angle + 10);
            arrows_.moveTo(line.p2());
            line.setAngle(angle - 10);
            arrows_.lineTo(line.p1());
            arrows_.lineTo(line.p2());
        }
    }
}

} // namespace Gi
