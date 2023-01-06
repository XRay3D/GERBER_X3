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
#include "app.h"
#include "qmath.h"
#include <QElapsedTimer>
#include <QLineF>
#include <myclipper.h>
#include <numbers>

Clipper2Lib::PathD CirclePath(double diametr, const Clipper2Lib::PointD& center) {
    if (diametr == 0.0)
        return Clipper2Lib::PathD();

    const double radius = diametr * 0.5;
    const int intSteps = App::settings().clpCircleSegments(radius * dScale);
    Clipper2Lib::PathD poligon(intSteps);
    for (int i = 0; i < intSteps; ++i) {
        poligon[i] = Clipper2Lib::PointD(
            static_cast<Clipper2Lib::cInt>(cos(i * 2 * std::numbers::pi / intSteps) * radius) + center.x,
            static_cast<Clipper2Lib::cInt>(sin(i * 2 * std::numbers::pi / intSteps) * radius) + center.y);
    }
    return poligon;
}

Clipper2Lib::PathD RectanglePath(double width, double height, const Clipper2Lib::PointD& center) {

    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Clipper2Lib::PathD poligon {
        Clipper2Lib::PointD(static_cast<Clipper2Lib::cInt>(-halfWidth + center.x), static_cast<Clipper2Lib::cInt>(+halfHeight + center.y)),
        Clipper2Lib::PointD(static_cast<Clipper2Lib::cInt>(-halfWidth + center.x), static_cast<Clipper2Lib::cInt>(-halfHeight + center.y)),
        Clipper2Lib::PointD(static_cast<Clipper2Lib::cInt>(+halfWidth + center.x), static_cast<Clipper2Lib::cInt>(-halfHeight + center.y)),
        Clipper2Lib::PointD(static_cast<Clipper2Lib::cInt>(+halfWidth + center.x), static_cast<Clipper2Lib::cInt>(+halfHeight + center.y)),
    };
    if (Area(poligon) < 0.0)
        ReversePath(poligon);

    return poligon;
}

void RotatePath(PathD& poligon, double angle, const Clipper2Lib::PointD& center) {
    const bool fl = Area(poligon) < 0;
    for (Clipper2Lib::PointD& pt : poligon) {
        const double dAangle = qDegreesToRadians(angle - center.angleTo(pt));
        const double length = center.distTo(pt);
        pt = Clipper2Lib::PointD(static_cast<Clipper2Lib::cInt>(cos(dAangle) * length), static_cast<Clipper2Lib::cInt>(sin(dAangle) * length));
        pt.x += center.x;
        pt.y += center.y;
    }
    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

void TranslatePath(PathD& path, const Clipper2Lib::PointD& pos) {
    if (pos.x == 0 && pos.y == 0)
        return;
    for (auto& pt : path) {
        pt.x += pos.x;
        pt.y += pos.y;
    }
}

double Perimeter(const PathD& path) {
    double p = 0.0;
    for (size_t i = 0, j = path.size() - 1; i < path.size(); ++i) {
        double x = path[j].x - path[i].x;
        double y = path[j].y - path[i].y;
        p += x * x + y * y;
        j = i;
    }
    return sqrt(p);
}

void mergeSegments(PathsD& paths, double glue) {
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
                Clipper2Lib::PointD pib = paths[i].back();
                Clipper2Lib::PointD pjf = paths[j].front();
                if (pib == pjf) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Clipper2Lib::PointD pif = paths[i].front();
                Clipper2Lib::PointD pjb = paths[j].back();
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
                Clipper2Lib::PointD pib = paths[i].back();
                Clipper2Lib::PointD pjf = paths[j].front();
                if (pib.distTo(pjf) < glue) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                Clipper2Lib::PointD pif = paths[i].front();
                Clipper2Lib::PointD pjb = paths[j].back();
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

void mergePaths(PathsD& paths, const double dist) {
    //    msg = tr("Merge PathsD");
    size_t max;
    do {
        max = paths.size();
        for (size_t i = 0; i < paths.size(); ++i) {
            //            setMax(max);
            //            setCurrent(max - paths.size());
            //            ifCancelThenThrow();
            for (size_t j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                else if (paths[i].front() == paths[j].front()) {
                    ReversePath(paths[j]);
                    paths[j].append(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                } else if (paths[i].back() == paths[j].back()) {
                    ReversePath(paths[j]);
                    paths[i].append(paths[j].mid(1));
                    paths.remove(j--);
                    break;
                } else if (paths[i].front() == paths[j].back()) {
                    paths[j].append(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                } else if (paths[j].front() == paths[i].back()) {
                    paths[i].append(paths[j].mid(1));
                    paths.remove(j--);
                    break;
                }
            }
        }
    } while (max != paths.size());
    if (dist != 0.0) {
        do {
            max = paths.size();
            for (size_t i = 0; i < paths.size(); ++i) {
                for (size_t j = 0; j < paths.size(); ++j) {
                    if (i == j)
                        continue;
                    /*  */ if (paths[i].back().distTo(paths[j].back()) < dist) {
                        ReversePath(paths[j]);
                        paths[i].append(paths[j].mid(1));
                        paths.remove(j--);
                        break; //
                    } else if (paths[i].back().distTo(paths[j].front()) < dist) {
                        paths[i].append(paths[j].mid(1));
                        paths.remove(j--);
                        break; //
                    } else if (paths[i].front().distTo(paths[j].back()) < dist) {
                        paths[j].append(paths[i].mid(1));
                        paths.remove(i--);
                        break;
                    } else if (paths[i].front().distTo(paths[j].front()) < dist) {
                        ReversePath(paths[j]);
                        paths[j].append(paths[i].mid(1));
                        paths.remove(i--);
                        break;
                    }
                }
            }
        } while (max != paths.size());
    }
}

#include <QMutex>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

QIcon drawIcon(const PathsD& paths) {
    static QMutex m;
    QMutexLocker l(&m);

    QPainterPath painterPath;

    for (auto&& polygon : paths)
        painterPath.addPolygon(polygon);

    const QRectF rect = painterPath.boundingRect();

    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;

    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
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

PathsD& normalize(PathsD& paths) {
    PolyTreeD polyTree;
    ClipperD clipper;
    clipper.AddSubject(paths); //    clipper.AddPaths(paths, PathType::Subject, true);
    RectD r(GetBounds(paths));
    Clipper2Lib::PathD outer = {
        Clipper2Lib::PointD(r.left - uScale, r.top - uScale),
        Clipper2Lib::PointD(r.right + uScale, r.top - uScale),
        Clipper2Lib::PointD(r.right + uScale, r.bottom + uScale),
        Clipper2Lib::PointD(r.left - uScale, r.bottom + uScale),
    };
    // ReversePath(outer);
    clipper.AddSubject({outer}); //      clipper.AddPath(outer, PathType::Subject, true);
    clipper.Execute(ClipType::Difference, FillRule::EvenOdd, paths);
    paths.erase(paths.begin());
    ReversePaths(paths);
    //    /****************************/
    //    std::function<void(PolyNode*)> grouping = [&grouping](PolyNode* node) {
    //         Clipper2Lib::PathsD paths;

    //        if (node->IsHole()) {
    //            PathD& path = node->Contour;
    //            paths.push_back(path);
    //            for (size_t i = 0, end = node->ChildCount(); i < end; ++i) {
    //                path = node->Childs[i]->Contour;
    //                paths.push_back(path);
    //            }
    //            groupedPss.push_back(paths);
    //        }
    //        for (size_t i = 0, end = node->ChildCount(); i < end; ++i)
    //            grouping(node->Childs[i], group);
    //    };
    //    /*********************************/
    //    groupedPss.clear();
    //    grouping(polyTree.GetFirst(), group);

    //    if (group == CutoffPaths) {
    //        if (groupedPss.size() > 1 && groupedPss.front().size() == 2)
    //            groupedPss.erase(groupedPss.begin());
    //    }

    return paths;
}
