#pragma once

#include <QPointF>

class QAction;

namespace Shapes {

class Constructor {
    inline static int type;
    inline static int counter;
    inline static QPointF point;
    inline static class Shape* item;
    inline static bool m_snap;
    inline static QAction* action;

public:
    static void addShapePoint(const QPointF& value);
    static void updateShape(const QPointF& value);
    static void finalizeShape();
    static void setType(const int value, QAction* act);
};

} // namespace ShapePr
