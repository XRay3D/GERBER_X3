/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#ifdef GERBER
#include "gbrtypes.h"
#include "gi/graphicsitem.h"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QVector>

//class QParallelAnimationGroup;
class ThermalNode;
class Tool;

class ThermalPreviewItem final : public QGraphicsObject {
    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor)

    // clang-format off
    QColor bodyColor() { return m_bodyColor; }
    void setBodyColor(const QColor& c) { m_bodyColor = c; update();  }
    QColor pathColor() { return m_pathColor; }
    void setPathColor(const QColor& c) { m_pathColor = c; update(); }
    // clang-format on
    static QPainterPath drawPoly(const Gerber::GraphicObject& go);
    friend class ThermalNode;

signals:
    void selectionChanged(const QModelIndex& s, const QModelIndex& d);

public:
    explicit ThermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool, double& depth);

    ~ThermalPreviewItem() override;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //////////////////////////////////////////
    int type() const override;

    Point64 pos() const;

    Paths bridge() const;
    Paths paths() const;

    bool isValid() const;
    void redraw();

private:
    QParallelAnimationGroup agr;
    QPropertyAnimation pa1;
    QPropertyAnimation pa2;
    Tool& tool;
    double& m_depth;

    const Gerber::GraphicObject* const grob = nullptr;
    const QPainterPath sourcePath;
    QPainterPath painterPath;

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
        QColor(128, 128, 128, dark), //  dark gray
        QColor(255, 255, 255, light), // light gray
        QColor(0x0, 255, 0x0, dark), //      dark green
        QColor(0x0, 255, 0x0, light), //     light green
        QColor(255, 0x0, 0x0, dark), //      dark red
        QColor(255, 0x0, 0x0, light), //     light red
        QColor(255, 255, 255, 0x0), //     transparent
    };

    enum ColorState {
        Default = 0,
        Hovered = 1,
        Selected = 2,
        Used = 4,
    };

    int colorState = Default;
    double diameter;

    Paths m_bridge;
    Paths previewPaths;

    Paths cashedPath;
    Paths cashedFrame;
    int isEmpty = -1;

    ThermalNode* m_node = nullptr;
    inline static QVector<ThermalPreviewItem*> thpi;
    void changeColor();

    // QGraphicsItem interface
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
#endif
