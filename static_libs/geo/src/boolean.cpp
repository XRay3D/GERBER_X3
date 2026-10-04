// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
// Раздувание и булевы операции поверх обёрток Geo::Polygon/Polygons
// (geo/polygon.h) -- то есть поверх точного CGAL-домена: центры и радиусы
// дуг проходят операции неизменными, дискретизации нет вовсе.
//
// Здесь нет ни одного вызова CGAL напрямую: всё выражается операциями
// региона (|, &, -, ^) и разложением офсета в сумму Минковского с кругом
// (geo/src/offsetcapsules.h).

#include "geo/boolean.h"
#include "cgal.h" // Cgal::weldTolerance
#include "geo/cancel.h"
#include "geo/util.h"
#include "offsetcapsules.h"
#include "phasestats.h"

#include <QtGlobal>

#include <map>

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

Polygons Inflate_::operator()(const Polygons& region, double delta, double coarse) const {
    const double d = std::abs(delta) * 0.5;
    if(d <= 0.0) return region;

    // Одна и та же полоса вдоль границы -- и наружу, и внутрь: снаружи она
    // прирастает к региону, изнутри съедается из него. Во втором случае
    // остаются ровно те точки региона, что дальше d от его границы, --
    // эрозия без всякого приближения (при coarse == 0; недобор черновой
    // полосы в обоих случаях сдвигает контур К границе, не от неё).
    const Polygons band = region.boundaryBand(d, coarse);
    PhaseScope stat{Phase::RegionOp};
    return delta > 0.0 ? region | band : region - band;
}

Polygons BooleanOp_::operator()(
    BooleanOp_::ClipType cliptype, BooleanOp_::FillRule fillrule,
    const Polylines& subjects, const Polylines& clips) {
    // Правило заливки применимо ровно здесь -- на СЫРЫХ контурах, пока
    // вложенность ещё не разобрана. Дальше её несёт сам регион.
    auto toRegion = [fillrule](const Polylines& contours) {
        return fillrule == FillRule::EvenOdd ? evenOdd(contours) : Polygons{contours};
    };
    return operator()(cliptype, fillrule, toRegion(subjects), toRegion(clips));
}

Polygons BooleanOp_::operator()(
    BooleanOp_::ClipType cliptype, BooleanOp_::FillRule fillrule,
    const Polygons& subjects, const Polygons& clips) {
    // Правило заливки задаётся уже самим регионом (вложенность разобрана
    // при его построении), так что различать здесь нечего. Кому нужен
    // even-odd от СЫРЫХ контуров -- см. Geo::evenOdd: перегрузка от Polylines
    // выше уже пропускает их через него.

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

//------------------------------------------------------------------------------
// Нормализация чертёжной геометрии (см. geo/boolean.h).

void stitch(Polylines& polylines, double glue) {
    auto joinable = [glue](QPointF a, QPointF b) { return distance(a, b) <= glue; };

    // Приклеивает `src` к хвосту `dst`, не дублируя общую точку стыка.
    auto append = [](Polyline& dst, const Polyline& src) {
        dst.insert(dst.end(), src.begin() + 1, src.end());
    };

    bool merged;
    do {
        merged = false;
        for(std::size_t i{}; i < polylines.size() && !merged; ++i) {
            if(polylines[i].closed || polylines[i].size() < 2) continue;
            for(std::size_t j{}; j < polylines.size(); ++j) {
                if(i == j || polylines[j].closed || polylines[j].size() < 2) continue;

                Polyline& a = polylines[i];
                Polyline b = polylines[j];

                if(joinable(a.back(), b.front())) {
                } else if(joinable(a.back(), b.back())) {
                    b.reverse();
                } else if(joinable(a.front(), b.back())) {
                    a.reverse(), b.reverse();
                } else if(joinable(a.front(), b.front())) {
                    a.reverse();
                } else
                    continue;

                append(a, b);
                polylines.erase(polylines.begin() + j);
                merged = true;
                break;
            }
        }
        checkCancelled(); // склейка квадратична по числу обрывков
    } while(merged);

    // Замкнувшийся сам на себя обрывок -- это контур, а не линия.
    for(Polyline& polyline: polylines)
        if(polyline.size() > 3 && joinable(polyline.front(), polyline.back()))
            polyline.close();
}

namespace {

// Тот же ли это контур: та же замкнутая граница, возможно начатая с другого
// узла -- стартовая вершина в чертеже произвольна. Оба контура уже приведены
// к одному обходу (ccw в evenOdd), так что встречный дубликат сюда попадает
// тоже. Сверяются и прогибы: у двух разных дуг через одни и те же вершины
// совпадать нечему.
bool sameContour(const Polyline& a, const Polyline& b, double tolerance) {
    if(a.size() != b.size()) return false;
    const std::size_t n = a.size();

    auto matchFrom = [&](std::size_t shift) {
        for(std::size_t i{}; i < n; ++i) {
            const Vertex& va = a[i];
            const Vertex& vb = b[(i + shift) % n];
            if(distance(va, vb) > tolerance) return false;
            if(std::abs(va.bulge - vb.bulge) > tolerance) return false;
        }
        return true;
    };

    for(std::size_t shift{}; shift < n; ++shift)
        if(distance(a.front(), b[shift]) <= tolerance && matchFrom(shift))
            return true;
    return false;
}

} // namespace

Polygons evenOdd(const Polylines& contours) {
    // Сперва отбор годных тел, и только потом XOR: дубликаты надо отсеять до
    // счёта чётности, иначе контур, нарисованный ДВАЖДЫ (обычный мусор
    // чертежа -- новая линия обведена поверх старой), взаимно уничтожился бы:
    // два накрытия -- пустота. Для чертежа же это всё одна линия, поэтому из
    // совпадающих контуров остаётся один; нечётные копии безвредны и так, но
    // правило одно на всех -- так предсказуемее.
    Polylines bodies;
    for(const Polyline& contour: contours) {
        // Порога по числу вершин здесь быть не может: в bulge-виде окружность
        // -- это ДВЕ вершины с прогибом 1 (полный оборот прогибом не
        // выражается, см. Geo::circle), и «меньше трёх -- не контур» выкидывало
        // бы ровно её. Годность решает isExactContour ниже.
        if(!contour.isClosed() || contour.size() < 2) continue;
        // Ориентация не значит НИЧЕГО: каждый контур входит телом, а дырки
        // получаются сами -- вложенный контур накрывает точку второй раз.
        Polyline body = contour;
        if(body.signedArea() < 0) body.reverse();
        if(!isExactContour(body)) continue; // вырожденный или самокасающийся

        // Допуск дубликата -- сварочный допуск выхода из точного домена: чьи
        // вершины неотличимы на выходе, те и есть одна линия.
        auto seenAlready = [&body](const Polyline& seen) {
            return sameContour(seen, body, exitWeldTolerance);
        };
        if(std::ranges::any_of(bodies, seenAlready)) continue;

        bodies.push_back(std::move(body));
        checkCancelled(); // сверка с уже отобранными квадратична
    }

    Polygons region;
    for(Polyline& body: bodies) {
        region ^= Polygons{Polylines{std::move(body)}};
        checkCancelled();
    }
    return region;
}

Polylines splitSelfTouching(const Polyline& contour) {
    if(!contour.closed || contour.size() < 3) return {contour};

    using Key = std::pair<double, double>;
    auto keyOf = [](const Vertex& v) { return Key{v.x(), v.y()}; };

    // Шип: в вершине b контур разворачивается и идёт назад по той же прямой
    // -- конец короткого плеча лежит на длинном. Площади у шипа нет, а
    // самокасание есть. KiCad оставляет такие в заливке зон: шаг в одну
    // единицу файла (10 нм) назад на стыке прямой со скруглением.
    auto isSpike = [](const Vertex& a, const Vertex& b, const Vertex& c) {
        if(a.isArc() || b.isArc()) return false; // только прямые плечи
        const QPointF ab = a - b, cb = c - b;
        if(QPointF::dotProduct(ab, cb) <= 0.0) return false; // не разворот
        const double longer = std::max(std::hypot(ab.x(), ab.y()), std::hypot(cb.x(), cb.y()));
        return std::abs(ab.x() * cb.y() - ab.y() * cb.x()) <= longer * Cgal::weldTolerance;
    };
    auto trimSpikes = [&](Polyline& loop) {
        for(std::size_t i{}; loop.size() >= 3 && i < loop.size();) {
            const std::size_t n = loop.size();
            if(isSpike(loop[(i + n - 1) % n], loop[i], loop[(i + 1) % n])) {
                loop.erase(loop.begin() + static_cast<std::ptrdiff_t>(i));
                i = i ? i - 1 : 0; // сосед слева мог сам стать шипом
            } else
                ++i;
        }
        // Стык через начало: срезанный в конце шип меняет соседа первой вершины.
        while(loop.size() >= 3 && isSpike(loop.back(), loop.front(), loop[1]))
            loop.erase(loop.begin());
    };

    Polylines loops;
    auto take = [&](Polyline&& loop) {
        trimSpikes(loop);
        if(loop.size() < 2) return;
        loop.closed = true;
        loop.width = contour.width;
        // Перемычка туда и обратно: площади нет, а с лишней вершиной на
        // одном из ходов она выходит ниточкой -- отсюда порог, а не ноль.
        if(std::abs(loop.signedArea()) <= loop.perimeter() * exitWeldTolerance) return;
        loops.push_back(std::move(loop));
    };

    // Незамкнутый пока хвост обхода. Точки в нём попарно различны: всякий
    // повтор тут же снимает петлю.
    Polyline stack;
    std::map<Key, std::size_t> seen; // точка -> её место в stack
    for(const Vertex& v: contour) {
        const auto it = seen.find(keyOf(v));
        if(it == seen.end()) {
            seen.emplace(keyOf(v), stack.size());
            stack.push_back(v);
            continue;
        }
        const auto from = stack.begin() + static_cast<std::ptrdiff_t>(it->second);
        Polyline loop(from, stack.end());
        for(auto gone = from + 1; gone != stack.end(); ++gone) seen.erase(keyOf(*gone));
        stack.erase(from + 1, stack.end());
        // Вершина стыка остаётся в хвосте, но ребро из неё теперь другое --
        // то, что выходит из её ПОВТОРА.
        stack.back().bulge = v.bulge;
        take(std::move(loop));
    }
    take(std::move(stack));
    return loops;
}

Normalized normalize(Polylines polylines, double glue) {
    std::erase_if(polylines, [](const Polyline& p) { return p.size() < 2; });

    auto isClosed = [glue](const Polyline& p) {
        return p.closed || (p.size() > 3 && distance(p.front(), p.back()) <= glue);
    };

    // Сперва отбираем уже замкнутое, потом склеиваем остальное: склейка
    // квадратична, и гонять её по тому, что и так замкнуто, незачем.
    Polylines closed;
    for(std::size_t i{}; i < polylines.size();)
        if(isClosed(polylines[i])) {
            closed.emplace_back(std::move(polylines[i])).close();
            polylines.erase(polylines.begin() + i);
        } else
            ++i;

    stitch(polylines, glue);

    Normalized result;
    for(Polyline& p: polylines)
        if(isClosed(p)) closed.emplace_back(std::move(p)).close();
        else result.open.emplace_back(std::move(p));

    result.region = evenOdd(closed);
    return result;
}

} // namespace Geo
