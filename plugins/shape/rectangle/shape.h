/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "editor.h"
#include "gi.h"
#include "shape.h"
#include "shapepluginin.h"

namespace ShRect {

class Shape final : public Shapes::AbstractShape {
    friend class Model;

public:
    explicit Shape(QPointF pt1 = {}, QPointF pt2 = {});
    ~Shape() override;

    // QGraphicsItem interface
    int type() const override { return GiType::ShRectangle; }
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    // GraphicsItem interface
    void redraw() override;

    // AbstractShape interface
    QString name() const override;
    QIcon icon() const override;
    void setPt(const QPointF& pt) override;

    enum PointEnum {
        Center,
        Point1,
        Point2,
        Point3,
        Point4,
        PtCount,
        Width = PtCount, // model
        Height,          // model
    };
    Model* model{};
};

class Plugin final : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "rectangle.json")
    Q_INTERFACES(Shapes::Plugin)

    mutable Editor editor_{this};

public:
    // Shapes::Plugin interface *
    uint32_t type() const override { return GiType::ShRectangle; }
    QIcon icon() const override { return QIcon::fromTheme("draw-rectangle"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) const override {
        auto shape = new Shape(point, point + QPointF{10, 10});
        editor_.addShape(shape);
        return shape;
    }
    QWidget* editor() override { return &editor_; };
};

} // namespace ShRect
