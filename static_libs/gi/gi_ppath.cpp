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
#include "gi_ppath.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Gi {

PPath::PPath(const Polygon_with_holes_2& paths, AbstractFile* file) {
    // for(const Path& path: paths)
    // shape_.addPolygon(~path);
    shape_ = ConstructPath(paths.outer_boundary());
    boundingRect_ = shape_.boundingRect();
}

PPath::PPath(const Polygon_2& path, AbstractFile* file) {
    shape_ = ConstructPath(path);
    boundingRect_ = shape_.boundingRect();
}

PPath::PPath(const QPainterPath& path, AbstractFile* file) {
    shape_ = path;
    boundingRect_ = shape_.boundingRect();
}

QRectF PPath::boundingRect() const { return boundingRect_; }

void PPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
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

    QPen pen{pen_};
    pen.setWidthF(pen.widthF() * scaleFactor());
    if(option->state & QStyle::State_MouseOver) {
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({2.0, 2.0});
    }
    painter->setPen(pen);

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(shape_);

    if(sc_ != scaleFactor())
        updateArrows();
    painter->drawPath(arrows_);
}

int PPath::type() const { return Type::Path_; }

Paths PPath::paths(int) const { return {} /*paths_*/; }

void PPath::updateArrows() {
    sc_ = scaleFactor();
    arrows_ = {};
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

} // namespace Gi
