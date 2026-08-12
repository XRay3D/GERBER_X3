/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
#include "tool.h"

namespace Gi {

class Drill final : public Item {
    using Item::update;

public:
    Drill(const QPolygonF& path, double diameter, AbstractFile* file, Tool::ID toolId);
    ~Drill() override { }

    // QGraphicsItem interface
    void paintGeometry(QPainter* painter, const RenderState& st) override;
    int type() const override { return int(Type::Drill); }

    // Item interface
    // Paths paths(int alternate = {}) const override;
    void updateColors() override;

    bool isSlot();
    double diameter() const { return diameter_; }
    void setDiameter(double diameter);
    void updatePath(const QPolygonF& path, double diameter);

    Tool::ID toolId() const { return toolId_; }
    void setToolId(Tool::ID newToolId);

private:
    void create();
    double diameter_{};
    QPolygonF path_;
    // QPolygonF fillPolygon;
    Tool::ID toolId_ = Tool::ID::Null;
};

} // namespace Gi
