/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_entity.h"
#include "dxf_vertex.h"
#include <QPolygonF>

namespace Dxf {

struct PolyLine final : Entity {
    PolyLine(SectionParser* sp);

    // Entity interface
public:
    // void draw(const InsertEntity* const i = nullptr) const override;

    void parse(CodeData& code) override;
    Type type() const override;

    enum DataEnum {
        SubclassMarker = 100,    // Маркер подкласса (AcDb2dPolyline или AcDb3dPolyline)
        EntitiesFollowFlag = 66, // Устарело; ранее — флаг наследования объекта (необязательно; игнорировать, если имеется)
        DummyX = 10,             // Файл DXF: всегда 0        // Приложение: "фиктивная" точка; значения X и Y всегда равны нулю, а значение Z является отметкой полилинии (ОСК в 2D, МСК в 3D)
        DummyY = 20,             // Файл DXF: всегда 0
        DummyZ = 30,             // Файл DXF: отметка полилинии (ОСК при 2D; МСК при 3D)
        Thickness = 39,          // Толщина (необязательно; значение по умолчанию = 0)
        PolylineFlag = 70,       // Флаг полилинии (кодовые биты; значение по умолчанию = 0):
        //        1 = замкнутая полилиния (или полигональная сеть, замкнутая в направлении М)
        //        2 = добавлены сглаженные по кривой вершины
        //        4 = добавлены сглаженные по сплайну вершины
        //        8 = 3D-полилиния
        //        16 = полигональная 3D-сеть
        //        32 = полигональная сеть замкнута в направлении N
        //        64 = полилиния является многогранной сетью
        //        128 = образец типа линий непрерывно формируется по периметру вершин этой полилинии
        StartWidth = 40,            // Начальная ширина по умолчанию (необязательно; значение по умолчанию = 0)
        EndWidth = 41,              // Конечная ширина по умолчанию (необязательно; значение по умолчанию = 0)
        VertexCountM = 71,          // Количество вершин полигональной сети в направлении М (необязательно; значение по умолчанию = 0)
        VertexCountN = 72,          // Количество вершин полигональной сети в направлении N (необязательно; значение по умолчанию = 0)
        SmoothSurfaceDensitMy = 73, // Плотность сглаживания поверхности M (необязательно; значение по умолчанию = 0)
        SmoothSurfaceDensityN = 74, // Плотность сглаживания поверхности в направлении N (необязательно; значение по умолчанию = 0)
        SurfaceType = 75,           // Кривые и тип сглаживания поверхности (необязательно; значение по умолчанию = 0); целые коды, не кодовые биты:
        //        0 = сглаженная поверхность не вписана
        //        5 = квадратичная В-сплайновая поверхность
        //        6 = кубическая В-сплайновая поверхность
        //        8 = поверхность Безье
        ExtrusionDirectionX = 210, // Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)        //Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionY = 220, // Файл DXF: значения Y и Z для направления выдавливания (необязательно)
        ExtrusionDirectionZ = 230,
    };

    enum PolylineFlags {
        //        1 = This is a  (or a polygon mesh closed in the M direction)
        //        2 = Curve-fit vertices have been added
        //        4 = Spline-fit vertices have been added
        //        8 = This is a 3D polyline
        //        16 = This is a 3D polygon mesh
        //        32 = The polygon mesh is closed in the N direction
        //        64 = The polyline is a polyface mesh
        //        128 = The linetype pattern is generated continuously around the vertices of this polyline
        ClosedPolyline = 1,    //        1 = замкнутая полилиния (или полигональная сеть, замкнутая в направлении М)
        CurveFitVertices = 2,  //        2 = добавлены сглаженные по кривой вершины
        SplineFitVertices = 4, //        4 = добавлены сглаженные по сплайну вершины
        //        8 = 3D-полилиния
        //        16 = полигональная 3D-сеть
        //        32 = полигональная сеть замкнута в направлении N
        //        64 = полилиния является многогранной сетью
        //        128 = образец типа линий непрерывно формируется по периметру вершин этой полилинии
    };

    QVector<struct Vertex> vertex;

    GraphicObject toGo() const override;
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    int polylineFlags = 0;
    double startWidth = 0.0;
    double endWidth = 0.0;
};

} // namespace Dxf
