/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_entity.h"
#include <QPolygonF>

namespace Dxf {
struct LwPolyline final : Entity {
    LwPolyline(SectionParser* sp);
    // 5
    // 8
    // 62
    enum DataEnum {
        SubclassMrker = 100,   // Маркер подкласса (AcDbPolyline)
        NumberOfVertices = 90, // Число вершин
        PolylineFlag = 70,     // Флаг полилинии (битовый код); значение по умолчанию = 0:
        // 1 = замкнутая; 128 = PLINEGEN
        ConstantWidth = 43,      // Постоянная ширина (необязательно; значение по умолчанию = 0). Не используется, если задана переменная ширина (коды 40 и/или 41)
        Elevation = 38,          // Отметка (необязательно; значение по умолчанию = 0)
        Thickness = 39,          // Толщина (необязательно; значение по умолчанию = 0)
        VertexCoordinatesX = 10, // Координаты вершин (в ОСК), несколько записей; одна запись для каждой вершины
        // Файл DXF: значение X; приложение: 2D-точка
        VertexCoordinatesY = 20, // Файл DXF: значение Y координат вершин (в ОСК), несколько записей; одна запись для каждой вершины
        VertexID = 91,           // Идентификатор вершины
        StartWidth = 40,         // Начальная ширина (несколько записей; одна запись для каждой вершины) (необязательно; значение по умолчанию = 0; несколько записей). Не используется, если задана постоянная ширина (код 43)
        EndWidth = 41,           // Конечная ширина (несколько записей; одна запись для каждой вершины) (необязательно; значение по умолчанию = 0; несколько записей). Не используется, если задана постоянная ширина (код 43)
        Bulge = 42,              // Прогиб (несколько записей; одна запись для каждой вершины) (необязательно; значение по умолчанию = 0)

        ExtrusionDirectionX = 210, // Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        ExtrusionDirectionY = 220, // Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionZ = 230, // Файл DXF: значения Y и Z для направления выдавливания (необязательно)
    };

    enum PFlag {
        Closed = 1,    // замкнутая
        Plinegen = 128 // PLINEGEN
    };
    // Entity interface
public:
    // void draw(const InsertEntity* const i = nullptr) const override;

    void parse(CodeData& code) override;
    Type type() const override;
    struct Segment : QPointF {
        double bulge = 0.0;
        friend QDataStream& operator<<(QDataStream& stream, const Segment& e) {
            stream << static_cast<QPointF>(e);
            stream << e.bulge;
            return stream;
        }
        friend QDataStream& operator>>(QDataStream& stream, Segment& e) {
            stream >> static_cast<QPointF&>(e);
            stream >> e.bulge;
            return stream;
        }
    };

    GraphicObject toGo() const override;
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    mvector<Segment> poly;
    int16_t counter = 0;
    int16_t polylineFlag = 0;
    int32_t numberOfVertices = 0;
    double startWidth = 0.0;
    double endWidth = 0.0;
    double constantWidth = 0.0;
    int16_t elevation = 0;
    double thickness = 0.0;
};

} // namespace Dxf
