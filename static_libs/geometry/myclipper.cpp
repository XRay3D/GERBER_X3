/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
// module;

#include "myclipper.h"
#include "curve.h"
#include "app.h"
#include "cancelation.h"
#include "gi_dbg.h"
#include "graphicsview.h"

#include "qmath.h"
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <boost/range/combine.hpp>
#include <forward_list>
#include <map>
#include <mutex>
#include <qglobal.h>
#include <set>

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

// export module myclipper;

// export namespace MC {

QIcon drawIcon(const Paths64& paths, QColor color) {
    static std::mutex m;
    std::lock_guard l{m};

    QPainterPath painterPath;

    for(auto&& polygon: paths)
        painterPath.addPolygon(~polygon);

    return drawIcon(painterPath, color);
}

QIcon drawIcon(const QPainterPath& pPath, QColor color, bool stroke) {
    auto rect    = pPath.boundingRect();
    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());
    double ky    = rect.bottom() * scale;
    double kx    = rect.left() * scale;
    QPixmap pixmap{IconSize, IconSize};
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    if(stroke) {
        painter.strokePath(pPath, {color, 0.0});
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawPath(pPath);
    }
    return pixmap;
}

QIcon drawDrillIcon(QColor color) {
    QPainterPath painterPath;
    painterPath.addEllipse(QRect(0, 0, IconSize - 1, IconSize - 1));
    return drawIcon(painterPath, color);
}

using namespace std::placeholders;

template <>
struct std::hash<Point64> {
    std::size_t operator()(const Point64& pt) const noexcept {
        // const std::pair pair{pt.x, pt.y};
        // return std::hash<decltype(pair)>{}(pair);
        std::size_t h1 = std::hash<int64_t>{}(pt.x);
        std::size_t h2 = std::hash<int64_t>{}(pt.y);
        return h1 ^ (h2 << 1);
    }
};

using CenterKey = std::pair<Point64, const void*>;

template <>
struct std::hash<CenterKey> {
    std::size_t operator()(const CenterKey& pt) const noexcept {
        // const std::pair pair{pt.x, pt.y};
        // return std::hash<decltype(pair)>{}(pair);
        std::size_t h1 = std::hash<int64_t>{}(pt.first.x);
        std::size_t h2 = std::hash<int64_t>{}(pt.first.y);
        std::size_t h3 = std::hash<const void*>{}(pt.second);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

QDataStream& operator<<(QDataStream& stream, const Point64& pt) {
    return stream
        << static_cast<int32_t>(pt.x)
        << static_cast<int32_t>(pt.y)
        << static_cast<qsizetype>(pt.z);
}

QDataStream& operator>>(QDataStream& stream, Point64& pt) {
    return stream
        >> reinterpret_cast<int32_t&>(pt.x)
        >> reinterpret_cast<int32_t&>(pt.y)
        >> reinterpret_cast<qsizetype&>(pt.z);
}

namespace {
struct CIndices {
    int32_t prev;
    int32_t next;
};

inline CIndices decodeIndices(const Point64& p) { return std::bit_cast<CIndices>(p.z); }
inline void encodeIndices(Point64& p, CIndices idx) { p.z = std::bit_cast<int64_t>(idx); }

struct CenterEntry {
    QPointF point;
    CenterKind kind{CenterKind::Source};
};

std::vector<CenterEntry>& centerRegistry() {
    static std::vector<CenterEntry> reg{CenterEntry{}}; // index 0 зарезервирован (не используется)
    return reg;
}

// координата центра -> уже выданный для неё индекс (исключает дублирование в реестре)
std::map<std::pair<double, double>, int32_t>& centerDedup() {
    static std::map<std::pair<double, double>, int32_t> m;
    return m;
}

std::mutex& centerRegistryMutex() {
    static std::mutex m;
    return m;
}
} // namespace

int32_t RegisterCenter(const QPointF& center, CenterKind kind) {
    std::lock_guard l{centerRegistryMutex()};
    auto& dedup       = centerDedup();
    const auto key    = std::pair{center.x(), center.y()};
    if(auto it = dedup.find(key); it != dedup.end()) return it->second;

    auto& reg = centerRegistry();
    reg.push_back({center, kind});
    const int32_t idx = static_cast<int32_t>(reg.size() - 1);
    dedup.emplace(key, idx);
    return idx;
}

QPointF CenterAt(int32_t index) {
    if(index <= 0) return {};
    std::lock_guard l{centerRegistryMutex()};
    auto& reg = centerRegistry();
    return static_cast<size_t>(index) < reg.size() ? reg[static_cast<size_t>(index)].point : QPointF{};
}

CenterKind CenterKindAt(int32_t index) {
    if(index <= 0) return CenterKind::Source;
    std::lock_guard l{centerRegistryMutex()};
    auto& reg = centerRegistry();
    return static_cast<size_t>(index) < reg.size() ? reg[static_cast<size_t>(index)].kind : CenterKind::Source;
}

static void SetCenterAt(int32_t index, const QPointF& center) {
    if(index <= 0) return;
    std::lock_guard l{centerRegistryMutex()};
    auto& reg = centerRegistry();
    if(static_cast<size_t>(index) >= reg.size()) return;

    auto& entry = reg[static_cast<size_t>(index)];
    auto& dedup = centerDedup();
    // старый ключ дедупликации становится недействительным после сдвига центра
    if(auto it = dedup.find(std::pair{entry.point.x(), entry.point.y()});
        it != dedup.end() && it->second == index)
        dedup.erase(it);

    entry.point                                       = center;
    dedup[std::pair{center.x(), center.y()}] = index;
}

int32_t GetCPrevIndex(const Point64& dst) { return decodeIndices(dst).prev; }
int32_t GetCNextIndex(const Point64& dst) { return decodeIndices(dst).next; }

void SetCIndices(Point64& dst, int32_t prevIndex, int32_t nextIndex) {
    encodeIndices(dst, {prevIndex, nextIndex});
}

Point64 GetC(const Point64& dst) {
    const CIndices idx = decodeIndices(dst);
    if(idx.prev) return ~CenterAt(idx.prev);
    if(idx.next) return ~CenterAt(idx.next);
    return dst; // нет смежной дуги
}

void SetCSelf(Point64& dst) { dst.z = 0; }

void SetCForce(Point64& dst, const Point64& center) {
    CIndices idx = decodeIndices(dst);
    idx.next     = RegisterCenter(~center);
    encodeIndices(dst, idx);
}

void SetC(Point64& dst, const Point64& center) {
    if(dst.z == 0) SetCForce(dst, center);
}

Path64 CirclePath(double diametr, const Point64& center) {
    if(qFuzzyIsNull(diametr)) return {};
    const double radius = diametr * 0.5;
    const int intSteps  = App::settings().clpCircleSegments(radius * dScale);
    Path64 polygon(intSteps);
    for(int i{}; auto&& pt: polygon) {
        pt = Point64{
                 cos(i * 2 * pi / intSteps) * radius,
                 sin(i * 2 * pi / intSteps) * radius,
             }
            + center;
        ++i;
    };
    const int32_t idx = RegisterCenter(~center);
    for(auto& pt: polygon) SetCIndices(pt, 0, idx);
    return polygon;
}

Path64 RectanglePath(double width, double height, const Point64& center) {
    const double halfWidth  = width * 0.5;
    const double halfHeight = height * 0.5;
    Path64 polygon{
        {-halfWidth + center.x, +halfHeight + center.y},
        {-halfWidth + center.x, -halfHeight + center.y},
        {+halfWidth + center.x, -halfHeight + center.y},
        {+halfWidth + center.x, +halfHeight + center.y},
        // {-halfWidth + center.x, +halfHeight + center.y},
    };
    r::for_each(polygon, SetCSelf);
    // if(Area(polygon) < 0.0) ReversePath(polygon);
    return polygon;
}

void RotatePath(Path64& path, double angle, const Point64& center) {
    QTransform m;
    if(center.x || center.y)
        m.translate(
            dScale * center.x,
            dScale * center.y);

    if(!qFuzzyIsNull(angle))
        m.rotate(angle);

    if(m.type()) TransformPath(path, m);
}

Path64& TranslatePath(Path64& path, const Point64& pos) {
    if(pos.x || pos.y) {
        const QPointF d = ~pos;
        std::set<int32_t> done;
        for(auto& pt: path) {
            for(int32_t idx: {GetCPrevIndex(pt), GetCNextIndex(pt)})
                if(idx && done.insert(idx).second)
                    SetCenterAt(idx, CenterAt(idx) + d);
            pt.x += pos.x;
            pt.y += pos.y;
        }
    }
    return path;
}

Paths64& TranslatePaths(Paths64& paths, const Point64& pos) {
    r::for_each(paths, std::bind(&TranslatePath, _1, pos));
    return paths;
}

double Perimeter(std::span<const Point64> path, bool open) {
    if(path.size() < 2 /*(open ? 2 : 3)*/) return std::nan("");
    while(path.back() == path.front()) path = path.subspan(1);
    double p{};
    static constexpr auto dist = +[](const Point64& from, const Point64& to) {
        Point64 v = to - from;
        return v.x * v.x + v.y * v.y;
    };
    // for(auto&& [from, to]: path | v::pairwise) p += dist(from, to);
    for(auto&& path: path | v::slide(2)) p += dist(path.front(), path.back());
    if(!open) p += dist(path.back(), path.front());
    return std::sqrt(p);
}

void mergeSegments(Paths64& paths, double glue) {
    size_t size;
    do {
        size = paths.size();
        for(size_t i{}; i < paths.size(); ++i) {
            if(i >= paths.size()) break;
            auto& pi = paths[i];
            for(size_t j{}; j < paths.size(); ++j) {
                if(i == j) continue;
                if(i >= paths.size()) break;
                auto& pj  = paths[j];
                Point64 pib = pi.back();
                Point64 pjf = pj.front();
                if(pib == pjf) {
                    pi.insert(pi.end(), ++pj.begin(), pj.end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point64 pif = pi.front();
                Point64 pjb = pj.back();
                if(pif == pjb) {
                    pj.insert(pj.end(), ++pi.begin(), pi.end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if(pib == pjb) {
                    ReversePath(pj);
                    pi.insert(pi.end(), ++pj.begin(), pj.end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
            }
        }
    } while(size != paths.size());
    if(qFuzzyIsNull(glue)) return;
    do {
        size = paths.size();
        for(size_t i{}; i < paths.size(); ++i) {
            if(i >= paths.size()) break;
            auto& pi = paths[i];
            for(size_t j{}; j < paths.size(); ++j) {
                auto& pj = paths[j];
                if(i == j) continue;
                if(i >= paths.size()) break;
                Point64 pib = pi.back();
                Point64 pjf = pj.front();
                if(distTo(pib, pjf) < glue) {
                    pi.insert(pi.end(), ++pj.begin(), pj.end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point64 pif = pi.front();
                Point64 pjb = pj.back();
                if(distTo(pif, pjb) < glue) {
                    pj.insert(pj.end(), ++pi.begin(), pi.end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if(distTo(pib, pjb) < glue) {
                    ReversePath(pj);
                    pi.insert(pi.end(), ++pj.begin(), pj.end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
            }
        }
    } while(size != paths.size());
}
#if 0
void mergePaths(Paths64& paths, const double dist) {
    //    msg = tr("Merge Paths64");
    size_t max;
    do {
        max = paths.size();
        for(size_t i{}; i < paths.size(); ++i) {
            ProgressCancel::setMax(max);
            ProgressCancel::setCurrent(max - paths.size());
            throwIfCancel();
            auto& pi = paths[i];
            for(size_t j = i + 1; j < paths.size(); ++j) {
                if(i == j) continue;
                auto& pj = paths[j];
                /*  */ if(pi.front() == pj.front()) {
                    ReversePath(pj);
                    pj.insert(pj.end(), ++pi.begin(), pi.end()); // pj.append(pi.mid(1));
                    paths.erase(paths.begin() + i--);            // paths.remove(i--);
                    break;
                } else if(pi.back() == pj.back()) {
                    ReversePath(pj);
                    pi.insert(pi.end(), ++pj.begin(), pj.end()); // pi.append(pj.mid(1));
                    paths.erase(paths.begin() + j--);            // paths.remove(j--);
                    break;
                } else if(pi.front() == pj.back()) {
                    pj.insert(pj.end(), ++pi.begin(), pi.end()); // pj.append(pi.mid(1));
                    paths.erase(paths.begin() + i--);            // paths.remove(i--);
                    break;
                } else if(pj.front() == pi.back()) {
                    pi.insert(pi.end(), ++pj.begin(), pj.end()); // pi.append(pj.mid(1));
                    paths.erase(paths.begin() + j--);            // paths.remove(j--);
                    break;
                } else if(dist != 0.0) {
                    /*  */ if(distTo(pi.back(), pj.back()) < dist) {
                        ReversePath(pj);
                        pi.insert(pi.end(), ++pj.begin(), pi.end()); // pi.append(pj.mid(1));
                        paths.erase(paths.begin() + j--);            // paths.remove(j--);
                        break;                                       //
                    } else if(distTo(pi.back(), pj.front()) < dist) {
                        pi.insert(pi.end(), ++pj.begin(), pi.end()); // pi.append(pj.mid(1));
                        paths.erase(paths.begin() + j--);            // paths.remove(j--);
                        break;                                       //
                    } else if(distTo(pi.front(), pj.back()) < dist) {
                        pj.insert(pj.end(), ++pi.begin(), pj.end()); // pj.append(pi.mid(1));
                        paths.erase(paths.begin() + i--);            // paths.remove(i--);
                        break;
                    } else if(distTo(pi.front(), pj.front()) < dist) {
                        ReversePath(pj);
                        pj.insert(pj.end(), ++pi.begin(), pj.end()); // pj.append(pi.mid(1));
                        paths.erase(paths.begin() + i--);            // paths.remove(i--);
                        break;
                    }
                }
            }
        }
    } while(max != paths.size());
}
#endif

Paths64& normalize(Paths64& paths) {
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddSubject(paths); //    clipper.AddPaths(paths, PathType::Subject, true);
    Rect r(GetBounds(paths));
    Path64 outer = {
        Point64(r.left - uScale, r.top - uScale),
        Point64(r.right + uScale, r.top - uScale),
        Point64(r.right + uScale, r.bottom + uScale),
        Point64(r.left - uScale, r.bottom + uScale),
    };
    // ReversePath(outer);
    clipper.AddSubject({outer}); //      clipper.AddPath(outer, PathType::Subject, true);
    clipper.Execute(ClipType::Difference, FillRule::EvenOdd, paths);
    paths.erase(paths.begin());
    ReversePaths(paths);
    //    /****************************/
    //    std::function<void(PolyTree*)> grouping = [&grouping](PolyTree* node) {
    //         Paths64 paths;

    //        if (node->IsHole()) {
    //            Path64& path = node->Polygon();
    //            paths.push_back(path);
    //            for (size_t i = 0, end = node->Count(); i < end; ++i) {
    //                path = node->Childs[i]->Polygon();
    //                paths.push_back(path);
    //            }
    //            groupedPss.push_back(paths);
    //        }
    //        for (size_t i = 0, end = node->Count(); i < end; ++i)
    //            grouping(node->Childs[i], group);
    //    };
    //    /*********************************/
    //    groupedPss.clear();
    //    grouping(polyTree.GetFirst(), group);

    //    if (group == Grouping::Cutoff) {
    //        if (groupedPss.size() > 1 && groupedPss.front().size() == 2)
    //            groupedPss.erase(groupedPss.begin());
    //    }

    return paths;
}

template <typename T>
struct span {
    size_t w{}, h{};
    T& val;
    span(T& val, size_t w, size_t h)
        : w{w}
        , h{h}
        , val{val} { }
    auto operator[](size_t i) {
        return std::span{val.begin() + i * h, h};
    }
    auto operator[](size_t i) const {
        return std::span{val.begin() + i * h, h};
    }
};

void reductionOfDistance(Path64& path, Point64 point) {
    if(point.x == 0 && point.y == 0) point = path.front();
    // sort by distance

    std::vector<double> data(path.size() * path.size());
    span matrix{data, path.size(), path.size()};

    for(size_t x{}; x < path.size(); ++x)
        for(size_t y{x + 1}; y < path.size(); ++y)
            matrix[x][y] = distTo(path[x], path[y]);

    size_t counter{};
    while(counter < path.size()) {
        size_t selector{};
        double length = std::numeric_limits<double>::max();
        for(size_t i = counter, end = path.size(); i < end; ++i) {
            double length2 = distTo(point, path[i]);
            if(length > length2) {
                length   = length2;
                selector = i;
            }
        }
        qSwap(path[counter], path[selector]);
        point = path[counter++];
    }

    {
        double dist{};
        auto data = path.data();
        for(size_t i{1}; i < path.size(); ++i) {
            double tmp = distTo(*data, *(data + 1));
            dist += tmp;
            ++data;
        }

        data = path.data();
        for(size_t i{0}; i < path.size(); ++i) {
            for(size_t j{i + 1}; j < path.size(); ++j) {
                double tmp = distTo(data[i], data[j]) * dScale;
                qCritical() << u"dist"_s << tmp << i << ~data[i] << j << ~data[j];
            }
        }

        qCritical() << u"length ="_s << dist * dScale;
    }
}

std::span<std::unique_ptr<CL2::PolyPath64>> rwPolyTree(PolyTree& polyTree) {
    auto itB = polyTree.begin();
    auto itE = polyTree.end();
    return {
        reinterpret_cast<CL2::PolyPath64List::iterator&>(itB), // FIXME очень грязный хак
        reinterpret_cast<CL2::PolyPath64List::iterator&>(itE), // FIXME очень грязный хак
    };
}

Path64 arc(const Point64& center, double radius, double start, double stop, int interpolation) {
    enum { // interpolation
        Linear                   = 1,
        ClockwiseCircular        = 2,
        CounterClockwiseCircular = 3
    };
    const double da_sign[4]{0, 0, -1.0, +1.0};
    Path64 points;

    const int intSteps = App::settings().clpCircleSegments(radius * dScale); // MinStepsPerCircle;

    /**/ if(interpolation == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    else if(interpolation == CounterClockwiseCircular && stop <= start)
        stop += 2.0 * pi;

    double angle       = std::abs(stop - start);
    double steps       = std::max(static_cast<int>(ceil(angle / (2.0 * pi) * intSteps)), 2);
    double delta_angle = da_sign[interpolation] * angle * 1.0 / steps;
    const int32_t idx  = RegisterCenter(~center);
    for(int i{1}; i <= steps; i++) { // 1 skip first - back of paths item set center it self
        double theta = start + delta_angle * i;
        SetCIndices(points.emplace_back(
                        center.x + radius * cos(theta),
                        center.y + radius * sin(theta)),
            idx, idx);
    }

    return points;
}

Path64 arc(Point64 p1, Point64 p2, Point64 center, int interpolation) {
    double radius = sqrt(pow((center.x - p1.x), 2) + pow((center.y - p1.y), 2));
    double start  = atan2(p1.y - center.y, p1.x - center.x);
    double stop   = atan2(p2.y - center.y, p2.x - center.x);
    return arc(center, radius, start, stop, interpolation);
}

void mergePaths(Paths64& paths, const double maxDist) {
    qDebug(__FUNCTION__);

    size_t max;

    auto append = [&](size_t& i, size_t& j) {
        paths[i].append_range(paths[j] | skipFront); // paths[i].append(paths[j].mid(1));
        paths -= j--;                                // paths.remove(j--;
    };

    do {
        max = paths.size();
        for(size_t i{}; i < paths.size(); ++i) {
            ProgressCancel::setMax(max);
            ProgressCancel::setCurrent(max - paths.size());
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
#if 0
void markPolyTreeDByNesting(PolyTree& polynode) {
    qDebug(__FUNCTION__);

    int nestCtr{};
    nesting.clear();
    std::function<int(PolyTree&)> sorter = [&sorter, &nestCtr](PolyTree& polynode) {
        ++nestCtr;
        for(auto&& node: polynode)
            sorter(*node);
        return nesting[&polynode] = nestCtr--;
    };
    sorter(polynode);
}

void sortPolyTreeByNesting(PolyTree& polynode) {
    qDebug(__FUNCTION__);

    int nestCtr{};
    nesting.clear();
    std::function<int(PolyTree&)> sorter = [&sorter, &nestCtr, this](PolyTree& polynode) {
        ++nestCtr;
        nesting[&polynode] = nestCtr;
        switch(polynode.Count()) {
        case 0:return nestCtr--;
        case 1:return std::max(nestCtr--, sorter(*reinterpret_cast<CL2::PolyPath64*>(polynode.begin()->get()))); // FIXME очень грязный хак
        default: std::map<int, std::vector<std::unique_ptr<PolyTree>>, std::greater<>> map;
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

Pathss64 stacking(Paths64& paths) {
    qDebug(__FUNCTION__);

    if(paths.empty()) return {};
    Timer t{"stacking"};

    PolyTree polyTree;
    {
        Timer t{"stacking 1"};
        Clipper clipper;
        clipper.AddSubject(paths);
        clipper.AddSubject({boundOfPaths(paths, uScale)});
        clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
        paths.clear();
    }
    sortPolyTreeByNesting(polyTree);

    Pathss64 returnPss;
    /**************************************************************************************/
    // повернуть для уменшения дистанции между путями
    auto rotateDiest = [this](Paths64& paths, Path64& path, std::pair<size_t, size_t> idx) -> bool {
        std::forward_list<size_t> list;
        list.emplace_front(idx.first);
        for(size_t i = paths.size() - 1, index = idx.first; i; --i) {
            double minDist = std::numeric_limits<double>::max();
            Point64 point;
            for(Point64 pt: paths[i - 1]) {
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

    std::function<void(PolyTree*, bool)> stacker = [&stacker, &rotateDiest, &returnPss](PolyTree* node, bool newPaths) {
        if(!returnPss.empty() || newPaths) {
            Path64 path(node->Polygon());
            if(!(gcp.convent() ^ !node->IsHole()) ^ (gcp.side() == Outer))
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
                //                    const Point64& ptd = returnPss.back().back()[id];
                //                    for(size_t is {}; is < path.size(); ++is) {
                //                        const Point64& pts = path[is];
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
                    returnPss.back().emplace_back(std::move(path)); // append to last Paths64
                else
                    returnPss.push_back({std::move(path)}); // new Paths64
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

    for(Paths64& retPaths: returnPss) {
        for(size_t i{}; i < retPaths.size(); ++i)
            if(retPaths[i].empty()) retPaths.erase(retPaths.begin() + i--);
        r::reverse(retPaths);
        for(Path64& path: retPaths)
            path.emplace_back(path.front());
    }

    sortB(returnPss, ~(App::home().pos() + App::zero().pos()));
}

// Pathss64& groupedPaths(Grouping group, int32_t offset, bool skipFrame) {
//     PolyTree polyTree;
//     {
//         Timer t{"Union EvenOdd"};
//         Clipper clipper;
//         clipper.AddSubject(closedSrcPaths);
//         clipper.AddSubject({boundOfPaths(closedSrcPaths, offset)});
//         clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
//     }
//     groupedPss.clear();
//     {
//         Timer t{"grouping"};
//         grouping(group, polyTree.Count() == 1 ? *polyTree[0] : polyTree);
//     }
//     if(skipFrame == false
//         && group == Grouping::Cutoff
//         && groupedPss.size() > 2
//         && groupedPss.front().size() == 2)
//         groupedPss.erase(groupedPss.begin());
//     return groupedPss;
// }

// void grouping(Grouping group, PolyTree& node) {

//     if((group == Grouping::Cutoff) ^ node.IsHole()) {
//         Paths64 paths;
//         paths.reserve(node.Count() + 1);
//         paths.emplace_back(std::move(node.Polygon()));
//         for(auto&& child: node)
//             paths.emplace_back(std::move(child->Polygon()));
//         groupedPss.emplace_back(std::move(paths));
//     }
//     for(auto&& child: node)
//         grouping(group, *child);
// }

#endif

Path64 boundOfPaths(const Paths64& paths, /*PType*/ int32_t k) {
    Rect rect = GetBounds(paths);
    rect.bottom += k;
    rect.left -= k;
    rect.right += k;
    rect.top -= k;
    // dbgPaths({rect.AsPath()}, u"boundOfPaths"_s, Qt::magenta);
    return rect.AsPath();
}

Paths64& sortB(Paths64& src, Point64 startPt) {
    qDebug(__FUNCTION__);
    // Point64 startPt{~(App::home().pos() + App::zero().pos())};
    for(size_t firstIdx{}; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for(size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = distTo(startPt, src[secondIdx].front());
            if(destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].back();
        if(swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Pathss64& sortBeginEnd(Pathss64& src, Point64 startPt) {
    qDebug(__FUNCTION__);

    // Point64 startPt{~(App::home().pos() + App::zero().pos())};
    for(size_t firstIdx{}; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse{};
        for(size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = distTo(startPt, src[secondIdx].front().front());
            const double lenLast  = distTo(startPt, src[secondIdx].back().back());
            if(lenFirst < lenLast) {
                if(destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else if(destLen > lenLast) {
                destLen = lenLast;
                swapIdx = secondIdx;
                reverse = true;
            }
        }
        //        if (reverse)
        //            std::reverse(src[swapIdx].begin(), src[swapIdx].end());
        //        startPt = src[swapIdx].back().back();
        if(swapIdx != firstIdx && !reverse) {
            startPt = src[swapIdx].back().back();
            std::swap(src[firstIdx], src[swapIdx]);
        }
    }
    return src;
}

Paths64& sortBeginEnd(Paths64& src, Point64 startPt) {
    qDebug(__FUNCTION__);

    // Point64 startPt{~(App::home().pos() + App::zero().pos())};
    for(size_t firstIdx{}; firstIdx < src.size(); ++firstIdx) {

        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse{};
        for(size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = distTo(startPt, src[secondIdx].front());
            const double lenLast  = distTo(startPt, src[secondIdx].back());
            if(lenFirst < lenLast) {
                if(destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else if(destLen > lenLast) {
                destLen = lenLast;
                swapIdx = secondIdx;
                reverse = true;
            }
            if(qFuzzyIsNull(destLen))
                break;
        }
        if(reverse)
            ReversePath(src[swapIdx]);
        startPt = src[swapIdx].back();
        if(swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Pathss64& sortB(Pathss64& src, Point64 startPt) {
    qDebug(__FUNCTION__);

    // Point64 startPt{~(App::home().pos() + App::zero().pos())};
    for(size_t i{}; i < src.size(); ++i)
        if(src[i].empty())
            src.erase(src.begin() + i--);
    for(size_t firstIdx{}; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for(size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = distTo(startPt, src[secondIdx].front().front());
            if(destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].back().back();
        if(swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

// } // namespace MC

Path64& TransformPath(Path64& path, const QTransform& m) {
    if(!m.type()) return path;
    std::set<int32_t> done;
    for(Point64& point: path) {
        for(int32_t idx: {GetCPrevIndex(point), GetCNextIndex(point)})
            if(idx && done.insert(idx).second)
                SetCenterAt(idx, m.map(CenterAt(idx)));
        point = ~m.map(~point);
    }
    if((m.m11() < 0) ^ (m.m22() < 0)) ReversePath(path);
    return path;
}

Paths64& TransformPaths(Paths64& paths, const QTransform& m) {
    for(auto&& path: paths) TransformPath(path, m);
    return paths;
}
//------------------------------------------------------------------------------

void addArcTo(QPainterPath& pPath, QPointF source, QPointF target, double bulge) {
    if(pPath.isEmpty())
        pPath.moveTo(source);

    if(qFuzzyIsNull(bulge)) {
        pPath.lineTo(target);
        return;
    }

    auto [center, start_angle, end_angle, radius] = bulgeToArc(source, target, bulge);

    // radius = GetRadius(source, target, bulge);

    start_angle = qRadiansToDegrees(start_angle);
    end_angle   = qRadiansToDegrees(end_angle);

    QLineF ls{center, source};
    // const double r = ls.length();
    const double asource = ls.angle();
    const double atarget = ls.angleTo(QLineF{center, target});

    double span = atarget;

    QRectF rect{
        center.x() - radius,
        center.y() - radius,
        radius * 2,
        radius * 2,
    };

    if(bulge > 0.) span = span - 360.;

#if 0
    const QLineF l1{source, target};
    const double lenght = l1.length() * 0.5;
    const double height = lenght * bulge;
    const double radius = (height * height + lenght * lenght) / (height * 2);

    QLineF l2((source + target) / 2, target);
    l2 = l2.normalVector();
    l2.setLength(height);
    QPointF c(l2.p2());

    QPointF center = [&source, &target, &c] {
        double ax2 = source.x() * source.x();
        double bx2 = target.x() * target.x();
        double cx2 = c.x() * c.x();
        double ay2 = source.y() * source.y();
        double by2 = target.y() * target.y();
        double cy2 = c.y() * c.y();
        double d = source.x() * target.y() + source.y() * c.x() - target.y() * c.x() - source.x() * c.y() - target.x() * source.y() + target.x() * c.y();
        return QPointF(
            +0.5 / d * (                                                                                                //
                source.y() * cx2 + source.y() * cy2 + target.y() * ax2 + target.y() * ay2 + c.y() * bx2 + c.y() * by2 - //
                source.y() * bx2 - source.y() * by2 - target.y() * cx2 - target.y() * cy2 - c.y() * ax2 - c.y() * ay2),
            -0.5 / d * (                                                                                                //
                source.x() * cx2 + source.x() * cy2 + target.x() * ax2 + target.x() * ay2 + c.x() * bx2 + c.x() * by2 - //
                source.x() * bx2 - source.x() * by2 - target.x() * cx2 - target.x() * cy2 - c.x() * ax2 - c.x() * ay2));
    }();

    // const double start_angle = qRadiansToDegrees(start_angle_);
    // const double end_angle = qRadiansToDegrees(end_angle_);
    double start_angle = qRadiansToDegrees(atan2(center.y() - source.y(), center.x() - source.x()));
    double end_angle = qRadiansToDegrees(atan2(center.y() - target.y(), center.x() - target.x()));

    if(end_angle <= start_angle)
        end_angle += 360;

    double span = end_angle - start_angle;

    const QPointF rad{radius, radius};
    const QRectF br{center + rad, center - rad};
    pPath.arcTo(br, -start_angle, -span);
#endif

    pPath.arcTo(rect, asource, span);
}

// индекс дуги, которой принадлежит ребро a->b (0, если ребро - отрезок
// или его концы принадлежат разным дугам)
static int32_t edgeCenterIndex(const Point64& a, const Point64& b) {
    const int32_t idx = GetCNextIndex(a);
    return (idx && idx == GetCPrevIndex(b)) ? idx : 0;
}

// кэш "исходная вершина угла -> индекс нового центра скругления", живёт на время
// одного вызова InflatePathsZ: все точки веера скругления одного и того же угла
// приходят в колбэк с одинаковыми e1top/e2bot, и должны получить ОДИН и тот же индекс
thread_local std::unordered_map<Point64, int32_t> roundJoinCache;

// option1 - static callback function
static void UpdateCenter(
    const Point64& e1bot, const Point64& e1top,
    const Point64& e2bot, const Point64& e2top, Point64& pt) {
    int32_t prevIdx = edgeCenterIndex(e1bot, e1top);
    int32_t nextIdx = edgeCenterIndex(e2bot, e2top);

    // угол между отрезком и отрезком или отрезком и дугой (хотя бы одна из сторон -
    // не продолжение уже известной дуги) при скруглении (JoinType::Round) порождает
    // веер новых точек вокруг исходной (не смещённой) вершины угла - регистрируем
    // её как центр новой дуги один раз на весь веер и переиспользуем индекс
    if((!prevIdx || !nextIdx) && e1top == e2bot) {
        auto [it, inserted] = roundJoinCache.try_emplace(e1top, 0);
        if(inserted) it->second = RegisterCenter(~e1top, CenterKind::RoundJoin);
        prevIdx = nextIdx = it->second;
    }

    SetCIndices(pt, prevIdx, nextIdx);
}

// ClipperOffset (DoMiter/DoSquare/DoBevel/DoRound) НЕ вызывает Z-колбэк при
// построении самого смещённого контура - он просто копирует z исходной вершины
// на ВСЕ новые точки угла/скругления без изменений. Поэтому для сочленений двух
// отрезков, отрезка с дугой, двух разных дуг и концов открытых линий (т.е. везде,
// кроме середины одной непрерывной дуги, где prevIdx==nextIdx!=0) нужно заранее,
// до вызова offset.Execute(), принудительно проставить корректный индекс - иначе
// он останется "старым" от исходной топологии и либо потеряется, либо (что хуже)
// ложно совпадёт с соседями и дуга не восстановится в toCurve().
static void PrepareCornersForOffset(Paths64& paths, JoinType jt) {
    for(auto& path: paths) {
        for(auto& pt: path) {
            const int32_t prevIdx = GetCPrevIndex(pt), nextIdx = GetCNextIndex(pt);
            if(prevIdx && prevIdx == nextIdx) continue; // середина дуги - не трогаем

            if(jt == JoinType::Round) {
                // угол будет скруглён веером новых точек вокруг ИСХОДНОЙ вершины -
                // регистрируем её один раз как центр новой дуги, все точки веера
                // унаследуют этот индекс через копирование z
                const int32_t idx = RegisterCenter(~pt, CenterKind::RoundJoin);
                SetCIndices(pt, idx, idx);
            } else {
                // митр/фаска/срез - реальной дуги здесь не будет, явно очищаем,
                // чтобы устаревший индекс не протёк на новые точки угла
                SetCIndices(pt, 0, 0);
            }
        }
    }
}

Paths64 InflatePathsZ(const Paths64& paths, double delta, JoinType jt, EndType et,
    double miterLimit, double arcTolerance) {
    if(delta == 0.0) return paths;
    roundJoinCache.clear();
    Paths64 input = paths;
    PrepareCornersForOffset(input, jt);
    CL2::ClipperOffset offset{miterLimit, arcTolerance};
    offset.SetZCallback(UpdateCenter);
    offset.AddPaths(input, jt, et);
    Paths64 solution;
    offset.Execute(delta, solution);
    return solution;
}

Paths64 Inflate(const Paths64& paths, double delta,
    JoinType jt, EndType et,
    double miterLimit, double arcTolerance) {
    if(!arcTolerance) arcTolerance = delta * 1e-3;
    return InflatePathsZ(paths, delta * 0.5, jt, et, miterLimit, arcTolerance);
}

Paths64 InflateRoundPolygon(const Paths64& paths,
    double delta, double miterLimit, double arcTolerance) {
    if(!arcTolerance) arcTolerance = delta * 1e-3;
    return InflatePathsZ(paths, delta * 0.5, JoinType::Round, EndType::Polygon, miterLimit, arcTolerance);
}

Paths64 InflateMiterPolygon(const Paths64& paths,
    double delta, double miterLimit, double arcTolerance) {
    if(!arcTolerance) arcTolerance = delta * 1e-3;
    return InflatePathsZ(paths, delta * 0.5, JoinType::Miter, EndType::Polygon, miterLimit, arcTolerance);
}

QDebug operator<<(QDebug d, const Point64& p) {
    Point64 c = GetC(p);
    if((~c).isNull())
        return d << "Point64(" << p.x << ", " << p.y << ')';
    return d << "Point64(" << p.x << ", " << p.y << ", " << c.x << ", " << c.y << ')';
}

bool pointOnPolygon(const QLineF &l2, const Curve &curve, QPointF *ret) {
    const size_t cnt = curve.size();
    qFatal();
    // if(cnt < 2) FIXME
    //     return false;
    // QPointF p;
    // for(size_t i{}; i < cnt; ++i) {
    //     const Point64& pt1 = curve[(i + 1) % cnt];
    //     const Point64& pt2 = curve[i];
    //     QLineF l1(~pt1, ~pt2);
    //     if(QLineF::BoundedIntersection == l1.intersects(l2, &p)) {
    //         if(ret)
    //             *ret = ~p;
    //         return true;
    //     }
    // }
    return false;
}
