/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_entity.h"

namespace Dxf {

struct Vertex final : Entity {
    Vertex(SectionParser* sp = nullptr);

    // Entity interface
public:
    //    void draw(const InsertEntity* const i = nullptr) const override;
    void parse(CodeData& code) override;
    Type type() const override { return Type::VERTEX; }
    DxfGo toGo() const override {
        qWarning("%s NOT IMPLEMENTED!", __FUNCTION__);
        return {};
    }
    // void write(QDataStream&) const override { }
    // void read(QDataStream&) override { }

    enum DataEnum {
        SubclassMarker = 100, // Маркер подкласса (AcDbVertex)
        // SubclassMarker2 = 100, // Маркер подкласса (AcDb2dVertex или AcDb3dPolylineVertex)

        PointX = 10, // Точка местоположения (ОСК в 2D-среде, МСК в 3D-среде)
        PointY = 20, // Файл DXF: значение X; приложение: 3D-точка
        PointZ = 30, // Файл DXF: значения Y и Z для точки местоположения (ОСК в 2D-среде, МСК в 3D-среде)

        StartingWidth = 40, // Начальная ширина (необязательно; значение по умолчанию — 0)
        EndingWidth = 41, // Конечная ширина (необязательно; значение по умолчанию — 0)

        Bulge = 42, // Прогиб (необязательно; значение по умолчанию — 0). Прогиб — это касательная одной четвертой центрального угла к дуговому сегменту. Если дуга идет в направлении по часовой стрелке от начальной точки к конечной, то значение касательной будет отрицательным. Если значение прогиба равно 0, то сегмент прямой, а если 1, то полукруглый.

        VertexFlags = 70, // Флаги вершин:
        // 1 = дополнительная вершина, созданная при дуговом сглаживании
        // 2 = для этой вершины задается касательная с дуговым сглаживанием. Касательная с дуговым сглаживанием с нулевым направлением может быть опущена при выводе в формате DXF, однако имеет большое значение, если данный бит задается
        // 4 = не используется
        // 8 = вершина сплайна, созданная при сглаживании сплайна
        // 16 = управляющая точка рамки сплайна
        // 32 = вершина 3D-полилинии
        // 64 = полигональная 3D-сеть
        // 128 = вершина многогранной сети

        CurveFitTangentDirection = 50, // Направление касательной с дуговым сглаживанием
        PolyfaceMeshVertexIndex1 = 71, // Индекс вершины многогранной сети (необязательно; присутствует, только если значение не равно нулю)
        PolyfaceMeshVertexIndex2 = 72, // Индекс вершины многогранной сети (необязательно; присутствует, только если значение не равно нулю)
        PolyfaceMeshVertexIndex3 = 73, // Индекс вершины многогранной сети (необязательно; присутствует, только если значение не равно нулю)
        PolyfaceMeshVertexIndex4 = 74, // Индекс вершины многогранной сети (необязательно; присутствует, только если значение не равно нулю)
        VertexIdentifier = 91, // Идентификатор вершины
    };

    enum VertexFlags {
        ExtraVertexCreatedByCurveFitting = 1, // дополнительная вершина, созданная при дуговом сглаживании
        CurveFitTangentDefinedForThisVertex = 2, // для этой вершины задается касательная с дуговым сглаживанием. Касательная с дуговым сглаживанием с нулевым направлением может быть опущена при выводе в формате DXF, однако имеет большое значение, если данный бит задается
        NotUsed = 4, // не используется
        SplineVertexCreatedBySplineFitting = 8, // вершина сплайна, созданная при сглаживании сплайна
        SplineFrameControlPoint = 16, // управляющая точка рамки сплайна
        _3DPolylineVertex = 32, // вершина 3D-полилинии
        _3DPolygonMesh = 64, // полигональная 3D-сеть
        PolyfaceMeshVertex = 128, // вершина многогранной сети
    };
    operator QPointF() const { return {x, y}; };
    int vertexFlags = 0;
    double x = 0.0;
    double y = 0.0;
    double bulge = 0.0;
    double curveFitTangentDirection = 0.0;
};

} // namespace Dxf
