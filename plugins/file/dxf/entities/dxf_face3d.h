/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "dxf_entity.h"
namespace Dxf {

// Сущность 3DFACE (AcDbFace). Плоская грань импортируется как четырёхугольник в проекции XY;
// грань с ненулевой Z (is3D()) уходит в Dxf::Model3D и попадает в проекционные слои.
// Признак невидимости рёбер (код 70) на 2D-геометрию не влияет.
struct Face3D final : Entity {
    using Entity::Entity;

    // Entity interface

    void parse(CodeData& code) override;
    Type type() const override;
    DxfGo toGo() const override;

    enum DataEnum {
        SubclassMarker = 100, // Маркер подкласса (AcDbFace)

        FirstCornerX = 10, // Первая угловая точка
        FirstCornerY = 20,
        FirstCornerZ = 30,

        SecondCornerX = 11, // Вторая угловая точка
        SecondCornerY = 21,
        SecondCornerZ = 31,

        ThirdCornerX = 12, // Третья угловая точка
        ThirdCornerY = 22,
        ThirdCornerZ = 32,

        FourthCornerX = 13, // Четвертая угловая точка. Если задано только три точки, совпадает с третьей.
        FourthCornerY = 23,
        FourthCornerZ = 33,

        InvisibleEdgeFlags = 70, // Флаги невидимости рёбер (необязательно; значение по умолчанию = 0):
        // 1 = невидимо первое ребро
        // 2 = невидимо второе ребро
        // 4 = невидимо третье ребро
        // 8 = невидимо четвертое ребро
    };

    enum Corners {
        NoCorner = 0,
        FirstCorner = 1,
        SecondCorner = 2,
        ThirdCorner = 4,
        FourthCorner = 8,
    };

    QPointF firstCorner;
    QPointF secondCorner;
    QPointF thirdCorner;
    QPointF fourthCorner;

    // Z углов. В поток не пишется (см. write/read): нужна только при разборе
    // файла для сборки Model3D, в проект сохраняются уже готовые проекции.
    double cornerZ[4]{};

    bool is3D() const {
        return r::any_of(cornerZ, [](double z) { return !qFuzzyIsNull(z); });
    }

    int corners = NoCorner;
    int invisibleEdgeFlags{};
};
} // namespace Dxf
