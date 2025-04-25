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

#include "gi.h"
#include "abstract_file.h"
#include "app.h"
// #include "gi_group.h"
#include "graphicsview.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <qbrush.h>
#include <qpen.h>

namespace Gi {

Item::Item(AbstractFile* file)
    : file_{file}
    , pen_{Qt::white, 0.0}
    , colorPtr_{file ? &file->color() : nullptr}
    , color_{Qt::white}
    , brushColor_{colorPtr_ ? *colorPtr_ : color_}
    , penColor_{Qt::transparent} {
    //    animation(this, "bodyColor")
    //    , visibleAnim(this, "opacity")     ,//    animation.setDuration(100);
    //    animation.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    connect(this, &Item::colorChanged, [this] { update(); });
    //    visibleAnim.setDuration(100);
    //    visibleAnim.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    connect(&visibleAnim, &QAbstractAnimation::finished, [this] {
    //    QGraphicsObject::setVisible(visibleAnim.currentValue().toDouble() >
    //    0.9); });
    QGraphicsItem::setVisible(false);
    //    connect(this, &QGraphicsObject::rotationChanged, [] {
    //    qDebug("rotationChanged"); });
}

void Item::setColor(const QColor& color) {
    brushColor_ = color_ = color;
    colorChanged();
}

void Item::setColorPtr(QColor* colorPtr) {
    if(colorPtr) brushColor_ = *(colorPtr_ = colorPtr);
    penColor_ = colorPtr_ ? *colorPtr_ : color_;
    colorChanged();
}

void Item::setPen(const QPen& pen) {
    pen_ = pen;
    colorChanged();
}

void Item::setPenColorPtr(const QColor* penColor) {
    if(penColor) pnColorPrt_ = penColor;
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

QVariant Item::itemChange(QGraphicsItem::GraphicsItemChange change,
    const QVariant& value) {
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
        scale /= scene()->views().front()->transform().m11();
        if(file_) scale /= std::min(file_->transform().scale.x(), file_->transform().scale.y());
    }
    return scale;
}

// double Item::penWidth(double w) const {
//     if(scene() && scene()->views().size())
//         w /= scene()->views().front()->transform().m11();
//     return w;
// };

QRectF Item::boundingRect() const {
    if(App::grView().boundingRectFl())
        return shape_.toFillPolygon(transform()).boundingRect();
    return boundingRect_;
}

QPainterPath Item::shape() const { return shape_; }

} // namespace Gi

#include "moc_gi.cpp"
