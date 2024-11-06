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
 ********************************************************************************/
#include "myclipper.h"

#include "app.h"
#include "graphicsview.h"

#include "qmath.h"
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QLineF>
#include <boost/range/combine.hpp>
#include <set>

using namespace std::placeholders;

bool operator<(const QPointF& r, const QPointF& l) {
    return r.x() < l.x() && r.y() < l.y();
}

bool operator<(const Point& r, const Point& l) {
    return r.x < l.x && r.y < l.y;
}

template <>
struct std::hash<Point> {
    std::size_t operator()(const Point& pt) const noexcept {
        // const std::pair pair{pt.x, pt.y};
        // return std::hash<decltype(pair)>{}(pair);
        std::size_t h1 = std::hash<int64_t>{}(pt.x);
        std::size_t h2 = std::hash<int64_t>{}(pt.y);
        return h1 ^ (h2 << 1);
    }
};

using CenterKey = std::pair<Point, const void*>;

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

void TestPaths(const Paths& paths_) {
    Paths paths = paths_;
    std::set<QPointF> set;
    QPainterPath pp;
    QPainterPath ppr;
    auto scene = App::grView().scene();
    // for(auto&& p: std::views::join(paths)) {
    //     auto ptCenter = ~GetZ(p);
    //     if(ptCenter.isNull()) continue;
    //     auto ptRadius = ~p;
    //     QLineF lineRadius{ptCenter, ptRadius};
    //     auto len = lineRadius.length();
    //     QRectF ellipseRect{
    //         ptCenter - QPointF{len, len},
    //         ptCenter + QPointF{len, len}
    //     };
    //     // if(set.emplace(ptCenter).second)
    //     //     scene->addEllipse(ellipseRect, {Qt::white, 0.0})->setZValue(std::numeric_limits<double>::max());
    //     // scene->addLine(lineRadius, {Qt::gray, 0.0})->setZValue(std::numeric_limits<double>::max());
    //     // if(set.emplace(p).second)
    //     // pp.addEllipse(pc, len, len);
    //     if(set.emplace(ptCenter).second)
    //         pp.addEllipse(ellipseRect);
    //     pp.moveTo(ptCenter);
    //     pp.lineTo(ptRadius);
    // }
    // // scene->addEllipse(r, {Qt::white, 0.0})->setZValue(std::numeric_limits<double>::max());
    // // scene->addLine(line, {Qt::gray, 0.0})->setZValue(std::numeric_limits<double>::max());
    // scene->addPath(pp, {Qt::gray, 0.0})->setZValue(std::numeric_limits<double>::max());

    // fix single point arc
    for(auto&& path: paths) {
        std::unordered_map<Point, std::vector<Path::iterator>> counter;
        for(auto it = path.begin(); it < path.end(); ++it) counter[GetZ(*it)].emplace_back(it);
        qCritical() << counter.size();
        const auto count = std::erase_if(counter, [](const auto& item) {
            const auto& [key, value] = item;
            return value.size() > 1;
        });
        qCritical() << counter.size() << count;
        for(auto&& [center, iters]: counter) {

        }
    }

    std::unordered_map<CenterKey, Path> arcs;
    for(auto&& path: paths) {
        if(path.empty()) continue;

        double radius{};

        auto count = ranges::count_if(path,
            [prevR = 0.0, center = GetZ(path.front()), &radius](const Point& pt) mutable {
                const double r = QLineF{~center, ~pt}.length();
                radius += r;
                if(prevR == 0.0) prevR = r;
                bool fl = (center == GetZ(pt)) && (abs(prevR - r) < 1e-5);
                prevR = r;
                return fl;
            });
        bool isCircle = count == path.size() || (count == path.size() - 1);
        if(isCircle) {
            qWarning() << count << path.size() << radius << (radius /= path.size());
            auto center = ~GetZ(path.front());
            pp.addEllipse(center, radius, radius);
        } else {
            for(auto it = path.begin(); it < path.end(); ++it) {
                auto&& p = *it;
                if(!p.z) continue;
                ppr.moveTo(~GetZ(p));
                ppr.lineTo(~p);
            }

            continue;

            Point center, currPt, prewPt, nextPt;
            double srcA, dstA;
            radius = 0.0;

            auto addArc = [&]() {
                if(radius == 0.0) return;
                QLineF line{~center, ~currPt};
                radius = line.length();
                dstA = line.angle();
                qDebug() << line.angleTo({~center, ~prewPt});
                QRectF arcRect{
                    ~center - QPointF{radius, radius},
                    ~center + QPointF{radius, radius}
                };
                pp.arcTo(arcRect, srcA, srcA - dstA);
            };

            for(int i = 0; i < path.size(); ++i) {
                currPt = path[i];
                prewPt = path[(i - 1) % path.size()];
                nextPt = path[(i + 1) % path.size()];
                if(!currPt.z) {
                    !i ? pp.moveTo(~currPt) : pp.lineTo(~currPt);
                    addArc();
                    radius = 0.0;
                    continue;
                } else if(auto c = GetZ(currPt); center == currPt) {
                    !i ? pp.moveTo(~currPt) : pp.lineTo(~currPt);
                    addArc();
                    radius = 0.0;
                } else if(center != c) {
                    !i ? pp.moveTo(~currPt) : pp.lineTo(~currPt);
                    center = c;
                    QLineF line{~center, ~currPt};
                    addArc();
                    radius = line.length();
                    srcA = line.angle();
                }

                // arcs[{center, path.data()}].emplace_back(p);
                // if(set.emplace(ptCenter).second) {
                //     QRectF ellipseRect{
                //         ptCenter - QPointF{len, len},
                //         ptCenter + QPointF{len, len}
                //     };
                //     pp.addEllipse(ellipseRect);
                // }
                // pp.moveTo(ptCenter);
                // pp.lineTo(ptRadius);
            }
        }
    }
    // for(auto&& [center, path]: arcs) {
    //     pp.moveTo(~path.front());
    //     pp.addPolygon(~path);
    // }
    // scene->addEllipse(r, {Qt::white, 0.0})->setZValue(std::numeric_limits<double>::max());
    // scene->addLine(line, {Qt::gray, 0.0})->setZValue(std::numeric_limits<double>::max());
    scene->addPath(ppr, {Qt::darkGreen, 0.0})->setZValue(std::numeric_limits<double>::max());
    scene->addPath(pp, {Qt::white, 0.0})->setZValue(std::numeric_limits<double>::max());
}

QDataStream& operator<<(QDataStream& stream, const Point& pt) {
    return stream << static_cast<int32_t>(pt.x) << static_cast<int32_t>(pt.y);
}

QDataStream& operator>>(QDataStream& stream, Point& pt) {
    int32_t x, y;
    return stream >> x >> y, pt.Init(x, y), stream;
}

Point GetZ(const Point& dst) {
    int32_t array[2];
    memcpy(&array, &dst.z, 8);
    return {array[0], array[1]};
}

void SetZs(Point& dst) { SetZ(dst, dst); }

void SetZf(Point& dst, const Point& center) {
    constexpr auto limInt32 = std::numeric_limits<int32_t>{};
    assert(limInt32.min() < center.x && center.x < limInt32.max());
    assert(limInt32.min() < center.y && center.y < limInt32.max());
    const int32_t array[]{static_cast<int32_t>(center.x), static_cast<int32_t>(center.y)};
    memcpy(&dst.z, array, 8);
}

void SetZ(Point& dst, const Point& center) {
    if(!/*~*/ dst.z) {
        constexpr auto limInt32 = std::numeric_limits<int32_t>{};
        assert(limInt32.min() < center.x && center.x < limInt32.max());
        assert(limInt32.min() < center.y && center.y < limInt32.max());
        const int32_t array[]{static_cast<int32_t>(center.x), static_cast<int32_t>(center.y)};
        memcpy(&dst.z, array, 8);
    }
}

Path CirclePath(double diametr, const Point& center) {
    if(qFuzzyIsNull(diametr)) return {};
    const double radius = diametr * 0.5;
    const int intSteps = App::settings().clpCircleSegments(radius * dScale);
    Path polygon(intSteps);
    for(int i{}; auto&& pt: polygon) {
        pt = Point{
                 cos(i * 2 * pi / intSteps) * radius,
                 sin(i * 2 * pi / intSteps) * radius,
             }
            + center;
        ++i;
    };
    std::ranges::for_each(polygon, std::bind(&SetZ, _1, center));
    return polygon;
}

Path RectanglePath(double width, double height, const Point& center) {
    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Path polygon{
        {-halfWidth + center.x, +halfHeight + center.y},
        {-halfWidth + center.x, -halfHeight + center.y},
        {+halfWidth + center.x, -halfHeight + center.y},
        {+halfWidth + center.x, +halfHeight + center.y},
        // {-halfWidth + center.x, +halfHeight + center.y},
    };
    std::ranges::for_each(polygon, &SetZs);
    // if(Area(polygon) < 0.0) ReversePath(polygon);
    return polygon;
}

void RotatePath(Path& polygon, double angle, const Point& center) {
    const bool fl = Area(polygon) < 0;
    for(Point& pt: polygon) {
        const double dAangle = qDegreesToRadians(angle - angleTo(center, pt));
        const double length = distTo(center, pt);
        pt = Point{cos(dAangle) * length, sin(dAangle) * length};
        pt.x += center.x;
        pt.y += center.y;
    }
    if(fl != (Area(polygon) < 0))
        ReversePath(polygon);
}

Path& TranslatePath(Path& path, const Point& pos) {
    if(pos.x || pos.y)
        for(auto& pt: path) {
            pt.x += pos.x;
            pt.y += pos.y;
            SetZf(pt, GetZ(pt) + pos);
        }
    return path;
}

Paths& TranslatePaths(Paths& paths, const Point& pos) {
    std::ranges::for_each(paths, std::bind(&TranslatePath, _1, pos));
    return paths;
}

double Perimeter(const Path& path) {
    double p = 0.0;
    for(size_t i = 0, j = path.size() - 1; i < path.size(); ++i) {
        double x = path[j].x - path[i].x;
        double y = path[j].y - path[i].y;
        p += x * x + y * y;
        j = i;
    }
    return sqrt(p);
}

void mergeSegments(Paths& paths, double glue) {
    size_t size;
    do {
        size = paths.size();
        for(size_t i = 0; i < paths.size(); ++i) {
            if(i >= paths.size()) break;
            auto& pi = paths[i];
            for(size_t j = 0; j < paths.size(); ++j) {
                if(i == j) continue;
                if(i >= paths.size()) break;
                auto& pj = paths[j];
                Point pib = pi.back();
                Point pjf = pj.front();
                if(pib == pjf) {
                    pi.insert(pi.end(), ++pj.begin(), pj.end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point pif = pi.front();
                Point pjb = pj.back();
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
        for(size_t i = 0; i < paths.size(); ++i) {
            if(i >= paths.size()) break;
            auto& pi = paths[i];
            for(size_t j = 0; j < paths.size(); ++j) {
                auto& pj = paths[j];
                if(i == j) continue;
                if(i >= paths.size()) break;
                Point pib = pi.back();
                Point pjf = pj.front();
                if(distTo(pib, pjf) < glue) {
                    pi.insert(pi.end(), ++pj.begin(), pj.end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point pif = pi.front();
                Point pjb = pj.back();
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

void mergePaths(Paths& paths, const double dist) {
    //    msg = tr("Merge Paths");
    size_t max;
    do {
        max = paths.size();
        for(size_t i = 0; i < paths.size(); ++i) {
            //            setMax(max);
            //            setCurrent(max - paths.size());
            //            ifCancelThenThrow();
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

#include <QMutex>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

QIcon drawIcon(const Paths& paths, QColor color) {
    static QMutex m;
    QMutexLocker l(&m);

    QPainterPath painterPath;

    for(auto&& polygon: paths)
        painterPath.addPolygon(~polygon);

    const QRectF rect = painterPath.boundingRect();

    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if(rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;

    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(painterPath);
    return pixmap;
}

QIcon drawDrillIcon(QColor color) {
    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(QRect(0, 0, IconSize - 1, IconSize - 1));
    return pixmap;
}

Paths& normalize(Paths& paths) {
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddSubject(paths); //    clipper.AddPaths(paths, PathType::Subject, true);
    Rect r(GetBounds(paths));
    Path outer = {
        Point(r.left - uScale, r.top - uScale),
        Point(r.right + uScale, r.top - uScale),
        Point(r.right + uScale, r.bottom + uScale),
        Point(r.left - uScale, r.bottom + uScale),
    };
    // ReversePath(outer);
    clipper.AddSubject({outer}); //      clipper.AddPath(outer, PathType::Subject, true);
    clipper.Execute(ClipType::Difference, FillRule::EvenOdd, paths);
    paths.erase(paths.begin());
    ReversePaths(paths);
    //    /****************************/
    //    std::function<void(PolyTree*)> grouping = [&grouping](PolyTree* node) {
    //         Paths paths;

    //        if (node->IsHole()) {
    //            Path& path = node->Polygon();
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
        : val{val}
        , w{w}
        , h{h} { }
    auto operator[](size_t i) {
        return std::span{val.begin() + i * h, h};
    }
    auto operator[](size_t i) const {
        return std::span{val.begin() + i * h, h};
    }
};

void reductionOfDistance(Path& path, Point point) {
    if(point.x == 0 & point.y == 0) point = path.front();
    // sort by distance

    std::vector<double> data(path.size() * path.size());
    span matrix{data, path.size(), path.size()};

    for(int x{}; x < path.size(); ++x)
        for(int y{x + 1}; y < path.size(); ++y)
            matrix[x][y] = distTo(path[x], path[y]);

    size_t counter = 0;
    while(counter < path.size()) {
        size_t selector = 0;
        double length = std::numeric_limits<double>::max();
        for(size_t i = counter, end = path.size(); i < end; ++i) {
            double length2 = distTo(point, path[i]);
            if(length > length2) {
                length = length2;
                selector = i;
            }
        }
        qSwap(path[counter], path[selector]);
        point = path[counter++];
    }

    {
        double dist{};
        auto data = path.data();
        for(int i{1}; i < path.size(); ++i) {
            double tmp = distTo(*data, *(data + 1));
            dist += tmp;
            ++data;
        }

        data = path.data();
        for(int i{0}; i < path.size(); ++i) {
            for(int j{i + 1}; j < path.size(); ++j) {
                double tmp = distTo(data[i], data[j]) * dScale;
                qCritical() << "dist" << tmp << i << ~data[i] << j << ~data[j];
            }
        }

        qCritical() << "length =" << dist * dScale;
    }
}

Path arc(const Point& center, double radius, double start, double stop, int interpolation) {
    enum { // interpolation
        Linear = 1,
        ClockwiseCircular = 2,
        CounterClockwiseCircular = 3
    };
    const double da_sign[4] = {0, 0, -1.0, +1.0};
    Path points;

    const int intSteps = App::settings().clpCircleSegments(radius * dScale); // MinStepsPerCircle;

    /**/ if(interpolation == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    else if(interpolation == CounterClockwiseCircular && stop <= start)
        stop += 2.0 * pi;

    double angle = qAbs(stop - start);
    double steps = std::max(static_cast<int>(ceil(angle / (2.0 * pi) * intSteps)), 2);
    double delta_angle = da_sign[interpolation] * angle * 1.0 / steps;
    for(int i = 0; i < steps; i++) {
        double theta = start + delta_angle * (i + 1);
        SetZ(points.emplace_back(
                 center.x + radius * cos(theta),
                 center.y + radius * sin(theta)),
            center);
    }

    return points;
}

Path arc(Point p1, Point p2, Point center, int interpolation) {
    double radius = sqrt(pow((center.x - p1.x), 2) + pow((center.y - p1.y), 2));
    double start = atan2(p1.y - center.y, p1.x - center.x);
    double stop = atan2(p2.y - center.y, p2.x - center.x);
    return arc(center, radius, start, stop, interpolation);
}
