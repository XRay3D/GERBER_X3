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

// DIMENSION сама по себе не хранит готовую геометрию размера (линии, выносные линии,
// стрелки, текст) — она ссылается (группа 2) на анонимный блок (обычно "*D<n>"),
// который AutoCAD генерирует и обновляет автоматически и в котором эта геометрия уже
// лежит в виде обычных сущностей. Поэтому DIMENSION::draw() не рисует что-то через
// toGo(), а просто просит нарисовать содержимое этого блока.
struct Dimension final : Entity {
    using Entity::Entity;

    Type type() const override { return Type::DIMENSION; }
    void draw(const InsertEntity* const i = nullptr) const override;
    DxfGo toGo() const override { return {}; }
    void parse(CodeData& code) override;

    enum DataEnum {
        SubclassMarker = 100, // Маркер подкласса (AcDbDimension)
        BlockName = 2,        // Имя блока, содержащего геометрию размера (стрелки, выносные линии, текст)
    };

    QString blockName;
};
} // namespace Dxf
