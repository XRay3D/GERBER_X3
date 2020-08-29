#pragma once

#include "shape.h"
#include <QFont>
#include <QIcon>
#include <abstractfile.h>

class ShTextDialog;

namespace Shapes {
class Text final : public Shape {
    friend ShTextDialog;
    friend Shape;

public:
    explicit Text(QPointF pt1);
    explicit Text() { }
    ~Text();

    // QGraphicsItem interface
    int type() const override { return GiShapeT; }
    void redraw() override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    QPainterPath shape() const override; // Shape interface
    QString name() const override { return QObject::tr("Text"); }
    QIcon icon() const override { return QIcon::fromTheme("draw-text"); };
    QPointF calcPos(Handler* sh) const override;

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

    QString text() const { return m_text; }
    void setText(const QString& value)
    {
        m_text = value;
        redraw();
    }

    Side side() const { return m_side; }
    void setSide(const Side& side)
    {
        m_side = side;
        redraw();
    }

protected:
    // Shape interface
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

private:
    void save()
    {
        copyAngle = angle;
        copyFont = font;
        copyHandleAlign = handleAlign;
        copyHeight = height;
        copySide = m_side;
        copyText = m_text;
    }

    void restore()
    {
        angle = copyAngle;
        font = copyFont;
        handleAlign = copyHandleAlign;
        height = copyHeight;
        m_side = copySide;
        m_text = copyText;
        redraw();
    }

    void ok()
    {
        font_ = font;
        side_ = m_side;
        angle_ = angle;
        height_ = height;
        handleAlign_ = handleAlign;
    }

    QString copyText;
    QString copyFont;
    Side copySide;
    double copyAngle;
    double copyHeight;
    int copyHandleAlign;

    QString m_text;
    QString font;
    Side m_side = Top;
    double angle = 0;
    double height = 10;
    int handleAlign = BotLeft;

    inline static QString font_;
    inline static Side side_ = Top;
    inline static double angle_ = 0;
    inline static double height_ = 10;
    inline static int handleAlign_ = BotLeft;
};

}
