#pragma once

// ВНУТРЕННИЙ заголовок библиотеки Geo: точные типы CGAL и перевод между
// ними и плоским bulge-видом (Geo::Polyline). Публичный API
// (geo/polygon.h) прячет всё это за PIMPL, поэтому сюда включаются только
// файлы самой библиотеки, работающие с точным представлением.

#include "geo/polyline.h"

#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>

#include <functional>
#include <optional>
#include <vector>

class QPainterPath;

namespace Geo::Cgal {
using K       = CGAL::Exact_predicates_exact_constructions_kernel;
using Traits  = CGAL::Gps_circle_segment_traits_2<K>;
using PolySet = CGAL::General_polygon_set_2<Traits>;
using GPoly   = Traits::Polygon_2;
using GPolyWH = Traits::Polygon_with_holes_2;
using Curve   = Traits::Curve_2;
using XCurve  = Traits::X_monotone_curve_2;

// Расстояние, ниже которого две точки считаются одной: вход, прошедший
// round-trip через double, несёт микрофрагменты в единицы ULP, и точный
// свип на них спотыкается.
inline constexpr double weldTolerance = 1e-9;

// Замкнутая Dxf-полилиния -> точный дуговой многоугольник, всегда против
// часовой стрелки. std::nullopt, когда контур вырожден либо невалиден
// (самокасание -- см. реализацию).
std::optional<GPoly> toGPoly(const Polyline& poly);

// Параллельный обход [0, count): body(i) вызывается ровно раз для каждого
// i, в неопределённом порядке и из разных потоков. Тело обязано писать
// только в СВОЮ ячейку общих данных -- синхронизации здесь нет никакой.
//
// Живёт рядом с точной геометрией потому, что затевалось ради неё: toGPoly
// каждого контура -- отдельный свип, и на тысячах контуров это заметный
// кусок времени.
void parallelFor(std::size_t count, const std::function<void(std::size_t)>& body);

// Точный контур -> Dxf-полилиния: каждая x-монотонная кривая даёт одну
// bulge-вершину, центр и радиус -- у точной опорной окружности.
Polyline toPolyline(const GPoly& pgn);

// Объединение множества контуров в регион -- МНОГОПОТОЧНОЕ: куски делятся
// между ядрами, а частичные регионы сливаются деревом. Результат тот же,
// что у region.join(begin, end), включая уже накопленное в region.
//
// Диапазон принимается по значению: куски раскладываются в
// пространственном порядке (см. реализацию) и разбираются потоками, так
// что копия всё равно понадобилась бы.
void joinAll(PolySet& region, std::vector<GPoly> parts);

// Точный контур -> путь Qt, НАПРЯМУЮ: отрезок становится lineTo, дуга --
// arcTo вокруг своей опорной окружности. Промежуточный bulge-вид не
// строится вовсе -- он для отрисовки и не нужен, а лишний перевод только
// копил бы погрешность.
void appendToPath(QPainterPath& path, const GPoly& pgn);

// Мерки -- прямо по точным кривым, без bulge-вида и без QPainterPath.
// Каждая кривая даёт свой вклад: отрезок -- хорду и шнурки, дуга -- длину
// дуги и сегмент круга над хордой. Промежуточная полилиния тут не только
// лишняя работа, но и лишняя погрешность: её вершины уже округлены.
double signedArea(const GPoly& pgn); // > 0 при обходе против часовой стрелки
double perimeter(const GPoly& pgn);
double area(const GPolyWH& pwh);      // тело минус дырки
double perimeter(const GPolyWH& pwh); // вся граница: внешняя плюс дырки

// Принадлежность точки -- счётом пересечений вертикального луча с точными
// кривыми (x-монотонными, отсюда не больше одного пересечения на кривую).
bool insideContour(const GPoly& pgn, QPointF point);
bool contains(const GPolyWH& pwh, QPointF point);

// Куски суммы Минковского ВСЕЙ границы региона (и внешней, и границ
// отверстий) с кругом радиуса d: тело каждой кривой -- прямоугольник у
// отрезка, кольцевой сектор у дуги -- плюс диск вокруг каждого стыка.
// Объединение кусков (joinAll) и есть полоса толщиной 2d вдоль границы.
//
// Здесь ЕДИНСТВЕННОЕ место, где офсет региона касается точного
// представления: сами капсулы строятся обычной арифметикой, зато регион, к
// которому их потом прибавляют или из которого вычитают, точным и
// остаётся.
std::vector<GPoly> boundaryCapsules(const PolySet& region, double d);

} // namespace Geo::Cgal
