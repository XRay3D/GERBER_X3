#pragma once

#include <gi/graphicsitem.h>

namespace Shapes {

class Handler;

class Shape : public GraphicsItem {
    friend class Handler;

public:
    Shape();
    Shape(QDataStream& stream);
    ~Shape();
    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    Paths paths() const override;

    void write(QDataStream& stream);

    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;
    virtual QPointF calcPos(Handler* sh) const = 0;

private:
    mutable double m_scale = std::numeric_limits<double>::max();
    mutable QPainterPath m_selectionShape;

protected:
    QVector<Handler*> sh;
    Paths m_paths;

    // QGraphicsItem interface
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}
