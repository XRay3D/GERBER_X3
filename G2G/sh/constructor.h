#pragma once

#include <QPointF>
class QGraphicsItem;
class QAction;
namespace ShapePr {

class Shape;

enum PrType {
    NullPT,
    Rect,
    Pline,
    Elipse,
    ArcPT,
    Text,
};

class Constructor {
    static PrType type;
    static int counter;
    static QPointF point;
    static Shape* item;
    static bool m_snap;

public:
    static void addShapePoint(const QPointF& value);
    static void updateShape(const QPointF& value);
    static void finalizeShape();

    static PrType getType();
    static void setType(const PrType& value, QAction* act);

    static bool snap();
    static void setSnap(bool snap);

private:
    static QAction* action;
};

} // namespace ShapePr
