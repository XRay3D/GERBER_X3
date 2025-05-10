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

namespace ShPoly {

class Shape final : public Shapes::AbstractShape {
    friend class Model;

public:
    explicit Shape(Shapes::Plugin* plugin, QPointF pt1 = {}, QPointF pt2 = {});
    ~Shape() override = default;

    // QGraphicsItem interface
    int type() const override { return Gi::Type::ShPolyLine; }

    // Gi::Item interface
    void redraw() override;

    // AbstractShape interface
    QString name() const override;
    QIcon icon() const override;
    void setPt(const QPointF& pt) override;
    bool addPt(const QPointF& pt) override;

    bool closed() const;

protected:
    void readAndInit(QDataStream& stream [[maybe_unused]]) override { redraw(); } // FIXME init()

private:
    QPointF centroid();
    QPointF centroidFast(); //??????
};

class Plugin : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "description.json")
    Q_INTERFACES(Shapes::Plugin)
    Editor editor_{this};

public:
    // Shapes::Plugin interface
    uint32_t type() const override { return Gi::Type::ShPolyLine; }
    QIcon icon() const override { return QIcon::fromTheme("draw-line"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) override {
        auto shape = new Shape{
            this,
            point, point + QPointF{1, 1}
        };
        editor_.add(shape);
        return shape;
    }
    Editor* editor() override { return &editor_; }
};

} // namespace ShPoly
