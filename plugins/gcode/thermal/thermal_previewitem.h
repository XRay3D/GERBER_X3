/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gi.h"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QVector>

#include <gerber/gbr_types.h>

class Tool;

namespace Thermal {
class Node;

// class QParallelAnimationGroup;
// class NodeI;

class AbstractThermPrGi : public QGraphicsObject {
    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor)

    QColor bodyColor() { return bodyColor_; }
    void setBodyColor(const QColor& c) { bodyColor_ = c, update(); }
    QColor pathColor() { return pathColor_; }
    void setPathColor(const QColor& c) { pathColor_ = c, update(); }
    friend class Node;

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
    // QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //////////////////////////////////////////
    uint32_t type() const override;

    Paths bridge() const { return bridge_; }
    virtual bool isValid() const;

    virtual Point pos() const = 0;
    virtual Paths paths() const = 0;
    virtual void redraw() = 0;

protected:
    Tool& tool;

private:
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
    };

    static constexpr int dark = 180;
    static constexpr int light = 255;
    inline static const QColor colors[] {
        QColor(128, 128, 128, dark),  // Default         dark gray
        QColor(128, 128, 128, light), // DefaultHovered  light gray
        QColor(0x0, 255, 0x0, dark),  // Selected        dark green
        QColor(0x0, 255, 0x0, light), // SelectedHovered light green
        QColor(255, 0x0, 0x0, dark),  // Used            dark red
        QColor(255, 0x0, 0x0, light), // UsedHovered     light red
        QColor(0x0, 0x0, 0x0, 0x0),   // UnUsed          transparent
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

    Paths bridge_;
    Paths previewPaths;
    Paths cashedPath;
    Paths cashedFrame;

    Node* node_ {nullptr};

    double diameter;
    int isEmpty {-1};
};

class PreviewItem final : public AbstractThermPrGi {
    const Paths& paths_;
    const Point pos_;

public:
    PreviewItem(const Paths& paths, const Point pos, Tool& tool);
    Point pos() const override;
    Paths paths() const override;
    void redraw() override;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
};

} // namespace Thermal
