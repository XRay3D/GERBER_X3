/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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

    // AbstractShape interface
    QString name() const override;
    QIcon icon() const override;
    void setPt(const QPointF& pt) override;
    bool addPt(const QPointF& pt) override;

    bool isClosed() const;
    void setClosed(bool fl);

protected:
    void rebuild() override;
    // Двойной клик по ручке-центру: дуга перестраивается касательной к
    // соседним сегментам (с обеих сторон — среднее двух касательных углов).
    bool handleDoubleClick(Shapes::Handle& handle) override;

private:
    Shapes::Handle* lastCorner() const; // последний угол (у замкнутой хвост — средняя ручка)
    QPointF nextCorner(size_t midIdx) const; // угол после средней ручки (с заворотом)
    QPointF arcCenter(size_t midIdx) const;  // проекция ручки на серединный перпендикуляр хорды
    double segBulge(size_t midIdx) const;    // прогиб сегмента по его средней ручке
    void updMiddle(size_t midIdx);           // средняя ручка следует за углами
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
    std::string_view typeName() const override { return "Polyline"; }
    QIcon icon() const override { return QIcon::fromTheme(u"draw-line"_s); }
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
