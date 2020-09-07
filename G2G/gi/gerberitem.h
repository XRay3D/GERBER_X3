#pragma once

#include "graphicsitem.h"

namespace Gerber {
class File;
}

class GerberItem : public GraphicsItem {
public:
    explicit GerberItem(Paths& m_paths, Gerber::File* file);
    ~GerberItem() override;

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
};
