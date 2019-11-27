#ifndef ThermalPreviewItem_H
#define ThermalPreviewItem_H

#include "gi/graphicsitem.h"
#include "tooldatabase/tool.h"
#include <gbrvars.h>
#include <myclipper.h>

class ThermalNode;

class ThermalPreviewItem : public QGraphicsItem {
    static QPainterPath drawPoly(const Gerber::GraphicObject& go);
    friend class ThermalNode;

public:
    explicit ThermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool, double& depth);

    ~ThermalPreviewItem() override = default;

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

    double angle() const;
    void setAngle(double angle);

    double tickness() const;
    void setTickness(double tickness);

    int count() const;
    void setCount(int count);

    ThermalNode* node() const;

    bool isValid() const;

private:
    Tool& tool;
    double& m_depth;
    bool m_isValid = false;

    const Gerber::GraphicObject* const grob = nullptr;

    const QPainterPath m_sourcePath;
    QPainterPath m_toolPath;
    QPen m_pen;
    QBrush m_brush;

    Paths m_bridge;

    double m_angle = 0.0;
    double m_tickness = 0.5;
    int m_count = 4;
    ThermalNode* m_node = nullptr;
};

#endif // ThermalPreviewItem_H
