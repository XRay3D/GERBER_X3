#pragma once

#include <gi/graphicsitem.h>

namespace ShapePr {

class SH;

class Shape : public GraphicsItem {
    friend class SH;

public:
    Shape();
    ~Shape() override;
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    // GraphicsItem interface
    Paths paths() const override;

protected:
    QVector<SH*> sh;
    Paths m_paths;

    // QGraphicsItem interface
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}
