/*******************************************************************************
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
namespace Dxf {

struct Point final : Entity {
    Point(SectionParser* sp);

    // Entity interface
public:
    //    void draw(const InsertEntity* const i = nullptr) const override;
    void parse(CodeData& code) override;
    Type type() const override;
    GraphicObject toGo() const override;
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    enum DataEnum {
        SubclassMarker = 100, // Маркер подкласса (AcDbPoint)
        Thickness = 39, // Толщина (необязательно; значение по умолчанию = 0)
        PointX = 10, // Центральная точка (в ОСК)
        PointY = 20, // Файл DXF: значение X; приложение: 3D-точка
        PointZ = 30, // Файл DXF: значения Y и Z для центральной точки (в ОСК)
        ExtrusionDirectionX = 210, // Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)       // Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionY = 220, // Файл DXF: значения Y и Z для направления выдавливания (необязательно)
        ExtrusionDirectionZ = 230,
        AngleOfTheXZxisForTheUCS = 50 //Угол оси X для ПСК, используемый при построении точки (необязательно, по умолчанию = 0); используется, если параметр PDMODE не равен нулю
    };

    QPointF point;
    double thickness = 0;
};

}
