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
 ********************************************************************************/
#include "gc_creator.h"
#include "gc_file.h"
#include "gc_types.h"

#include "app.h"
#include "gi_error.h"
#include "gi_point.h"
#include "myclipper.h"
#include "utils.h"

#include <stdexcept>

static int id = qRegisterMetaType<GCode::File*>("GCode::File*");

void dbgPaths(Paths ps, const QString& fileName, bool close, const Tool& tool) {
    if (ps.empty())
        return;

    for (size_t i = 0; i < ps.size(); ++i)
        if (ps[i].empty())
            ps.erase(ps.begin() + i--);

    if (close)
        for (Path& p : ps)
            p.push_back(p.front());

    // assert(ps.isEmpty());
    auto file = new GCode::File({ps}, {tool, 0.0, GCode::Profile});
    file->setFileName(fileName + "_" + tool.name());
    // file->itemGroup()->setPen({ Qt::green, 0.0 });
    emit App::project()->addFileDbg(file);

    //    static QMutex m;
    //    m.lock();
    //    if (ps.isEmpty()) {
    //        return;
    //    }
    //    const auto polygons = toQPolygons(ps);
    //    for (auto& path : polygons) {
    //        if (path.size() < 2)
    //            continue;
    //        auto pl = new Shapes::PolyLine(path[0], path[1]);
    //        for (size_t i = 2; i < path.size(); ++i)
    //            pl->addPt(path[i]);
    //        //        for (auto& p : path.mid(3))
    //        //            pl->addPt(p);
    //        pl->setSelected(true);
    //        pl->setSelected(false);
    //        App::project()->addShape(pl);
    //    }
    //    m.unlock();
};

namespace GCode {

Creator::Creator() { }

void Creator::reset() {
    ProgressCancel::reset();

    file_ = nullptr;

    workingPs.clear();
    workingRawPs.clear();
    returnPs.clear();
    returnPss.clear();
    supportPss.clear();
    groupedPss.clear();

    toolDiameter = 0.0;
    dOffset = 0.0;
    stepOver = 0.0;
}

Creator::~Creator() { ProgressCancel::reset(); }

Pathss& Creator::groupedPaths(Grouping group, Point::Type k, bool fl) {
    qDebug(__FUNCTION__);
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddSubject(workingPs);
    Rect r(Bounds(workingPs));
    Path outer = {
        Point(r.left - k, r.top - k),
        Point(r.right + k, r.top - k),
        Point(r.right + k, r.bottom + k),
        Point(r.left - k, r.bottom + k),
    };
    // ReversePath(outer);
    clipper.AddSubject({outer});
    clipper.Execute(ClipType::Union, FillRule::NonZero, polyTree);
    /****************************/
    std::function<void(PolyTree&)> grouping = [&grouping, group, this](PolyTree& node) {
        if ((group == CutoffPaths) ^ node.IsHole()) {
            Paths paths;
            paths.reserve(node.Count() + 1);
            paths.emplace_back(std::move(node.Polygon()));
            for (auto&& child : node)
                paths.emplace_back(std::move(child->Polygon()));
            groupedPss.emplace_back(std::move(paths));
        }
        for (auto&& child : node)
            grouping(*child);
    };
    /*********************************/
    groupedPss.clear();
    grouping(polyTree);

    if (!fl && group == CutoffPaths) {
        if (groupedPss.size() > 1 && groupedPss.front().size() == 2)
            groupedPss.erase(groupedPss.begin());
    }

    return groupedPss;
}
////////////////////////////////////////////////////////////////
/// \brief Creator::addRawPaths
/// \param paths
///
void Creator::addRawPaths(Paths rawPaths) {
    qDebug(__FUNCTION__);

    if (rawPaths.empty())
        return;

    //    if (gcp_.side() == On) {
    //        workingRawPs_.push_back(rawPaths);
    //        return;
    //    }

    for (size_t i = 0; i < rawPaths.size(); ++i)
        if (rawPaths[i].size() < 2)
            rawPaths.erase(rawPaths.begin() + i--);

    const double glueLen = App::project()->glue() * uScale;
    Clipper clipper;
    for (size_t i = 0; i < rawPaths.size(); ++i) {
        Point& pf = rawPaths[i].front();
        Point& pl = rawPaths[i].back();
        if (rawPaths[i].size() > 3 && (pf == pl || pf.distTo(pl) < glueLen)) {
            clipper.AddSubject({rawPaths[i]});
            rawPaths.erase(rawPaths.begin() + i--);
        }
    }

    mergeSegments(rawPaths, App::project()->glue() * uScale);

    for (Path& path : rawPaths) {
        Point& pf = path.front();
        Point& pl = path.back();
        if (path.size() > 3 && (pf == pl || pf.distTo(pl) < glueLen))
            clipper.AddSubject({path});
        else
            workingRawPs.push_back(path);
    }

    Rect r(Bounds(rawPaths));
    int k = uScale;
    Paths paths;
    clipper.AddClip({
        {{r.left - k, r.bottom + k},
         {r.right + k, r.bottom + k},
         {r.right + k, r.top - k},
         {r.left - k, r.top - k}}
    });
    clipper.Execute(ClipType::Xor, FillRule::EvenOdd, paths);
    workingPs.insert(workingPs.end(), paths.begin() + 1, paths.end()); // paths.takeFirst();
}

void Creator::addSupportPaths(Pathss supportPaths) { supportPss.append(supportPaths); }

void Creator::addPaths(const Paths& paths) { workingPs.append(paths); }

void Creator::createGc() {
    qDebug(__FUNCTION__);

    try {
        if (type() == Profile || //
            type() == Pocket ||  //
            type() == Raster) {
            switch (gcp_.side()) {
            case Outer:
                createability(CutoffPaths);
                break;
            case Inner:
                createability(CopperPaths);
                //                 groupedPaths(CopperPaths, uScale);
                // groupedPaths(CutoffPaths, gcp_.tools.front().getDiameter(gcp_.getDepth()) * uScale + 100, true);
                // TODO    Inner           createability(CopperPaths);
                break;
            case On:
                break;
            }
        }
        if (!isCancel()) {
            Timer t("createGc");
            create();
        }
        qWarning() << "Creator::createGc() finish";
    } catch (const cancelException& e) {
        qWarning() << "Creator::createGc() canceled:" << e.what();
    } catch (const std::exception& e) {
        qWarning() << "Creator::createGc() exeption:" << e.what();
    } catch (...) {
        qWarning() << "Creator::createGc() exeption:" << errno;
    }
}

GCode::File* Creator::file() const { return file_; }

std::pair<int, int> Creator::getProgress() {
    return {static_cast<int>(max()), static_cast<int>(current())};
}

void Creator::stacking(Paths& paths) {
    qDebug(__FUNCTION__);

    if (paths.empty())
        return;
    Timer t("stacking");

    PolyTree polyTree;
    {
        Timer t("stacking 1");
        Clipper clipper;
        clipper.AddSubject(paths);
        Rect r(Bounds(paths));
        int k = uScale;
        Path outer {
            Point(r.left - k, r.bottom + k),
            Point(r.right + k, r.bottom + k),
            Point(r.right + k, r.top - k),
            Point(r.left - k, r.top - k)};
        clipper.AddSubject({outer});
        clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
        paths.clear();
    }
    sortPolyTreeByNesting(polyTree);
    returnPss.clear();
    /************************************************************************************************/

    auto mathBE = [this](Paths& paths, Path& path, std::pair<size_t, size_t> idx) -> bool {
        QList<std::iterator_traits<Path::iterator>::difference_type> list;
        list.push_back(idx.first);
        for (size_t i = paths.size() - 1, index = idx.first; i; --i) {
            double d = std::numeric_limits<double>::max();
            Point pt;
            for (const Point& pts : paths[i - 1]) {
                double l = pts.distTo(paths[i][index]);
                if (d >= l) {
                    d = l;
                    pt = pts;
                }
            }
            if (d <= toolDiameter) {
                list.prepend(paths[i - 1].indexOf(pt));
                index = list.front();
            } else
                return false;
        }
        for (size_t i = 0; i < paths.size(); ++i)
            std::rotate(paths[i].begin(), paths[i].begin() + list[i], paths[i].end());
        std::rotate(path.begin(), path.begin() + idx.second, path.end());
        return true;
    };

    using Worck = std::pair<PolyTree*, bool>;
    std::function<void(Worck)> stacker = [&stacker, &mathBE, this](Worck w) {
        auto [node, newPaths] = w;
        if (!returnPss.empty() || newPaths) {
            Path path(node->Polygon());
            if (!(gcp_.convent() ^ !node->IsHole()) ^ !(gcp_.side() == Outer))
                ReversePath(path);
            //            if (App::settings().gbrCleanPolygons())
            //                CleanPolygon(path, uScale * 0.0005);
            if (returnPss.empty() || newPaths) {
                returnPss.push_back({path});
            } else {
                // check distance;
                std::pair<size_t, size_t> idx;
                double d = std::numeric_limits<double>::max();
                for (size_t id = 0; id < returnPss.back().back().size(); ++id) {
                    const Point& ptd = returnPss.back().back()[id];
                    for (size_t is = 0; is < path.size(); ++is) {
                        const Point& pts = path[is];
                        const double l = ptd.distTo(pts);
                        if (d >= l) {
                            d = l;
                            idx.first = id;
                            idx.second = is;
                        }
                    }
                }
                if (d <= toolDiameter && mathBE(returnPss.back(), path, idx))
                    returnPss.back().push_back(path);
                else
                    returnPss.push_back({path});
            }
            // for (size_t i = 0, end = node->Count(); i < end; ++i)
            //      stacker({(*node)[i]->get(), static_cast<bool>(i)});
            for (size_t i {}; auto&& var : *node)
                stacker({var.get(), static_cast<bool>(i++)});
        } else { // Start from here
            for (auto&& var : *node)
                stacker({var.get(), true});
            // for (size_t i = 0, end = node->Count(); i < end; ++i)
            //     stacker({(*node)[i]->get(), true});
        }
        // PROG setProgInc();
    };
    /************************************************************************************************/
    // PROG .3setProgMax(polyTree.Total());
    stacker({&polyTree, false});

    for (Paths& retPaths : returnPss) {
        std::reverse(retPaths.begin(), retPaths.end());
        for (size_t i = 0; i < retPaths.size(); ++i) {
            if (retPaths[i].empty())
                retPaths.erase(retPaths.begin() + i--);
        }
        for (Path& path : retPaths)
            path.push_back(path.front());
    }
    sortB(returnPss);
}

void Creator::mergeSegments(Paths& paths, double glue) {
    qDebug(__FUNCTION__);

    size_t size;
    do {
        size = paths.size();
        for (size_t i = 0; i < paths.size(); ++i) {
            if (i >= paths.size())
                break;
            for (size_t j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                if (i >= paths.size())
                    break;
                Point pib = paths[i].back();
                Point pjf = paths[j].front();
                if (pib == pjf) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point pif = paths[i].front();
                Point pjb = paths[j].back();
                if (pif == pjb) {
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[i].end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if (pib == pjb) {
                    ReversePath(paths[j]);
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
            }
        }
    } while (size != paths.size());
    if (qFuzzyIsNull(glue))
        return;
    do {
        size = paths.size();
        for (size_t i = 0; i < paths.size(); ++i) {
            if (i >= paths.size())
                break;
            for (size_t j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                if (i >= paths.size())
                    break;
                Point pib = paths[i].back();
                Point pjf = paths[j].front();
                if (pib.distTo(pjf) < glue) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point pif = paths[i].front();
                Point pjb = paths[j].back();
                if (pif.distTo(pjb) < glue) {
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[i].end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if (pib.distTo(pjb) < glue) {
                    ReversePath(paths[j]);
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
            }
        }
    } while (size != paths.size());
}

void Creator::mergePaths(Paths& paths, const double dist) {
    qDebug(__FUNCTION__);

    msg = tr("Merge Paths");
    size_t max;

    auto append = [&](size_t& i, size_t& j) {
        paths[i].append(paths[j].mid(1));
        paths.remove(j--);
    };

    do {
        max = paths.size();
        for (size_t i = 0; i < paths.size(); ++i) {
            setMax(max);
            setCurrent(max - paths.size());
            ifCancelThenThrow();
            for (size_t j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                else if (paths[i].front() == paths[j].front()) {
                    ReversePath(paths[j]);
                    append(j, i);
                    break;
                } else if (paths[i].back() == paths[j].back()) {
                    ReversePath(paths[j]);
                    append(i, j);
                    break;
                } else if (paths[i].front() == paths[j].back()) {
                    append(j, i);
                    break;
                } else if (paths[j].front() == paths[i].back()) {
                    append(i, j);
                    break;
                } else if (dist > 0.0) {
                    if /*  */ (paths[i].back().distTo(paths[j].back()) < dist) {
                        ReversePath(paths[j]);
                        append(i, j);
                        break; //
                    } else if (paths[i].back().distTo(paths[j].front()) < dist) {
                        append(i, j);
                        break; //
                    } else if (paths[i].front().distTo(paths[j].back()) < dist) {
                        append(j, i);
                        break;
                    } else if (paths[i].front().distTo(paths[j].front()) < dist) {
                        ReversePath(paths[j]);
                        append(j, i);
                        break;
                    }
                }
            }
        }
    } while (max != paths.size());
}

void Creator::markPolyTreeDByNesting(PolyTree& polynode) {
    qDebug(__FUNCTION__);

    int nestCtr = 0;
    nesting.clear();
    std::function<int(PolyTree&)> sorter = [&sorter, &nestCtr, this](PolyTree& polynode) {
        ++nestCtr;
        for (auto&& node : polynode)
            sorter(*node);
        return nesting[&polynode] = nestCtr--;
    };
    sorter(polynode);
}

void Creator::sortPolyTreeByNesting(PolyTree& polynode) {
    qDebug(__FUNCTION__);

    int nestCtr = 0;
    nesting.clear();
    std::function<int(PolyTree&)> sorter = [&sorter, &nestCtr, this](PolyTree& polynode) {
        ++nestCtr;
        nesting[&polynode] = nestCtr;
        switch (polynode.Count()) {
        case 0:
            return nestCtr--;
        case 1:
            return std::max(nestCtr--, sorter(*polynode.begin()->get()));
        default:
            std::map<int, std::vector<std::unique_ptr<PolyTree>>, std::greater<>> map;
            for (auto&& node : polynode)
                map[sorter(*node)].emplace_back(std::move(node));
            size_t i = polynode.Count();
            auto it = polynode.end(); // std::reverse_iterator(polynode);
            for (auto&& [nest, nodes] : map) {
                for (auto&& node : nodes)
                    *(--it) = std::move(node);
            }
            return std::max(nestCtr--, map.begin()->first);
        }
    };
    sorter(polynode);
}

static int condition = 0;

void Creator::isContinueCalc() {
    condition = 0;
    emit errorOccurred();
    std::unique_lock lk(mutex);
    cv.wait(lk, [] { return condition == 1; });
    items.clear();
    lk.unlock();
}

void Creator::continueCalc(bool fl) { // direct connection!!
    setCancel(!fl);
    condition = 1;
    cv.notify_all();
}

bool Creator::createability(bool side) {
    qDebug(__FUNCTION__);

    Timer t("createability");
    //    if (0) {
    //        if (side == CopperPaths)
    //            groupedPaths(CopperPaths);
    //        else
    //            groupedPaths(CutoffPaths, gcp_.tools.front().getDiameter(gcp_.getDepth()) * uScale + 100);

    //        //    Paths wpe;
    //        const double toolDiameter = gcp_.tools.back().getDiameter(gcp_.getDepth()) * uScale;
    //        const double toolRadius = toolDiameter * 0.5;
    //        const double testArea = toolDiameter * toolDiameter - pi * toolRadius * toolRadius;

    //        Paths srcPaths;
    //        for (size_t pIdx = 0; pIdx < groupedPss.size(); ++pIdx) {
    //            srcPaths.append(groupedPss[pIdx]);
    //        }

    //        Paths frPaths;
    //        {
    //            ClipperOffset offset(uScale);
    //            offset.AddPaths(srcPaths, JoinType::Round, EndType::Polygon);
    //            frPaths = offset.Execute(-toolRadius);
    //            //        if (App::settings().gbrCleanPolygons())
    //            //            CleanPaths(frPaths, uScale * 0.0005);
    //            offset.Clear();
    //            offset.AddPaths(frPaths, JoinType::Round, EndType::Polygon);
    //            frPaths = offset.Execute(toolRadius + 100);
    //        }
    //        if (side == CopperPaths)
    //            ReversePaths(srcPaths);
    //        {
    //            Clipper clipper;
    //            clipper.AddPaths(frPaths, PathType::Clip);
    //            clipper.AddPaths(srcPaths, PathType::Subject);
    //            clipper.Execute(ClipType::Difference, frPaths, FillRule::Positive);
    //        }
    //        QString last(msg);
    //        if (!frPaths.empty()) {
    //            PolyTree polyTree;
    //            {
    //                Clipper clipper;
    //                clipper.AddPaths(frPaths, PathType::Subject, true);
    //                Rect rect(clipper.GetBounds());
    //                int k = uScale;
    //                Path outer = {Point(rect.left - k, rect.bottom + k), Point(rect.right + k, rect.bottom + k),
    //                    Point(rect.right + k, rect.top - k), Point(rect.left - k, rect.top - k)};
    //                clipper.AddPath(outer, PathType::Subject, true);
    //                clipper.Execute(ClipType::Union, polyTree, FillRule::EvenOdd);
    //            }

    //            auto test = [&srcPaths, side](PolyTree* node) -> bool {
    //                if (node->Count() > 0) {
    //                    return true;
    //                } else {
    //                    QSet<size_t> skip;
    //                    for (auto& point : node->Polygon()) {
    //                        for (size_t i = 0; i < srcPaths.size(); ++i) {
    //                            if (skip.contains(i))
    //                                continue;
    //                            const auto& path = srcPaths[i];
    //                            if (int fl = PointInPolygon(point, path); side == CopperPaths || fl == -1) { ////////////////
    //                                skip.insert(i);
    //                                break;
    //                            }
    //                        }
    //                    }
    //                    if (skip.size() > 1) {
    //                        return true;
    //                    }
    //                }
    //                return false;
    //            };

    //            setMax(frPaths.size());
    //            setCurrent();

    //            msg = tr("Creativity check");
    //            const std::function<void(PolyTreeDD*, double)> creator = [&creator, test, testArea, this](PolyTree* node, double area) {
    //                if (node && !node->IsHole()) { // init run
    //                    for (size_t i = 0; i < node->Count(); ++i) {
    //                        if (area = -Area(node->Childs[i]->Polygon()); testArea < area) {
    //                            if (test(node->Childs[i])) {
    //                                creator(node->Childs[i], area);
    //                            }
    //                        }
    //                    }
    //                } else {
    //                    static Paths paths;
    //                    paths.clear();
    //                    paths.reserve(node->Count() + 1);
    //                    paths.push_back(std::move(node->Polygon())); // init path
    //                    for (size_t i = 0; i < node->Count(); ++i) {
    //                        area += Area(node->Childs[i]->Polygon());
    //                        paths.push_back(std::move(node->Childs[i]->Polygon()));
    //                    }
    //                    incCurrent();
    //                    items.push_back(new GiError(paths, area * dScale * dScale));
    //                    for (size_t i = 0; i < node->Count(); ++i) {
    //                        creator(node->Childs[i], area);
    //                    }
    //                }
    //            };
    //            creator(polyTree.GetFirst(), 0);
    //        }
    //        msg = last;
    //        if (!items.empty())
    //            isContinueCalc();
    //    }

    //    if (side == CutoffPaths) {
    //        const double toolDiameter = gcp_.tools.back().getDiameter(gcp_.getDepth()) * uScale;
    //        const double toolRadius = toolDiameter * 0.5;
    //        const double testArea = toolDiameter * toolDiameter - pi * toolRadius * toolRadius;
    //        Paths nonCutPaths;

    //        constexpr auto k = 1;

    //        groupedPaths(CopperPaths);

    //        auto mill = [](Paths& paths, double offset1, double offset2) {
    //            Timer t("mill");
    //            Paths retPaths;

    //            ReversePaths(paths);

    //            ClipperOffset offset(uScale);
    //            offset.AddPaths(paths, JoinType::Round, EndType::Polygon);
    //            retPaths = offset.Execute(offset1);
    //            // dbgPaths(retPaths, "mill 1");

    //            offset.Clear();
    //            offset.AddPaths(retPaths, JoinType::Round, EndType::Polygon);
    //            retPaths = offset.Execute(offset2);
    //            // dbgPaths(retPaths, "mill 2");

    //            return retPaths;
    //        };

    //        auto nonCuts = [](const Paths& subject, const Paths& clip) {
    //            Timer t("nonCuts");
    //            Paths retPaths;

    //            Clipper clipper;
    //            clipper.AddPaths(subject, PathType::Subject);
    //            clipper.AddPaths(clip, PathType::Clip);
    //            clipper.Execute(ClipType::Difference, retPaths, FillRule::Positive);

    //            ClipperOffset offset(uScale);
    //            offset.AddPaths(retPaths, Clipper2Lib::JoinType::Miter, EndType::Polygon);
    //            retPaths = offset.Execute(k);
    //            return retPaths;
    //        };

    //        if (side == CutoffPaths) { // FIXME V547 Expression is always true.
    //            Paths srcPaths;
    //            mvector<QPainterPath> srcPPaths;
    //            mvector<QPainterPath> nonCutPPaths;
    //            srcPPaths.reserve(groupedPss.size());
    //            for (auto&& paths : groupedPss) {
    //                srcPaths.append(paths);
    //                srcPPaths.push_back({});
    //                for (auto&& path : paths)
    //                    srcPPaths.back().addPolygon(path);
    //            }

    //            nonCutPaths = nonCuts(mill(srcPaths, +toolRadius, -toolRadius - k), srcPaths);

    //            nonCutPPaths.reserve(nonCutPaths.size());
    //            for (auto&& frPath : nonCutPaths) {
    //                nonCutPPaths.push_back({});
    //                nonCutPPaths.back().addPolygon(frPath);
    //            }

    //            for (auto&& nonCut : nonCutPPaths) {
    //                std::set<QPainterPath*> set;
    //                for (auto&& srcPath : srcPPaths) {
    //                    if (nonCut.intersects(srcPath))
    //                        set.emplace(&srcPath);
    //                }
    //                if (set.size() < 2 && Area(nonCut.toFillPolygon()) < testArea)
    //                    nonCut.clear();
    //            }

    //            nonCutPaths.clear();
    //            for (auto&& frPath : nonCutPPaths)
    //                if (!frPath.isEmpty())
    //                    nonCutPaths.emplace_back(frPath.toFillPolygon());

    //        } else {
    //            for (auto srcPaths : groupedPss) {
    //                Paths nonCutPaths_;
    //                mvector<QPainterPath> srcPPaths;
    //                mvector<QPainterPath> nonCutPPaths;

    //                dbgPaths(srcPaths, "srcPaths");

    //                nonCutPaths_ = mill(srcPaths, -toolRadius, +toolRadius + k);

    //                dbgPaths(nonCutPaths_, "nonCutPaths 1");

    //                for (auto&& path : nonCutPaths_) {
    //                    srcPPaths.push_back({});
    //                    srcPPaths.back().addPolygon(path);
    //                }

    //                nonCutPaths_ = nonCuts(srcPaths, nonCutPaths_);

    //                dbgPaths(nonCutPaths_, "nonCutPaths 2");

    //                nonCutPPaths.reserve(nonCutPaths_.size());

    //                for (auto&& frPath : nonCutPaths_) {
    //                    nonCutPPaths.push_back({});
    //                    nonCutPPaths.back().addPolygon(frPath);
    //                }

    //                for (auto&& frPath : nonCutPPaths) {
    //                    std::set<QPainterPath*> set;
    //                    for (auto&& srcPath : srcPPaths) {
    //                        if (frPath.intersects(srcPath))
    //                            set.emplace(&srcPath);
    //                    }
    //                    if (set.size() < 2 && Area(frPath.toFillPolygon()) < testArea)
    //                        frPath.clear();
    //                }

    //                for (auto&& frPath : nonCutPPaths) // frames
    //                {
    //                    if (!frPath.isEmpty())
    //                        nonCutPaths.emplace_back(frPath.toFillPolygon());
    //                    dbgPaths(frPath.toSubpathPolygons(), "toSubpathPolygons");
    //                }
    //                break;
    //            }
    //        }

    //        std::erase_if(nonCutPaths, [](Path& path) { return !path.size(); }); // убрать пустые

    //        std::ranges::for_each(nonCutPaths, [this](auto&& path) {
    //            items.push_back(new GiError({path}, Area(path) * dScale * dScale));
    //        });

    //        QString last(msg);

    //        msg = last;
    //        if (!items.empty())
    //            isContinueCalc();
    //    }
    return true;
}

GCodeParams Creator::getGcp() const { return gcp_; }

void Creator::setGcp(const GCodeParams& gcp) {
    gcp_ = gcp;
    reset();
}

Paths& Creator::sortB(Paths& src) {
    qDebug(__FUNCTION__);

    Point startPt(App::home()->pos() + App::zero()->pos());
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = startPt.distTo(src[secondIdx].front());
            if (destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].back();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Paths& Creator::sortBE(Paths& src) {
    qDebug(__FUNCTION__);

    Point startPt(App::home()->pos() + App::zero()->pos());
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        // PROG //PROG .3setProgMaxAndVal(src.size(), firstIdx);
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse = false;
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = startPt.distTo(src[secondIdx].front());
            const double lenLast = startPt.distTo(src[secondIdx].back());
            if (lenFirst < lenLast) {
                if (destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else {
                if (destLen > lenLast) {
                    destLen = lenLast;
                    swapIdx = secondIdx;
                    reverse = true;
                }
            }
            if (qFuzzyIsNull(destLen))
                break;
        }
        if (reverse)
            ReversePath(src[swapIdx]);
        startPt = src[swapIdx].back();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Pathss& Creator::sortB(Pathss& src) {
    qDebug(__FUNCTION__);

    Point startPt(App::home()->pos() + App::zero()->pos());
    for (size_t i = 0; i < src.size(); ++i) {
        if (src[i].empty())
            src.erase(src.begin() + i--);
    }
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = startPt.distTo(src[secondIdx].front().front());
            if (destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].back().back();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Pathss& Creator::sortBE(Pathss& src) {
    qDebug(__FUNCTION__);

    Point startPt(App::home()->pos() + App::zero()->pos());
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse = false;
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = startPt.distTo(src[secondIdx].front().front());
            const double lenLast = startPt.distTo(src[secondIdx].back().back());
            if (lenFirst < lenLast) {
                if (destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else {
                if (destLen > lenLast) {
                    destLen = lenLast;
                    swapIdx = secondIdx;
                    reverse = true;
                }
            }
        }
        //        if (reverse)
        //            std::reverse(src[swapIdx].begin(), src[swapIdx].end());
        //        startPt = src[swapIdx].back().back();
        if (swapIdx != firstIdx && !reverse) {
            startPt = src[swapIdx].back().back();
            std::swap(src[firstIdx], src[swapIdx]);
        }
    }
    return src;
}

bool Creator::pointOnPolygon(const QLineF& l2, const Path& path, Point* ret) {
    qDebug(__FUNCTION__);

    const size_t cnt = path.size();
    if (cnt < 2)
        return false;
    QPointF p;
    for (size_t i = 0; i < cnt; ++i) {
        const Point& pt1 = path[(i + 1) % cnt];
        const Point& pt2 = path[i];
        QLineF l1(pt1, pt2);
        if (QLineF::BoundedIntersection == l1.intersects(l2, &p)) {
            if (ret)
                *ret = (p);
            return true;
        }
    }

    return false;
}

} // namespace GCode

#include "moc_gc_creator.cpp"
