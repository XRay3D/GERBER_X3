#pragma once

#include "dxf_entity.h"

namespace Dxf {
struct Arc final : Entity {
    Arc(SectionParser* sp);

    // Entity interface
public:
    void draw(const InsertEntity* const i) const override;
    void parse(CodeData& code) override;
    Type type() const override { return Type::ARC; };
    GraphicObject toGo() const;

    enum VarType {
        SubclassMarker = 100, // Маркер подкласса (AcDbCircle)
        Thickness = 39, // Толщина (необязательно; значение по умолчанию = 0)
        CenterPointX = 10, // Центральная точка (в ОСК)
        // Файл DXF: значение X; приложение: 3D-точка
        CenterPointY = 20,
        CenterPointZ = 30, // Файл DXF: значения Y и Z для центральной точки (в ОСК)
        Radius = 40, // Радиус
        StartAngle = 50,
        EndAngle = 51,
        ExtrusionDirectionX = 210, // Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)       // Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionY = 220, // Файл DXF: значения Y и Z для направления выдавливания (необязательно)
        ExtrusionDirectionZ = 230,
    };


    QPointF centerPoint;
    double thickness = 0;
    double radius = 0;
    double startAngle = 0;
    double endAngle = 0;
};
}
