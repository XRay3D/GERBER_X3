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

class DrillPreview : public GraphicsItem {
public:
    DrillPreview(Excellon::Hole* hole, FileInterface* file);
    DrillPreview(double diameter, FileInterface* file);
    ~DrillPreview() override;
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
