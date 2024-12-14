﻿/********************************************************************************
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

#include "myclipper.h"
#include <QAnimationGroup>
#include <QGraphicsItem>
#include <QPen>
#include <QPropertyAnimation>
#include <qmath.h>

class AbstractFile;
namespace Shapes {
class AbstractShape;
}

class Project;

namespace Gi {

namespace Type {
enum /*class*/ Type : int {
    DataPath = QGraphicsItem::UserType,
    DataSolid,
    Drill,

    MarkHome = QGraphicsItem::UserType + 100,
    MarkLayoutFrames,
    MarkPin,
    MarkZero,

    Path_ = QGraphicsItem::UserType + 200,
    Bridge,

    Preview = QGraphicsItem::UserType + 300, // Form
    Debug,
    // PrSlot,                                    // DrillForm
    // PrDrill,                                   // DrillForm
    // PrApetrure,                                // DrillForm

    Error = QGraphicsItem::UserType + 400, // Form

    ShapeBegin = QGraphicsItem::UserType + 500,
    ShCircle = ShapeBegin,
    ShRectangle,
    ShPolyLine,
    ShCirArc,
    ShText,
    ShHandler,
    ShapeEnd
};
}; // namespace Type

class Group;

class Item : public /*QGraphicsObject*/ QGraphicsItem {

    friend class Group;
    friend class ::Project;

    //    Q_OBJECT
    Q_GADGET
    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)

    inline QColor bodyColor() { return brushColor_; }
    inline void setBodyColor(const QColor& c) { brushColor_ = c, colorChanged(); }

    // signals:
public:
    void colorChanged() { update(); };

public:
    explicit Item(AbstractFile* file = nullptr);
    ~Item() override = default;

    QColor color() const { return color_; }
    void setColor(const QColor& brush);
    void setColorPtr(QColor* brushColor);

    QPen pen() const { return pen_; }
    void setPen(const QPen& pen);
    void setPenColorPtr(const QColor* penColor);

    virtual Paths paths(int /*alternate*/ = {}) const { return ~shape_.toSubpathPolygons(transform()); }
    virtual void setPaths(Paths paths, int /*alternate*/ = {}) {
        auto t{transform()};
        auto a{qRadiansToDegrees(asin(t.m12()))};
        t = t.rotateRadians(-t.m12());
        auto x{t.dx()};
        auto y{t.dy()};
        shape_ = {};
        t = {};
        t.translate(-x, -y);
        t.rotate(-a);
        for(auto&& path: ~paths)
            shape_.addPolygon(t.map(path));
        redraw();
    }
    virtual void redraw() { }
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    Q_INVOKABLE void setVisible(bool visible);

    const AbstractFile* file() const;
    template <typename T>
    const T* typedFile() const { return dynamic_cast<const T* const>(file_); }

    int32_t id() const;
    void setId(int32_t id);
    virtual void changeColor() = 0;

protected:
    //    QPropertyAnimation animation;
    //    QPropertyAnimation visibleAnim;

    mutable QRectF boundingRect_;

    const AbstractFile* file_;
    Group* itemGroup = nullptr;
    QPainterPath shape_;

    QPen pen_;

    const QColor* pnColorPrt_ = nullptr;
    const QColor* colorPtr_ = nullptr;

    QColor color_;
    QColor brushColor_;
    QColor penColor_;

    int32_t id_ = -1;
    double scaleFactor() const;
    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
    };

    int colorState = Default;

    // QGraphicsItem interface
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

} // namespace Gi
