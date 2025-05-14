#include "Connectivity.h"

#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>

#if CONNECTIVITY

namespace TopoR {

QGraphicsItem* Connectivity::Wire::Subwire::graphicsItem(const QColor& color) const {
    QPainterPath path;
    auto arc = [&path, this](ArcDir dir, const QPointF& center, const QPointF& stop) {
        const auto start = path.currentPosition();
        // double radius = sqrt(pow((center.x() - p1.x()), 2) + pow((center.y() - p1.y()), 2));
        // double start = atan2(p1.y() - center.y(), p1.x() - center.x());
        // double stop = atan2(p2.y() - center.y(), p2.x() - center.x());
        // const double sign[]{-1.0, +1.0};
        // if(!ccw && stop >= start)
        //     stop -= 2.0 * M_PI;
        // else if(ccw && stop <= start)
        //     stop += 2.0 * M_PI;
        // start = qRadiansToDegrees(start);
        // stop = qRadiansToDegrees(stop);
        // double angle = qAbs(stop - start);
        // angle *= sign[ccw];
        // path.arcTo(
        //     -radius + center.x(), -radius + center.y(), radius * 2, radius * 2,
        //     start, angle);
        // QPainterPath path_;
        // path_.moveTo(path.currentPosition());
        // path_.arcTo(
        //     -radius + center.x(), -radius + center.y(), radius * 2, radius * 2,
        //     start, angle);
        // auto item = new QGraphicsPathItem{path_};
        // // auto item = new QGraphicsEllipseItem{-r + center.x, -r + center.y, r * 2, r * 2};
        // item->setPen({Qt::green, 0.0});
        // item->setZValue(100000);
        // item->setToolTip(QString{"A1=%1\nA2=%2\n%3"}.arg(start).arg(stop).arg(angle));
        // ui->graphicsView->addItem(item);
        const QLineF line1{center, start};
        const QLineF line2{center, stop};
        const auto a1 = line1.angle();
        const auto a2 = line2.angle();
        const auto radius = line1.length();

        auto aSpan = a2 - a1;

        if(dir == CCW) {
            if(aSpan > 0.0) aSpan -= 360.0;
        } else {
            if(aSpan < 0.0) aSpan += 360.0;
        }

        path.arcTo(
            -radius + center.x(),
            -radius + center.y(),
            radius * 2,
            radius * 2,
            a1, aSpan);

#if 0 // debug
                    QPainterPath path_;
                    path_.moveTo(p1);
                    path_.arcTo(
                        -radius + center.x(), -radius + center.y(), radius * 2, radius * 2,
                        a1, a);
                    auto item = new QGraphicsPathItem{path_};
                    // auto item = new QGraphicsEllipseItem{-r + center.x, -r + center.y, r * 2, r * 2};
                    item->setPen({Qt::magenta, 0.0});
                    item->setZValue(100000);
                    item->setToolTip(QString{"A1=%1\nA2=%2\n%3"}.arg(a1).arg(a2).arg(a));
                    ui->graphicsView->addItem(item);
#endif
    };

    path.moveTo(start);

    for(auto&& track: Tracks)
        track.visit(
            [&path](const TrackLine& track) { path.lineTo(track.end); },
            [&arc](const TrackArc& track) { arc(CCW, track.center, track.end); },
            [&arc](const TrackArcCW& track) { arc(CW, track.center, track.end); });
    auto item = new QGraphicsPathItem{path};
    item->setPen({color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin});
    return item;
}


} // namespace TopoR

#endif
