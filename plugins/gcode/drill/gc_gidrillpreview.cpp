#include "gc_gidrillpreview.h"
#include "utils.h"

GiDrillPreview::GiDrillPreview(PosPath&& hv, double diameter, int toolId, Row& row, const Paths& draw_)
    : hv { std::move(hv) }
    , row { row }
    , toolId_ { toolId } {
    sourceDiameter_ = diameter;
    auto draw = Overload {
        [this](const QPolygonF& val) {
            qDebug(__FUNCTION__);
            QPainterPath painterPath;
            for (auto&& path : offset(val, sourceDiameter_))
                painterPath.addPolygon(path);
            return painterPath;
        },
        [this](const QPointF& val) {
            qDebug(__FUNCTION__);
            QPainterPath painterPath;
            const double radius = sourceDiameter_ * 0.5;
            painterPath.addEllipse({}, radius, radius);
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

void GiDrillPreview::updateTool() {
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
                offset.AddPath(val, jtRound, etOpenRound);
                offset.Execute(tmpPpath, diameter * 0.5 * uScale);
                for (Path& path : tmpPpath) {
                    path.push_back(path.front());
                    painterPath.addPolygon(path);
                }
                Path path(val);

                if (path.size()) {
                    for (auto&& point : path) {
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

Paths GiDrillPreview::paths() const {
    auto getPath = Overload {
        [this](const QPointF& val) {
            auto path { CirclePath(sourceDiameter_ * uScale, val) };
            return ReversePath(path);
        },
        [this](const QPolygonF& val) {
            return Path { val };
        },
    };
    return { std::visit(getPath, hv) };
}

bool GiDrillPreview::fit(double depth) const {
    const auto diameter = App::toolHolder().tool(toolId()).getDiameter(depth);
    return sourceDiameter_ > diameter && !qFuzzyCompare(sourceDiameter_, diameter);
}

int GiDrillPreview::toolId() const {
    return toolId_ < 0 ? row.toolId : toolId_;
}

Paths GiDrillPreview::offset(const Path& path, double offset) {
    ClipperOffset cpOffset;
    // cpOffset.AddPath(path, jtRound, etClosedLine);
    cpOffset.AddPath(path, jtRound, etOpenRound);
    Paths tmpPpaths;
    cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
    for (Path& path : tmpPpaths)
        path.push_back(path.front());
    return tmpPpaths;
}

int GiDrillPreview::type() const { return int(GiType::Preview) + hv.index(); }

bool GiDrillPreview::isSlot() const { return hv.index(); }
