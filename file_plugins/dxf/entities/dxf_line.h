/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "dxf_entity.h"

namespace Dxf {

struct Line final : Entity {
    Line(SectionParser* sp);

    // Entity interface
public:
    // void draw(const InsertEntity* const i = nullptr) const override;

    void parse(CodeData& code) override;
    Type type() const override { return Type::LINE; }

    enum DataEnum {
        SubclassMarker = 100, // Маркер подкласса (AcDbLine)
        Thickness = 39, // Толщина (необязательно; значение по умолчанию = 0)
        StartPointX = 10, // Начальная точка (в МСК)  //Файл DXF: значение X; приложение: 3D-точка
        StartPointY = 20, // Файл DXF: значения Y и Z для начальной точки (в МСК)
        StartPointZ = 30,
        EndPointX = 11, // Конечная точка (в МСК)//Файл DXF: значение X; приложение: 3D-точка
        EndPointY = 21, // Файл DXF: значения Y и Z конечной точки (в МСК)
        EndPointZ = 31,
        ExtrusionDirectionX = 210, // Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)//Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionY = 220, //  Файл DXF: значения Y и Z для направления выдавливания (необязательно)
        ExtrusionDirectionZ = 230,
    };

    GraphicObject toGo() const override;

    QPointF startPoint;
    QPointF endPoint;
    double thickness = 0;
};

}
