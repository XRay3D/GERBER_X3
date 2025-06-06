/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_preview.h"

#include "graphicsview.h"
#include "tool_pch.h"
#include <QPainter>
#include <QPropertyAnimation>

namespace Gi {

AbstractPreview::AbstractPreview()
    : propAnimGr{this}
    , propAnimBr(this, "bodyColor")
    , propAnimPn(this, "pathColor")
    , bodyColor_(colors[(int)Colors::Default])
    , pathColor_(colors[(int)Colors::UnUsed]) {
    propAnimGr.addAnimation(&propAnimBr);
    propAnimGr.addAnimation(&propAnimPn);

    propAnimBr.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    propAnimBr.setDuration(150);
    propAnimPn.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    propAnimPn.setDuration(150);

    connect(this, &AbstractPreview::colorChanged, [this] { update(); });
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setOpacity(0);
    setZValue(std::numeric_limits<double>::max() - 10);
    App::grView().addItem(this);
}

void AbstractPreview::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setPen({bodyColor_, 0.0});
    painter->setBrush(bodyColor_);
    painter->drawPath(sourcePath_);
    // draw tool
    if(toolId() > -1) {
        painter->setPen(QPen(pathColor_, 2 * App::grView().scaleFactor()));
        painter->setBrush(Qt::NoBrush);
        if(toolPath_.isEmpty())
            painter->drawPath(App::toolHolder().tool(toolId()).path(pos()));
        else
            painter->drawPath(toolPath_);
    }
}

QRectF AbstractPreview::boundingRect() const { return sourcePath_.boundingRect(); }

QPainterPath AbstractPreview::shape() const { return sourcePath_; }

int AbstractPreview::type() const { return int(Type::Preview); }

double AbstractPreview::sourceDiameter() const { return sourceDiameter_; }

void AbstractPreview::changeColor() {
    if(flags() & ItemIsSelectable)
        colorState |= Used;
    else
        colorState &= ~Used;

    propAnimBr.setStartValue(bodyColor_);
    propAnimPn.setStartValue(pathColor_);

    if(colorState & Selected)
        propAnimBr.setEndValue(colors[int((colorState & Hovered) ? Colors::SelectedHovered : Colors::Selected)]);
    else if(colorState & Used)
        propAnimBr.setEndValue(colors[int((colorState & Hovered) ? Colors::UsedHovered : Colors::Used)]);
    else
        propAnimBr.setEndValue(colors[int((colorState & Hovered) ? Colors::DefaultHovered : Colors::Default)]);

    if(colorState & Used)
        propAnimPn.setEndValue(colors[int((colorState & Tool) ? Colors::Tool : Colors::UnUsed)]);
    else
        propAnimPn.setEndValue(colors[int((colorState & Tool) ? Colors::Default : Colors::UnUsed)]);

    propAnimGr.start();
}

void AbstractPreview::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void AbstractPreview::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant AbstractPreview::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if(change == ItemSelectedChange) {
        if(value.toInt())
            colorState |= Selected;
        else
            colorState &= ~Selected;
        changeColor();
    } else if(change == ItemEnabledChange) {
        if(value.toInt())
            colorState |= Used;
        else
            colorState &= ~Used;
        changeColor();
    } else if(change == ItemVisibleChange) {
        auto animation = new QPropertyAnimation{this, "opacity"};
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    return QGraphicsItem::itemChange(change, value);
}

} // namespace Gi

#include "moc_gi_preview.cpp"
