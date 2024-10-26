// This is a personal academic project. Dear PVS-Studio, please check it.
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
#include "gi_gcpath.h"

#include "gcode.h"

#include "graphicsview.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#define QT_DEBUG
// #undef QT_DEBUG

namespace Gi {

GcPath::GcPath(const Paths& paths, AbstractFile* file)
    : gcFile_{file} {
    for(const Path& path: paths)
        shape_.addPolygon(~path);
    double k;
    if(gcFile_)
        k = 0; // FIXME gcFile_->gcp_.getToolDiameter() * 0.5;
    else
        k = pen_.widthF() * 0.5;
    boundingRect_ = shape_.boundingRect() + QMarginsF(k, k, k, k);
#ifdef QT_DEBUG
    // setAcceptHoverEvents(true);
#endif
}

GcPath::GcPath(const Path& path, AbstractFile* file)
    : gcFile_{file} {
    shape_.addPolygon(~path);
    double k;
    if(gcFile_)
        k = 0; // FIXME  gcFile_->gcp_.getToolDiameter() * 0.5;
    else
        k = pen_.widthF() * 0.5;
    boundingRect_ = shape_.boundingRect() + QMarginsF(k, k, k, k);
#ifdef QT_DEBUG
    // setAcceptHoverEvents(true);
#endif
}

QRectF GcPath::boundingRect() const { return boundingRect_; }

void GcPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    Q_UNUSED(option)

    if(pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if(colorPtr_)
        color_ = *colorPtr_;

    if(pen_.widthF() == 0) {
        QPen pen(pen_);
        pen.setWidthF(1.5 * scaleFactor());
        painter->setPen(pen);
    } else
        painter->setPen(pen_);
#ifdef QT_DEBUG
    if(option->state & QStyle::State_MouseOver) {
        QPen pen(pen_);
        pen.setWidthF(2.0 * scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({2.0, 2.0});
        painter->setPen(pen);
    }
#endif
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(shape_);

    //    painter->setPen(QPen(Qt::magenta, 0.0));
    //    painter->drawRect(rect_);

    ////////////////////////////////////////////////////// for debug cut direction
#ifdef QT_DEBUG
    if(sc_ != scaleFactor())
        updateArrows();
    painter->drawPath(arrows_);
#endif
}

int GcPath::type() const { return Type::Path_; }

Paths GcPath::paths(int) const { return {} /*paths_*/; }
#ifdef QT_DEBUG
void GcPath::updateArrows() {
    sc_ = scaleFactor();
    arrows_ = QPainterPath(); //.clear();
    if(qFuzzyIsNull(pen_.widthF())) {
        for(const QPolygonF& path: shape_.toSubpathPolygons()) {
            for(int i = 0; i < path.size() - 1; ++i) {
                QLineF line(path[i + 1], path[i]);
                double length = 30 * scaleFactor();
                if(line.length() < length && i)
                    continue;
                if(length > 0.5)
                    length = 0.5;
                const double angle = line.angle();
                line.setLength(length);
                line.setAngle(angle + 10);
                arrows_.moveTo(line.p1());
                arrows_.lineTo(line.p2());
                // painter->drawLine(line);
                line.setAngle(angle - 10);
                arrows_.moveTo(line.p1());
                arrows_.lineTo(line.p2());
                // painter->drawLine(line);
            }
        }
    }
}
#endif
} // namespace Gi
