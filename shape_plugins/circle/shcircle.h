/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "shape.h"
#include <QJsonObject>
#include <graphicsitem.h>
#include <interfaces/shapepluginin.h>

namespace Shapes {

class Circle final : public Shape {
public:
    explicit Circle(QPointF center, QPointF pt);
    explicit Circle() { }
    ~Circle();

    // QGraphicsItem interface
    int type() const override { return static_cast<int>(GiType::ShapeC); }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt);
    double radius() const;
    void setRadius(double radius);
    enum {
        Center,
        Point1,
    };

private:
    double m_radius;
};

class PluginCircle : public QObject, public ShapePluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "circle.json")
    Q_INTERFACES(ShapePluginInterface)

    Circle* circle = nullptr;

public:
    PluginCircle() { }
    virtual ~PluginCircle() override { }

    // ShapePluginInterface interface
public:
    QObject* getObject() override { return this; }
    int type() const override { return static_cast<int>(GiType::ShapeC); }
    void setupInterface(App* a) override { app.set(a); }
    QJsonObject info() const
    {
        return QJsonObject {
            { "Name", "Circle" },
            { "Version", "1.0" },
            { "VendorAuthor", "X-Ray aka Bakiev Damir" },
            { "Info", "Circle" }
        };
    }
    Shapes::Shape* createShape(const QPointF& point) override { return circle = new Circle(point, point + QPointF { 1, 1 }); }
    bool addShapePoint(const QPointF& value) override { return false; }
    void updateShape(const QPointF& value) override { }
    void finalizeShape() override { circle = nullptr; }
};
}
