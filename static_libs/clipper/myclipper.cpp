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
#include "app.h"
#include "qmath.h"
#include <QElapsedTimer>
#include <QLineF>
#include <myclipper.h>
#include <numbers>

QDataStream& operator<<(QDataStream& stream, const Point& pt) {
    return stream << static_cast<int32_t>(pt.x) << static_cast<int32_t>(pt.y);
}

QDataStream& operator>>(QDataStream& stream, Point& pt) {
    int32_t x, y;
    return stream >> x >> y, pt.Init(x, y), stream;
}

Path CirclePath(double diametr, const Point& center) {
    if(diametr == 0.0)
        return Path();

    const double radius = diametr * 0.5;
    const int intSteps = App::settings().clpCircleSegments(radius * dScale);
    Path poligon(intSteps);
    for(int i{}; auto&& pt: poligon) {
        pt = Point{cos(i * 2 * pi / intSteps) * radius, sin(i * 2 * pi / intSteps) * radius}
            + center;
        ++i;
    };
    return poligon;
}

Path RectanglePath(double width, double height, const Point& center) {

    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Path poligon{
        Point{-halfWidth + center.x, +halfHeight + center.y},
        Point{-halfWidth + center.x, -halfHeight + center.y},
        Point{+halfWidth + center.x, -halfHeight + center.y},
        Point{+halfWidth + center.x, +halfHeight + center.y},
    };
    if(Area(poligon) < 0.0)
        ReversePath(poligon);

    return poligon;
}

void RotatePath(Path& poligon, double angle, const Point& center) {
    const bool fl = Area(poligon) < 0;
    for(Point& pt: poligon) {
        const double dAangle = qDegreesToRadians(angle - angleTo(center, pt));
        const double length = distTo(center, pt);
        pt = Point{cos(dAangle) * length, sin(dAangle) * length};
        pt.x += center.x;
        pt.y += center.y;
    }
    if(fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

Path& TranslatePath(Path& path, const Point& pos) {
    if(pos.x != 0 || pos.y != 0)
        for(auto& pt: path) {
            pt.x += pos.x;
            pt.y += pos.y;
        }
    return path;
}

Paths& TranslatePaths(Paths& paths, const Point& pos) {
    for(auto&& path: paths)
        TranslatePath(path, pos);
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
            if(i >= paths.size())
                break;
            for(size_t j = 0; j < paths.size(); ++j) {
                if(i == j)
                    continue;
                if(i >= paths.size())
                    break;
                Point pib = paths[i].back();
                Point pjf = paths[j].front();
                if(pib == pjf) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point pif = paths[i].front();
                Point pjb = paths[j].back();
                if(pif == pjb) {
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[i].end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if(pib == pjb) {
                    ReversePath(paths[j]);
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
            }
        }
    } while(size != paths.size());
    if(qFuzzyIsNull(glue))
        return;
    do {
        size = paths.size();
        for(size_t i = 0; i < paths.size(); ++i) {
            if(i >= paths.size())
                break;
            for(size_t j = 0; j < paths.size(); ++j) {
                if(i == j)
                    continue;
                if(i >= paths.size())
                    break;
                Point pib = paths[i].back();
                Point pjf = paths[j].front();
                if(distTo(pib, pjf) < glue) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Point pif = paths[i].front();
                Point pjb = paths[j].back();
                if(distTo(pif, pjb) < glue) {
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[i].end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if(distTo(pib, pjb) < glue) {
                    ReversePath(paths[j]);
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
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
            for(size_t j = 0; j < paths.size(); ++j) {
                if(i == j)
                    continue;
                else if(paths[i].front() == paths[j].front()) {
                    ReversePath(paths[j]);
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[j].end()); // paths[j].append(paths[i].mid(1));
                    paths.erase(paths.begin() + i--);                                      // paths.remove(i--);
                    break;
                } else if(paths[i].back() == paths[j].back()) {
                    ReversePath(paths[j]);
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[i].end()); // paths[i].append(paths[j].mid(1));
                    paths.erase(paths.begin() + j--);                                      // paths.remove(j--);
                    break;
                } else if(paths[i].front() == paths[j].back()) {
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[j].end()); // paths[j].append(paths[i].mid(1));
                    paths.erase(paths.begin() + i--);                                      // paths.remove(i--);
                    break;
                } else if(paths[j].front() == paths[i].back()) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[i].end()); // paths[i].append(paths[j].mid(1));
                    paths.erase(paths.begin() + j--);                                      // paths.remove(j--);
                    break;
                }
            }
        }
    } while(max != paths.size());
    if(dist != 0.0) {
        do {
            max = paths.size();
            for(size_t i = 0; i < paths.size(); ++i) {
                for(size_t j = 0; j < paths.size(); ++j) {
                    if(i == j)
                        continue;
                    /*  */ if(distTo(paths[i].back(), paths[j].back()) < dist) {
                        ReversePath(paths[j]);
                        paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[i].end()); // paths[i].append(paths[j].mid(1));
                        paths.erase(paths.begin() + j--);                                      // paths.remove(j--);
                        break;                                                                 //
                    } else if(distTo(paths[i].back(), paths[j].front()) < dist) {
                        paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[i].end()); // paths[i].append(paths[j].mid(1));
                        paths.erase(paths.begin() + j--);                                      // paths.remove(j--);
                        break;                                                                 //
                    } else if(distTo(paths[i].front(), paths[j].back()) < dist) {
                        paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[j].end()); // paths[j].append(paths[i].mid(1));
                        paths.erase(paths.begin() + i--);                                      // paths.remove(i--);
                        break;
                    } else if(distTo(paths[i].front(), paths[j].front()) < dist) {
                        ReversePath(paths[j]);
                        paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[j].end()); // paths[j].append(paths[i].mid(1));
                        paths.erase(paths.begin() + i--);                                      // paths.remove(i--);
                        break;
                    }
                }
            }
        } while(max != paths.size());
    }
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
