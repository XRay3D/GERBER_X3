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

namespace ShArc {

class Shape final : public Shapes::AbstractShape {
    friend class Model;

public:
    explicit Shape(QPointF center = {}, QPointF pt1 = {}, QPointF pt2 = {});
    ~Shape() override;

    // QGraphicsItem interface
    int type() const override { return Gi::Type::ShCirArc; }
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    // Gi::Item interface
    void redraw() override;

    // AbstractShape interface
    QString name() const override;
    QIcon icon() const override;

    bool addPt(const QPointF& pt) override;
    void setPt(const QPointF& pt) override;

    double radius() const;
    void setRadius(double radius);

    double angle(int i) const;
    void setAngle(int i, double radius);

    enum PointEnum {
        Point1,
        Point2,
        Center,
        PtCount,
        Radius = PtCount, // model
        Diameter,         // model
        Angle1,           // model
        Angle2,           // model
    };
    Model* model{};

protected:
    void init() override;

private:
    mutable double radius_{};
    int ptCtr{};
};

class Plugin : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "description.json")
    Q_INTERFACES(Shapes::Plugin)

    mutable Editor editor_{this};

public:
    // Shapes::Plugin interface
    uint32_t type() const override { return Gi::Type::ShCirArc; }
    QIcon icon() const override { return QIcon::fromTheme("draw-ellipse-arc"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) const override {
        auto shape = new Shape{
            point, point + QPointF{5, 0},
            point + QPointF{0, 5}
        };
        editor_.addShape(shape);
        return shape;
    }
    QWidget* editor() override { return &editor_; };
};

} // namespace ShArc
