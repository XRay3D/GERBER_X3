#pragma once

#include "excellon.h"
#include "gbrtypes.h"
#include "gi/graphicsitem.h"

class DrillPrGI : public QGraphicsItem {
    static QPainterPath drawApetrure(const Gerber::GraphicObject& go, int id);
    static QPainterPath drawDrill(const Excellon::Hole& hole);
    static QPainterPath drawSlot(const Excellon::Hole& hole);

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
    IntPoint pos() const;
    Paths paths() const;
    bool fit(double depth);

private:
    const int id = 0;
    const Gerber::GraphicObject* const grob = nullptr;
    const Excellon::Hole* const hole = nullptr;

    const QPainterPath m_sourcePath;
    QPainterPath m_toolPath;

    const double m_sourceDiameter;
    int m_toolId = -1;
    const int m_type;

    QPen m_pen;
    QBrush m_brush;
};
