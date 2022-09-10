/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
#include "qparallelanimationgroup.h"

class GiAbstractPreview : public QGraphicsObject {
    friend class Node;

    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor NOTIFY colorChanged FINAL)

    QColor bodyColor() { return bodyColor_; }
    void setBodyColor(const QColor& c) { bodyColor_ = c, colorChanged(); }
    QColor pathColor() { return pathColor_; }
    void setPathColor(const QColor& c) { pathColor_ = c, colorChanged(); }

    QParallelAnimationGroup propAnimGr;
    QPropertyAnimation propAnimBr;
    QPropertyAnimation propAnimPn;

signals:
    void colorChanged();

public:
    GiAbstractPreview();
    ~GiAbstractPreview() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //////////////////////////////////////////
    int type() const override;
    double sourceDiameter() const;
    virtual int toolId() const = 0;

    virtual void updateTool() = 0;
    virtual Paths paths() const = 0;
    virtual bool fit(double depth) const = 0;

    void changeColor();

protected:
    QPainterPath sourcePath_;
    QPainterPath toolPath_;

    bool used {};
    double sourceDiameter_ {};

    QColor bodyColor_;
    QColor pathColor_;

    enum class Colors : int {
        Default,
        DefaultHovered,
        Selected,
        SelectedHovered,
        Used,
        UsedHovered,
        UnUsed,
        Tool,
    };

    static constexpr int dark = 180;
    static constexpr int light = 255;
    inline static const QColor colors[] {
        QColor(128, 128, 128, dark),  // 0 Default         gray dark
        QColor(255, 255, 255, light), // 1 DefaultHovered  gray light
        QColor(0, 255, 0, dark),      // 2 Selected        green dark
        QColor(0, 255, 0, light),     // 3 SelectedHovered green light
        QColor(255, 0, 0, dark),      // 4 Used            red dark
        QColor(255, 0, 0, light),     // 5 UsedHovered     red light
        QColor(255, 255, 255, 255),   // 6 UnUsed          transparent
        QColor(255, 255, 255, dark),  // 7 Tool            white
    };

    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
        Used = 4,
        Tool = 8,
    };
    int colorState = Default;

    //    std::unordered_map<int, QColor> colors {
    //        { Default, QColor(128, 128, 128, dark) },            // 0 Default         gray dark
    //        { Default | Hovered, QColor(255, 255, 255, light) }, // 1 DefaultHovered  gray light
    //        { Selected, QColor(0, 255, 0, dark) },               // 2 Selected        green dark
    //        { Selected | Hovered, QColor(0, 255, 0, light) },    // 3 SelectedHovered green light
    //        { Used, QColor(255, 0, 0, dark) },                   // 4 Used            red dark
    //        { Used | Hovered, QColor(255, 0, 0, light) },        // 5 UsedHovered     red light
    //        { Default, QColor(255, 255, 255, 255) },             // 6 UnUsed          transparent
    //        { Default | Hovered, QColor(255, 255, 255, dark) },  // 7 Tool            white
    //    };
    //};

    // QGraphicsItem interface
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
