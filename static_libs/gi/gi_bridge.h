/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
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
    //    Q_OBJECT
    Q_GADGET
    friend class ProfileForm;

public:
    explicit GiBridge(double& lenght, double& toolDiam, GCode::SideOfMilling& side);
    ~GiBridge() override { moveBrPtr = nullptr; }

    // QGraphicsItem interface
    QRectF boundingRect() const override { return pPath.boundingRect(); }
    QPainterPath shape() const override { return pPath; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // GraphicsItem interface
    Paths paths(int alternate = {}) const override;
    void changeColor() override { }

    bool ok() const;
    double lenght() const;
    double angle() const;

    void update();

    QLineF testLine() const;

    static inline GiBridge* moveBrPtr; // FIXME приватизировать в будущем??

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    double angle_ {};
    double& lenght_;
    double& toolDiam_;

    QPainterPath pPath;
    QPainterPath cutoff;

    QPointF snapedPos(const QPointF& pos);
    QPointF lastPos;

    GCode::SideOfMilling& side_;

    bool ok_ = false;
};
