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
#include "gi.h"
#include "shape.h"
#include "shapepluginin.h"
#include <QJsonObject>

namespace Shapes {

class Rectangle final : public AbstractShape {
public:
    explicit Rectangle(QPointF pt1 = {}, QPointF pt2 = {});
    ~Rectangle() override = default;

    // QGraphicsItem interface
    int type() const override { return GiType::ShRectangle; }
    // GraphicsItem interface
    void redraw() override;
    // AbstractShape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt) override;
    enum {
        Center,
        Point1,
        Point2,
        Point3,
        Point4,
        PtCount
    };
};

class PluginImpl : public Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "rectangle.json")
    Q_INTERFACES(Shapes::Plugin)

public:
    // Shapes::Plugin interface
    int type() const override;
    QIcon icon() const override;
    AbstractShape* createShape(const QPointF& point) const override;
};

} // namespace Shapes
