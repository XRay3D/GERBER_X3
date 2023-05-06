// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 * Attributions:                                                                *
 * The code in this library is an extension of Bala Vatti's clipping algorithm: *
 * "A generic solution to polygon clipping"                                     *
 * Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
 * http://portal.acm.org/citation.cfm?id=129906                                 *
 ********************************************************************************/

#include "gi.h"
#include "abstract_file.h"
#include "gi_group.h"
#include "graphicsview.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

namespace Gi {

Item::Item(AbstractFile* file)
    : //    animation(this, "bodyColor")
      //    , visibleAnim(this, "opacity")     ,
    file_(file)
    , pen_(QPen(Qt::white, 0.0))
    , colorPtr_(file ? &file->color() : nullptr)
    , color_(Qt::white)
    , bodyColor_(colorPtr_ ? *colorPtr_ : color_)
    , pathColor_(Qt::transparent) {
    //    animation.setDuration(100);
    //    animation.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    connect(this, &Item::colorChanged, [this] { update(); });
    //    visibleAnim.setDuration(100);
    //    visibleAnim.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    connect(&visibleAnim, &QAbstractAnimation::finished, [this] { QGraphicsObject::setVisible(visibleAnim.currentValue().toDouble() > 0.9); });
    QGraphicsItem::setVisible(false);

    //    connect(this, &QGraphicsObject::rotationChanged, [] { qDebug("rotationChanged"); });
}

void Item::setColor(const QColor& brush) {
    bodyColor_ = color_ = brush;
    colorChanged();
}

void Item::setColorPtr(QColor* brushColor) {
    if(brushColor)
        bodyColor_ = *(colorPtr_ = brushColor);
    pathColor_ = colorPtr_ ? *colorPtr_ : color_;
    colorChanged();
}

void Item::setPen(const QPen& pen) {
    pen_ = pen;
    colorChanged();
}

void Item::setPenColorPtr(const QColor* penColor) {
    if(penColor)
        pnColorPrt_ = penColor;
    colorChanged();
}

void Item::setVisible(bool visible) {
    //    if (visible == isVisible() && (visible && opacity() < 1.0))
    //        return;
    //    visibleAnim.setStartValue(visible ? 0.0 : 1.0);
    //    visibleAnim.setEndValue(visible ? 1.0 : 0.0);
    //    visibleAnim.start();
    //    if (visible) {
    //        setOpacity(0.0);
    setOpacity(1.0 * visible);
    QGraphicsItem /*QGraphicsObject*/ ::setVisible(visible);
    //    }
}

const AbstractFile* Item::file() const { return file_; }

int Item::id() const { return id_; }

void Item::setId(int32_t id) { id_ = id; }

void Item::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void Item::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant Item::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if(change == ItemSelectedChange) {
        const bool fl = value.toInt();
        fl ? colorState |= Selected : colorState &= ~Selected;
        changeColor();
    } else if(change == ItemSceneChange) {
    }
    return QGraphicsItem::itemChange(change, value);
}

double Item::scaleFactor() const {
    double scale = 1.0;
    if(scene() && scene()->views().size()) {
        scale /= scene()->views().first()->transform().m11();
        if(file_)
            scale /= std::min(file_->transform().scale.x(), file_->transform().scale.y());
    }
    return scale;
};

QRectF Item::boundingRect() const {
    if(App::graphicsView().boundingRectFl())
        return shape_.toFillPolygon(transform()).boundingRect();
    return boundingRect_;
}

QPainterPath Item::shape() const { return shape_; }

} // namespace Gi

#include "moc_gi.cpp"
