/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "geo/polygon.h"

#include <QGraphicsItem>

namespace Gi {

class Error final : public QGraphicsItem {
    QPainterPath shape_;
    const QRectF boundingRect_;
    const double area_;

public:
    Error(const Geo::Polylines& curves, double area);
    ~Error() override;
    double area() const;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
    int type() const override;

protected:
    // Пульсация цвета на выделении -- единственное, ради чего элемент просит
    // вид крутить таймер анимации.
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

} // namespace Gi

Q_DECLARE_METATYPE(Gi::Error*)
