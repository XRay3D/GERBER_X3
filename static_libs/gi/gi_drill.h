/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"

class GiDrill final : public GraphicsItem {
    using GraphicsItem::update;

public:
    GiDrill(const Path& path, double diameter, FileInterface* file, int toolId);
    ~GiDrill() override { }

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return int(GiType::Drill); }

    // GraphicsItem interface
    Paths paths(int alternate = {}) const override;
    void changeColor() override;

    bool isSlot();
    double diameter() const { return diameter_; }
    void setDiameter(double diameter);
    void update(const Path& path, double diameter);

    int toolId() const { return toolId_; }
    void setToolId(int newToolId) {
        toolId_ = newToolId;
        setToolTip(QObject::tr("Tool %1, Ø%2mm").arg(toolId_).arg(diameter_));
    }

private:
    void create();
    double diameter_ = 0.0;
    Path path_;
    QPolygonF fillPolygon;
    int toolId_ = -1;
};
