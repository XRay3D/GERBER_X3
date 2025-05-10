/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
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
    explicit Shape(Shapes::Plugin* plugin, QPointF center = {}, QPointF pt = {});
    ~Shape() override = default;

    // QGraphicsItem interface
    int type() const override { return Gi::Type::ShCircle; }

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

protected:
    void readAndInit(QDataStream& stream) override; // FIXME init()

private:
    double radius_;
};

class Plugin final : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "description.json")
    Q_INTERFACES(Shapes::Plugin)

    Editor editor_{this};

public:
    // Shapes::Plugin interface
    uint32_t type() const override { return Gi::Type::ShCircle; }
    QIcon icon() const override { return QIcon::fromTheme("draw-ellipse"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) override {
        auto shape = new Shape{
            this,
            point, point + QPointF{1, 0}
        };
        editor_.add(shape);
        return shape;
    }
    Editor* editor() override { return &editor_; }
};

} // namespace ShCirc
