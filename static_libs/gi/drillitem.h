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

#include "gi.h"

namespace Excellon {
class Hole;
}

class Hole;

class DrillItem : public GraphicsItem {
public:
    DrillItem(Excellon::Hole* hole, FileInterface* file);
    DrillItem(double diameter, FileInterface* file);
    ~DrillItem() override;
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // GraphicsItem interface
    Paths paths(int alternate = {}) const override;
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
