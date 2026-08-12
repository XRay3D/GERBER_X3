/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

// Работа с ОТКРЫТЫМИ путями: склейка обрывков, порядок обхода, вложенность
// концентрических петель.
//
// Точной геометрии здесь нет и быть не должно. Всё замкнутое считается в CGAL
// (Geo::Polygons), а тут -- маршрут инструмента: в каком порядке проходить уже
// посчитанные контуры и откуда начинать каждый. Это задача о переездах, а не о
// множествах точек, и точная арифметика ей ничего не даёт.
//
// Координаты -- обычные миллиметры. Целочисленного uScale, в котором жил
// прежний слой на Clipper2, здесь нет вовсе, и все допуски заданы в мм.

#include "geo/polyline.h"

#include <QPointF>
#include <cstddef>
#include <vector>

namespace GCode {

// Габаритная рамка вокруг контуров, раздутая на `margin`. Заменяет прежний
// boundOfPaths: рамка нужна там, где вырезы надо получить как «всё, что вне
// тела» -- вычитанием региона из неё.
Geo::Polyline boundingFrame(const Geo::Polylines& polylines, double margin);
Geo::Polyline boundingFrame(QRectF rect, double margin);

// Дерево вложенности набора НЕПЕРЕСЕКАЮЩИХСЯ замкнутых контуров -- ровно то,
// ради чего прежде строился PolyTree. Концентрические петли офсета друг друга
// не режут, поэтому достаточно проверки «точка контура внутри другого контура».
struct NestingForest {
    std::vector<std::vector<std::size_t>> children; // дети каждого контура
    std::vector<std::size_t> roots;                 // контуры, никем не объемлемые
    std::vector<int> depth;                         // глубина вложенности, 0 у корней
};
NestingForest nestingForest(const Geo::Polylines& contours);

// Склеивает контуры, у которых концы совпадают или отстоят не дальше maxDist.
// Открытые обрывки, пришедшие из файла, обычно и есть один разрезанный контур.
//
// Сама склейка живёт в Geo (Geo::stitch): она нужна и там, где регион
// собирается по even-odd (Geo::normalize), а не только маршруту инструмента.
void mergePolylines(Geo::Polylines& polylines, double maxDist);

// Жадный обход «от ближайшего к ближайшему»: каждый следующий путь тот, чьё
// начало ближе к концу предыдущего. Не оптимум коммивояжёра, но именно так
// маршрут строился и раньше.
// Порядок обхода ТОЧЕК одной полилинии -- центров отверстий у сверловки.
// Ближайший сосед от start, как и у вариантов ниже.
void sortByProximity(Geo::Polyline& points, QPointF start);
void sortByProximity(Geo::Polylines& polylines, QPointF start);
void sortByProximity(std::vector<Geo::Polylines>& groups, QPointF start);

// Циклический сдвиг замкнутого контура так, чтобы обход начинался с вершины,
// ближайшей к `point`. Врезаться в деталь дешевле там, где инструмент уже
// оказался.
void rotateToNearest(Geo::Polyline& polyline, QPointF point);

} // namespace GCode
