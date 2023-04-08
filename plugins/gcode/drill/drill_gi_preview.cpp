// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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

GiPreview::GiPreview(Path&& hv, double diameter, int toolId, Row& row, const Paths& draw_)
    : hv_ {std::move(hv)}
    , row {row}
    , toolId_ {toolId} {
    sourceDiameter_ = diameter;
    if (hv_.size() > 1) {
        for (auto&& path : offset(hv, sourceDiameter_))
            sourcePath_.addPolygon(path);
    } else {
        for (auto&& path : draw_)
            sourcePath_.addPolygon(path);
        //        setPos(hv_.front());
    }
    row.items.emplace_back(this);
    update();
}

void GiPreview::updateTool() {
    if (toolId() > -1) {
        colorState |= Tool;
        if (hv_.size() > 1)
            toolPath_ = [this](const QPolygonF& val) {
                QPainterPath painterPath;
                auto& tool(App::toolHolder().tool(toolId()));
                const double diameter = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;
                Paths tmpPpath;
                ClipperOffset offset;
                offset.AddPath(Path {val}, JoinType::Round, EndType::Round);
                tmpPpath = offset.Execute(diameter * 0.5 * uScale);
                for (Path& path : tmpPpath) {
                    path.push_back(path.front());
                    painterPath.addPolygon(path);
                }
                Path path(val);

                if (path.size()) {
                    for (QPointF point : path) {
                        painterPath.moveTo(point - QPointF(0.0, lineKoeff));
                        painterPath.lineTo(point + QPointF(0.0, lineKoeff));
                        painterPath.moveTo(point - QPointF(lineKoeff, 0.0));
                        painterPath.lineTo(point + QPointF(lineKoeff, 0.0));
                    }
                    painterPath.addPolygon(path);
                }
                return painterPath;
            }(hv_);
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
            }(hv_.front());
    } else {
        colorState &= ~Tool;
        toolPath_ = {};
    }
    changeColor();
}

Paths GiPreview::paths() const {
    if (hv_.size() > 1)
        return [this](const QPointF& val) {
            Paths paths {sourcePath_.translated(val).toSubpathPolygons()};
            return ReversePaths(paths);
        }(hv_.front());
    else
        return sourcePath_.toSubpathPolygons(QTransform::fromTranslate(x(), y()));
}

bool GiPreview::fit(double depth) const {
    const auto diameter = App::toolHolder().tool(toolId()).getDiameter(depth);
    return sourceDiameter_ > diameter && !qFuzzyCompare(sourceDiameter_, diameter);
}

int GiPreview::toolId() const {
    return toolId_ < 0 ? row.toolId : toolId_;
}

Paths GiPreview::offset(const Path& path, double offset) {
    ClipperOffset cOffset;
    // cpOffset.AddPath(path, JoinType::Round, EndType::Round);
    cOffset.AddPath(path, JoinType::Round, EndType::Round);
    Paths retPaths = cOffset.Execute(offset * uScale);
    for (Path& path : retPaths)
        path.push_back(path.front());
    qDebug() << __FUNCTION__ << retPaths.size();
    ReversePaths(retPaths);
    return retPaths;
}

int GiPreview::type() const { return int(GiType::Preview) + (hv_.size() > 1); }

bool GiPreview::isSlot() const { return hv_.size() > 1; }

Paths GiPreview::offset() const {
    return offset(paths().front(), sourceDiameter_);
}

} // namespace Drilling
