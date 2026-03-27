/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
#include <qpainterpath.h>

namespace Gi {

class Debug_ final : public Item {
    friend struct Node;
    friend struct Dymmy;
    struct Node* node;

    friend Debug_* Debug(const Curves&, const QColor&, double);
    friend Debug_* Debug(const Paths&, const QColor&, double);
    friend Debug_* Debug(const QPainterPath&, const QColor&, double);

    Debug_(const QColor& color, double width);
    Paths paths_;
    std::set<QPointF> centers;

    Debug_(const Path& path, const QColor& color = Qt::white, double width = {});
    Debug_(const Paths& paths, const QColor& color = Qt::white, double width = {});
    Debug_(const QPainterPath& path, const QColor& color = Qt::white, double width = {});

public:
    ~Debug_() override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths(int alternate = {}) const override;
    bool arrows{true};

private:
    QPainterPath arrows_;

protected:
    void changeColor() override { }
};

inline Debug_* Debug(const QPainterPath& path, const QColor& color = Qt::white, double width = {}) {
    return new Debug_{path, color, width};
}

inline Debug_* Debug(const Curves& curves, const QColor& color = Qt::white, double width = {}) {
    return new Debug_{toPPath(curves), color, width};
}

inline Debug_* Debug(const Paths& paths, const QColor& color = Qt::white, double width = {}) {
    return new Debug_{paths, color, width};
}

} // namespace Gi
