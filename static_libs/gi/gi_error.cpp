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
#include "gi_error.h"
#include "gi.h"
#include "graphicsview.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTime>
#include <QtMath>

#include <numbers>

namespace Gi {

Error::Error(const Geo::Polylines& curves, double area)
    : shape_{Geo::toPath(curves)}
    , boundingRect_{shape_.boundingRect()}
    , area_{area} {
    setFlag(ItemIsSelectable);
    setZValue(std::numeric_limits<double>::max());
}

Error::~Error() {
    if(isSelected())
        if(auto* view = App::grViewPtr()) view->removeAnimated(this);
}

double Error::area() const { return area_; }

// Габарит считается один раз: boundingRect() -- самая горячая виртуальная
// функция сцены, вызывать в ней QPainterPath::boundingRect() незачем.
QRectF Error::boundingRect() const { return boundingRect_; }

QVariant Error::itemChange(GraphicsItemChange change, const QVariant& value) {
    if(change == ItemSelectedChange) {
        if(auto* view = App::grViewPtr())
            value.toBool() ? view->addAnimated(this) : view->removeAnimated(this);
    } else if(change == ItemSceneChange && !value.value<QGraphicsScene*>()) {
        if(auto* view = App::grViewPtr()) view->removeAnimated(this);
    }
    return QGraphicsItem::itemChange(change, value);
}

static const QBrush errorBrush{
    QColor{255, 0, 255}
};

void Error::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    painter->setPen(Qt::NoPen);
    if(option->state & QStyle::State_Selected) {
        static QTime t(QTime::currentTime());
        painter->setBrush(QColor::fromHsv(cos(t.msecsTo(QTime::currentTime()) / (2 * std::numbers::pi * 8)) * 30 + 30, 255, 255, 255));
    } else {
        painter->setBrush(errorBrush);
    }

    painter->drawPath(shape_);
}

QPainterPath Error::shape() const { return shape_; }

int Error::type() const { return Gi::Type::Error; }

} // namespace Gi
