#ifndef CIRCLE_H
#define CIRCLE_H

#include <gi/graphicsitem.h>
namespace ShapePr {

class Circle : public GraphicsItem {
public:
    explicit Circle(QPointF center, QPointF rh);
    ~Circle() override = default;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return Shape; }
    // GraphicsItem interface
    Paths paths() const override;
    void redraw() override;

    QPointF center() const;
    void setCenter(const QPointF& center);
    QPointF rh() const;
    void setRh(const QPointF& rh);
    double radius() const;
    void setRadius(double radius);

private:
    Path m_path;
    QPointF m_rh;
    double m_radius;
    QRectF handle();

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}

#endif // CIRCLE_H
