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
#include "drill_gi_preview.h"
#include "utils.h"

namespace Drilling {
namespace Gi {
Preview::Preview(Geo::Polyline&& path, double diameter, Tool::ID toolId, Row& row, const Geo::Polylines& draw_)
    : path_{std::move(path)}
    , row{row}
    , toolId_{toolId} {

#if 0 // FIXME
   sourceDiameter_ = diameter;
    if(path_.size() > 1) {
        Timer<mS> t{__FUNCTION__};
        for(auto&& path_: Inflate64(Paths64{path_}, sourceDiameter_ * uScale, cl::JoinType::Round, cl::EndType::Round, uScale)
            /*offset(path_, sourceDiameter_)*/)
            sourcePath_.addPolygon(~path_);
    } else {
        for(auto&& path_: draw_)
            sourcePath_.addPolygon(~path_);
        // setPos(hv_.front());
    }
    row.items.emplace_back(this);
#endif

    update();
}

void Preview::updateTool() {

#if 0 // FIXME
  if(toolId() > Tool::ID{}) {
        colorState |= ColorState::Tool;
        if(path_.size() > 1)
            toolPath_ = [this](const QPolygonF& val) {
                QPainterPath painterPath;
                auto& tool(App::toolHolder().tool(toolId()));
                const double diameter  = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;
                for(Geo::Polyline& path_: Inflate64(Paths64{path_}, diameter * uScale, cl::JoinType::Round, cl::EndType::Round, uScale)) {
                    path_.push_back(path_.front());
                    painterPath.addPolygon(~path_);
                }
                QPolygonF path_{val};
                if(path_.size()) {
                    for(QPointF point: path_) {
                        painterPath.moveTo(point - QPointF(0.0, lineKoeff));
                        painterPath.lineTo(point + QPointF(0.0, lineKoeff));
                        painterPath.moveTo(point - QPointF(lineKoeff, 0.0));
                        painterPath.lineTo(point + QPointF(lineKoeff, 0.0));
                    }
                    painterPath.addPolygon(path_);
                }
                return painterPath;
            }(~path_);
        else
            toolPath_ = [this](const QPointF& val) {
                QPainterPath painterPath;
                auto& tool(App::toolHolder().tool(toolId()));
                const double diameter  = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;
                painterPath.moveTo(-QPointF(0.0, lineKoeff));
                painterPath.lineTo(+QPointF(0.0, lineKoeff));
                painterPath.moveTo(-QPointF(lineKoeff, 0.0));
                painterPath.lineTo(+QPointF(lineKoeff, 0.0));
                painterPath.addEllipse({}, diameter * .5, diameter * .5);
                return painterPath.translated(val);
            }(~path_.front());
    } else {
        colorState &= ~ColorState::Tool;
        toolPath_ = {};
    }
#endif

    changeColor();
}

Geo::Polylines Preview::paths() const {
    if(path_.size() > 1)
        return {path_};

#if 0 // FIXME
    else
        return ~sourcePath_.toSubpathPolygons();
#endif
}

bool Preview::fit(double depth) const {
    const auto diameter = App::toolHolder().tool(toolId()).getDiameter(depth) * 0.999;
    return sourceDiameter_ > diameter; // && !qFuzzyCompare(sourceDiameter_, diameter); FIXME logic
}

Tool::ID Preview::toolId() const { return (toolId_ < Tool::ID::Tool) ? row.toolId : toolId_; }

int Preview::type() const { return int(::Gi::Type::Preview) + (path_.size() > 1); }

bool Preview::isSlot() const { return path_.size() > 1; }

Geo::Polylines Preview::offset() const {

#if 0 // FIXME
    return ~sourcePath_.toSubpathPolygons();

#endif
    /*Inflate(Paths64 {hv_}, sourceDiameter_ * uScale, cl::JoinType::Round, cl::EndType::Round, uScale);*/
}

void Preview::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    colorState ^= CS::Used;
    changeColor();
}

}

} // namespace Drilling::Gi
