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
#include "gi_gcpath.h"
#include "gi_dbg.h"
#include <QPainter>
#include <ranges>

namespace Gi {

GcPath::GcPath(Path path, AbstractFile* file)
    : GcPath{Paths{std::move(path)}, file} { }

GcPath::GcPath(Paths paths, AbstractFile* file)
    : GcPath{toCurves(paths), file} { }

GcPath::GcPath(Curves curves, AbstractFile* file)
    : gcFile_{file} {

    // Gi::Debug(paths, Qt::magenta)->arrows = {};
    // Gi::Debug(toPaths(toCurves(paths)), Qt::green)->arrows = {};

    shape_ = toPPath(curves);

    // for(const Path& path: paths) shape_.addPolygon(~path);
    double k;
    // if(gcFile_)
    // k = 0; // FIXME gcFile_->gcp.getToolDiameter() * 0.5;
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
        if(auto ar = updateArrows()) arrows_ = std::move(*ar); // for direction
        painter->strokePath(arrows_, pen);
    }
    painter->strokePath(shape_, pen);
}

int GcPath::type() const { return Type::Path_; }

Paths GcPath::paths(int) const { return {} /*paths_*/; }

} // namespace Gi
