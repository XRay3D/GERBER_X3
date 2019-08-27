#ifndef Rectangle_H
#define Rectangle_H

#include <gi/graphicsitem.h>
namespace ShapePr {

class Rectangle : public GraphicsItem {
public:
    explicit Rectangle(QPointF center, QPointF rh);
    ~Rectangle() override = default;

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

private:
    Path m_path;
    QPointF m_rh;

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}

#endif // Rectangle_H
