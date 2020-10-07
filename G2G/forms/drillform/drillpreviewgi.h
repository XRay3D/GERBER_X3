#pragma once

#include "excellon.h"
#include "gbrtypes.h"
#include "gi/graphicsitem.h"

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
    static QPainterPath drawPoly(const Gerber::GraphicObject& go);
    friend class ThermalNode;

signals:
    void colorChanged();

public:
    explicit DrillPrGI(const Gerber::GraphicObject& go, int id);
    explicit DrillPrGI(const Excellon::Hole& hole);

    ~DrillPrGI() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;

    //////////////////////////////////////////
    int type() const override;
    double sourceDiameter() const;
    int toolId() const;
    void setToolId(int toolId);
    void setUsed(bool fl);
    IntPoint pos() const;
    Paths paths() const;
    bool fit(double depth);

private:
    static QPainterPath drawApetrure(const Gerber::GraphicObject& go, int id);
    static QPainterPath drawDrill(const Excellon::Hole& hole);
    static QPainterPath drawSlot(const Excellon::Hole& hole);

    const int id = 0;
    const Gerber::GraphicObject* const grob = nullptr;
    const Excellon::Hole* const hole = nullptr;

    const QPainterPath m_sourcePath;
    QPainterPath m_toolPath;

    const double m_sourceDiameter;
    int m_toolId = -1;
    const int m_type;

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

    static constexpr int dark = 200;
    static constexpr int light = 255;
    inline static const QColor colors[] {
        QColor(128, 128, 128, dark), //  Default         gray dark
        QColor(255, 255, 255, light), // DefaultHovered  gray light
        QColor(0, 255, 0, dark), //      Selected        green dark
        QColor(0, 255, 0, light), //     SelectedHovered green light
        QColor(255, 0, 0, dark), //      Used            red dark
        QColor(255, 0, 0, light), //     UsedHovered     red light
        QColor(255, 255, 255, 0), //     UnUsed          transparent
        QColor(255, 255, 255, dark), //  Tool            white
    };

    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
        Used = 4,
        Tool = 8,
    };
    int colorState = Default;

    void changeColor();

    // QGraphicsItem interface
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
