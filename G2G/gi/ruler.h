#ifndef RULER_H
#define RULER_H

#include <QFont>
#include <QGraphicsItem>

class Ruler : public QGraphicsObject {
    Q_OBJECT
public:
    Ruler(const QPointF& point);
    ~Ruler();

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;

    void setPoint2(const QPointF& point2);

private:
    QPointF m_pt1;
    QPointF m_pt2;
    QString m_text;
    QRectF m_textRect;
    QFont m_font;
};

#endif // RULER_H
