/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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
    friend class std::unique_ptr<Debug_>;
    struct Node* node;

    Debug_(const QColor& color, double width);
    std::set<QPointF> centers;

    Debug_(const Geo::Polyline& polyline, const QColor& color = Qt::white, double width = {});
    Debug_(const Geo::Polylines& polylines, const QColor& color = Qt::white, double width = {});
    Debug_(const QPainterPath& path, const QColor& color = Qt::white, double width = {});

public:
    ~Debug_() override;
    QRectF boundingRect() const override;
    void paintGeometry(QPainter* painter, const RenderState& st) override;
    int type() const override;
    // Paths paths(int alternate = {}) const override;
    bool arrows{true};

#if DEBUG
    static Debug_* Debug(const QPainterPath& path, const QColor& color = Qt::white, double width = {}) {
        return new Debug_{path, color, width};
    }

    static Debug_* Debug(const Geo::Polygons& polygons, const QColor& color = Qt::white, double width = {}) {
        return new Debug_{polygons.toPath(), color, width};
    }

    static Debug_* Debug(const Geo::Polylines& polylines, const QColor& color = Qt::white, double width = {}) {
        return new Debug_{polylines, color, width};
    }
#else
    static auto Debug(auto&&... args) {
        return std::unique_ptr<Debug_>(new Debug_{std::forward<decltype(args)>(args)...});
    }
#endif

private:
    QPainterPath arrows_;

protected:
    void updateColors() override { }
};

#if DEBUG
    #define Debug(...) Debug_::Debug(__VA_ARGS__)
#else
    #define Debug(...) Debug_::Debug(__VA_ARGS__).get()
#endif

} // namespace Gi
