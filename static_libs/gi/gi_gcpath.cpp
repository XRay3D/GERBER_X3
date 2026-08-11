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

// GcPath::GcPath(Path path, AbstractFile* file)
//     : GcPath{Paths{std::move(path)}, file} { }

// GcPath::GcPath(Paths paths, AbstractFile* file)
//     : GcPath{toCurves(paths), file} { }

GcPath::GcPath(Geo::Polylines curves, AbstractFile* file)
    : gcFile_{file} {

    // Gi::Debug(paths, Qt::magenta)->arrows = {};
    // Gi::Debug(toPaths(toCurves(paths)), Qt::green)->arrows = {};

    shape_ = Geo::toPath(curves);

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

void GcPath::paintGeometry(QPainter* painter, const RenderState& st) {
    // Цвета из указателей разрешает базовый Item::paint.
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::NoPen);

    if(!qFuzzyIsNull(pen_.widthF())) { // перо задано явно -- ни стрелок, ни копии
        painter->strokePath(shape_, pen_);
        return;
    }

    QPen pen{pen_};
    pen.setWidthF(1.5 * st.sf);
    if(auto ar = updateArrows()) arrows_ = std::move(*ar); // for direction
    painter->strokePath(arrows_, pen);
    painter->strokePath(shape_, pen);
}

int GcPath::type() const { return Type::Path_; }

Geo::Polylines GcPath::curves(int /*alternate*/) const { return {}; }

// Paths GcPath::paths(int) const { return {} /*paths_*/; }

} // namespace Gi
