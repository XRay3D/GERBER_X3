/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_polyline.h"
#include "gi_dbg.h"
#include <QPainterPath>

namespace Dxf {

void PolyLine::parse(CodeData& code) {
    do {
        data.push_back(code);
        if(code != u"VERTEX"_s) {
            code = sp->nextCode();
            switch(code.code()) {
            case StartWidth  : startWidth = code; break;
            case EndWidth    : endWidth = code; break;
            case PolylineFlag: polylineFlags = code; break;
            default          : Entity::parse(code);
            }
        } else {
            polyLine.emplace_back(sp).parse(code);
        }
    } while(code != u"SEQEND"_s);
    do {
        code = sp->nextCode();
        Entity::parse(code);
    } while(code.code() != 0);
}

Entity::Type PolyLine::type() const { return Type::POLYLINE; }

DxfGo PolyLine::toGo() const {
    qInfo("PolyLine");
#if 1
    QPainterPath path;
    auto addSeg = [&path](const Vertex& source, const Vertex& target) mutable {
        addArcTo(path, source, target, source.bulge);
    };

    for(auto&& [from, to]: polyLine | v::pairwise) addSeg(from, to);

    if(polylineFlags & ClosedPolyline) addSeg(polyLine.back(), polyLine.front());

    QTransform m;
    m.scale(u, u);
    QPainterPath path2;

    // new Gi::Debug{path, Qt::yellow};

    for(auto& polyLine: path.toSubpathPolygons(m))
        path2.addPolygon(polyLine);
    QTransform m2;
    m2.scale(d, d);
    auto polygon(path2.toSubpathPolygons(m2));

    if(polylineFlags & ClosedPolyline) // FIXME
        return DxfGo{id, ~polygon.value(0), ~polygon};
    else
        return DxfGo{id, ~polygon.value(0), {}};
#else // TODO direct construct path

    if(startWidth || endWidth)
        qWarning("TODO startWidth | endWidth");

    if(polylineFlags & ClosedPolyline
        && polyLine.size() == 2
        && qFuzzyCompare(polyLine.front().bulge, 1.)
        && qFuzzyCompare(polyLine.back().bulge, 1.)) {
        ::Point center = ~((polyLine.front().point() + polyLine.back()) / 2);

        DxfGo go{
            id,
            CirclePath((length(polyLine.front(), polyLine.back())) * uScale, center),
            {CirclePath((length(polyLine.front(), polyLine.back()) /*+ constantWidth*/) * uScale, center)},
        };

        go.GraphicObject::pos = center;
        go.type = DxfGo::Type{DxfGo::FlStamp | DxfGo::Circle};
        go.name = u"id|LwPolyline/Circle"_s;
        return go;
    }

    ::Path path;

    enum InterpolationMode {
        Linear = 1,
        ClockwiseCircular = 2,
        CounterClockwiseCircular = 3
    };

    static auto constructArc = [](::Point center, int dir, double radius, double start, double stop) {
        auto arcPath = arc(center, radius, start, stop, dir);
        // Последняя точка в вычисленной дуге может иметь числовые ошибки.
        // Точной конечной точкой является указанная (x, y). Замена.
        // if(arcPath.size()) arcPath.back() = state_.curPos();
        // else arcPath.emplace_back(state_.curPos());
        // SetC(arcPath.back(), center);
        return arcPath;
    };
    auto addSeg = [&path](const Vertex& source, const Vertex& target) {
        auto [center, asource, atarget, radius] = bulgeToArc(source, target, source.bulge);

        // radius = GetRadius(source, target, bulge);

        constexpr auto square = +[](double val) { return val * val; };

        const double common = (1 - square(source.bulge)) / (4 * source.bulge);
    #if 0
        QPointF center{(source.point() + target) / 2 + common * (source.point() - target)};

        const double cx = ((source.x + target.x) / 2) + common * (source.y - target.y);
        const double cy = ((source.y + target.y) / 2) + common * (target.x - source.x);
        const double sqr_bulge = square(source.bulge);
        const double sqr_rad = square(length(source, target)) * (1 / sqr_bulge + 2 + sqr_bulge) / 16;
        const double radius = std::sqrt(sqr_rad);

        QLineF ls{center, source};
        // const double r = ls.length();
        double asource = ls.angle();
        double atarget = ls.angleTo(QLineF{center, target});
        double span = atarget;
        atarget = asource + span;
center=QPointF{cx, cy};
    #endif
        // if(source.bulge > 0.) span = span - 360.;

        if(path.empty())
            path.emplace_back(~source);

        if(qFuzzyIsNull(source.bulge)) {
            path.emplace_back(~target);
            return;
        }

        // auto [c, a1, a2, r] = bulgeToArc(source, target, source.bulge);

        int dir = source.bulge > 0 //^ Area(Path{~center, ~source, ~target}) < 0
            ? CounterClockwiseCircular
            : ClockwiseCircular;

        if(Area(Path{~center, ~source, ~target}) > 0 && dir == ClockwiseCircular)
            std::swap(asource, atarget);

        ::Path arcPath = constructArc(~center, dir, radius * uScale, asource, atarget /*-qDegreesToRadians(asource), -qDegreesToRadians(atarget)*/);

        // if(Area(Path{~center, ~source, ~target}) < 0 && dir == CounterClockwiseCircular)
        //     r::reverse(arcPath);

        arcPath.front() = ~source;
        arcPath.back() = ~target;
        path.append_range(arcPath);
    };

    for(auto&& [from, to]: polyLine | v::pairwise)
        addSeg(from, to);

    if(polylineFlags & ClosedPolyline)
        addSeg(polyLine.back(), polyLine.front());

    // r::for_each(path, SetCSelf);

    // Paths paths = Inflate({path}, constantWidth * uScale, JoinType::Round, EndType::Round, 2.0, uScale / 1000);

    // Gi::Debug(paths, Qt::red);

    DxfGo go; //{id, path, {/*paths*/}}; // return {id, ~p.value(0), paths};

    if(polylineFlags & ClosedPolyline && 0) // FIXME
        go = {id, path, {path}};
    else
        go = {id, path, {}};

    go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::PolyLine);
    return go;
#endif
}

void PolyLine::write(QDataStream& stream) const {
    stream << polylineFlags;
    stream << startWidth;
    stream << endWidth;
}

void PolyLine::read(QDataStream& stream) {
    stream >> polylineFlags;
    stream >> startWidth;
    stream >> endWidth;
}

} // namespace Dxf
