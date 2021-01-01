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
#include "excellon.h"
#ifdef GERBER
#include "gbrtypes.h"
#endif
#include "gi/graphicsitem.h"

struct Row;

class DrillPrGI : public QGraphicsObject {
    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor NOTIFY colorChanged FINAL)

    // clang-format off
    QColor bodyColor() { return m_bodyColor; }
    void setBodyColor(const QColor& c) { m_bodyColor = c; colorChanged();  }
    QColor pathColor() { return m_pathColor; }
    void setPathColor(const QColor& c) { m_pathColor = c; colorChanged(); }
    // clang-format on
#ifdef GERBER
    static QPainterPath drawPoly(const Gerber::GraphicObject& go);
#endif
    friend class ThermalNode;

signals:
    void colorChanged();

public:
#ifdef GERBER
    explicit DrillPrGI(const Gerber::GraphicObject* go, int id, Row& row);
#endif
    explicit DrillPrGI(const Excellon::Hole* hole, Row& row);

    ~DrillPrGI() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;

    //////////////////////////////////////////
    int type() const override;
    double sourceDiameter() const;
    int toolId() const;
    void updateTool();
    Point64 pos() const;
    Paths paths() const;
    bool fit(double depth);

    void changeColor();

private:
#ifdef GERBER
    static QPainterPath drawApetrure(const Gerber::GraphicObject* go, int id);
#endif
    static QPainterPath drawDrill(const Excellon::Hole* hole);
    static QPainterPath drawSlot(const Excellon::Hole* hole);

    struct Row& row;

    const int id = 0;
#ifdef GERBER
    const Gerber::GraphicObject* const gbrObj = nullptr;
#endif
    const Excellon::Hole* const hole = nullptr;

    const QPainterPath m_sourcePath;
    QPainterPath m_toolPath;

    const double m_sourceDiameter;
    const GiType m_type;

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
        qRgba(255, 255, 255, 0), //     UnUsed          transparent
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
