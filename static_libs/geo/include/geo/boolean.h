#pragma once

#include "geo/polygon.h"
#include "geo/polyline.h"

namespace Geo {

// Офсет геометрии на delta/2 -- НАРУЖУ при delta > 0 и ВНУТРЬ при
// delta < 0. Результат всегда полигоны с отверстиями: тело со всеми его
// дырками -- один Geo::Polygon, а не набор контуров, чью вложенность
// пришлось бы восстанавливать вызывающему.
//
// Делится пополам потому, что delta -- ПОЛНАЯ ширина: число совпадает с
// шириной линии в DXF, а контур уезжает на её половину.
//
// Обе стороны -- одна и та же полоса вдоль границы (Polygons::boundaryBand)
// шириной 2 * |delta| / 2, только раздувание её ПРИБАВЛЯЕТ, а сжатие
// ВЫЧИТАЕТ. Отверстия при этом ведут себя зеркально телу: раздувание их
// сжимает (дырка тоньше delta исчезает целиком), сжатие -- растит.
struct Inflate_ {
    // Плоский список контуров.
    //
    // Ориентация ВХОДНЫХ контуров значима, потому что в плоском списке ею
    // одной и выражается вложенность: контур с положительной площадью --
    // тело, с отрицательной -- пустота внутри чьего-то тела. Поэтому весь
    // слой стоит подавать одним вызовом: пустота, поданная отдельно от
    // своего тела, вычитать ей нечего, и результат будет пуст.
    //
    // При delta > 0 раздуваются и ОТКРЫТЫЕ контуры: площади у них нет, но
    // капсулы их рёбер дают привычный «штрих» шириной delta. При delta < 0
    // они не значат ничего -- сжимать у линии нечего.
    Polygons operator()(const Polylines& polylines, double delta) const;

    // То же наружу, но каждая полилиния раздувается на свою собственную
    // ширину (Geo::Polyline::width) -- обычный случай для слоя, пришедшего
    // из DXF. Ширина -- величина неотрицательная, поэтому сжатия здесь нет.
    Polygons operator()(const Polylines& polylines) const;

    // Готовый регион -- результат прежней операции. Цепочка (раздуть ->
    // объединить -> сжать) не выходит из точного домена, так что строить её
    // можно сколь угодно длинной; в частности, Inflate(Inflate(r, +d), -d)
    // -- это честное морфологическое замыкание, скругляющее внутренние
    // углы и затягивающее щели тоньше d.
    Polygons operator()(const Polygons& region, double delta) const;
} inline Inflate;

struct BooleanOp_ {
    enum FillRule {
        EvenOdd,
        NonZero,
        Positive,
        Negative
    };

    enum ClipType {
        NoClip,
        Intersection,
        Union,
        Difference,
        Xor
    };

    // Плоский список контуров: вложенность восстанавливается по ней самой
    // (см. Geo::Polygons), ориентация входа не важна.
    Polygons operator()(ClipType cliptype, FillRule fillrule,
        const Polylines& subjects, const Polylines& clips = {});

    // Полигоны с отверстиями -- то, что вернула предыдущая операция:
    // структура известна точно, восстанавливать нечего.
    Polygons operator()(ClipType cliptype, FillRule fillrule,
        const Polygons& subjects, const Polygons& clips = {});
} inline BooleanOp;

using FillRule_ = BooleanOp_::FillRule;
using ClipType_ = BooleanOp_::ClipType;

} // namespace Geo
