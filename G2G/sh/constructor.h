#pragma once

#include <QPointF>

class QGraphicsItem;
class QAction;

namespace Shapes {

class Shape;

enum ShapeType {
    NullShape,
    Rect,
    PolyLine,
    Elipse,
    ArcPT,
    Text,
};

class Constructor {
    inline static ShapeType type;
    inline static int counter;
    inline static QPointF point;
    inline static Shape* item;
    inline static bool m_snap;
    inline static QAction* action;

public:
    static bool snap();
    static void setSnap(bool snap);

    static void addShapePoint(const QPointF& value);
    static void updateShape(const QPointF& value);
    static void finalizeShape();
    static void setType(const ShapeType& value, QAction* act);

private:
};

} // namespace ShapePr
