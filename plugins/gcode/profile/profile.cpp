/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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
#include <gi_dbg.h>

#ifdef Q_OS_UNIX
    #undef emit
    #include <execution>
    #define emit
#endif

namespace Profile {

void Creator::create() {
    // WARNING App::fileTreeView().closeFiles();
    createProfile(gcp.tools.front(), gcp.params[GCode::Params::Depth].toDouble());
}

void Creator::createProfile(const Tool& tool, const double depth) {
    Finaly _{[this] { emit fileReady(file_); }};

    toolDiameter = tool.getDiameter(depth);

    const double dOffset = ((gcp.side() == GCode::Outer) ? +toolDiameter : -toolDiameter) /** 0.5*/ * uScale;

    if(gcp.side() == GCode::On) {
        if(gcp.params[TrimmingOpenPaths].toBool())
            trimmingOpenPaths(openSrcPaths);
        returnPs = std::move(closedSrcPaths);
    } else {
        if(closedSrcPaths.size()) {
            // ClipperOffset offset;
            // for(Paths64& paths: groupedPaths(GCode::Grouping::Copper))
            // offset.AddPaths(paths, cl::JoinType::Round, cl::EndType::Polygon);
            // returnPs = offset.Execute(dOffset);
            auto it = v::join(groupedPaths(GCode::Grouping::Copper));
            returnPs = Inflate64(Paths64{it.begin(), it.end()}, dOffset, cl::JoinType::Round, cl::EndType::Polygon);
        }
        if(openSrcPaths.size()) {
            // ClipperOffset offset;
            // offset.AddPaths(openSrcPaths, cl::JoinType::Round, cl::EndType::Round);
            // openSrcPaths = offset.Execute(dOffset);
            openSrcPaths = Inflate64(openSrcPaths, dOffset, cl::JoinType::Round, cl::EndType::Round);
            if(!openSrcPaths.empty())
                returnPs.append_range(openSrcPaths);
        }
    }

    if(returnPs.empty() && openSrcPaths.empty()) return;

    reorder();

    if(gcp.side() == GCode::On && openSrcPaths.size()) {
        returnPss.reserve(returnPss.size() + openSrcPaths.size());
        mergePaths(openSrcPaths);
        sortBeginEnd(openSrcPaths, ~(App::home().pos() + App::zero().pos()));
        for(auto&& path: openSrcPaths)
            returnPss.push_back({std::move(path)});
    }

    makeBridges();

    if(gcp.params.contains(TrimmingCorners) && gcp.params[TrimmingCorners].toInt())
        cornerTrimming();

    if(returnPss.empty()) return;

    // Gi::Debug(returnPss | v::join | r::to<std::vector>(), Qt::yellow);

    gcp.toolPathss = toCurvess(returnPss);

    file_ = new File{std::move(gcp)};
    file_->setFileName(tool.nameEnc());
}

void Creator::trimmingOpenPaths(Paths64& paths) {
    const double dOffset = toolDiameter * uScale * 0.5;
    for(size_t i{}; i < paths.size(); ++i) {
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
            p = Path64{~b.p2(), ~e.p2()};
        } else if(double l = Perimeter(p); l <= toolDiameter * uScale) {
            paths -= i--;
            continue;
        } else {
            Paths64 ps;
            {
                // ClipperOffset offset;
                // offset.AddPath();
                // ps = offset.Execute(dOffset + 100);
                ps = Inflate64({p}, dOffset + 100, cl::JoinType::Miter, cl::EndType::Butt);

                // offset.Clear();
                // offset.AddPath(ps.front(), cl::JoinType::Miter, cl::EndType::Polygon);
                // ps = offset.Execute(-dOffset);
                ps = Inflate64({ps.front()}, -dOffset, cl::JoinType::Miter, cl::EndType::Polygon);
                if(ps.empty()) {
                    paths -= i--;
                    continue;
                }
            }
            {
                cl::Clipper64 clipper;
                clipper.AddOpenSubject({p});
                clipper.AddClip(ps);
                clipper.Execute(cl::ClipType::Intersection, cl::FillRule::Positive, ps, ps); // FIXME open paths ???
                p = ps.front();
            }
        }
    }
}

void Creator::cornerTrimming() {
    Timer_mS t{};
    const double trimDepth = (toolDiameter - toolDiameter * sqrt1_2) * sqrt1_2;
    const double sqareSide = toolDiameter * sqrt1_2 * 0.5;
    const double testAngle = gcp.convent() ? 90.0 : 270.0;
    const double trimAngle = gcp.convent() ? -45.0 : +45;

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

    auto paths = v::join(returnPss);

    std::for_each(std::execution::par_unseq, std::begin(paths), std::end(paths), [insert](Path64& path) {
        path.reserve(path.size() * 3);
        for(auto corner = std::next(path.begin()); corner < std::prev(path.end()); ++corner)
            insert(path, std::prev(corner), corner, std::next(corner));
        if(path.front() == path.back()) // for trimming between the beginning and the end of the path
            insert(path, std::prev(path.end(), 2), std::prev(path.end()), std::next(path.begin()));
        path.shrink_to_fit();
    });
#else
    auto insert = [=](Path64& path, auto cornerPrev, auto&& corner, auto cornerNext) {
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

    auto paths = v::join(returnPss);

    std::for_each(
    #ifdef Q_OS_UNIX
        std::execution::par_unseq,
    #endif
        std::begin(paths), std::end(paths), [insert](Path64& path) {
            path.reserve(path.size() * 3);
            for(Point64* corner = std::next(path.data()); corner < std::prev(path.data() + path.size()); ++corner)
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

    std::for_each(
#ifdef Q_OS_UNIX
        std::execution::par_unseq,
#endif
        returnPss.begin(), returnPss.end(), [&bridgeItems, this](Paths64& rPaths) -> void {
            // find Bridges
            auto biStack = bridgeItems
                | v::filter([&rPaths](Gi::Bridge* bi) {
                      return bi->test(toCurve(rPaths.front()));
                  });
            if(r::empty(biStack)) return;
            auto isPositive1 = cl::IsPositive(rPaths.front());

            // create frame
            Paths64 frame = Inflate64(rPaths, toolDiameter * uScale * 0.1, cl::JoinType::Miter, cl::EndType::Butt, uScale);
            Paths64 clip;
            for(Gi::Bridge* bip: biStack)
                clip.append_range(toPaths(bip->curves()));

            frame = cl::Intersect(frame, clip, cl::FillRule::Positive);

            // cut toolPath
            cl::Clipper64 clipper;
            clipper.AddOpenSubject(rPaths);
            clipper.AddClip(frame);
            PolyTree polytree;
            clipper.Execute(cl::ClipType::Difference, cl::FillRule::Positive, frame, rPaths);

            if(rPaths.empty())
                return;

            mergePaths(rPaths);
            sortBeginEnd(rPaths, ~(App::home().pos() + App::zero().pos()));

            auto IsPositive = [](Paths64 paths) {
                for(auto&& path: paths | v::drop(1))
                    paths.front().append_range(std::move(path)); // NOTE  move?
                return cl::IsPositive(paths.front());
            };

            if(isPositive1 ^ IsPositive(rPaths)) // Вернуть исходное направление пути
                ReversePaths(rPaths), r::reverse(rPaths);
        });

    std::erase_if(returnPss, [](auto&& paths) { return paths.empty(); });
}

void Creator::reorder() {
    // returnPss = {returnPs};
    // return;
    PolyTree polyTree;
    {
        cl::Clipper64 clipper;
        clipper.AddSubject(returnPs);
        Rect r(GetBounds(returnPs));
        int k = uScale;
        Path64 outer = {
            {r.left - k,  r.bottom + k},
            {r.right + k, r.bottom + k},
            {r.right + k, r.top - k   },
            {r.left - k,  r.top - k   }
        };
        clipper.AddSubject({outer});
        clipper.Execute(cl::ClipType::Union, cl::FillRule::EvenOdd, polyTree);
        returnPs.clear();
    }

    polyTreeToPaths(polyTree, returnPs);

    std::reverse(returnPs.begin(), returnPs.end());

    if((gcp.side() == GCode::Inner) ^ gcp.convent())
        ReversePaths(returnPs);

    returnPss.reserve(returnPs.size());

    for(auto&& path: returnPs) {

        cl::StripDuplicates(path, true);
        cl::SimplifyPath(path, uScale / 100);
        path.push_back(path.front());

        returnPss.push_back({path});
    }
}

void Creator::reduceDistance(Point64& from, Path64& to) {
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

void Creator::polyTreeToPaths(PolyTree& polytree, Paths64& rpaths) {
    rpaths.clear();

    // auto Total = [i = 0](this auto&& total, PolyTree& polytree) mutable {
    // return i;
    // };
    // rpaths.reserve(Total(polytree));

    std::function<void(PolyTree&, Creator::NodeType)> addPolyNodeToPaths;

    if(!settings.sort) { // Grouping by nesting

        markPolyTreeDByNesting(polytree);

        std::map<int, Paths64> pathsMap;
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
            qDebug() << u"nest"_s << nest << paths.size();
            if(paths.size() > 1)
                sortB(paths, ~(App::home().pos() + App::zero().pos()));
            rpaths.append_range(std::move(paths)); // NOTE move?
        }
    } else { // Grouping by nesting depth
        sortPolyTreeByNesting(polytree);
        Point64 from = ~App::settings().mkrZeroOffset();
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

                // std::map<int, std::vector<PolyTree*>, std::greater<>> map;
                // for (auto node : polynode.Childs)
                // map[node->Nesting].emplace_back(node);
                // size_t i = polynode.Count();
                // for (auto& [nest, nodes] : map) {
                // for (auto node : nodes)
                // polynode.Childs[--i] = node;
                // }
                for(auto&& node: rwPolyTree(polynode))
                    addPolyNodeToPaths(*node, nodetype);
            };

        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);
    }
}
////////////////////////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project().worckRect();
    for(size_t x{}; x < App::project().stepsX(); ++x) {
        for(size_t y{}; y < App::project().stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);

            if(toolType == Tool::Laser)
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);

            if(gcp.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    Gi::Item* item;
    for(const Curves& paths: gcp.toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }

    for(const Curves& paths: gcp.toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
    }

    for(auto&& [from, to]: gcp.toolPathss | v::join | v::pairwise)
        g0path_.push_back(Curve{{from.back().pt}, {to.front().pt}});

    item = new Gi::GcPath{g0path_};
    // item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
    itemGroup()->setVisible(true);
}

} // namespace Profile
