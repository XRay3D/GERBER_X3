/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gcode.h"

#include "gi.h"
#include <QObject>

class GraphicsView;

class GiBridge final : public GraphicsItem {
    Q_OBJECT
    friend class ProfileForm;

public:
    explicit GiBridge(double& lenght, double& size, GCode::SideOfMilling& side, GiBridge*& ptr);
    ~GiBridge() override { ptr_ = nullptr; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    void setNewPos(const QPointF& pos);
    // GraphicsItem interface
    Paths paths(int alternate = {}) const override;

    bool ok() const;
    double lenght() const;
    double angle() const;

    void update();

    IntPoint getPoint(const int side) const;
    QLineF getPath() const;

    void setOk(bool ok);
    void changeColor() override { }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    GiBridge*& ptr_;
    GCode::SideOfMilling& side_;
    QPainterPath path_;
    QPointF calculate(const QPointF& pos);
    QPointF lastPos_;
    bool ok_ = false;
    double angle_ = 0.0;
    double& lenght_;
    double& size_;
};
