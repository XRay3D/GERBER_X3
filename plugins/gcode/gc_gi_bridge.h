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

#include "gc_types.h"
#include "gi.h"

namespace Gi {

class Bridge final : public Item {

public:
    explicit Bridge();
    ~Bridge() override { moveBrPtr = nullptr; }

    // QGraphicsItem interface
    QRectF boundingRect() const override { return pPath.boundingRect(); }
    QPainterPath shape() const override { return pPath; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // Item interface
    Paths paths(int alternate = {}) const override;
    void changeColor() override { }

    bool ok() const;
    void update();
    bool test(const Path& path);
    QPointF snapedPos(const QPointF& pos);

    static inline Bridge* moveBrPtr;           // NOTE приватизировать в будущем??
    static inline double lenght{};             //
    static inline double toolDiam{};           //
    static inline GCode::SideOfMilling side{}; //

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QLineF testLine() const;
    Point intersectPoint;

    double angle_{};

    QPainterPath pPath;
    QPainterPath cutoff;

    QPointF lastPos;

    bool ok_ = false;
};

} // namespace Gi
