#pragma once
//#ifndef SHAPES_H
//#define SHAPES_H

#include <QPointF>
class QGraphicsItem;
class QAction;
namespace ShapePr {

enum PrType {
    NullPT,
    Rect,
    Line,
    Elipse,
    ArcPT,
    Text,
};

class Constructor {
    static PrType type;
    static int counter;
    static QPointF point;
    static QGraphicsItem* item;
    static bool m_snap;

public:
    static void addShapePoint(const QPointF& value);
    static void updateShape(const QPointF& value);

    static PrType getType();
    static void setType(const PrType& value, QAction* act);

    static bool snap();
    static void setSnap(bool snap);

private:
    static QAction* action;
};

} // namespace ShapePr
//#endif // SHAPES_H
