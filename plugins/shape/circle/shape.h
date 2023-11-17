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

namespace ShCirc {

class Shape final : public Shapes::AbstractShape {
    friend class Model;

public:
    explicit Shape(QPointF center = {}, QPointF pt = {});
    ~Shape() override;

    // QGraphicsItem interface
    int type() const override { return Gi::Type::ShCircle; }
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    // Gi::Item interface
    void redraw() override;

    // AbstractShape interface
    QIcon icon() const override;
    QString name() const override;
    void setPt(const QPointF& pt) override;

    double radius() const;
    void setRadius(double radius);

    enum PointEnum {
        Center,
        Point1,
        PtCount,
        Radius = PtCount, // model
        Diameter,         // model
    };
    Model* model{};

private:
    double radius_;
};

class Plugin final : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "circle.json")
    Q_INTERFACES(Shapes::Plugin)

    mutable Editor editor_{this};

public:
    // Shapes::Plugin interface
    uint32_t type() const override { return Gi::Type::ShCircle; }
    QIcon icon() const override { return QIcon::fromTheme("draw-ellipse"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) const override {
        auto shape = new Shape{point, point + QPointF{5, 0}};
        editor_.addShape(shape);
        return shape;
    }
    QWidget* editor() override { return &editor_; }
};

} // namespace ShCirc
