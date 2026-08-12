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
#include "dxf_model3d.h"

#include "entities/dxf_face3d.h"
#include "entities/dxf_polyline.h"

#include <QCoreApplication>

namespace Dxf {

QString viewName(View v) {
    switch(v) {
    case View::Top   : return QCoreApplication::translate("Dxf", "3D Top (XY)");
    case View::Bottom: return QCoreApplication::translate("Dxf", "3D Bottom (XY)");
    case View::Front : return QCoreApplication::translate("Dxf", "3D Front (XZ)");
    case View::Back  : return QCoreApplication::translate("Dxf", "3D Back (XZ)");
    case View::Right : return QCoreApplication::translate("Dxf", "3D Right (YZ)");
    case View::Left  : return QCoreApplication::translate("Dxf", "3D Left (YZ)");
    default          : return {};
    }
}

QColor viewColor(View v) {
    switch(v) {
    case View::Top   : return {0x4C, 0xAF, 0x50};
    case View::Bottom: return {0x2E, 0x7D, 0x32};
    case View::Front : return {0x21, 0x96, 0xF3};
    case View::Back  : return {0x15, 0x65, 0xC0};
    case View::Right : return {0xFF, 0x98, 0x00};
    case View::Left  : return {0xE6, 0x51, 0x00};
    default          : return Qt::gray;
    }
}

void Model3D::addPolyLine(const PolyLine& pl) {
    // Записи координат и записи граней лежат в одном списке VERTEX вперемешку;
    // индексы граней (коды 71..74) — 1-based по списку координат этой полилинии.
    const int base = int(verts.size());

    for(const Vertex& v: pl.polyLine)
        if(!v.isFace()) verts.push_back(v.point3D());

    const int count = int(verts.size()) - base;
    if(!count) return;

    if(pl.polylineFlags & PolyLine::PolyfaceMesh) {
        for(const Vertex& v: pl.polyLine) {
            if(!v.isFace()) continue;
            std::array<int, 4> face{-1, -1, -1, -1};
            int n{};
            for(int i: v.faceIndex) {
                // Знак — признак скрытого ребра, на силуэт не влияет.
                if(int idx = std::abs(i) - 1; i && idx < count)
                    face[n++] = base + idx;
            }
            // Треугольник AutoCAD часто пишет как четырёхугольник с повтором.
            if(n == 4 && face[3] == face[2]) --n;
            if(n >= 3) {
                if(n == 3) face[3] = -1;
                faces.push_back(face);
            }
        }
    } else if(pl.polylineFlags & PolyLine::_3DPolygonMesh) {
        // Регулярная сетка M×N, вершины идут по строкам.
        const int m = pl.vertexCountM, n = pl.vertexCountN;
        if(m < 2 || n < 2 || m * n != count) {
            qWarning("Dxf: polygon mesh %dx%d != %d vertices, skipped", m, n, count);
            return;
        }
        const bool closedM = pl.polylineFlags & PolyLine::ClosedPolyline;
        const bool closedN = pl.polylineFlags & PolyLine::ClosedInN;
        auto at = [&](int i, int j) { return base + (i % m) * n + (j % n); };
        for(int i{}; i < (closedM ? m : m - 1); ++i)
            for(int j{}; j < (closedN ? n : n - 1); ++j)
                faces.push_back({at(i, j), at(i + 1, j), at(i + 1, j + 1), at(i, j + 1)});
    }
}

void Model3D::addFace3D(const Face3D& face) {
    const int base = int(verts.size());
    const QPointF corner[4]{face.firstCorner, face.secondCorner, face.thirdCorner, face.fourthCorner};
    for(int i{}; i < 4; ++i)
        verts.emplace_back(float(corner[i].x()), float(corner[i].y()), float(face.cornerZ[i]));
    // Вырожденный четвёртый угол (треугольная грань) — совпадает с третьим.
    const bool tri = verts[base + 3] == verts[base + 2];
    faces.push_back({base, base + 1, base + 2, tri ? -1 : base + 3});
}

Geo::Polygons Model3D::project(View v) const {
    if(faces.empty()) return {};

    // Проекция вершины на плоскость вида. Оси подобраны так, чтобы вид читался
    // «снаружи» детали, поэтому у обратных видов первая ось зеркалится.
    auto flat = [v](const QVector3D& p) -> QPointF {
        switch(v) {
        case View::Top   : return {p.x(), p.y()};
        case View::Bottom: return {-p.x(), p.y()};
        case View::Front : return {p.x(), p.z()};
        case View::Back  : return {-p.x(), p.z()};
        case View::Right : return {p.y(), p.z()};
        case View::Left  : return {-p.y(), p.z()};
        default          : return {};
        }
    };

    std::vector<Geo::Polygon> projected;
    projected.reserve(faces.size());

    for(const auto& face: faces) {
        Geo::Polyline path;
        path.reserve(4);
        for(int idx: face) {
            if(idx < 0) continue;
            QPointF pt = flat(verts[size_t(idx)]);
            if(path.empty() || QPointF{path.back()} != pt) path.emplace_back(pt);
        }
        if(path.size() < 3) continue; // грань видна с ребра
        path.close();                 // снимет и повтор первой точки в конце
        if(path.size() < 3) continue;
        // Внешняя граница полигона обходится против часовой стрелки; грань,
        // повёрнутая к нам изнанкой, приходит по часовой -- разворачиваем,
        // иначе она вычтется из соседних вместо объединения с ними.
        const double area = path.signedArea();
        if(qFuzzyIsNull(area)) continue;
        if(area < 0) path.reverse();
        // Вырожденные и самокасающиеся грани точный домен не берёт -- отсев
        // здесь, а не внутри, чтобы не терять остальные молча.
        if(!Geo::isExactContour(path)) continue;
        projected.emplace_back(path);
    }

    if(projected.empty()) return {};

    // Объединение деревом слияний и в несколько потоков: граней в сети бывают
    // десятки тысяч, и цепочка из стольких же точных операций не поднялась бы.
    return Geo::Polygons{std::span<const Geo::Polygon>{projected}};
}

} // namespace Dxf
