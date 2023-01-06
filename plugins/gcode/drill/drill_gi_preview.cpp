// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "drill_gi_preview.h"
#include "utils.h"

namespace DrillPlugin {

GiPreview::GiPreview(PosOrPath&& hv, double diameter, int toolId, Row& row, const Paths& draw_)
    : hv {std::move(hv)}
    , row {row}
    , toolId_ {toolId} {
    sourceDiameter_ = diameter;
    auto draw = Overload {
        [this](const QPolygonF& val) {
            QPainterPath painterPath;
            for (auto&& path : offset(val, sourceDiameter_))
                painterPath.addPolygon(path);
            return painterPath;
        },
        [this](const QPointF& val) {
            QPainterPath painterPath;
            painterPath.addPolygon(CirclePath(sourceDiameter_ * uScale));
            return painterPath;
        },
    };

    auto setPos_ = Overload {
        [this](const QPolygonF& val) {},
        [this](const QPointF& val) { setPos(val); },
    };

    if (draw_.size()) {
        for (auto&& path : draw_)
            sourcePath_.addPolygon(path);
    } else {
        sourcePath_ = std::visit(draw, hv);
    }

    std::visit(setPos_, hv);

    row.items.emplace_back(this);
    update();
}

void GiPreview::updateTool() {
    if (toolId() > -1) {
        colorState |= Tool;

        auto draw = Overload {
            [this](const QPolygonF& val) {
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
            },
            [this](const QPointF& val) {
                QPainterPath painterPath;
                auto& tool(App::toolHolder().tool(toolId()));
                const double diameter = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;
                painterPath.moveTo(-QPointF(0.0, lineKoeff));
                painterPath.lineTo(+QPointF(0.0, lineKoeff));
                painterPath.moveTo(-QPointF(lineKoeff, 0.0));
                painterPath.lineTo(+QPointF(lineKoeff, 0.0));
                painterPath.addEllipse({}, diameter * .5, diameter * .5);
                return painterPath;
            },
        };
        toolPath_ = std::visit(draw, hv);
    } else {
        colorState &= ~Tool;
        toolPath_ = {};
    }
    changeColor();
}

Paths GiPreview::paths() const {
    auto getPath = Overload {
        [this](const QPointF& val) {
            //            auto path { CirclePath(sourceDiameter_ * uScale, val) };
            //            return ReversePath(path);

            Paths paths {sourcePath_.translated(val).toSubpathPolygons()};
            return ReversePaths(paths);
        },
        [](const QPolygonF& val) { return Paths {val}; },
    };
    return std::visit(getPath, hv);
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

int GiPreview::type() const { return int(GiType::Preview) + hv.index(); }

bool GiPreview::isSlot() const { return hv.index(); }

Paths GiPreview::offset() const {
    return offset(paths().front(), sourceDiameter_);
}

} // namespace DrillPlugin
