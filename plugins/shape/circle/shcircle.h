/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#pragma once

#include "shape.h"
#include "shapepluginin.h"
#include <QJsonObject>
#include <graphicsitem.h>

namespace Shapes {

class Circle final : public Shape {
public:
    explicit Circle(QPointF center, QPointF pt);
    explicit Circle() { }
    ~Circle();

    // QGraphicsItem interface
    int type() const override { return static_cast<int>(GiType::ShCircle); }
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
        PtCount
    };

    // Shape interface
    //        bool setData(const QModelIndex& index, const QVariant& value, int role) override { }
    //        Qt::ItemFlags flags(const QModelIndex& index) const override { }
    //        QVariant data(const QModelIndex& index, int role) const override { }
    //        void menu(QMenu& menu, FileTree::View* tv) const override { }

private:
    double m_radius;
};

class Plugin : public ShapePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "circle.json")
    Q_INTERFACES(ShapePlugin)

    Circle* shape = nullptr;

public:
    Plugin();
    virtual ~Plugin() override;

    // ShapePlugin interface
public:
    int type() const override;

    QIcon icon() const override;
    Shape* createShape() override;
    Shape* createShape(const QPointF& point) override;
    bool addShapePoint(const QPointF& value) override;
    void updateShape(const QPointF& value) override;
    void finalizeShape() override;

signals:
};
} // namespace Shapes
