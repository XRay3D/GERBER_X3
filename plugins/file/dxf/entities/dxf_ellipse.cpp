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
#include "dxf_ellipse.h"

#include "geo/util.h"

#include "dxf_insert.h"
#include <QGraphicsEllipseItem>

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"

#include <QPainter>
#include <qmath.h>

namespace Dxf {

void Ellipse::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker             : break; // 100 Маркер подкласса (AcDbEllipse)

        case CenterPointX               : CenterPoint.setX(code); break; // 10 Центральная точка (в МСК)
        case CenterPointY               : CenterPoint.setY(code); break; // 20 // Файл DXF: значение X; приложение: 3D-точка
        case CenterPointZ               : break;                         // 30 // 20, 30 Файл DXF: значения Y и Z для центральной точки (в МСК)

        case EndpointOfMajorAxisX       : EndpointOfMajorAxis.setX(code); break; // 11 Конечная точка главной оси относительно центральной точки (в МСК)
        case EndpointOfMajorAxisY       : EndpointOfMajorAxis.setY(code); break; // 21 // Файл DXF: значение X; приложение: 3D-точка
        case EndpointOfMajorAxisZ       : break;                                 // 31 // 21, 31 Файл DXF: значения Y и Z для конечной точки главной оси относительно центральной точки (в МСК)

        case ExtrusionDirectionX        :        // 210
        case ExtrusionDirectionY        :        // 220
        case ExtrusionDirectionZ        : break; // 230 //

        case RatioOfMinorAxisToMajorAxis: ratioOfMinorAxisToMajorAxis = code; break; // 40 Соотношение малой и главной осей

        case StartParameter             : startParameter = code; break; // 41 Начальный параметр (значение для полного эллипса — 0,0)
        case EndParameter               : endParameter = code; break;   // 42 Конечный параметр (значение для полного эллипса — 2 пи)
        default                         : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Ellipse::type() const { return Entity::ELLIPSE; }

DxfGo Ellipse::toGo() const {
    const double majorLen = std::hypot(EndpointOfMajorAxis.x(), EndpointOfMajorAxis.y());
    if(qFuzzyIsNull(majorLen) || qFuzzyIsNull(ratioOfMinorAxisToMajorAxis))
        return {};

    const double minorLen = majorLen * ratioOfMinorAxisToMajorAxis;
    const double rotation = std::atan2(EndpointOfMajorAxis.y(), EndpointOfMajorAxis.x());
    const double cosR = std::cos(rotation);
    const double sinR = std::sin(rotation);

    auto pointAt = [&](double t) {
        const double lx = majorLen * std::cos(t);
        const double ly = minorLen * std::sin(t);
        return QPointF{
            CenterPoint.x() + lx * cosR - ly * sinR,
            CenterPoint.y() + lx * sinR + ly * cosR,
        };
    };

    double span = endParameter - startParameter;
    const bool closed = qFuzzyIsNull(span) || span >= 2.0 * pi - 1.0e-9;
    if(qFuzzyIsNull(span))
        span = 2.0 * pi;

    // Эллипс представляется цепочкой круговых дуг: на каждом малом участке подбирается
    // окружность, проходящая через начало, середину и конец участка (как и Circle/Arc,
    // Geo::Polyline хранит именно круговые дуги, а не плотную ломаную).
    const int segments = std::clamp(int(std::round(std::abs(span) / (2.0 * pi) * 36.0)), 4, 72);

    Geo::Polyline curve;
    QPointF prev = pointAt(startParameter);
    curve.emplace_back(prev);
    for(int i{}; i < segments; ++i) {
        const double t0 = startParameter + span * i / segments;
        const double t1 = startParameter + span * (i + 1) / segments;
        const double tm = (t0 + t1) / 2;
        const QPointF p0 = prev;
        const QPointF pm = pointAt(tm);
        const QPointF p1 = pointAt(t1);

        const double ax = p0.x(), ay = p0.y();
        const double bx = pm.x(), by = pm.y();
        const double cx = p1.x(), cy = p1.y();
        const double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));

        if(qFuzzyIsNull(d)) {
            curve.emplace_back(p1); // вырожденный (почти прямой) участок
        } else {
            const double a2 = ax * ax + ay * ay, b2 = bx * bx + by * by, c2 = cx * cx + cy * cy;
            const QPointF center{
                (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d,
                (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d,
            };
            // Направление обхода -- знак векторного произведения радиус-векторов:
            // участок здесь заведомо меньше полуокружности, так что знака довольно.
            const double cross = (p0.x() - center.x()) * (p1.y() - center.y())
                - (p0.y() - center.y()) * (p1.x() - center.x());
            // прогиб пишется на НАЧАЛЬНОЙ вершине дуги -- она уже в кривой
            curve.back().bulge = Geo::bulgeOf(p0, p1, center,
                cross > 0 ? Geo::Vertex::Ccw : Geo::Vertex::Cw);
            curve.emplace_back(p1);
        }
        prev = p1;
    }

    if(closed) {
        curve.close();
        DxfGo go{id, Geo::Polyline{curve}, Geo::Polygons{Geo::Polylines{std::move(curve)}}};
        go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::FlStamp | DxfGo::Elipse);
        go.GraphicObject::pos = CenterPoint;
        return go;
    }

    DxfGo go{id, std::move(curve)};
    go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::Elipse);
    return go;
}

} // namespace Dxf
