#ifndef CIRCLE_H
#define CIRCLE_H

#include <gi/graphicsitem.h>
namespace Shape {
class Circle : public GraphicsItem {
public:
    explicit Circle(QPointF center, QPointF rh);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // GraphicsItem interface
    Paths paths() const override;

    QPointF center() const;
    void setCenter(const QPointF& center);
    QPointF rh() const;
    void setRh(const QPointF& rh);
    double radius() const;
    void setRadius(double radius);

private:
    void calc();
    QPointF m_rh;
    double m_radius;
    Path m_path;

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    // QGraphicsItem interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}

#endif // CIRCLE_H
