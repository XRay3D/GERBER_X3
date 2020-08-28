#pragma once

#include <QPointF>

class QGraphicsItem;
class QAction;

namespace Shapes {

class Shape;


class Constructor {
    inline static int type;
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
    static void setType(const int value, QAction* act);

private:
};

} // namespace ShapePr
