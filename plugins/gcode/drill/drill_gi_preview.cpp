// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "drill_gi_preview.h"
#include "utils.h"

namespace Drilling {
namespace Gi {
Preview::Preview(Path&& path, double diameter, int toolId, Row& row, const Paths& draw_)
    : path_{std::move(path)}
    , row{row}
    , toolId_{toolId} {
    sourceDiameter_ = diameter;
    if(path_.size() > 1) {
        Timer<mS> t{__FUNCTION__};
        for(auto&& path_: Inflate(Paths{path_}, sourceDiameter_ * uScale, JoinType::Round, EndType::Round, uScale)
            /*offset(path_, sourceDiameter_)*/)
            sourcePath_.addPolygon(~path_);
    } else {
        for(auto&& path_: draw_)
            sourcePath_.addPolygon(~path_);
        //        setPos(hv_.front());
    }
    row.items.emplace_back(this);
    update();
}

void Preview::updateTool() {
    if(toolId() > -1) {
        colorState |= Tool;
        if(path_.size() > 1)
            toolPath_ = [this](const QPolygonF& val) {
                QPainterPath painterPath;
                auto& tool(App::toolHolder().tool(toolId()));
                const double diameter = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;
                for(Path& path_: Inflate(Paths{path_}, diameter * uScale, JoinType::Round, EndType::Round, uScale)) {
                    path_.push_back(path_.front());
                    painterPath.addPolygon(~path_);
                }
                QPolygonF path_(val);
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
                const double diameter = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;
                painterPath.moveTo(-QPointF(0.0, lineKoeff));
                painterPath.lineTo(+QPointF(0.0, lineKoeff));
                painterPath.moveTo(-QPointF(lineKoeff, 0.0));
                painterPath.lineTo(+QPointF(lineKoeff, 0.0));
                painterPath.addEllipse({}, diameter * .5, diameter * .5);
                return painterPath.translated(val);
            }(~path_.front());
    } else {
        colorState &= ~Tool;
        toolPath_ = {};
    }
    changeColor();
}

Paths Preview::paths() const {
    if(path_.size() > 1)
        return {path_};
    else
        return ~sourcePath_.toSubpathPolygons();
}

bool Preview::fit(double depth) const {
    const auto diameter = App::toolHolder().tool(toolId()).getDiameter(depth);
    return sourceDiameter_ > diameter && !qFuzzyCompare(sourceDiameter_, diameter);
}

int Preview::toolId() const {
    return toolId_ < 0 ? row.toolId : toolId_;
}

// Paths Preview::offset(const Path& path_, double offset) {
////    ClipperOffset cOffset;
////    // cpOffset.AddPath(path_, JoinType::Round, EndType::Round);
////    cOffset.AddPath(path_, JoinType::Round, EndType::Round);
////    Paths retPaths = cOffset.Execute(offset * uScale);
////    for (Path& path_ : retPaths)
////        path_.push_back(path_.front());
////    qDebug() << __FUNCTION__ << retPaths.size();
////    ReversePaths(retPaths);
//    return {}/*retPaths*/;
//}

int Preview::type() const { return int(::Gi::Type::Preview) + (path_.size() > 1); }

bool Preview::isSlot() const { return path_.size() > 1; }

Paths Preview::offset() const {
    return ~sourcePath_.toSubpathPolygons(); /*Inflate(Paths {hv_}, sourceDiameter_ * uScale, JoinType::Round, EndType::Round, uScale);*/
}

}

} // namespace Drilling::Gi
