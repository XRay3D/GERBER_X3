/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gi.h"
#include "shape.h"
#include "shapepluginin.h"
#include <QJsonObject>

namespace Shapes {
class Arc final : public Shape {
public:
    explicit Arc(QPointF center = {}, QPointF pt1 = {}, QPointF pt2 = {});
    ~Arc() override = default;

    // QGraphicsItem interface
    int type() const override { return GiType::ShCirArc; }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    bool addPt(const QPointF& pt) override;
    void setPt(const QPointF& pt) override;
    double radius() const;
    void setRadius(double radius);

    enum {
        Center,
        Point1,
        Point2,
        PtCount
    };

private:
    mutable double radius_ {};
    int ptCtr {};
};

class PluginImpl : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "circlearc.json")
    Q_INTERFACES(Shapes::Plugin)

public:
    // Shapes::Plugin interface
    int type() const override;
    QIcon icon() const override;
    Shape* createShape(const QPointF& point) const override;
};

} // namespace Shapes
