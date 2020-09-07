#pragma once

#include "graphicsitem.h"

namespace Gerber {
class File;
}

class AperturePathItem : public GraphicsItem {
public:
    AperturePathItem(const Path& path, Gerber::File* file);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;
    QPainterPath shape() const override;

protected:
    QPolygonF m_polygon;
    const Path& m_path;
    mutable QPainterPath m_selectionShape;
    mutable double m_scale = std::numeric_limits<double>::max();

    // QGraphicsItem interface
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
