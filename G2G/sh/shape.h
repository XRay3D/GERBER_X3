#pragma once

#include <gi/graphicsitem.h>

namespace Shapes {

class Handler;
class Constructor;

class Shape : public GraphicsItem {
    friend class Handler;
    friend class Constructor;
    friend QDataStream& operator<<(QDataStream& stream, const Shape& sh);
    friend QDataStream& operator>>(QDataStream& stream, Shape& sh);
    //    {
    //        stream << sh.m_id;
    //        stream << sh.sh.size();
    //        for (Shapes::Handler* item : sh.sh) {
    //            stream << item->pos();
    //            stream << item->center;
    //        }
    //        write(stream);
    //        return stream;
    //    }

public:
    Shape();
    ~Shape();
    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    Paths paths() const override;

    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;
    virtual QPointF calcPos(Handler* sh) = 0;

private:
    mutable QPainterPath m_selectionShape;

protected:
    mutable double m_scale = std::numeric_limits<double>::max();
    mutable QVector<Handler*> sh;
    Paths m_paths;

    // QGraphicsItem interface
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual void write(QDataStream& stream) const = 0;
    virtual void read(QDataStream& stream) = 0;
};
}
