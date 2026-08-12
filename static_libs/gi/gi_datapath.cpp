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
#include "gi_datapath.h"

#include "abstract_file.h"
#include "project.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <numbers>
#include <set>

namespace Gi {

// Ширина «прицела» -- 5 экранных пикселей. zoomOut() ограничен
// getScale() >= 1.0 (graphicsview.cpp), значит sf <= 1 мм/px и обводка ни при
// каком масштабе не уходит дальше 2.5 мм от геометрии. Берём 3 мм с запасом на
// скруглённые стыки -- и получаем boundingRect, НЕ зависящий от масштаба.
//
// Плата за это -- раздутый габарит: трасса 0.2x5 мм получает 6.2x11 мм, куллинг
// грубее. Альтернатива (пересчёт запаса на каждом шаге зума с
// prepareGeometryChange) всё равно O(N) на шаг, и каждый вызов вынимает элемент
// из BSP и вставляет обратно -- строго дороже.
inline constexpr double kSelWidthPx = 5.0;
inline constexpr double kSelMarginMm = 3.0;

const QPainterPath& DataPath::selectionShape(double sf) const {
    if(!qFuzzyCompare(strokeScale_, sf)) {
        strokeScale_ = sf;
        // Штрихователь локальный: прежний был общим static'ом, который каждый
        // элемент мутировал под себя -- гонка, если DataPath создадут в
        // рабочем потоке.
        QPainterPathStroker str{
            {Qt::transparent, kSelWidthPx * sf, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin}
        };
        selectionShape_ = str.createStroke(shape_);
    }
    return selectionShape_;
}

void DataPath::geometryChanged() {
    prepareGeometryChange();
    boundingRect_ = shape_.boundingRect()
                        .adjusted(-kSelMarginMm, -kSelMarginMm, kSelMarginMm, kSelMarginMm);
    strokeScale_ = std::numeric_limits<double>::quiet_NaN();
}

DataPath::DataPath(Geo::Polylines curves, AbstractFile* file)
    : Item{file} {
    curves_ = std::move(curves);
    shape_ = Geo::toPath(curves_);
    geometryChanged();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QPainterPath DataPath::shape() const { return selectionShape(scaleFactor()); }

int DataPath::type() const { return Type::DataPath; }

void DataPath::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    Item::mouseReleaseEvent(event);

    // using v::filter;
    // using v::transform;
    // auto constexpr filter = v::filter([](auto* item) {
    // return item->type() == int(Type::DataPath);
    // });

    // auto constexpr transform = v::transform([](auto* item) {
    // return static_cast<Item*>(item);
    // });

    const auto glue = App::project().glue() /** uScale*/;

    // std::map<void*, Path> set;
    std::set<void*> set;
    std::function<void(Item*)> selector = [&](Item* item) {
        const auto& collidingItems = *itemGroup;
        // auto collidingItems = scene()->collidingItems(item, Qt::/*IntersectsItemBoundingRect*/ IntersectsItemBoundingRect/*IntersectsItemShape*/);
        auto pathFrom = item->curves().front();
        for(auto&& item: collidingItems /*| filter | transform*/) {
            auto pathTo = item->curves().front();
            // if(/*itemGroup->contains(item) &&*/ !set.contains(item)) {
            if(set.insert(item.get()).second) {
                // auto [i, _] = set.emplace(item, Path{pathTo.front(), pathTo.back()});

                const double min = r::min({
                    Geo::distance(pathFrom.back(), pathTo.back()),
                    Geo::distance(pathFrom.back(), pathTo.front()),
                    Geo::distance(pathFrom.front(), pathTo.back()),
                    Geo::distance(pathFrom.front(), pathTo.front()),
                });

                if(min > glue) continue;
                item->setSelected(true);
                selector(item.get());
            }
        }
    };

    if(event->modifiers() & Qt::ShiftModifier && itemGroup)
        selector(this);
}

// Узор «бегущих муравьёв». Файловая константа, а не литерал в paint():
// QList неявно разделяемый, копия в перо не аллоцирует -- прежний вариант
// строил новый QList<qreal> на каждый кадр каждого выделенного элемента.
static const QList<qreal> dashPattern{std::numbers::pi * 2, std::numbers::pi * 2 - 1};
// Полупрозрачная подсветка под курсором.
static const QBrush hoverBrush{
    QColor{255, 255, 255, 128}
};

void DataPath::paintHighlight(QPainter* painter, const RenderState& st) {
    if(!st.hovered) return;
    painter->setPen(Qt::NoPen);
    painter->setBrush(hoverBrush);
    painter->drawPath(selectionShape(st.sf));
}

void DataPath::paintGeometry(QPainter* painter, const RenderState& st) {
    painter->setBrush(Qt::NoBrush);

    if(st.plain()) [[likely]] { // быстрый путь: ни одного временного объекта
        painter->setPen(pen_);
        painter->drawPath(shape_);
        return;
    }

    QPen pen{pen_};
    pen.setWidthF(2 * st.sf);
    pen.setStyle(Qt::CustomDashLine);
    pen.setCapStyle(Qt::FlatCap);
    pen.setDashPattern(dashPattern);
    if(st.selected) {
        QColor color{pen_.color()};
        color.setAlpha(255);
        pen.setColor(color);
        pen.setDashOffset(st.dashOffset);
    }
    painter->setPen(pen);
    painter->drawPath(shape_);
}

} // namespace Gi
