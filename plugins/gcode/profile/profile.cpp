// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "profile.h"
#include "app.h"
#include "gc_gi_bridge.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "graphicsview.h"
#include "project.h"
#include "utils.h"

#include <execution>

namespace Profile {

void Creator::create() {
    // WARNING App::fileTreeView().closeFiles();
    createProfile(gcp_.tools.front(), gcp_.params[GCode::Params::Depth].toDouble());
}

void Creator::createProfile(const Tool& tool, const double depth) {
    do {

        toolDiameter = tool.getDiameter(depth);

        const double dOffset = ((gcp_.side() == GCode::Outer) ? +toolDiameter : -toolDiameter) /** 0.5*/ * uScale;

        if(gcp_.side() == GCode::On) {
            if(gcp_.params[TrimmingOpenPaths].toBool())
                trimmingOpenPaths(openSrcPaths);
            returnPs = std::move(closedSrcPaths);
        } else {
            if(closedSrcPaths.size()) {
                // ClipperOffset offset;
                // for(Paths& paths: groupedPaths(GCode::Grouping::Copper))
                //     offset.AddPaths(paths, JT::Round, ET::Polygon);
                // returnPs = offset.Execute(dOffset);
                auto it = std::views::join(groupedPaths(GCode::Grouping::Copper));
                returnPs = Inflate(Paths{it.begin(), it.end()}, dOffset, JT::Round, ET::Polygon);
            }
            if(openSrcPaths.size()) {
                // ClipperOffset offset;
                // offset.AddPaths(openSrcPaths, JT::Round, ET::Round);
                // openSrcPaths = offset.Execute(dOffset);
                openSrcPaths = Inflate(openSrcPaths, dOffset, JT::Round, ET::Round);
                if(!openSrcPaths.empty())
                    returnPs += openSrcPaths;
            }
        }

        if(returnPs.empty() && openSrcPaths.empty())
            break;

        reorder();

        if(gcp_.side() == GCode::On && openSrcPaths.size()) {
            returnPss.reserve(returnPss.size() + openSrcPaths.size());
            mergePaths(openSrcPaths);
            sortBeginEnd(openSrcPaths, ~(App::home().pos() + App::zero().pos()));
            for(auto&& path: openSrcPaths)
                returnPss.push_back({std::move(path)});
        }

        makeBridges();

        if(gcp_.params.contains(TrimmingCorners) && gcp_.params[TrimmingCorners].toInt())
            cornerTrimming();

        if(returnPss.empty())
            break;

        file_ = new File{std::move(gcp_), std::move(returnPss)};
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
        return;
    } while(0);
    emit fileReady(nullptr);
}

void Creator::trimmingOpenPaths(Paths& paths) {
    const double dOffset = toolDiameter * uScale * 0.5;
    for(size_t i = 0; i < paths.size(); ++i) {
        auto& p = paths[i];
        if(p.size() == 2) {
            double l = distTo(p.front(), p.back());
            if(l <= toolDiameter * uScale) {
                paths -= i--;
                continue;
            }
            QLineF b{~p.front(), ~p.back()};
            QLineF e{~p.back(), ~p.front()};
            b.setLength(b.length() - toolDiameter * 0.5);
            e.setLength(e.length() - toolDiameter * 0.5);
            p = Path{~b.p2(), ~e.p2()};
        } else if(double l = Perimeter(p); l <= toolDiameter * uScale) {
            paths -= i--;
            continue;
        } else {
            Paths ps;
            {
                // ClipperOffset offset;
                // offset.AddPath();
                // ps = offset.Execute(dOffset + 100);
                ps = Inflate({p}, dOffset + 100, JT::Miter, ET::Butt);

                // offset.Clear();
                // offset.AddPath(ps.front(), JT::Miter, ET::Polygon);
                // ps = offset.Execute(-dOffset);
                ps = Inflate({ps.front()}, -dOffset, JT::Miter, ET::Polygon);
                if(ps.empty()) {
                    paths -= i--;
                    continue;
                }
            }
            {
                Clipper clipper;
                clipper.AddOpenSubject({p});
                clipper.AddClip(ps);
                clipper.Execute(ClipType::Intersection, FillRule::Positive, ps, ps); // FIXME open paths ???
                p = ps.front();
            }
        }
    }
}

void Creator::cornerTrimming() {
    Timer<mS> t{};
    const double trimDepth = (toolDiameter - toolDiameter * sqrt1_2) * sqrt1_2;
    const double sqareSide = toolDiameter * sqrt1_2 * 0.5;
    const double testAngle = gcp_.convent() ? 90.0 : 270.0;
    const double trimAngle = gcp_.convent() ? -45.0 : +45;

#if _ITERATOR_DEBUG_LEVEL == /*0*/ 100 // FIXME
    auto insert = [=](auto& path, auto cornerPrev, auto&& corner, auto cornerNext) {
        QLineF l1{~*cornerPrev, ~*corner};
        QLineF l2{~*corner, ~*cornerNext};
        if(abs(l1.angleTo(l2) - testAngle) < 1.e-3                     // Angle is 90
            && sqareSide <= l1.length() && sqareSide <= l2.length()) { // Dog bone fit in
            l2.setAngle(l1.angle() + trimAngle), l2.setLength(trimDepth);
            auto it = std::find(path.begin(), path.end(), *corner);
            path.insert(it, {l2.p1(), l2.p2()});
            std::advance(corner, 2);
        }
    };

    auto paths = std::views::join(returnPss);

    std::for_each(std::execution::par_unseq, std::begin(paths), std::end(paths), [insert](Path& path) {
        path.reserve(path.size() * 3);
        for(auto corner = std::next(path.begin()); corner < std::prev(path.end()); ++corner)
            insert(path, std::prev(corner), corner, std::next(corner));
        if(path.front() == path.back()) // for trimming between the beginning and the end of the path
            insert(path, std::prev(path.end(), 2), std::prev(path.end()), std::next(path.begin()));
        path.shrink_to_fit();
    });
#else
    auto insert = [=](Path& path, auto cornerPrev, auto&& corner, auto cornerNext) {
        QLineF l1{~*cornerPrev, ~*corner};
        QLineF l2{~*corner, ~*cornerNext};
        if(abs(l1.angleTo(l2) - testAngle) < 1.e-3                     // Angle is 90
            && sqareSide <= l1.length() && sqareSide <= l2.length()) { // Dog bone fit in
            l2.setAngle(l1.angle() + trimAngle), l2.setLength(trimDepth);
            auto tmp = {~l2.p1(), ~l2.p2()};
            auto it = path.begin() + std::distance(path.data(), corner);
            path.insert(it, tmp.begin(), tmp.end());
            std::advance(corner, 2);
        }
    };

    auto paths = std::views::join(returnPss);

    std::for_each(std::execution::par_unseq, std::begin(paths), std::end(paths), [insert](Path& path) {
        path.reserve(path.size() * 3);
        for(Point* corner = std::next(path.data()); corner < std::prev(path.data() + path.size()); ++corner)
            insert(path, std::prev(corner), corner, std::next(corner));
        if(path.front() == path.back()) // for trimming between the beginning and the end of the path
            insert(path, &path.back() - 1, &path.back(), &path.front() + 1);
        path.shrink_to_fit();
    });
#endif
}

void Creator::makeBridges() {
    auto bridgeItems{App::grView().items<Gi::Bridge>(Gi::Type::Bridge)};
    if(bridgeItems.empty())
        return;

    std::for_each(std::execution::par_unseq, returnPss.begin(), returnPss.end(), [&bridgeItems, this](Paths& rPaths) -> void {
        // find Bridges
        auto biStack = bridgeItems | rviews::filter([&rPaths](Gi::Bridge* bi) { return bi->test(rPaths.front()); });
        if(ranges::empty(biStack)) return;
        auto isPositive1 = CL2::IsPositive(rPaths.front());

        // create frame
        Paths frame = Inflate(rPaths, toolDiameter * uScale * 0.1, JT::Miter, ET::Butt, uScale);
        Paths clip;
        for(Gi::Bridge* bip: biStack)
            clip += bip->paths();

        frame = CL2::Intersect(frame, clip, FR::Positive);

        // cut toolPath
        Clipper clipper;
        clipper.AddOpenSubject(rPaths);
        clipper.AddClip(frame);
        PolyTree polytree;
        clipper.Execute(CT::Difference, FR::Positive, frame, rPaths);

        if(rPaths.empty())
            return;

        mergePaths(rPaths);
        sortBeginEnd(rPaths, ~(App::home().pos() + App::zero().pos()));

        auto IsPositive = [](Paths paths) {
            for(auto&& path: paths | std::views::drop(1))
                paths.front() += std::move(path); // NOTE  move?
            return CL2::IsPositive(paths.front());
        };

        if(isPositive1 ^ IsPositive(rPaths)) // Вернуть исходное направление пути
            ReversePaths(rPaths), ranges::reverse(rPaths);
    });

    std::erase_if(returnPss, [](auto&& paths) { return paths.empty(); });
}

void Creator::reorder() {
    //    returnPss = {returnPs};
    //    return;
    PolyTree polyTree;
    {
        Clipper clipper;
        clipper.AddSubject(returnPs);
        Rect r(GetBounds(returnPs));
        int k = uScale;
        Path outer = {
            { r.left - k, r.bottom + k},
            {r.right + k, r.bottom + k},
            {r.right + k,    r.top - k},
            { r.left - k,    r.top - k}
        };
        clipper.AddSubject({outer});
        clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
        returnPs.clear();
    }

    polyTreeToPaths(polyTree, returnPs);

    std::reverse(returnPs.begin(), returnPs.end());

    if((gcp_.side() == GCode::Inner) ^ gcp_.convent())
        ReversePaths(returnPs);

    returnPss.reserve(returnPs.size());

    for(auto&& path: returnPs) {
        path.push_back(path.front());
        returnPss.push_back({path});
    }
}

void Creator::reduceDistance(Point& from, Path& to) {
    double d = std::numeric_limits<double>::max();
    int ctr2 = 0, idx = 0;
    for(auto pt2: to) {
        if(auto tmp = distToSq(from, pt2); d > tmp) {
            d = tmp;
            idx = ctr2;
        }
        ++ctr2;
    }
    std::rotate(to.begin(), to.begin() + idx, to.end());
    from = to.back();
}

void Creator::polyTreeToPaths(PolyTree& polytree, Paths& rpaths) {
    rpaths.clear();

    //    auto Total = [i = 0](this auto&& total, PolyTree& polytree) mutable {
    //        return i;
    //    };
    //    rpaths.reserve(Total(polytree));

    std::function<void(PolyTree&, Creator::NodeType)> addPolyNodeToPaths;

    if(!settings.sort) { // Grouping by nesting

        markPolyTreeDByNesting(polytree);

        std::map<int, Paths> pathsMap;
        addPolyNodeToPaths = [&addPolyNodeToPaths, &pathsMap, this](PolyTree& polynode, Creator::NodeType nodetype) {
            bool match = true;
            if(nodetype == ntClosed)
                match = true; // NOTE ! polynode.IsOpen();
            else if(nodetype == ntOpen)
                return;

            if(!polynode.Polygon().empty() && match)
                pathsMap[nesting[&polynode]].emplace_back(std::move(polynode.Polygon()));

            for(auto&& node: polynode)
                addPolyNodeToPaths(*node, nodetype);
        };
        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);

        pathsMap.extract(pathsMap.begin());

        for(auto& [nest, paths]: pathsMap) {
            qDebug() << "nest" << nest << paths.size();
            if(paths.size() > 1)
                sortB(paths, ~(App::home().pos() + App::zero().pos()));
            rpaths += std::move(paths); // NOTE move?
        }
    } else { // Grouping by nesting depth
        sortPolyTreeByNesting(polytree);
        Point from = ~App::settings().mkrZeroOffset();
        std::function<void(PolyTree&, Creator::NodeType)> addPolyNodeToPaths =
            [&addPolyNodeToPaths, &rpaths, &from, this](PolyTree& polynode, Creator::NodeType nodetype) {
                bool match = true;
                if(nodetype == ntClosed)
                    match = true; // NOTE ! polynode.IsOpen();
                else if(nodetype == ntOpen)
                    return;

                if(!polynode.Polygon().empty() && match && nesting[std::addressof(polynode)] > 2) {
                    auto path{polynode.Polygon()};
                    reduceDistance(from, path);
                    rpaths.emplace_back(std::move(path));
                }

                //                std::map<int, std::vector<PolyTree*>, std::greater<>> map;
                //                for (auto node : polynode.Childs)
                //                    map[node->Nesting].emplace_back(node);
                //                size_t i = polynode.Count();
                //                for (auto& [nest, nodes] : map) {
                //                    for (auto node : nodes)
                //                        polynode.Childs[--i] = node;
                //                }
                for(auto&& node: rwPolyTree(polynode))
                    addPolyNodeToPaths(*node, nodetype);
            };

        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);
    }
}
////////////////////////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& gcp, Pathss&& toolPathss)
    : GCode::File(std::move(gcp), std::move(toolPathss)) {
    if(gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project().worckRect();
    for(size_t x = 0; x < App::project().stepsX(); ++x) {
        for(size_t y = 0; y < App::project().stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);

            if(toolType() == Tool::Laser)
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);

            if(gcp_.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {

    Gi::Item* item;
    for(const Paths& paths: toolPathss_) {
        item = new Gi::GcPath{paths, this};
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }

    for(size_t i{}; const Paths& paths: toolPathss_) {
        item = new Gi::GcPath{toolPathss_[i], this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j = 0; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < toolPathss_.size() - 1)
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
    }

    item = new Gi::GcPath{g0path_};
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);

    itemGroup()->setVisible(true);
}

} // namespace Profile
