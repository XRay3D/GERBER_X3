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
#pragma once

// Сборка 3D-геометрии DXF (многогранные/полигональные сети POLYLINE, 3DFACE)
// и построение по ней ортогональных проекций-силуэтов.
// Плоские сущности сюда не попадают: они, как и раньше, рисуются в свой слой.

#include <QVector3D>
#include <array>
#include <vector>

namespace Dxf {

struct Face3D;
struct PolyLine;

// Сторона, с которой смотрим на модель.
enum class View {
    Top,    // сверху,  +Z → плоскость XY
    Bottom, // снизу,   -Z
    Front,  // спереди, -Y → плоскость XZ
    Back,   // сзади,   +Y
    Right,  // справа,  +X → плоскость YZ
    Left,   // слева,   -X
    Count
};

// Битовая маска включённых видов (для настроек плагина).
constexpr uint8_t viewBit(View v) { return uint8_t(1u << unsigned(v)); }
constexpr uint8_t AllViews = viewBit(View::Top) | viewBit(View::Bottom)
    | viewBit(View::Front) | viewBit(View::Back)
    | viewBit(View::Right) | viewBit(View::Left);

// Имя синтетического слоя для вида; общий префикс держит слои рядом
// в отсортированном по имени Layers (std::map<QString, Layer*>).
QString viewName(View v);
QColor viewColor(View v);

struct Model3D {
    std::vector<QVector3D> verts;
    // Индексы в verts. Четвёртый < 0 — грань треугольная.
    std::vector<std::array<int, 4>> faces;

    // POLYLINE с флагом 64 (многогранная сеть) или 16 (полигональная сеть).
    void addPolyLine(const PolyLine& pl);
    // 3DFACE с ненулевой Z.
    void addFace3D(const Face3D& face);

    bool empty() const { return faces.empty(); }
    void clear() { verts.clear(), faces.clear(); }

    // Проекция всех граней на плоскость вида и объединение их в силуэт.
    // Возвращает контуры с дырками (пустой результат — если проецировать нечего).
    Paths64 project(View v) const;
};

} // namespace Dxf
