// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "app.h"
#include "qmath.h"
#include <QElapsedTimer>
#include <QLineF>
#include <myclipper.h>

// static inline bool qt_is_finite(double d)
//{
//     uchar* ch = (uchar*)&d;
//#ifdef QT_ARMFPA
//     return (ch[3] & 0x7f) != 0x7f || (ch[2] & 0xf0) != 0xf0;
//#else
//     if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
//         return (ch[0] & 0x7f) != 0x7f || (ch[1] & 0xf0) != 0xf0;
//     } else {
//         return (ch[7] & 0x7f) != 0x7f || (ch[6] & 0xf0) != 0xf0;
//     }
//#endif
// }

Path CirclePath(double diametr, const IntPoint& center) {
    if (diametr == 0.0)
        return Path();

    const double radius = diametr * 0.5;
    const int intSteps = App::settings().clpCircleSegments(radius * dScale);
    Path poligon(intSteps);
    for (int i = 0; i < intSteps; ++i) {
        poligon[i] = IntPoint(
            static_cast<cInt>(cos(i * 2 * pi / intSteps) * radius) + center.X,
            static_cast<cInt>(sin(i * 2 * pi / intSteps) * radius) + center.Y);
    }
    return poligon;
}

Path RectanglePath(double width, double height, const IntPoint& center) {

    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Path poligon {
        IntPoint(static_cast<cInt>(-halfWidth + center.X), static_cast<cInt>(+halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(-halfWidth + center.X), static_cast<cInt>(-halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(+halfWidth + center.X), static_cast<cInt>(-halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(+halfWidth + center.X), static_cast<cInt>(+halfHeight + center.Y)),
    };
    if (Area(poligon) < 0.0)
        ReversePath(poligon);

    return poligon;
}

void RotatePath(Path& poligon, double angle, const IntPoint& center) {
    const bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {
        const double dAangle = qDegreesToRadians(angle - center.angleTo(pt));
        const double length = center.distTo(pt);
        pt = IntPoint(static_cast<cInt>(cos(dAangle) * length), static_cast<cInt>(sin(dAangle) * length));
        pt.X += center.X;
        pt.Y += center.Y;
    }
    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

void TranslatePath(Path& path, const IntPoint& pos) {
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (auto& pt : path) {
        pt.X += pos.X;
        pt.Y += pos.Y;
    }
}

double Perimeter(const Path& path) {
    double p = 0.0;
    for (size_t i = 0, j = path.size() - 1; i < path.size(); ++i) {
        double x = path[j].X - path[i].X;
        double y = path[j].Y - path[i].Y;
        p += x * x + y * y;
        j = i;
    }
    return sqrt(p);
}

void mergeSegments(Paths& paths, double glue) {
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
                IntPoint pib = paths[i].back();
                IntPoint pjf = paths[j].front();
                if (pib == pjf) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                IntPoint pif = paths[i].front();
                IntPoint pjb = paths[j].back();
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
                IntPoint pib = paths[i].back();
                IntPoint pjf = paths[j].front();
                if (pib.distTo(pjf) < glue) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                IntPoint pif = paths[i].front();
                IntPoint pjb = paths[j].back();
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

void mergePaths(Paths& paths, const double dist) {
    //    msg = tr("Merge Paths");
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

QIcon drawIcon(const Paths& paths) {
    static QMutex m;
    QMutexLocker l(&m);

    QPainterPath painterPath;

    for (const QPolygonF& polygon : paths)
        painterPath.addPolygon(polygon);

    const QRectF rect = painterPath.boundingRect();

    double scale = static_cast<double>(IconSize) / qMax(rect.width(), rect.height());

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
