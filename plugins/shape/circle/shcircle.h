/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gi.h"
#include "shape.h"
#include "shapepluginin.h"
#include <QJsonObject>

namespace Shapes {

class Circle final : public Shape {
public:
    explicit Circle(QPointF center = {}, QPointF pt = {});
    ~Circle() override = default;
    ;

    // QGraphicsItem interface
    int type() const override { return GiType::ShCircle; }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;
    void setPt(const QPointF& pt) override;
    double radius() const;
    void setRadius(double radius);
    enum {
        Center,
        Point1,
        PtCount
    };

private:
    double radius_;
};

class PluginImpl : public Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "circle.json")
    Q_INTERFACES(Shapes::Plugin)

public:
    // Shapes::Plugin interface
    int type() const override;
    QIcon icon() const override;
    Shape* createShape(const QPointF& point) const override;
};

} // namespace Shapes
