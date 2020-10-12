#pragma once

#include "graphicsitem.h"

namespace Gerber {
class File;
}

class GiGerber final : public GraphicsItem {
public:
    explicit GiGerber(Paths& m_paths, Gerber::File* file);
    ~GiGerber() override;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // GraphicsItem interface
    void redraw() override;
    Paths paths() const override;
    Paths* rPaths() override;

private:
    Paths& m_paths;
    QPolygonF fillPolygon;

    // GraphicsItem interface
protected:
    void changeColor() override;
};
