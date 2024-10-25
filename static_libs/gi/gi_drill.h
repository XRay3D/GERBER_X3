/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"

namespace Gi {

class Drill final : public Item {
    using Item::update;

public:
    Drill(const Path& path, double diameter, AbstractFile* file, int toolId);
    ~Drill() override { }

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return int(Type::Drill); }

    // Item interface
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

} // namespace Gi
