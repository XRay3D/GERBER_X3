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
#pragma once

// Мостики (табы) профиля: разбивка траектории по кругам мостов и горб над
// мостом. Геометрия -- здесь, в C++; порядок проходов пишет profile.js через
// Profile::BridgesApi (profile_jsapi.h).

#include "geo/polygon.h"
#include "geo/polyline.h"

#include <utility>
#include <vector>

namespace Profile {

// Кусок траектории между кругами мостов. bridge == true -- кусок ПОД мостом:
// на проходах ниже верха таба фреза идёт по нему горбом, не опускаясь до
// глубины прохода.
struct Piece {
    Geo::Polyline path;
    bool bridge{};
};

// Горб над мостом: подъём / полка / спуск. Короткий мост (периметр не длиннее
// двух скосов) -- треугольник с вершиной посередине, полка пустая; длинный --
// трапеция со скосами по rampLen с каждого конца.
struct Hump {
    Geo::Polyline up, flat, down;
};

// Режет разомкнутый кусок в точке на расстоянии `length` вдоль дуги от начала.
std::pair<Geo::Polyline, Geo::Polyline> splitAt(const Geo::Polyline& path, double length);

Hump splitBridge(const Geo::Polyline& piece, double rampLen);

// Куски пути по кругам мостов, сцепленные в порядке обхода. Пустым не бывает:
// путь, не задетый мостами, -- один небриджевый кусок; целиком под мостом --
// один бриджевый.
std::vector<Piece> chainPieces(const Geo::Polyline& path, const Geo::Polygons& region);

} // namespace Profile
