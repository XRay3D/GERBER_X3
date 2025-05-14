#include "Commons.h"

#include <QPoint>
#include <QTransform>

namespace TopoR {

void arc(ArcDir dir, QPainterPath& path, const std::optional<QPointF>& startOpt, const QPointF& center, const QPointF& stop) {
    QPointF start;
    if(startOpt.has_value()) {
        start = startOpt.value();
        path.moveTo(start);
    } else
        start = path.currentPosition();

    const auto a1 = QLineF{center, start}.angle();
    const auto a2 = QLineF{center, stop}.angle();
    const auto radius = QLineF{center, start}.length();

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
}

QPainterPath SegmentLine::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentLine::drawTo(QPainterPath& path) const {
    path.lineTo(end);
}

QPainterPath SegmentArcCCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcCCW::drawTo(QPainterPath& path) const {
    arc(CCW, path, {}, center, end);
}

QPainterPath SegmentArcCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcCW::drawTo(QPainterPath& path) const {
    arc(CW, path, {}, center, end);
}

QPainterPath SegmentArcByAngle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcByAngle::drawTo(QPainterPath& /*path*/) const {
    // FIXME path.lineTo(end);
}

QPainterPath SegmentArcByMiddle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcByMiddle::drawTo(QPainterPath& /*path*/) const {
    // FIXME path.lineTo(end);
}

QPainterPath ArcCCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcCCW::drawTo(QPainterPath& path) const {
    arc(CCW, path, start, center, end);
}

QPainterPath ArcCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcCW::drawTo(QPainterPath& path) const {
    arc(CW, path, start, center, end);
}

QPainterPath ArcByAngle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcByAngle::drawTo(QPainterPath& /*path*/) const {
    // FIXME path.lineTo(end);
}

QPainterPath ArcByMiddle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcByMiddle::drawTo(QPainterPath& /*path*/) const {
    // FIXME path.lineTo(end);
}

QPainterPath Circle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Circle::drawTo(QPainterPath& path) const {
    path.addEllipse(center, diameter * 0.5, diameter * 0.5);
}

QPainterPath Line::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Line::drawTo(QPainterPath& path) const {
    for(int fl{}; auto&& pt: Dots)
        if(!fl++) path.moveTo(pt);
        else [[likely]] path.lineTo(pt);
}

QPainterPath Polyline::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Polyline::drawTo(QPainterPath& path) const {
    path.moveTo(start);
    for(auto&& segment: segments)
        segment.visit([&path](auto&& segment) { segment.drawTo(path); });
}

QPainterPath Contour::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Contour::drawTo(QPainterPath& path) const {
    path.moveTo(start);
    for(auto&& segment: segments)
        segment.visit([&path](auto&& segment) { segment.drawTo(path); });
    if(path.currentPosition() != start)
        path.lineTo(start);
}

QPainterPath Rect::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Rect::drawTo(QPainterPath& path) const {
    QRectF rect;
    rect.setTopLeft(Dots.front());
    rect.setBottomRight(Dots.back());
    path.addRect(rect);
}

QPainterPath FilledContour::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void FilledContour::drawTo(QPainterPath& path) const {
    path.moveTo(start);
    for(auto&& segment: segments)
        segment.visit([&path](auto&& segment) { segment.drawTo(path); });
    if(path.currentPosition() != start)
        path.lineTo(start);
}

QPainterPath FilledCircle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void FilledCircle::drawTo(QPainterPath& path) const {
    path.addEllipse(center, diameter * 0.5, diameter * 0.5);
}

QPainterPath FilledRect::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void FilledRect::drawTo(QPainterPath& path) const {
    QRectF rect;
    rect.setTopLeft(Dots.front());
    rect.setBottomRight(Dots.back());
    path.addRect(rect);
}

QPainterPath Polygon::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Polygon::drawTo(QPainterPath& path) const {
    for(auto&& pt: Dots)
        path.lineTo(pt);
}

QPainterPath TrackArcCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void TrackArcCW::drawTo(QPainterPath& path) const {
    arc(CW, path, {}, center, end);
}

QPainterPath TrackArc::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void TrackArc::drawTo(QPainterPath& path) const {
    arc(CCW, path, {}, center, end);
}

QPainterPath TrackLine::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void TrackLine::drawTo(QPainterPath& path) const {
    path.lineTo(end);
}

double Ut::UnitsConvert(dist inUnits, dist outUnits) {
    // clang-format off
    auto k = [](dist in_units) {
        switch (in_units) {
        case dist::mkm:  return 0.001;
        case dist::cm:   return 10.0;
        case dist::dm:   return 100.0;
        case dist::m:    return 1000.0;
        case dist::mil:  return 0.0254;
        case dist::inch: return 25.4;
        case dist::mm:
        default:         return 1.0;
        }
    };
    switch (outUnits) {
    case dist::mkm:  return k(inUnits) * 1000;
    case dist::cm:   return k(inUnits) * 0.1;
    case dist::dm:   return k(inUnits) * 0.01;
    case dist::m:    return k(inUnits) * 0.001;
    case dist::mil:  return k(inUnits) * 39.37007874015748;
    case dist::inch: return k(inUnits) * 0.03937007874015748;
    case dist::mm:   return k(inUnits);
    default:         return 1.0;
    }
    // clang-format on
}

} // namespace TopoR
