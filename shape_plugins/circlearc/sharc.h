/*******************************************************************************
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

#include "shape.h"
#include <QJsonObject>
#include <graphicsitem.h>
#include "shapepluginin.h"

namespace Shapes {
class Arc final : public Shape {
public:
    explicit Arc(QPointF center, QPointF pt, QPointF pt2);
    explicit Arc() { }
    ~Arc();

    // QGraphicsItem interface
    int type() const override { return static_cast<int>(GiType::ShCirArc); }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt);
    void setPt2(const QPointF& pt);
    double radius() const;
    void setRadius(double radius);

    enum {
        Center,
        Point1,
        Point2,
        PtCount
    };

    // Shape interface
    //    bool setData(const QModelIndex& index, const QVariant& value, int role) override { }
    //    Qt::ItemFlags flags(const QModelIndex& index) const override { }
    //    QVariant data(const QModelIndex& index, int role) const override { }
    //    void menu(QMenu& menu, FileTree::View* tv) const override { }
private:
    mutable double m_radius;

protected:
    // Shape interface
    void updateOtherHandlers(Handler* handler) override;
};

class Plugin : public QObject, public ShapePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "circlearc.json")
    Q_INTERFACES(ShapePlugin)

    Arc* shape = nullptr;
    int ctr = 0;

public:
    Plugin();
    virtual ~Plugin() override;

    // ShapePlugin interface
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

} // namespace Shapes
