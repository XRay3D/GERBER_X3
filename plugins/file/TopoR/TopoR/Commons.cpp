#include "Commons.h"
namespace TopoR {
void Coord::Shift(double x_, double y_) {
    x += x_;
    y += y_;
}
void Coord::UnitsConvert(dist in_units, dist out_units) {
    x = Ut::UnitsConvert(x, in_units, out_units);
    y = Ut::UnitsConvert(y, in_units, out_units);
}
void SegmentLine::Shift(double x, double y) {
    End.Shift(x, y);
}
void SegmentLine::UnitsConvert(dist in_units, dist out_units) {
    End.UnitsConvert(in_units, out_units);
}
void SegmentArcCCW::Shift(double x, double y) {
    Center.Shift(x, y);
    End.Shift(x, y);
}
void SegmentArcCCW::UnitsConvert(dist in_units, dist out_units) {
    Center.UnitsConvert(in_units, out_units);
    End.UnitsConvert(in_units, out_units);
}
void SegmentArcByMiddle::Shift(double x, double y) {
    Middle.Shift(x, y);
    End.Shift(x, y);
}
void SegmentArcByMiddle::UnitsConvert(dist in_units, dist out_units) {
    Middle.UnitsConvert(in_units, out_units);
    End.UnitsConvert(in_units, out_units);
}
void ArcCCW::Shift(double x, double y) {
    Start.Shift(x, y);
    Center.Shift(x, y);
    End.Shift(x, y);
}
void ArcCCW::UnitsConvert(dist in_units, dist out_units) {
    Start.UnitsConvert(in_units, out_units);
    Center.UnitsConvert(in_units, out_units);
    End.UnitsConvert(in_units, out_units);
}
void ArcByAngle::Shift(double x, double y) {
    Start.Shift(x, y);
    End.Shift(x, y);
}
void ArcByAngle::UnitsConvert(dist in_units, dist out_units) {
    Start.UnitsConvert(in_units, out_units);
    End.UnitsConvert(in_units, out_units);
}
void ArcByMiddle::Shift(double x, double y) {
    Start.Shift(x, y);
    Middle.Shift(x, y);
    End.Shift(x, y);
}
void ArcByMiddle::UnitsConvert(dist in_units, dist out_units) {
    Start.UnitsConvert(in_units, out_units);
    Middle.UnitsConvert(in_units, out_units);
    End.UnitsConvert(in_units, out_units);
}
void Circle::Shift(double x, double y) {
    Center.Shift(x, y);
}
void Circle::UnitsConvert(dist in_units, dist out_units) {
    diameter = Ut::UnitsConvert(diameter, in_units, out_units);
    Center.UnitsConvert(in_units, out_units);
}
bool Line::ShouldSerialize_Dots() {
    return Dots.size();
}
void Line::Shift(double x, double y) {
    for(int i{}; i < Dots.size(); i++)
        Dots[i].Shift(x, y);
}
void Line::UnitsConvert(dist in_units, dist out_units) {
    for(int i{}; i < Dots.size(); i++)
        Dots[i].UnitsConvert(in_units, out_units);
}
bool Polyline::ShouldSerialize_Segments() {
    return Segments.size();
}
void Polyline::Shift(double x, double y) {
    Start.Shift(x, y);
    // for(int i{}; i < Segments.size(); i++)
    //     (std::dynamic_pointer_cast<IBaseSegment>(Segments[])).Shift(x, y);
}
void Polyline::UnitsConvert(dist in_units, dist out_units) {
    Start.UnitsConvert(in_units, out_units);
    // for(int i{}; i < Segments.size(); i++)
    //     (std::dynamic_pointer_cast<IBaseSegment>(Segments[])).UnitsConvert(in_units, out_units);
}
void TrackArcCW::Shift(double x, double y) {
    Center.Shift(x, y);
    End.Shift(x, y);
}
void TrackArcCW::UnitsConvert(dist in_units, dist out_units) {
    Center.UnitsConvert(in_units, out_units);
    End.UnitsConvert(in_units, out_units);
}
void TrackLine::Shift(double x, double y) {
    End.Shift(x, y);
}
void TrackLine::UnitsConvert(dist in_units, dist out_units) {
    End.UnitsConvert(in_units, out_units);
}
void Thermal::UnitsConvert(dist in_units, dist out_units) {
    spokeWidth = Ut::UnitsConvert(spokeWidth, in_units, out_units);
}
void Detail::Shift(double x, double y) {
    // if((std::dynamic_pointer_cast<IBaseFigure>(Figure)) != nullptr)
    //     (std::dynamic_pointer_cast<IBaseFigure>(Figur)).Shift(x, y);
}
void Detail::UnitsConvert(dist in_units, dist out_units) {
    lineWidth = Ut::UnitsConvert(lineWidth, in_units, out_units);
    // if((std::dynamic_pointer_cast<IBaseFigure>(Figure)) != nullptr)
    //     (std::dynamic_pointer_cast<IBaseFigure>(Figur)).UnitsConvert(in_units, out_units);
}
bool Text::getMirrorSpecified() const {
    return mirror != Bool::off;
}
void Text::Shift(double x, double y) {
    Org.Shift(x, y);
}
void Text::UnitsConvert(dist in_units, dist out_units) {
    Org.UnitsConvert(in_units, out_units);
}
double Ut::UnitsConvert(double value, dist in_units, dist out_units) {
    double k;
    switch(in_units) {
    case dist::mkm: k = 0.001; break;
    case dist::cm: k = 10; break;
    case dist::dm: k = 100; break;
    case dist::m: k = 1000; break;
    case dist::mil: k = 0.0254000000000000002032; break;
    case dist::inch: k = 25.4000000000000002032; break;
    case dist::mm:
    default: k = 1; break;
    }
    switch(out_units) {
    case dist::mkm: return static_cast<double>(value * k * 1000);
    case dist::cm: return static_cast<double>(value * k * 0.1);
    case dist::dm: return static_cast<double>(value * k * 0.01);
    case dist::m: return static_cast<double>(value * k * 0.001);
    case dist::mil: return static_cast<double>(value * k * 39.37007874015748);
    case dist::inch: return static_cast<double>(value * k * 0.03937007874015748);
    case dist::mm: return static_cast<double>(value * k);
    default: return value;
    }
}

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
    path.lineTo(End);
}

QPainterPath SegmentArcCCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcCCW::drawTo(QPainterPath& path) const {
    arc(CCW, path, {}, Center, End);
}

QPainterPath SegmentArcCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcCW::drawTo(QPainterPath& path) const {
    arc(CW, path, {}, Center, End);
}

QPainterPath SegmentArcByAngle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcByAngle::drawTo(QPainterPath& path) const {
    // FIXME path.lineTo(End);
}

QPainterPath SegmentArcByMiddle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void SegmentArcByMiddle::drawTo(QPainterPath& path) const {
    // FIXME path.lineTo(End);
}

QPainterPath ArcCCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcCCW::drawTo(QPainterPath& path) const {
    arc(CCW, path, Start, Center, End);
}

QPainterPath ArcCW::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcCW::drawTo(QPainterPath& path) const {
    arc(CW, path, Start, Center, End);
}

QPainterPath ArcByAngle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcByAngle::drawTo(QPainterPath& path) const {
    // FIXME path.lineTo(End);
}

QPainterPath ArcByMiddle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void ArcByMiddle::drawTo(QPainterPath& path) const {
    // FIXME path.lineTo(End);
}

QPainterPath Circle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Circle::drawTo(QPainterPath& path) const {
    path.addEllipse(Center, diameter * 0.5, diameter * 0.5);
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
    path.moveTo(Start);
    for(auto&& segment: Segments)
        segment.visit([&path](auto&& segment) { segment.drawTo(path); });
}

QPainterPath Contour::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void Contour::drawTo(QPainterPath& path) const {
    path.moveTo(Start);
    for(auto&& segment: Segments)
        segment.visit([&path](auto&& segment) { segment.drawTo(path); });
    if(path.currentPosition() != Start)
        path.lineTo(Start);
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
    path.moveTo(Start);
    for(auto&& segment: Segments)
        segment.visit([&path](auto&& segment) { segment.drawTo(path); });
    if(path.currentPosition() != Start)
        path.lineTo(Start);
}

QPainterPath FilledCircle::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void FilledCircle::drawTo(QPainterPath& path) const {
    path.addEllipse(Center, diameter * 0.5, diameter * 0.5);
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
    arc(CW, path, {}, Center, End);
}

QPainterPath TrackArc::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void TrackArc::drawTo(QPainterPath& path) const {
    arc(CCW, path, {}, Center, End);
}

QPainterPath TrackLine::toPPath() const {
    QPainterPath path;
    return drawTo(path), path;
}
void TrackLine::drawTo(QPainterPath& path) const {
    path.lineTo(End);
}

} // namespace TopoR
