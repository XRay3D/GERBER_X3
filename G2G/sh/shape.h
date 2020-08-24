#pragma once

#include <gi/graphicsitem.h>

namespace Shapes {

class SH;

class Shape : public GraphicsItem {
    friend class SH;

public:
    Shape();
    Shape(QDataStream& stream);
    ~Shape();
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    // GraphicsItem interface
    Paths paths() const override;
    void write(QDataStream& stream);
    virtual QString name() const = 0;

protected:
    QVector<SH*> sh;
    Paths m_paths;

    // QGraphicsItem interface
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}
