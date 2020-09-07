#pragma once

#include "graphicsitem.h"

namespace Excellon {
class File;
class Hole;
}
namespace GCode {
class File;
}

class DrillItem : public GraphicsItem {
public:
    DrillItem(Excellon::Hole* hole, Excellon::File* file);
    DrillItem(double diameter, GCode::File* file);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    bool isSlot();

    double diameter() const;
    void setDiameter(double diameter);
    void updateHole();

    //    const Excellon::File* file() const
    //    {
    //        if (m_hole)
    //            return m_hole->file;
    //        return nullptr;
    //    }

    // GraphicsItem interface
    Paths paths() const override;

private:
    void create();
    Excellon::Hole* const m_hole = nullptr;
    double m_diameter = 0.0;
};
