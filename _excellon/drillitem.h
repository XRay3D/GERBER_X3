/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "graphicsitem.h"

namespace Excellon {
class File;
class Hole;
}
namespace GCode {
class File;
}

class DrillItem : public GraphicsItem {
public:
    DrillItem(Excellon::Hole* hole, Excellon::File* file);
    DrillItem(double diameter, GCode::File* file);
    ~DrillItem() override;
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // GraphicsItem interface
    Paths paths() const override;
    void changeColor() override;

    bool isSlot();
    double diameter() const;
    void setDiameter(double diameter);
    void updateHole();

private:
    void create();
    double m_diameter = 0.0;
    Excellon::Hole* const m_hole = nullptr;
    QPolygonF fillPolygon;
};
