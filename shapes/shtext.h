#pragma once

#include "shape.h"
#include <abstractfile.h>

class ShTextDialog;

namespace Shapes {
class Text final : public Shape {
    friend ShTextDialog;
    friend Shape;

public:
    explicit Text(QPointF pt1);
    explicit Text() { }
    ~Text() = default;

    enum {
        BotCenter = 1,
        BotLeft,
        BotRight,
        Center,
        CenterLeft,
        CenterRight,
        TopCenter,
        TopLeft,
        TopRight,
    };

    struct InternalData {
        friend QDataStream& operator<<(QDataStream& stream, const InternalData& d);
        friend QDataStream& operator>>(QDataStream& stream, InternalData& d);
        InternalData()
            : font("")
            , text("Text")
            , side(Top)
            , angle(0.0)
            , height(10.0)
            , handleAlign(BotLeft)
        {
        }
        QString font;
        QString text;
        Side side;
        double angle;
        double height;
        int handleAlign;
    };

    // QGraphicsItem interface
    int type() const override { return GiShapeT; }
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override; // Shape interface

    // Shape interface
    void redraw() override;
    QString name() const override;
    QIcon icon() const override;

    QString text() const;
    void setText(const QString& value);

    Side side() const;
    void setSide(const Side& side);

protected:
    // Shape interface
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    QPointF calcPos(Handler* sh) override;

private:
    InternalData d;
    InternalData dCopy;
    inline static InternalData d_;

    const QString fileName;

    void save();
    void restore();
    void ok();
};

}
