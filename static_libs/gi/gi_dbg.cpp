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
#include "gi_dbg.h"
#include "abstract_file.h"
#include "app.h"
#include "ft_model.h"
#include "graphicsview.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#define QT_DEBUG
// #undef QT_DEBUG

namespace Gi {

struct Node final : FileTree::Node {
    Debug_* giDbg;

    Node(Debug_* giDbg)
        : FileTree::Node{FileTree::File}, giDbg{giDbg} { }

    QVariant data(const QModelIndex& index, int role) const override {
        if(role == Qt::DecorationRole && !index.column())
            return drawIcon(giDbg->shape_, giDbg->pen_.color(), true);

        return {};
    }
    bool setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/) override {
        return {};
    }
    Qt::ItemFlags flags(const QModelIndex& /*index*/) const override {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    void menu(QMenu& /*menu*/, FileTree::View* /*tv*/) override { }
};

struct Dymmy final : AbstractFile {
    Debug_* giDbg;
    Dymmy(Debug_* giDbg)
        : giDbg{giDbg} { }

    // AbstractFile interface
    uint32_t type() const override { return Type::Debug; }
    void createGi() override { }
    FileTree::Node* node() override { return giDbg->node; }
    QIcon icon() const override { return {}; }
    void serialize(Serial::Writer& /*sb*/) const override { } // отладочный, не сохраняется
};

Debug_::Debug_(const QColor& color, double width) {
    pen_ = {color, width};
    App::grView().addItem(this);
    setZValue(std::numeric_limits<double>::max());
    setVisible(true);
}

Debug_::Debug_(const Geo::Polyline& polyline, const QColor& color, double width)
    : Debug_{Geo::Polylines{polyline}, color, width} { }

Debug_::Debug_(const Geo::Polylines& polylines, const QColor& color, double width)
    : Debug_{color, width} {
    curves_ = polylines;
    shape_ = Geo::toPath(curves_);
    boundingRect_ = shape_.boundingRect();

    if(shape_.isEmpty()) {
#if DEBUG
        delete this; // NOTE
#endif
        return;
    }

    // node = new Node{this};
    // Dymmy dymmy{this};
    // App::fileModel().addFile(&dymmy);
}

Debug_::Debug_(const QPainterPath& path, const QColor& color, double width)
    : Debug_{color, width} {
    shape_ = path;
    boundingRect_ = shape_.boundingRect();

    if(shape_.isEmpty()) {
#if DEBUG
        delete this; // NOTE
#endif
        return;
    }

    // node = new Node{this};
    // Dymmy dymmy{this};
    // App::fileModel().addFile(&dymmy);
}

Debug_::~Debug_() {
    // node->giDbg = nullptr;
    // delete node;
}

QRectF Debug_::boundingRect() const { return boundingRect_; }

void Debug_::paintGeometry(QPainter* painter, const RenderState& st) {
    // Цвета из указателей разрешает базовый Item::paint.
    const double scale = st.sf;

    QPen pen{pen_};
    if(pen_.widthF() == 0) {
        pen.setWidthF(1.5 * scale);
        painter->setPen(pen);
    } else
        painter->setPen(pen_);

    // if(option->state & QStyle::State_MouseOver) {
    // QPen pen{pen_};
    // pen.setWidthF(2.0 * scale);
    // pen.setStyle(Qt::CustomDashLine);
    // pen.setCapStyle(Qt::FlatCap);
    // pen.setDashPattern({2.0, 2.0});
    // painter->setPen(pen);
    // }

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(shape_);

    // painter->setPen(QPen(Qt::magenta, 0.0));
    // painter->drawRect(rect_);

    // pen.setColor({0, 255, 0, 255});
    // painter->setPen(pen);

    double len = 10 * scale;

    // Центры дуг. В bulge-виде они не хранятся, а восстанавливаются из пары
    // точек и прогиба -- ровно то, что раньше приезжало в Z-координате
    // клипперовской точки и доставалось через GetC.

    for(const Geo::Polyline& polyline: curves_)
        for(std::size_t i{}; i < polyline.size(); ++i) {
            const Geo::Vertex& from = polyline[i];
            if(!from.isArc()) continue;
            if(i + 1 == polyline.size() && !polyline.closed) break;
            const QPointF to = polyline[(i + 1) % polyline.size()];
            if(auto a = Geo::arcOf(from, to, from.bulge))
                painter->drawLine(a->center, static_cast<const QPointF&>(from));
        }

    for(const QPointF& p: centers) {
        painter->drawLines({
            {p, p + QPointF{.0, -len}},
            {p, p + QPointF{.0, +len}},
            {p, p + QPointF{-len, .0}},
            {p, p + QPointF{+len, .0}},
        });
    }

    ////////////////////////////////////////////////////// for Debug_ cut direction
    if(arrows) {
        if(auto ar = updateArrows()) arrows_ = std::move(*ar); // for direction
        painter->drawPath(arrows_);
    }
}

int Debug_::type() const { return Type::Debug; }

// Paths Debug_::paths(int) const { return {} /*paths_*/; }

} // namespace Gi
