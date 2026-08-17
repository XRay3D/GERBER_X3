/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gc_file.h"
#include "gc_types.h"
#include "geo/util.h"

#include <cmath>
#include <functional>
#include <unordered_map>

namespace Voronoi {

constexpr auto VORONOI = "Voronoi"_hash32;

inline const QString FrameOffset = u"FrameOffset"_s;
inline const QString Tolerance = u"Tolerance"_s;
inline const QString VoronoiType = u"VoronoiType"_s;
inline const QString Width = u"Width"_s;

// Стрела прогиба, с которой дуги меди крошатся в хорды для диаграммы. Своя,
// а не «Precision» формы: та -- шаг РАССТАНОВКИ ТОЧЕК вдоль контура у
// jc_voronoi (и в UI видна только для него), и в её масштабе (0.1 мм)
// круглый пад 0.3 мм превращался бы в треугольник. Хорда лежит внутри дуги,
// так что бисектриса уходит к паду не дальше половины стрелы -- на 5 мкм это
// заведомо мельче любого допуска фрезеровки.
inline constexpr double arcSagitta = 5e-3;

// На сколько хорд крошить дугу, чтобы стрела прогиба каждой не превысила
// sagitta: sagitta = R*(1-cos(шаг/2)). И Boost.Polygon (voronoi_builder), и
// jc_voronoi понимают вход только как отрезки/точки -- прежний Clipper2-слой
// дуг не хранил вовсе, так что дискретизация была сделана ещё до него, а тут
// её делают сами бэкенды, прямо при подаче сегментов в диаграмму.
inline int arcChordSteps(const Geo::Arc& arc, double sagitta = arcSagitta) {
    const double tol = std::max(sagitta, 1e-6);
    const double maxStepAngle = tol < 2.0 * arc.radius
        ? 2.0 * std::acos(1.0 - tol / arc.radius)
        : 2.0 * pi;
    return std::max(1, static_cast<int>(std::ceil(std::abs(arc.theta) / std::max(maxStepAngle, 1e-9))));
}

// Склеивает сырые (обычно двухточечные) рёбра диаграммы Вороного в более
// длинные полилинии. НЕ Geo::stitch/GCode::mergePolylines -- та склейка
// квадратична (а на деле кубична: полное пересканирование после КАЖДОЙ
// склейки) по числу обрывков и рассчитана на десятки маршрутных фрагментов,
// а тут их -- сырых рёбер диаграммы -- многие тысячи; тот же алгоритм на
// реальной плате уводил счёт на минуты.
//
// Ключ -- точное равенство координат, без допуска: соседние рёбра одной
// диаграммы делят один и тот же вычисленный вертекс (общий указатель что у
// Boost.Polygon, что у jc_voronoi), а не пересчитывают его заново, так что
// общая точка стыка побитово одинакова у обоих рёбер.
inline Geo::Polylines chainDiagramEdges(Geo::Polylines segments) {
    struct Key {
        double x, y;
        bool operator==(const Key&) const = default;
    };
    struct KeyHash {
        size_t operator()(const Key& k) const noexcept {
            const size_t h1 = std::hash<double>{}(k.x);
            const size_t h2 = std::hash<double>{}(k.y);
            return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };
    const auto keyOf = [](QPointF p) { return Key{p.x(), p.y()}; };

    std::unordered_multimap<Key, std::size_t, KeyHash> byPoint;
    byPoint.reserve(segments.size() * 2);
    for(std::size_t i{}; i < segments.size(); ++i) {
        if(segments[i].empty()) continue;
        byPoint.emplace(keyOf(segments[i].front()), i);
        byPoint.emplace(keyOf(segments[i].back()), i);
    }

    std::vector<bool> used(segments.size());
    const auto popNeighbor = [&](QPointF pt) -> std::size_t {
        auto [b, e] = byPoint.equal_range(keyOf(pt));
        for(auto it = b; it != e; ++it)
            if(!used[it->second])
                return it->second;
        return segments.size();
    };

    Geo::Polylines result;
    result.reserve(segments.size());
    for(std::size_t i{}; i < segments.size(); ++i) {
        if(used[i] || segments[i].empty()) continue;
        used[i] = true;
        Geo::Polyline chain = std::move(segments[i]);

        for(std::size_t next; (next = popNeighbor(chain.back())) != segments.size();) {
            used[next] = true;
            Geo::Polyline piece = std::move(segments[next]);
            if(Geo::distance(piece.front(), chain.back()) > Geo::distance(piece.back(), chain.back()))
                piece.reverse();
            chain.insert(chain.end(), piece.begin() + 1, piece.end());
        }
        for(std::size_t prev; (prev = popNeighbor(chain.front())) != segments.size();) {
            used[prev] = true;
            Geo::Polyline piece = std::move(segments[prev]);
            if(Geo::distance(piece.back(), chain.front()) > Geo::distance(piece.front(), chain.front()))
                piece.reverse();
            chain.insert(chain.begin(), piece.begin(), piece.end() - 1);
        }
        result.push_back(std::move(chain));
    }
    return result;
}

class [[= Serial::name("Voronoi")]] File final : public GCode::File {

public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    explicit File();
    explicit File(GCode::Params&& gcp);
    QIcon icon() const override { return QIcon::fromTheme(u"voronoi-path"_s); }
    uint32_t type() const override { return VORONOI; }
    void createGi() override;
};

} // namespace Voronoi
