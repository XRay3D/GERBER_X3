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

Curve toCurve(Path path) {
    Curve curve;

    auto eqCenter = [](Point l, Point r) {
        auto cl{~GetZ(l)}, cr{~GetZ(r)};
        // qInfo() << ~cl << ~cr;
        // qCritical() << QLineF{~l, cl}.length() << QLineF{~r, cr}.length();
        return cl == cr
            && std::abs(QLineF{~l, cl}.length() - QLineF{~r, cr}.length()) < 1.e-2;
    };

    auto notEqCenter = [](Point l, Point r) {
        auto cl{~GetZ(l)}, cr{~GetZ(r)};
        // qInfo() << ~cl << ~cr;
        // qCritical() << QLineF{~l, cl}.length() << QLineF{~r, cr}.length();
        return cl != cr;
        // && std::abs(QLineF{~l, cl}.length() - QLineF{~r, cr}.length()) < 1.e-2;
    };

    r::rotate(path, r::adjacent_find(path, notEqCenter));

    auto chunks = v::chunk_by(path, eqCenter);

    for(auto chunk: chunks) {
        if(chunk.size() > 2) {
            double ange = QLineF{~chunk[0], ~chunk[1]}.angleTo(QLineF{~chunk[1], ~chunk[2]});
            curve.emplace_back(~chunk.front());
            curve.emplace_back(
                ~chunk.back(),
                ~GetZ(chunk.front()),
                Area(Path{chunk[0], chunk[1], GetZ(chunk[0])}) > .0 ? Vertex::Ccw : Vertex::Cw);
        } else if(chunk.size() == 1) {
            curve.emplace_back(~chunk.front());
        } else {
            curve.emplace_back(~chunk.front());
            curve.emplace_back(~chunk.back());
        }
    }

    curve.emplace_back(~path.front());

    new Debug{toPPath(curve), Qt::green};
    return curve;
}

Debug::Debug(const QColor& color, double width) {
    pen_ = {color, width};
    App::grView().addItem(this);
    setZValue(std::numeric_limits<double>::max());
    setVisible(true);
}

Debug::Debug(const Path& path, const QColor& color, double width)
    : Debug{Paths{path}, color, width} { }

Debug::Debug(const Paths& paths, const QColor& color, double width)
    : Debug{color, width} {
    paths_ = paths;
#if 0
    for(const Path& path: paths)
        shape_.addPolygon(~path);
    boundingRect_ = shape_.boundingRect();
#else
    for(const Path& path: paths) {
        toCurve(path);
        for(const Point& pt: path) {
            // auto qp = ~GetZ(pt);
            // shape_.addEllipse(qp.x() - 0.05, qp.y() - 0.05, 0.1, 0.1);
        }
    }

    auto bounds = GetBounds(paths);
    boundingRect_ = QRectF{
        dScale * bounds.left,
        dScale * bounds.top,
        dScale * (bounds.right - bounds.left),
        dScale * (bounds.bottom - bounds.top),
    };
#endif
}

Debug::Debug(const QPainterPath& path, const QColor& color, double width)
    : Debug{color, width} {
    shape_ = path;
    boundingRect_ = shape_.boundingRect();
    if(path.isEmpty()) delete this; // NOTE
}

QRectF Debug::boundingRect() const { return boundingRect_; }

void Debug::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    Q_UNUSED(option)

    if(pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if(colorPtr_)
        color_ = *colorPtr_;

    double scale = scaleFactor();

    if(pen_.widthF() == 0) {
        QPen pen(pen_);
        pen.setWidthF(1.5 * scale);
        painter->setPen(pen);
    } else
        painter->setPen(pen_);

    // if(option->state & QStyle::State_MouseOver) {
    //     QPen pen(pen_);
    //     pen.setWidthF(2.0 * scale);
    //     pen.setStyle(Qt::CustomDashLine);
    //     pen.setCapStyle(Qt::FlatCap);
    //     pen.setDashPattern({2.0, 2.0});
    //     painter->setPen(pen);
    // }

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(shape_);

    //    painter->setPen(QPen(Qt::magenta, 0.0));
    //    painter->drawRect(rect_);

    double len = 10 * scale;
    for(const Path& path: paths_) {
        for(const Point& pt: path) {
            QPointF p = ~GetZ(pt);
            painter->drawLine(p, p + QPointF{.0, -len});
            painter->drawLine(p, p + QPointF{.0, +len});
            painter->drawLine(p, p + QPointF{-len, .0});
            painter->drawLine(p, p + QPointF{+len, .0});
        }
    }

    ////////////////////////////////////////////////////// for debug cut direction
    if(sc_ != scale) updateArrows();
    painter->drawPath(arrows_);
}

int Debug::type() const { return Type::Debug; }

Paths Debug::paths(int) const { return {} /*paths_*/; }

void Debug::updateArrows() {
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

} // namespace Gi
