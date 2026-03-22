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
#include "gi_dbg.h"
#include "app.h"
#include "graphicsview.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#define QT_DEBUG
// #undef QT_DEBUG

namespace Gi {

Debug_::Debug_(const QColor& color, double width) {
    pen_ = {color, width};
    App::grView().addItem(this);
    setZValue(std::numeric_limits<double>::max());
    setVisible(true);
}

Debug_::Debug_(const Path& path, const QColor& color, double width)
    : Debug_{Paths{path}, color, width} { }

Debug_::Debug_(const Paths& paths, const QColor& color, double width)
    : Debug_{color, width} {
    paths_ = paths;
#if 1
    for(const Path& path: paths)
        shape_.addPolygon(~path);
    boundingRect_ = shape_.boundingRect();
#else
    for(Path& path: paths_) {
        toCurve(path);
        // for(const Point& pt: path) {
        // auto qp = ~GetZ(pt);
        // shape_.addEllipse(qp.x() - 0.05, qp.y() - 0.05, 0.1, 0.1);
        // }
    }

    for(auto&& pt: paths_ | v::join | v::transform(GetC) | v::transform(toQPointF))
        centers.emplace(pt);

    auto bounds = GetBounds(paths);
    boundingRect_ = QRectF{
        dScale * bounds.left,
        dScale * bounds.top,
        dScale * (bounds.right - bounds.left),
        dScale * (bounds.bottom - bounds.top),
    };

#endif
}

Debug_::Debug_(const QPainterPath& path, const QColor& color, double width)
    : Debug_{color, width} {
    shape_ = path;
    boundingRect_ = shape_.boundingRect();
    if(path.isEmpty()) delete this; // NOTE
}

QRectF Debug_::boundingRect() const { return boundingRect_; }

void Debug_::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    Q_UNUSED(option)

    if(pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if(colorPtr_)
        color_ = *colorPtr_;

    double scale = scaleFactor();

    QPen pen{pen_};
    if(pen_.widthF() == 0) {
        pen.setWidthF(1.5 * scale);
        painter->setPen(pen);
    } else
        painter->setPen(pen_);

    // if(option->state & QStyle::State_MouseOver) {
    // QPen pen{pen_};
    // pen.setWidthF(2.0 * scale);
    // pen.setStyle(Qt::CustomDashLine);
    // pen.setCapStyle(Qt::FlatCap);
    // pen.setDashPattern({2.0, 2.0});
    // painter->setPen(pen);
    // }

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(shape_);

    // painter->setPen(QPen(Qt::magenta, 0.0));
    // painter->drawRect(rect_);

    pen.setColor({0, 255, 0, 255});
    painter->setPen(pen);

    double len = 10 * scale;

    for(const Point& pt: paths_ | v::join | v::filter(&Point::z)) {
        QPointF p = ~GetC(pt);
        painter->drawLine(p, ~pt);
    }

    for(const QPointF& p: centers) {
        painter->drawLines({
            {p, p + QPointF{.0, -len}},
            {p, p + QPointF{.0, +len}},
            {p, p + QPointF{-len, .0}},
            {p, p + QPointF{+len, .0}},
        });
    }

    ////////////////////////////////////////////////////// for Debug_ cut direction
    if(sc_ != scale) updateArrows();
    painter->drawPath(arrows_);
}

int Debug_::type() const { return Type::Debug; }

Paths Debug_::paths(int) const { return {} /*paths_*/; }

void Debug_::updateArrows() {
    sc_ = scaleFactor();
    arrows_ = QPainterPath(); //.clear();
    if(qFuzzyIsNull(pen_.widthF())) {
        for(const QPolygonF& path: shape_.toSubpathPolygons()) {
            for(auto&& [b, e]: path | v::pairwise | v::reverse) {
                QLineF line{e, b};
                double length = 30 * scaleFactor();

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
                break;
            }
        }
    }
}

} // namespace Gi
