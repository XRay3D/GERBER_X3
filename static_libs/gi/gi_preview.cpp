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
#include "gi_preview.h"

#include "graphicsview.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

namespace Gi {

AbstractPreview::AbstractPreview()
    : propAnimGr{this}
    , propAnimBr{this, "bodyColor"_ba}
    , propAnimPn{this, "pathColor"_ba}
    , bodyColor_{colors[0].body}
    , pathColor_{colors[0].path} {
    propAnimGr.addAnimation(&propAnimBr);
    propAnimGr.addAnimation(&propAnimPn);

    propAnimBr.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    propAnimBr.setDuration(100);
    propAnimPn.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    propAnimPn.setDuration(100);

    connect(this, &AbstractPreview::colorChanged, [this] { update(); });
    setAcceptHoverEvents(true);
    // ItemIsSelectable здесь двойного назначения: помимо штатной семантики Qt,
    // changeColor() читает этот флаг как признак "инструмент назначен" (Used -> красный).
    // Пока инструмент не выбран (Model::setToolId ещё не вызывался с id > -1),
    // флаг должен быть выключен — иначе превью до первого назначения инструмента
    // всегда красное вместо серого.
    setFlag(ItemIsSelectable, false);
    setOpacity(0);
    setZValue(std::numeric_limits<double>::max() - 10);
    App::grView().addItem(this);
}

void AbstractPreview::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    painter->setPen({bodyColor_, 0.0});
    painter->setBrush(bodyColor_);
    painter->drawPath(sourcePath_);
    // draw tool
    const auto id = toolId(); // чисто виртуальный вызов -- ровно один на кадр
    if(id > Tool::ID{}) {
        const double sf = 1.0 / QStyleOptionGraphicsItem::levelOfDetailFromTransform(painter->worldTransform());
        painter->setPen(QPen{pathColor_, 2 * sf});
        painter->setBrush(Qt::NoBrush);
        if(!toolPath_.isEmpty())
            painter->drawPath(toolPath_);
        else {
            // Tool::path() строит целый QPainterPath -- раньше это делалось на
            // каждый кадр каждого превью (одно превью на отверстие).
            if(toolPathCacheId_ != id || toolPathCachePos_ != pos()) {
                toolPathCacheId_ = id;
                toolPathCachePos_ = pos();
                toolPathCache_ = App::toolHolder().tool(id).path(pos());
            }
            painter->drawPath(toolPathCache_);
        }
    }
}

QRectF AbstractPreview::boundingRect() const { return sourcePath_.boundingRect(); }

QPainterPath AbstractPreview::shape() const { return sourcePath_; }

int AbstractPreview::type() const { return int(Type::Preview); }

double AbstractPreview::sourceDiameter() const { return sourceDiameter_; }

void AbstractPreview::changeColor() {
    // ItemIsSelectable отражает "инструмент назначен строке" (см. Model::setToolId/
    // setCreate), used — персональный признак конкретного превью, переключаемый
    // двойным кликом. Бит Used выставлен только если оба условия верны — без if,
    // подстановкой 0/4 по булеву значению.

    // colorState = ColorState{+(colorState & CS::Used) | (+CS::Used * ((flags() & ItemIsSelectable) && used))};

    // qWarning() << (+(cs & CS::Hovered) ? "Hovered " : "        ")
    //            << (+(cs & CS::Selected) ? "Selected" : "        ")
    //            << (+(cs & CS::Used) ? "Used    " : "        ")
    //            << (+(cs & CS::Tool) ? "Tool    " : "        ");
    auto cs = colorState & (flags() & ItemIsSelectable ? ~CS::Default : ~(CS::Used | CS::Tool));

    propAnimBr.setStartValue(bodyColor_);
    propAnimPn.setStartValue(pathColor_);
    const StateColors& c = colors[+cs];
    propAnimBr.setEndValue(c.body);
    propAnimPn.setEndValue(c.path);

    propAnimGr.start();
}

void AbstractPreview::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsItem::hoverEnterEvent(event);
    colorState |= CS::Hovered;
    changeColor();
}

void AbstractPreview::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsItem::hoverLeaveEvent(event);
    colorState &= ~CS::Hovered;
    changeColor();
}

QVariant AbstractPreview::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    auto val = QGraphicsItem::itemChange(change, value);
    if(change == ItemSelectedChange) {
        value.toInt() ? colorState |= CS::Selected
                      : colorState &= ~CS::Selected;
        changeColor();
    } else if(change == ItemEnabledChange) {
        value.toInt() ? colorState |= CS::Used
                      : colorState &= ~CS::Used;
        changeColor();
    } else if(change == ItemVisibleChange) {
        auto animation = new QPropertyAnimation{this, "opacity"_ba};
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    return val;
}

} // namespace Gi

#include "moc_gi_preview.cpp"
