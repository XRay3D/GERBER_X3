/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include <graphicsitem.h>

struct Row;

class AbstractDrillPrGI : public QGraphicsObject {
    friend class ThermalNode;

    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor NOTIFY colorChanged FINAL)

    // clang-format off
    QColor bodyColor() { return m_bodyColor; }
    void setBodyColor(const QColor& c) { m_bodyColor = c; colorChanged();  }
    QColor pathColor() { return m_pathColor; }
    void setPathColor(const QColor& c) { m_pathColor = c; colorChanged(); }
    // clang-format on

signals:
    void colorChanged();

public:
    AbstractDrillPrGI(Row& row);
    ~AbstractDrillPrGI() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;

    //////////////////////////////////////////
    int type() const override;
    double sourceDiameter() const;
    int toolId() const;

    virtual void updateTool() = 0;
    virtual Point64 pos() const = 0;
    virtual Paths paths() const = 0;
    virtual bool fit(double depth) = 0;

    void changeColor();

protected:
    QPainterPath m_sourcePath;
    QPainterPath m_toolPath;

    struct Row& row;

    double m_sourceDiameter;
    GiType m_type;

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
        Tool,
    };

    static constexpr int dark = 180;
    static constexpr int light = 255;
    inline static constexpr QRgb colors[] {
        qRgba(128, 128, 128, dark), //  Default         gray dark
        qRgba(255, 255, 255, light), // DefaultHovered  gray light
        qRgba(0, 255, 0, dark), //      Selected        green dark
        qRgba(0, 255, 0, light), //     SelectedHovered green light
        qRgba(255, 0, 0, dark), //      Used            red dark
        qRgba(255, 0, 0, light), //     UsedHovered     red light
        qRgba(255, 255, 255, 255), //   UnUsed          transparent
        qRgba(255, 255, 255, dark), //  Tool            white
    };

    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
        Used = 4,
        Tool = 8,
    };
    int colorState = Default;

    // QGraphicsItem interface
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
