/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
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
class PolyLine final : public Shape {
public:
    explicit PolyLine(QPointF pt1, QPointF pt2);
    explicit PolyLine() { }
    ~PolyLine() = default;

    // QGraphicsItem interface
    int type() const override { return static_cast<int>(GiType::ShPolyLine); }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt);
    void addPt(const QPointF& pt);
    bool closed();

    // Shape interface
    //    bool setData(const QModelIndex& index, const QVariant& value, int role) override { }
    //    Qt::ItemFlags flags(const QModelIndex& index) const override { }
    //    QVariant data(const QModelIndex& index, int role) const override { }
    //    void menu(QMenu& menu, FileTree::View* tv) const override { }
protected:
    // Shape interface
    void updateOtherHandlers(Handler* handler) override;

private:
    QPointF centroid();
    QPointF centroidFast(); //??????
};

class Plugin : public QObject, public ShapePluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "polyline.json")
    Q_INTERFACES(ShapePluginInterface)

    PolyLine* shape = nullptr;

public:
    Plugin();
    virtual ~Plugin() override;

    // ShapePluginInterface interface
public:
    QObject* getObject() override;
    int type() const override;
    QJsonObject info() const override;
    QIcon icon() const override;
    Shape* createShape() override;
    Shape* createShape(const QPointF& point) override;
    bool addShapePoint(const QPointF& value) override;
    void updateShape(const QPointF& value) override;
    void finalizeShape() override;

signals:
    void actionUncheck(bool = false) override;
};
}
