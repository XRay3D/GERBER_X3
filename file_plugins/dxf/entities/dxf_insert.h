/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "dxf_block.h"
#include "dxf_entity.h"

namespace Dxf {

struct InsertEntity final : Entity {
    InsertEntity(Blocks& blocks, SectionParser* sp);

    // Entity interface
public:
    void draw(const InsertEntity* const i = nullptr) const override;
    void parse(CodeData& code) override;
    Type type() const override { return Type::INSERT; }
    GraphicObject toGo() const override { return { sp->file, this, {}, {} }; }

    void transform(GraphicObject& item, QPointF tr) const;

    enum DataEnum {
        SubclassMrker = 100, // Subclass marker (AcDbBlockReference)
        VariableAttributes = 66, /* Variable attributes-follow flag (optional; default = 0);
                         if the value of attributes-follow flag is 1,
                         a series of attribute entities is expected to follow the insert, terminated by a seqend entity*/
        BlockName = 2, // Block name
        InsPtX = 10, // Insertion point (in OCS) // DXF: X value; APP: 3D point
        InsPtY = 20, // DXF: Y value of insertion point (in OCS)
        InsPtZ = 30, // DXF: Z value of insertion point (in OCS)
        ScaleX = 41, // X scale factor (optional; default = 1)
        ScaleY = 42, // Y scale factor (optional; default = 1)
        ScaleZ = 43, // Z scale factor (optional; default = 1)
        RotationAngle = 50, // Rotation angle (optional; default = 0)+
        ColCount = 70, // Column count (optional; default = 1)
        RowCount = 71, // Row count (optional; default = 1)
        ColSpacing = 44, // Column spacing (optional; default = 0)
        RowSpacing = 45, // Row spacing (optional; default = 0)
        ExtrusionDirectionX = 210, // Extrusion direction (optional; default = 0, 0, 1) // DXF: X value; APP: 3D vector
        ExtrusionDirectionY = 220, // DXF: Y value of extrusion direction (optional)
        ExtrusionDirectionZ = 230, // DXF: Z value of extrusion direction (optional)
    };

    Blocks& blocks;
    QString blockName; // Block name
    QPointF insPos; // Точка вставки (в ОСК). Файл DXF: значение X; приложение: 3D-точка
    mutable QPointF basePoint;
    double scaleX = 1; // Масштабный коэффициент по оси X(необязательно; значение по умолчанию = 1)
    double scaleY = 1; // Масштабный коэффициент по оси Y (необязательно; значение по умолчанию = 1)
    double rotationAngle = 0;
    int colCount = 1; // Column count (optional; default = 1)
    int rowCount = 1; // Row count (optional; default = 1)
    double colSpacing = 0; // Column spacing (optional; default = 0)
    double rowSpacing = 0; // Row spacing (optional; default = 0)
};

}
