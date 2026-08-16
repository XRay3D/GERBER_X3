/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thr_gi_preview.h"
#include "geo/boolean.h"

namespace Threading {
namespace Gi {
Preview::Preview(Geo::Polyline&& path, double diameter, Tool::ID toolId, Row& row, const Geo::Polylines& draw_)
    : path_{std::move(path)}
    , row{row}
    , toolId_{toolId} {
    sourceDiameter_ = diameter;
    if(path_.size() > 1)
        // Паз: тело -- осевая линия, раздутая на всю свою ширину. delta у
        // Geo::Inflate это ПОЛНАЯ ширина полосы, поэтому здесь идёт сам
        // диаметр, а не его половина.
        source_ = Geo::Inflate(Geo::Polylines{path_}, sourceDiameter_).contours();
    else
        // Отверстие: контуры уже посчитаны источником (заливка вспышки
        // апертуры либо путь фигуры), раздувать нечего.
        source_ = draw_;
    sourcePath_ = source_.toPath();
    row.items.emplace_back(this);
    update();
}

void Preview::updateTool() {
    if(toolId() > Tool::ID{}) {
        colorState |= ColorState::Tool;
        const auto& tool = App::toolHolder().tool(toolId());
        const double diameter = tool.getDiameter(tool.getDepth());
        // Перекрестье в центре -- отметка врезки; вылет за габарит инструмента
        // делает её видимой и на мелких отверстиях.
        const double lineKoeff = diameter * 0.7;
        auto crossAt = [lineKoeff](QPainterPath& painterPath, QPointF point) {
            painterPath.moveTo(point - QPointF{0.0, lineKoeff});
            painterPath.lineTo(point + QPointF{0.0, lineKoeff});
            painterPath.moveTo(point - QPointF{lineKoeff, 0.0});
            painterPath.lineTo(point + QPointF{lineKoeff, 0.0});
        };
        QPainterPath painterPath;
        if(path_.size() > 1) {
            // Паз: контур, который опишет инструмент своим диаметром, плюс сама
            // осевая линия и перекрестья в её вершинах.
            painterPath = Geo::Inflate(Geo::Polylines{path_}, diameter).toPath();
            for(const Geo::Vertex& vertex: path_)
                crossAt(painterPath, vertex);
            painterPath.addPath(path_.toPath());
        } else {
            crossAt(painterPath, {});
            painterPath.addEllipse({}, diameter * .5, diameter * .5);
            painterPath.translate(path_.front());
        }
        toolPath_ = painterPath;
    } else {
        colorState &= ~ColorState::Tool;
        toolPath_ = {};
    }
    changeColor();
}

Geo::Polylines Preview::paths() const {
    // У паза наружу идёт ОСЕВАЯ линия: по ней инструмент и пойдёт, а раздутое
    // тело нужно лишь для вида. У отверстия осевая -- одна точка, пути из неё
    // нет, поэтому отдаётся его контур.
    if(path_.size() > 1)
        return {path_};
    else
        return source_;
}

bool Preview::fit(double depth) const {
    const auto diameter = App::toolHolder().tool(toolId()).getDiameter(depth) * 0.999;
    return sourceDiameter_ > diameter; // && !qFuzzyCompare(sourceDiameter_, diameter); FIXME logic
}

Tool::ID Preview::toolId() const { return (toolId_ < Tool::ID::Tool) ? row.toolId : toolId_; }

int Preview::type() const { return int(::Gi::Type::Preview) + (path_.size() > 1); }

bool Preview::isSlot() const { return path_.size() > 1; }

Geo::Polylines Preview::offset() const { return source_; }

void Preview::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    colorState ^= CS::Used;
    changeColor();
}

}

} // namespace Threading::Gi
