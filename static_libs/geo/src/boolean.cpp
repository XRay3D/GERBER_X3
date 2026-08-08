// Раздувание и булевы операции поверх обёрток Geo::Polygon/Polygons
// (geo/polygon.h) -- то есть поверх точного CGAL-домена: центры и радиусы
// дуг проходят операции неизменными, дискретизации нет вовсе.
//
// Здесь нет ни одного вызова CGAL напрямую: всё выражается операциями
// региона (|, &, -, ^) и разложением офсета в сумму Минковского с кругом
// (geo/src/offsetcapsules.h).

#include "geo/boolean.h"
#include "geo/cancel.h"
#include "offsetcapsules.h"

#include <QtGlobal>

namespace Geo {

namespace {

// Куски Минковского границы одной полилинии, раздутой на d.
//
// Исходная окружность -- особый случай: её куски (два полусектора) имели бы
// точно совпадающие радиальные торцы, худший вход для булева движка, тогда
// как раздутая окружность -- это просто диск радиуса R0 + d.
Polylines capsulesOf(const Polyline& poly, double d) {
    if(const auto circle = Offset::fullCircleOf(poly))
        return {Offset::disc(circle->first, circle->second + d)};
    return Offset::capsulesFor(poly, d);
}

// Раздутый регион: R (+) диск(d) = R объединить с кусками ВСЕЙ его границы
// -- и внешней, и границ отверстий. Отсюда и берётся то, что дырка при
// раздувании сжимается: куски её границы заполняют ободок изнутри, а дырка
// тоньше 2d исчезает целиком.
//
// Куски берутся из ПОДАННЫХ контуров, а не из границы самого региона:
// лишние (те, что легли внутри R) объединению не мешают, зато так
// раздуваются и открытые контуры, у которых площади нет вовсе.
Polygons grow(Polygons region, const Polylines& boundary, double d) {
    if(d <= 0.0) return region;
    Polylines parts;
    for(const Polyline& contour: boundary) {
        checkCancelled();
        for(Polyline& capsule: capsulesOf(contour, d))
            parts.push_back(std::move(capsule));
    }
    return region | Polygons{parts};
}

} // namespace

Polygons Inflate_::operator()(const Polylines& polylines, double delta) const {
    // Сжатие -- через границу САМОГО региона, а не через поданные контуры:
    // у наложившихся друг на друга контуров общий шов границей уже не
    // является, и вычитание его капсул прорезало бы регион насквозь. Для
    // раздувания та же разница безвредна (лишние капсулы и так внутри), а
    // капсулы открытых контуров там ещё и нужны.
    if(delta < 0.0) return operator()(Polygons{polylines}, delta);
    return grow(Polygons{polylines}, polylines, delta * 0.5);
}

Polygons Inflate_::operator()(const Polylines& polylines) const {
    // Каждая полилиния -- на свою собственную ширину; регион собирается
    // один на весь список, поэтому вложенность (тело/пустота) учитывается
    // сразу и целиком.
    Polygons region{polylines};
    Polylines parts;
    for(const Polyline& poly: polylines) {
        checkCancelled();
        const double d = std::abs(poly.width) * 0.5;
        if(d <= 0.0) continue;
        for(Polyline& capsule: capsulesOf(poly, d)) parts.push_back(std::move(capsule));
    }
    return parts.empty() ? region : region | Polygons{parts};
}

Polygons Inflate_::operator()(const Polygons& region, double delta) const {
    const double d = std::abs(delta) * 0.5;
    if(d <= 0.0) return region;

    // Одна и та же полоса вдоль границы -- и наружу, и внутрь: снаружи она
    // прирастает к региону, изнутри съедается из него. Во втором случае
    // остаются ровно те точки региона, что дальше d от его границы, --
    // эрозия без всякого приближения.
    const Polygons band = region.boundaryBand(d);
    return delta > 0.0 ? region | band : region - band;
}

Polygons BooleanOp_::operator()(
    BooleanOp_::ClipType cliptype, BooleanOp_::FillRule fillrule,
    const Polylines& subjects, const Polylines& clips) {
    return operator()(cliptype, fillrule, Polygons{subjects}, Polygons{clips});
}

Polygons BooleanOp_::operator()(
    BooleanOp_::ClipType cliptype, BooleanOp_::FillRule fillrule,
    const Polygons& subjects, const Polygons& clips) {
    // Правило заливки задаётся уже самим регионом (вложенность разобрана
    // при его построении), так что различать здесь нечего.
    if(fillrule == FillRule::EvenOdd)
        qWarning("BooleanOp: FillRule::EvenOdd не поддержан -- регион уже разобран по вложенности");

    Polygons result = subjects;
    switch(cliptype) {
    case ClipType::NoClip      : break;
    case ClipType::Union       : result |= clips; break;
    case ClipType::Intersection: result &= clips; break;
    case ClipType::Difference  : result -= clips; break;
    case ClipType::Xor         : result ^= clips; break;
    }
    return result;
}

} // namespace Geo
