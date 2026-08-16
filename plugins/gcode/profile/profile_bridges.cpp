/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "profile_bridges.h"

#include "geo/boolean.h"
#include "geo/util.h"

#include <QDebug>
#include <limits>

namespace Profile {

//------------------------------------------------------------------------------
// Мостики (табы).

// Режет разомкнутый кусок в точке на расстоянии `length` вдоль дуги от начала.
// Логика вершины реза та же, что в trimFront: у остатка дуги свой прогиб.
// `length` вне (0, периметр) отдаёт весь кусок одной половиной.
std::pair<Geo::Polyline, Geo::Polyline> splitAt(const Geo::Polyline& path, double length) {
    std::pair<Geo::Polyline, Geo::Polyline> halves;
    auto& [head, tail] = halves;

    if(length <= 0.0) {
        tail = path;
        return halves;
    }

    double rest = length;
    std::size_t i{};
    for(; i + 1 < path.size(); ++i) {
        const double len = Geo::segmentLength(path[i], path[i + 1]);
        if(rest < len) break;
        rest -= len;
        head.push_back(path[i]);
    }
    if(i + 1 >= path.size()) { // длины не хватило -- всё в голову
        head = path;
        tail.clear();
        return halves;
    }

    const Geo::Vertex from = path[i];
    const Geo::Vertex to = path[i + 1];
    const double len = Geo::segmentLength(from, to);
    const double t = len > 0.0 ? rest / len : 0.0;

    // Вершина реза: голове достаётся без прогиба (она замыкающая), хвосту --
    // с прогибом остатка дуги.
    Geo::Vertex cut{from};
    if(auto arc = Geo::arcOf(from, to, from.bulge)) {
        static_cast<QPointF&>(cut) = arc->pointAt(t);
        cut.bulge = Geo::bulgeOf(arc->theta * (1.0 - t));
        head.emplace_back(static_cast<const QPointF&>(from), Geo::bulgeOf(arc->theta * t));
    } else {
        static_cast<QPointF&>(cut) = from + (to - from) * t;
        cut.bulge = 0.0;
        head.push_back(from);
    }
    head.emplace_back(static_cast<const QPointF&>(cut));
    tail.push_back(cut);
    tail.insert(tail.end(), path.begin() + i + 1, path.end());
    return halves;
}

// Горб над мостом: подъём / полка / спуск. Короткий мост (периметр не длиннее
// двух скосов) -- треугольник с вершиной посередине, полка пустая; длинный --
// трапеция со скосами по rampLen с каждого конца.
Hump splitBridge(const Geo::Polyline& piece, double rampLen) {
    Hump hump;
    const double perimeter = piece.perimeter();
    if(perimeter <= rampLen * 2.0) { // треугольник
        std::tie(hump.up, hump.down) = splitAt(piece, perimeter * 0.5);
    } else { // трапеция
        std::tie(hump.up, hump.flat) = splitAt(piece, rampLen);
        std::tie(hump.flat, hump.down) = splitAt(hump.flat, perimeter - rampLen * 2.0);
    }
    return hump;
}

// Куски пути по кругам мостов, сцепленные в порядке обхода. clipOpen отдаёт
// куски без порядка, а фрезеровать их надо подряд, поэтому цепь собирается по
// совпадению концов. Пустой не бывает: путь, не задетый мостами (или не
// собравшийся в цепь -- с предупреждением), возвращается одним небриджевым
// куском; путь целиком под мостом -- одним бриджевым.
std::vector<Piece> chainPieces(const Geo::Polyline& path, const Geo::Polygons& region) {
    auto whole = [&path](bool bridge) {
        std::vector<Piece> chain;
        chain.push_back({path, bridge});
        return chain;
    };

    Geo::Polylines inside = Geo::clipOpen(Geo::ClipType_::Intersection, {path}, region);
    if(inside.empty()) return whole(false);
    Geo::Polylines outside = Geo::clipOpen(Geo::ClipType_::Difference, {path}, region);
    if(outside.empty()) return whole(true); // целиком под мостом

    std::vector<Piece> pieces;
    pieces.reserve(inside.size() + outside.size());
    for(Geo::Polyline& p: outside) pieces.push_back({std::move(p), false});
    for(Geo::Polyline& p: inside) pieces.push_back({std::move(p), true});

    auto nearest = [&pieces](QPointF to, bool skipBridges) {
        std::size_t found{};
        double best = std::numeric_limits<double>::max();
        for(std::size_t i{}; i < pieces.size(); ++i) {
            if(skipBridges && pieces[i].bridge) continue;
            if(const double dist = Geo::distance(pieces[i].path.front(), to); dist < best)
                best = dist, found = i;
        }
        return std::pair{found, best};
    };
    auto take = [&pieces](std::size_t i) {
        Piece piece = std::move(pieces[i]);
        pieces.erase(pieces.begin() + ptrdiff_t(i));
        return piece;
    };

    std::vector<Piece> chain;
    chain.reserve(pieces.size());
    // Старт: у замкнутого пути -- небриджевый кусок (врезаться на мосту
    // незачем), ближайший к прежнему началу; у разомкнутого выбора нет --
    // кусок, с которого путь начинается.
    chain.push_back(take(nearest(path.front(), path.closed).first));
    while(!pieces.empty()) {
        // clipOpen выбрасывает обрывки короче допуска сварки, так что стык
        // соседних кусков может отстоять на такой огрызок. Дальше десятка
        // допусков -- это уже не стык, цепь порвана.
        auto [next, dist] = nearest(chain.back().path.back(), false);
        if(dist > Geo::exitWeldTolerance * 10) break;
        chain.push_back(take(next));
    }
    if(!pieces.empty()) { // не собралось -- фрезеруем без мостов, но не молча
        qWarning() << "bridges: pieces did not chain, path milled without tabs";
        return whole(false);
    }
    return chain;
}

} // namespace Profile
