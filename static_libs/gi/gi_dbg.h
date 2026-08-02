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
    Paths64 paths_;
    std::set<QPointF> centers;

    Debug_(const Path64& path, const QColor& color = Qt::white, double width = {});
    Debug_(const Paths64& paths, const QColor& color = Qt::white, double width = {});
    Debug_(const QPainterPath& path, const QColor& color = Qt::white, double width = {});

public:
    ~Debug_() override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // Paths paths(int alternate = {}) const override;
    bool arrows{true};

#if DEBUG
    static Debug_* Debug(const QPainterPath& path, const QColor& color = Qt::white, double width = {}) {
        return new Debug_{path, color, width};
    }

    static Debug_* Debug(const Curves& curves, const QColor& color = Qt::white, double width = {}) {
        return new Debug_{toPPath(curves), color, width};
    }

    static Debug_* Debug(const Paths64& paths, const QColor& color = Qt::white, double width = {}) {
        return new Debug_{paths, color, width};
    }
#else
    static auto Debug(auto&&... args) {
        return std::unique_ptr<Debug_>(new Debug_{std::forward<decltype(args)>(args)...});
    }
#endif

private:
    QPainterPath arrows_;

protected:
    void changeColor() override { }
};

#if DEBUG
    #define Debug(...) Debug_::Debug(__VA_ARGS__)
#else
    #define Debug(...) Debug_::Debug(__VA_ARGS__).get()
#endif

} // namespace Gi
