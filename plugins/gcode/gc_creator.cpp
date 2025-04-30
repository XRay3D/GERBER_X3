/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_creator.h"
// #include "gc_file.h"
#include "gc_file.h"
#include "gc_types.h"

#include "app.h"
#include "gi.h"
#include "gi_datapath.h"
#include "gi_error.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "md5.h"
#include "myclipper.h"

#include "project.h"
#include "utils.h"

#undef emit
#include <execution>
#define emit
 //std::execution::parallel_policy
#include <forward_list>
#include <set>
#include <stdexcept>

class GCDbgFile final : public GCode::File {
    QColor color;

public:
    explicit GCDbgFile(GCode::Params&& gcp, Paths&& toolPaths, QColor color)
        : GCode::File(std::move(gcp), {}, std::move(toolPaths))
        , color{color} {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
    void write(QDataStream& stream) const override { }
    void read(QDataStream& stream) override { }
    void initFrom(AbstractFile* file) override { qWarning(__FUNCTION__); }
    QIcon icon() const override { return QIcon::fromTheme("crosshairs"); }
    uint32_t type() const override { return GC_DBG_FILE; }
    void createGi() override {
        Gi::Item* item = new Gi::GcPath{pocketPaths_, this};
        // item->setPen(QPen(color, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        // item->setPenColorPtr(&color);
        itemGroup()->push_back(item);
        // for(int i{}; i < pocketPaths_.size() - 1; ++i)
        // g0path_.emplace_back(Path{pocketPaths_[i].back(), pocketPaths_[i + 1].front()});
        // item = new Gi::GcPath{g0path_};
        // item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
        // itemGroup()->push_back(item);
        itemGroup()->setVisible(true);
    }
    void genGcodeAndTile() override { } // saveLaserProfile({});
    // AbstractFile interface
};

void dbgPaths(Paths ps, const QString& fileName, QColor color, bool close, const Tool& tool) {
    std::erase_if(ps, [](const Path& path) { return path.size() < 2; });
    if(ps.empty())
        return;
    if(close)
        std::ranges::for_each(ps, [](Path& p) { p.push_back(p.front()); });
    GCode::Params gcp{tool, 0.0};
    auto file = new GCDbgFile{std::move(gcp), std::move(ps), color};
    file->setFileName(fileName);
    emit App::project().addFileDbg(file);
};

namespace GCode {

Creator::Creator() { }

void Creator::reset() {
    ProgressCancel::reset();

    file_ = nullptr;

    closedSrcPaths.clear();
    openSrcPaths.clear();
    returnPs.clear();
    returnPss.clear();
    supportPss.clear();
    groupedPss.clear();

    toolDiameter = 0.0;
    dOffset = 0.0;
    stepOver = 0.0;
}

Creator::~Creator() { ProgressCancel::reset(); }

Pathss& Creator::groupedPaths(Grouping group, /*Point::Type*/ int32_t offset, bool skipFrame) {
    PolyTree polyTree;
    {
        Timer t("Union EvenOdd");
        Clipper clipper;
        clipper.AddSubject(closedSrcPaths);
        clipper.AddSubject({boundOfPaths(closedSrcPaths, offset)});
        clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
    }
    groupedPss.clear();
    {
        Timer t("grouping");
        grouping(group, polyTree.Count() == 1 ? *polyTree[0] : polyTree);
    }
    if(skipFrame == false
        && group == Grouping::Cutoff
        && groupedPss.size() > 2
        && groupedPss.front().size() == 2)
        groupedPss.erase(groupedPss.begin());
    return groupedPss;
}

void Creator::grouping(Grouping group, PolyTree& node) {

    if((group == Grouping::Cutoff) ^ node.IsHole()) {
        Paths paths;
        paths.reserve(node.Count() + 1);
        paths.emplace_back(std::move(node.Polygon()));
        for(auto&& child: node)
            paths.emplace_back(std::move(child->Polygon()));
        groupedPss.emplace_back(std::move(paths));
    }
    for(auto&& child: node)
        grouping(group, *child);
}

Path Creator::boundOfPaths(const Paths& paths, /*Point::Type*/ int32_t k) const {
    Rect rect(GetBounds(paths));
    rect.bottom += k;
    rect.left -= k;
    rect.right += k;
    rect.top -= k;
    // dbgPaths({rect.AsPath()}, "boundOfPaths", Qt::magenta);
    return rect.AsPath();
}

void Creator::addRawPaths(Paths&& rawPaths) {
    qDebug(__FUNCTION__);

    if(rawPaths.empty())
        return;

    //    if (gcp_.side() == On) {
    //        workingRawPs_.push_back(rawPaths);
    //        return;
    //    }
    std::erase_if(rawPaths, [](auto&& path) { return path.size() < 2; });

    const double mergDist = App::project().glue() * uScale;

    Clipper clipper;
    for(size_t i{}; i < rawPaths.size(); ++i) {
        Point& pf = rawPaths[i].front();
        Point& pl = rawPaths[i].back();
        if(rawPaths[i].size() > 3 && (pf == pl || distTo(pf, pl) < mergDist)) {
            clipper.AddSubject({rawPaths[i]});
            rawPaths.erase(rawPaths.begin() + i--);
        }
    }

    mergePaths(rawPaths, mergDist);

    for(Path& path: rawPaths) {
        Point& pf = path.front();
        Point& pl = path.back();
        if(path.size() > 3 && (pf == pl || distTo(pf, pl) < mergDist))
            clipper.AddSubject({path});
        else
            openSrcPaths.push_back(path);
    }

    Paths paths;
    clipper.AddClip({boundOfPaths(rawPaths, uScale)});
    clipper.Execute(ClipType::Xor, FillRule::EvenOdd, paths);
    closedSrcPaths.insert(closedSrcPaths.end(), paths.begin() + 1, paths.end()); // paths.takeFirst();
}

void Creator::createGc(Params* gcp) {
    qDebug(__FUNCTION__);

    reset();

    if(gcp->closedPaths.size())
        closedSrcPaths.insert(closedSrcPaths.end(), gcp->closedPaths.begin(), gcp->closedPaths.end());
    if(gcp->openPaths.size())
        addRawPaths(std::move(gcp->openPaths));
    if(gcp->supportPathss.size())
        supportPss.append(std::move(gcp->supportPathss));

    // dbgPaths(closedSrcPaths, "closedPaths");
    // dbgPaths(openSrcPaths, "openPaths");

    //    dbgPaths(closedSrcPaths, "closedSrcPaths");
    //    dbgPaths(supportPss, "supportPathss");
    //    dbgPaths(openSrcPaths, "openPaths");

    gcp_ = std::move(*gcp);
    //    gcp_ = *gcp;

    try {
        if(possibleTest() && !App::isDebug()) {
            try {
                checkMillingFl = true;
                checkMilling(gcp_.side());
            } catch(const Cancel& e) {
                ProgressCancel::reset();
                qWarning() << "checkMilling canceled:" << e.what();
            } catch(...) {
                throw;
            }
        }
        if(!isCancel()) {
            checkMillingFl = false;
            msg = tr("createGc");
            Timer t("createGc");
            create();
        }
        qWarning() << "Creator::createGc() finish";
    } catch(const Cancel& e) {
        qWarning() << "Creator::createGc() canceled:" << e.what();
    } catch(const std::exception& e) {
        qWarning() << "Creator::createGc() exeption:" << e.what();
    } catch(...) {
        qWarning() << "Creator::createGc() exeption:" << errno;
    }
    delete gcp;
}

File* Creator::file() const { return file_; }

std::pair<int, int> Creator::getProgress() {
    return {max(), current()};
}

void Creator::stacking(Paths& paths) {
    qDebug(__FUNCTION__);

    if(paths.empty())
        return;
    Timer t("stacking");

    PolyTree polyTree;
    {
        Timer t("stacking 1");
        Clipper clipper;
        clipper.AddSubject(paths);
        clipper.AddSubject({boundOfPaths(paths, uScale)});
        clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
        paths.clear();
    }
    sortPolyTreeByNesting(polyTree);
    returnPss.clear();
    /**************************************************************************************/
    // повернуть для уменшения дистанции между путями
    auto rotateDiest = [this](Paths& paths, Path& path, std::pair<size_t, size_t> idx) -> bool {
        std::forward_list<size_t> list;
        list.emplace_front(idx.first);
        for(size_t i = paths.size() - 1, index = idx.first; i; --i) {
            double minDist = std::numeric_limits<double>::max();
            Point point;
            for(Point pt: paths[i - 1]) {
                double dist = distTo(pt, paths[i][index]);
                if(minDist >= dist) {
                    minDist = dist;
                    point = pt;
                }
            }
            if(minDist <= toolDiameter) {
                list.emplace_front(indexOf(paths[i - 1], point));
                index = list.front();
            } else
                return false;
        }
        for(size_t i{}; auto it: list)
            std::rotate(paths[i].begin(), paths[i].begin() + it, paths[i].end()), ++i;
        std::rotate(path.begin(), path.begin() + idx.second, path.end());
        return true;
    };

    std::function<void(PolyTree*, bool)> stacker = [&stacker, &rotateDiest, this](PolyTree* node, bool newPaths) {
        if(!returnPss.empty() || newPaths) {
            Path path(node->Polygon());
            if(!(gcp_.convent() ^ !node->IsHole()) ^ (gcp_.side() == Outer))
                ReversePath(path);

            // if(false && App::settings().cleanPolygons())
            //     CleanPolygon(path, uScale * 0.0005);

            if(returnPss.empty() || newPaths) {
                returnPss.push_back({std::move(path)});
            } else {
                // check distance;
                std::pair<size_t, size_t> idx;
                double d = std::numeric_limits<double>::max();
                //                for(size_t id {}; id < returnPss.back().back().size(); ++id) {
                //                    const Point& ptd = returnPss.back().back()[id];
                //                    for(size_t is {}; is < path.size(); ++is) {
                //                        const Point& pts = path[is];
                //                        const double l = distTo(ptdpts);
                //                        if(d >= l) {
                //                            d = l;
                //                            idx.first = id;
                //                            idx.second = is;
                //                        }
                //                    }
                //                }

                for(size_t iDst{}; auto ptd: returnPss.back().back()) {
                    for(size_t iSrc{}; auto pts: path) {
                        if(const double l = distTo(ptd, pts); d >= l) {
                            d = l;
                            idx.first = iDst;
                            idx.second = iSrc;
                        }
                        ++iSrc;
                    }
                    ++iDst;
                }

                if(d <= toolDiameter && rotateDiest(returnPss.back(), path, idx))
                    returnPss.back().emplace_back(std::move(path)); // append to last Paths
                else
                    returnPss.push_back({std::move(path)}); // new Paths
            }

            for(size_t i{}; auto&& var: *node)
                stacker(var.get(), static_cast<bool>(i++));
        } else { // Start from here
            for(auto&& var: *node)
                stacker(var.get(), true);
        }
    };
    /**************************************************************************************/

    stacker(polyTree.Count() == 1 ? polyTree[0] : &polyTree, false);

    for(Paths& retPaths: returnPss) {
        for(size_t i{}; i < retPaths.size(); ++i)
            if(retPaths[i].empty()) retPaths.erase(retPaths.begin() + i--);
        std::ranges::reverse(retPaths);
        for(Path& path: retPaths)
            path.emplace_back(path.front());
    }

    sortB(returnPss, ~(App::home().pos() + App::zero().pos()));
}

#if 0
void Creator::mergePaths(Paths& paths, const double maxDist) {
    qDebug(__FUNCTION__);

    msg = tr("Merge Paths");
    size_t max;

    auto append = [&](size_t& i, size_t& j) {
        paths[i] += paths[j] | skipFront; // paths[i].append(paths[j].mid(1));
        paths -= j--;                     // paths.remove(j--;
    };

    do {
        max = paths.size();
        for(size_t i{}; i < paths.size(); ++i) {
            setMax(max);
            setCurrent(max - paths.size());
            throwIfCancel();
            for(size_t j{}; j < paths.size(); ++j) {
                if(i == j)
                    continue;
                else if(paths[i].front() == paths[j].front()) {
                    ReversePath(paths[j]);
                    append(j, i);
                    break;
                } else if(paths[i].back() == paths[j].back()) {
                    ReversePath(paths[j]);
                    append(i, j);
                    break;
                } else if(paths[i].front() == paths[j].back()) {
                    append(j, i);
                    break;
                } else if(paths[j].front() == paths[i].back()) {
                    append(i, j);
                    break;
                } else if(maxDist > 0.0) {
                    if /*  */ (distTo(paths[i].back(), paths[j].back()) < maxDist) {
                        ReversePath(paths[j]);
                        append(i, j);
                        break; //
                    } else if(distTo(paths[i].back(), paths[j].front()) < maxDist) {
                        append(i, j);
                        break; //
                    } else if(distTo(paths[i].front(), paths[j].back()) < maxDist) {
                        append(j, i);
                        break;
                    } else if(distTo(paths[i].front(), paths[j].front()) < maxDist) {
                        ReversePath(paths[j]);
                        append(j, i);
                        break;
                    }
                }
            }
        }
    } while(max != paths.size());
}
#endif

void Creator::markPolyTreeDByNesting(PolyTree& polynode) {
    qDebug(__FUNCTION__);

    int nestCtr{};
    nesting.clear();
    std::function<int(PolyTree&)> sorter = [&sorter, &nestCtr, this](PolyTree& polynode) {
        ++nestCtr;
        for(auto&& node: polynode)
            sorter(*node);
        return nesting[&polynode] = nestCtr--;
    };
    sorter(polynode);
}

void Creator::sortPolyTreeByNesting(PolyTree& polynode) {
    qDebug(__FUNCTION__);

    int nestCtr{};
    nesting.clear();
    std::function<int(PolyTree&)> sorter = [&sorter, &nestCtr, this](PolyTree& polynode) {
        ++nestCtr;
        nesting[&polynode] = nestCtr;
        switch(polynode.Count()) {
        case 0:
            return nestCtr--;
        case 1:
            return std::max(nestCtr--, sorter(*reinterpret_cast<CL2::PolyPath64*>(polynode.begin()->get()))); // FIXME очень грязный хак
        default:
            std::map<int, std::vector<std::unique_ptr<PolyTree>>, std::greater<>> map;
            for(auto&& node: rwPolyTree(polynode))
                map[sorter(*node)].emplace_back(std::move(node));
            size_t i = polynode.Count();
            auto it_ = polynode.end();                                         // std::reverse_iterator(polynode);
            auto it = *reinterpret_cast<CL2::PolyPath64List::iterator*>(&it_); // FIXME очень грязный хак
            for(auto&& [nest, nodes]: map)
                for(auto&& node: nodes)
                    *(--it) = std::move(node);
            return std::max(nestCtr--, map.begin()->first);
        }
    };
    sorter(polynode);
}

static int condition{};

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

bool Creator::checkMilling(SideOfMilling side) {
    qDebug(__FUNCTION__);
    Timer t(__FUNCTION__);

    const double toolDiameter = gcp_.tools.back().getDiameter(gcp_.getDepth()) * uScale;
    const double toolRadius = toolDiameter * 0.5;

    QString last{msg};
    msg = tr("Check milling for errors");

    auto createFrame = [toolRadius](auto& srcPaths) {
        auto retPaths = InflateRoundPolygon(srcPaths, -toolRadius);
        CleanPaths(retPaths, uScale);
        return InflateRoundPolygon(retPaths, toolRadius + 10);
    };

    auto testFrame = [](Paths& frames, Paths& sources, bool intersect = {}) {
        // Timer t("testFrame");
        std::unordered_map<Path*, std::set<void*>> checker;
        std::mutex m;

        // dbgPaths(frames, "F", Qt::green, true);
        // dbgPaths(sources, "S", Qt::yellow, true);
        setMax(frames.size());
        setCurrent();

        std::for_each(std::execution::par, std::begin(frames), std::end(frames), [&](auto&& frame) {
            incCurrent();
            QPainterPath F;
            F.addPolygon(~frame);
            for(auto&& srcPath: sources) {
                if(intersect) {
                    //                    QPainterPath S;
                    //                    S.addPolygon(ReversePath(srcPath));
                    //                    if (F.intersects(S)) {
                    //                        std::lock_guard guard {m};
                    //                        checker[std::addressof(frame)].insert(srcPath.data());
                    //                        break;
                    //                    }
                } else {
                    for(auto&& pt: srcPath) {
                        if(auto result = Clipper2Lib::PointInPolygon(pt, frame);
                            result == PointInPolygonResult::IsOn || result == PointInPolygonResult::IsInside) {
                            std::lock_guard guard{m};
                            checker[std::addressof(frame)].insert(srcPath.data());
                            break;
                        }
                    }
                }
            }
        });
        std::erase_if(checker, [](const auto& p) {
            return p.second.size() < 2; // 5??
        });
        return checker;
    };

    switch(side) {
    case Outer: {
        Timer t("Outer");
        if constexpr(1) {
            Paths nonCutPaths;
            constexpr auto k = 1;
            groupedPaths(Grouping::Copper);

            auto mill = [](Paths& paths, double offset1, double offset2) {
                Timer t("mill");
                Paths retPaths;
                ReversePaths(paths);
                retPaths = InflateRoundPolygon(paths, offset1);
                retPaths = InflateRoundPolygon(retPaths, offset2);
                return retPaths;
            };

            auto nonCuts = [](const Paths& subject, const Paths& clip) {
                Timer t("nonCuts");
                Paths retPaths;
                retPaths = Clipper2Lib::Difference(subject, clip, FillRule::Positive);
                retPaths = InflateMiterPolygon(retPaths, k);
                return retPaths;
            };

            const double testArea = toolDiameter * toolDiameter - pi * toolRadius * toolRadius;

            Paths srcPaths;
            mvector<QPainterPath> srcPPaths;
            mvector<QPainterPath> nonCutPPaths;
            srcPPaths.reserve(groupedPss.size());
            for(auto&& paths: groupedPss) {
                srcPaths.insert(srcPaths.end(), paths.begin(), paths.end()); //  srcPaths.append(paths);
                srcPPaths.push_back({});
                for(auto&& path: paths)
                    srcPPaths.back().addPolygon(~path);
            }
            nonCutPaths = nonCuts(mill(srcPaths, +toolRadius, -toolRadius - k), srcPaths);
            nonCutPPaths.reserve(nonCutPaths.size());
            for(auto&& frPath: nonCutPaths) {
                nonCutPPaths.push_back({});
                nonCutPPaths.back().addPolygon(~frPath);
            }

            std::mutex m;
            setMax(nonCutPPaths.size());
            setCurrent();

            std::for_each(std::execution::par, std::begin(nonCutPPaths), std::end(nonCutPPaths), [&srcPPaths, testArea, &m](auto&& nonCut) {
                //            for (auto&& nonCut : nonCutPPaths) {
                incCurrent();
                std::set<QPainterPath*> set;
                for(auto&& srcPath: srcPPaths) {
                    if(nonCut.intersects(srcPath)) {
                        std::lock_guard guard{m};
                        set.emplace(&srcPath);
                    }
                }
                if(set.size() < 2 && Area(~nonCut.toFillPolygon()) < testArea)
                    nonCut.clear();
            });

            nonCutPaths.clear();
            for(auto&& frPath: nonCutPPaths)
                if(!frPath.isEmpty())
                    nonCutPaths.emplace_back(~frPath.toFillPolygon());
            std::erase_if(nonCutPaths, [](Path& path) { return path.empty(); }); // убрать пустые
            std::ranges::for_each(nonCutPaths, [this](auto&& path) {
                items.push_back(new Gi::Error{{path}, Area(path) * dScale * dScale});
            });
        } else {
            Paths srcPaths{closedSrcPaths};
            srcPaths.emplace_back(boundOfPaths(closedSrcPaths, uScale));

            Paths frPaths = createFrame(srcPaths);
            CleanPaths(frPaths, uScale * 0.0001);
            frPaths = Clipper2Lib::Difference(srcPaths, frPaths, FillRule::EvenOdd);
            if(frPaths.empty()) // ????
                break;

            std::for_each(std::execution::par, std::begin(frPaths), std::end(frPaths), [](auto&& frPath) {
                // ClipperOffset offset(uScale);
                // offset.AddPath(frPath, JoinType::Round, EndType::Polygon);
                // frPath = offset.Execute(100).front();
                frPath = InflateRoundPolygon(frPath, 100).front();
            });

            //        ClipperOffset offset(uScale);
            //        offset.AddPaths(frPaths, JoinType::Round, EndType::Polygon);
            //        frPaths = offset.Execute(100);
            //        ClipperOffset offset(uScale);
            //        offset.AddPaths(srcPaths, JoinType::Miter, EndType::Polygon);
            //        srcPaths = offset.Execute(-1000);
            //        ReversePaths(srcPaths);

            setCurrent();
            setMax(frPaths.size());

            auto checker{testFrame(frPaths, srcPaths, true)};
            items.reserve(checker.size());
            std::for_each(std::execution::par, std::begin(checker), std::end(checker), [&](auto&& checker) {
                auto&& [frame, set] = checker;
                items.push_back(new Gi::Error{{*frame}, Area(*frame) * dScale * dScale});
            });
        }
    } break;
    case Inner: {
        Timer t("Inner");
        groupedPaths(Grouping::Copper);
        std::mutex m;
        std::for_each(std::execution::par, std::begin(groupedPss), std::end(groupedPss), [&](auto&& srcPaths) {
            // for (int i {}; auto&& srcPaths : groupedPss) {
            Paths frPaths = createFrame(srcPaths);
            if(frPaths.size() == 0) { // Doesn't fit at all
                std::lock_guard guard{m};
                items.push_back(new Gi::Error{srcPaths, Area(srcPaths) * dScale * dScale});
            } else if(frPaths.size() > 1) { // Fits with breaks
                srcPaths = Clipper2Lib::Difference(srcPaths, frPaths, FillRule::EvenOdd);
                if(srcPaths.empty())
                    return; //
                setCurrent();
                setMax(frPaths.size());
                auto checker{testFrame(srcPaths, frPaths)};
                std::lock_guard guard{m};
                for(auto&& [frame, set]: checker)
                    items.push_back(new Gi::Error{{*frame}, Area(*frame) * dScale * dScale});
            }
        });
    } break;
    case On:
        break;
    }
    msg = last;

    if(items.size())
        isContinueCalc();
    return true;
}

Params Creator::getGcp() const { return gcp_; }

void Creator::setGcp(const Params& gcp) {
    gcp_ = gcp;
    reset();
}

} // namespace GCode

#include "moc_gc_creator.cpp"
