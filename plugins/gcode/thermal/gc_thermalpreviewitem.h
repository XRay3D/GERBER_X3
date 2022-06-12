/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gi.h"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QVector>

#include <gerber/gbr_types.h>

// class QParallelAnimationGroup;
class ThermalNodeI;
class Tool;

class AbstractThermPrGi : public QGraphicsObject {
    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor)

    // clang-format off
    QColor bodyColor() { return m_bodyColor; }
    void setBodyColor(const QColor& c) { m_bodyColor = c; update();  }
    QColor pathColor() { return m_pathColor; }
    void setPathColor(const QColor& c) { m_pathColor = c; update(); }
    // clang-format on
    // static QPainterPath drawPoly(const GerberAbstrGraphicObject& go);
    friend class ThermalNode;

    QParallelAnimationGroup agr;
    QPropertyAnimation pa1;
    QPropertyAnimation pa2;

signals:
    void selectionChanged(const QModelIndex& s, const QModelIndex& d);

public:
    explicit AbstractThermPrGi(Tool& tool);

    ~AbstractThermPrGi() override;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //////////////////////////////////////////
    int type() const override;

    Paths bridge() const { return m_bridge; }
    virtual bool isValid() const;

    virtual IntPoint pos() const = 0;
    virtual Paths paths() const = 0;
    virtual void redraw() = 0;

protected:
    Tool& tool;

private:
    QColor m_bodyColor;
    QColor m_pathColor;

    enum class Colors : int {
        Default,
        DefaultHovered,
        Selected,
        SelectedHovered,
        Used,
        UsedHovered,
        UnUsed,
    };

    static constexpr int dark = 180;
    static constexpr int light = 255;
    inline static const QColor colors[] {
        QColor(128, 128, 128, dark),  //     Default         dark gray
        QColor(128, 128, 128, light), //    DefaultHovered  light gray
        QColor(0x0, 255, 0x0, dark),  //     Selected        dark green
        QColor(0x0, 255, 0x0, light), //    SelectedHovered light green
        QColor(255, 0x0, 0x0, dark),  //     Used            dark red
        QColor(255, 0x0, 0x0, light), //    UsedHovered     light red
        QColor(0x0, 0x0, 0x0, 0x0),   //      UnUsed          transparent
    };

    enum ColorState {
        Default = 0,
        Hovered = 1,
        Selected = 2,
        Used = 4,
    };

    int colorState = Default;

    inline static mvector<AbstractThermPrGi*> thpi;

    // QGraphicsItem interface
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void changeColor();

    QPainterPath sourcePath;
    QPainterPath painterPath;

    Paths m_bridge;
    Paths previewPaths;
    Paths cashedPath;
    Paths cashedFrame;

    ThermalNodeI* m_node = nullptr;

    double diameter;
    int isEmpty = -1;
};

class ThermalPreviewItem final : public AbstractThermPrGi {
    const Paths* paths_;
    const IntPoint pos_;

public:
    ThermalPreviewItem(const Paths* paths, const IntPoint pos, Tool& tool);
    IntPoint pos() const override;
    Paths paths() const override;
    void redraw() override;
};
