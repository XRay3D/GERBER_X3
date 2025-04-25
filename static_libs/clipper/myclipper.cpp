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
// module;

#include "myclipper.h"

#include "app.h"
#include "cancelation.h"
#include "graphicsview.h"

#include "qmath.h"
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QLineF>
#include <boost/range/combine.hpp>
#include <qglobal.h>
#include <set>

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <forward_list>

// export module myclipper;

// export namespace MC {

QIcon drawIcon(const Paths& paths, QColor color) {
    static std::mutex m;
    std::lock_guard l{m};

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

double Length(const Point& A, const Point& B) {
    return hypot(A.x - B.x, A.y - B.y);
}

double Angle(double a, double b, double c) {
    return acos((b * b + c * c - a * a) / (2 * b * c));
}

// double c = Length(~*it, ~side);
// double aa = acos((a * a + c * c - b * b) / (2 * a * c)); // Для угла α: cos(α)
// double ab = acos((a * a + b * b - c * c) / (2 * a * b)); // Для угла β: cos(β)
// double ac = acos((b * b + c * c - a * a) / (2 * b * c)); // Для угла γ: cos(γ)
// После нахождения косинусов углов можно получить сами углы
// путём нахождения арккосинусов соответствующих значений:
// α = arccos(cos(α)), β = arccos(cos(β)), γ = arccos(cos(γ)).
// Результаты арккосинусов будут выражены в радианах,
// их можно перевести в градусы, умножив на (180/pi). 3

void TestPaths(const Paths& paths_) {
    return;
    Paths paths = paths_;
    std::set<QPointF> set;
    QPainterPath pp;
    QPainterPath ppr;
    auto const scene = App::grView().scene();

    auto test = [](auto& toErace, const Point& side, Path::iterator it) {
        if(side == *it) { // toErace
            toErace.emplace_back(it);
            return false;
        }
        Point center = GetZ(side);
        if(center == side) return false; //"skip side"

        double a = Length(center, side) * dScale;
        double b = Length(center, *it) * dScale;

        if(abs(a - b) < 1e-3) {
            qWarning() << "same side" << a << b << abs(a - b);
            SetZf(*it, center);
            return true;
        }
        return false;
    };

    // fix single point arc centers
    for(auto&& path: paths) {
        std::unordered_map<Point, std::vector<Path::iterator>> counter;
        for(auto it = path.begin(); it < path.end(); ++it) counter[GetZ(*it)].emplace_back(it);

        const auto count = std::erase_if(counter, [](const auto& item) {
            const auto& [key, value] = item;
            return value.size() > 1;
        });

        auto getPrev = [&path](Path::iterator it) {
            size_t pos = static_cast<size_t>(std::distance(path.begin(), it));
            return path[--pos % path.size()];
        };
        auto getNext = [&path](Path::iterator it) {
            size_t pos = static_cast<size_t>(std::distance(path.begin(), it));
            return path[++pos % path.size()];
        };

        std::vector<Path::iterator> toErace;

        qCritical() << counter.size() << count;

        for(auto it = path.begin(); it < path.end(); ++it) {
            if(it->z == 0 || *it == GetZ(*it))
                if(qInfo("getPrev"); !test(toErace, getPrev(it), it))
                    if(qInfo("getNext"); !test(toErace, getNext(it), it))
                        continue;
        }

        // for(auto&& [center, iters]: counter) {
        //     auto it = iters.front();
        //     if(qInfo("getPrev"); !test(getPrev(it), it))
        //         if(qInfo("getNext"); !test(getNext(it), it))
        //             continue;
        // }
        if(toErace.size())
            qCritical() << "toErace" << toErace.size();

        // break;
    }

    // std::unordered_map<CenterKey, Path> arcs;
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

            // continue;

            Point center, currPt, source, target;
            double srcA, dstA;
            radius = 0.0;

            auto addArc = [&]() {
                radius = Length(center, source) * dScale;
                if(qFuzzyIsNull(radius)) return;
                QRectF arcRect{
                    ~center - QPointF{radius, radius},
                    ~center + QPointF{radius, radius}
                };
                bool isClockwise{};
                // angles
                const double asource = atan2(source.y - center.y, source.x - center.x);
                const double atarget = atan2(target.y - center.y, target.x - center.x);
                double aspan = atarget - asource;
                /**/ if(aspan < -pi || (qFuzzyCompare(aspan, -pi) && !isClockwise))
                    aspan += 2.0 * pi;
                else if(aspan > +pi || (qFuzzyCompare(aspan, -pi) && isClockwise))
                    aspan -= 2.0 * pi;
                pp.arcTo(arcRect, qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
            };

            // enum {
            //     Inner,
            //     Corner,
            //     Source,
            //     Target,
            // };

            source = path.front();
            for(int i{}; i < path.size(); ++i) {
                if(source.z == path[i].z) continue;
                source = path[i];
                pp.moveTo(~source);
                for(int j{}, k; j <= path.size(); ++j) {
                    currPt = path[i % path.size()];
                    if(!currPt.z || currPt == GetZ(currPt)) {
                        if(i - k == 1) {
                            source = currPt;
                            pp.lineTo(~source);
                        } else {
                            center = GetZ(source);
                            target = path[(i - 1) % path.size()];
                            addArc();
                            source = currPt;
                        }
                        k = i;
                    } else if(source.z != currPt.z) {
                        qCritical() << "arc" << (i - k);
                        pp.lineTo(~source);
                        if(i - k == 1) {
                            source = currPt;
                            pp.lineTo(~source);
                        } else {
                            center = GetZ(source);
                            target = path[(i - 1) % path.size()];
                            addArc();
                            source = currPt;
                        }
                        k = i;
                    }
                    ++i;
                    pp.lineTo(~source);
                }
                // break;
            }
        }
        // break;
    }

    scene->addPath(ppr, {Qt::darkGreen, 0.0})->setZValue(std::numeric_limits<double>::max());
    scene->addPath(pp, {Qt::white, 0.0})->setZValue(std::numeric_limits<double>::max());
    // new Gi::Debug{pp};
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
#if 0
void mergePaths(Paths& paths, const double dist) {
    //    msg = tr("Merge Paths");
    size_t max;
    do {
        max = paths.size();
        for(size_t i = 0; i < paths.size(); ++i) {
            ProgressCancel::setMax(max);
            ProgressCancel::setCurrent(max - paths.size());
            ifCancelThenThrow();
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

void mergePaths(Paths& paths, const double maxDist) {
    qDebug(__FUNCTION__);

    size_t max;

    auto append = [&](size_t& i, size_t& j) {
        paths[i] += paths[j] | skipFront; // paths[i].append(paths[j].mid(1));
        paths -= j--;                     // paths.remove(j--;
    };

    do {
        max = paths.size();
        for(size_t i{}; i < paths.size(); ++i) {
            ProgressCancel::setMax(max);
            ProgressCancel::setCurrent(max - paths.size());
            ifCancelThenThrow();
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

Pathss stacking(Paths& paths) {
    qDebug(__FUNCTION__);

    if(paths.empty()) return {};
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

    Pathss returnPss;
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

    std::function<void(PolyTree*, bool)> stacker = [&stacker, &rotateDiest, &returnPss](PolyTree* node, bool newPaths) {
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

// Pathss& groupedPaths(Grouping group, int32_t offset, bool skipFrame) {
//     PolyTree polyTree;
//     {
//         Timer t("Union EvenOdd");
//         Clipper clipper;
//         clipper.AddSubject(closedSrcPaths);
//         clipper.AddSubject({boundOfPaths(closedSrcPaths, offset)});
//         clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
//     }
//     groupedPss.clear();
//     {
//         Timer t("grouping");
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
//         Paths paths;
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

Path boundOfPaths(const Paths& paths, /*Point::Type*/ int32_t k) {
    Rect rect(GetBounds(paths));
    rect.bottom += k;
    rect.left -= k;
    rect.right += k;
    rect.top -= k;
    // dbgPaths({rect.AsPath()}, "boundOfPaths", Qt::magenta);
    return rect.AsPath();
}

Paths& sortB(Paths& src, Point startPt) {
    qDebug(__FUNCTION__);
    // Point startPt{~(App::home().pos() + App::zero().pos())};
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

Pathss& sortBeginEnd(Pathss& src, Point startPt) {
    qDebug(__FUNCTION__);

    // Point startPt{~(App::home().pos() + App::zero().pos())};
    for(size_t firstIdx{}; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse = false;
        for(size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = distTo(startPt, src[secondIdx].front().front());
            const double lenLast = distTo(startPt, src[secondIdx].back().back());
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

Paths& sortBeginEnd(Paths& src, Point startPt) {
    qDebug(__FUNCTION__);

    // Point startPt{~(App::home().pos() + App::zero().pos())};
    for(size_t firstIdx{}; firstIdx < src.size(); ++firstIdx) {

        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse = false;
        for(size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = distTo(startPt, src[secondIdx].front());
            const double lenLast = distTo(startPt, src[secondIdx].back());
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

Pathss& sortB(Pathss& src, Point startPt) {
    qDebug(__FUNCTION__);

    // Point startPt{~(App::home().pos() + App::zero().pos())};
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
