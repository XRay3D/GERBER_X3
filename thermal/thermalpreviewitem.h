#pragma once

#include "gbrtypes.h"
#include "gi/graphicsitem.h"

#include <QVector>

class Tool;

class ThermalNode;

class ThermalPreviewItem : public QObject, public QGraphicsItem {
    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor)

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
    IntPoint pos() const;
    Paths paths() const;
    Paths bridge() const;
    void redraw();
    bool isValid() const;

private:
    Tool& tool;
    double& m_depth;
    bool m_isValid = false;

    const Gerber::GraphicObject* const grob = nullptr;

    const QPainterPath m_sourcePath;

    QPainterPath m_toolPath;

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

    inline static const QColor colors[] {
        QColor(255, 255, 255, 100), //  dark gray
        QColor(255, 255, 255, 200), //  light gray
        QColor(0, 255, 0, 100), //      dark green
        QColor(0, 255, 0, 200), //      light green
        QColor(255, 0, 0, 100), //      dark red
        QColor(255, 0, 0, 200), //      light red
        QColor(255, 0, 0, 0), //        transparent
    };

    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
        Used = 4,
    };

    int colorState = Default;
    double diameter;
    Paths m_bridge;
    ThermalNode* m_node = nullptr;
    inline static QVector<ThermalPreviewItem*> hhh;
    void changeColor();

    // QGraphicsItem interface
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
