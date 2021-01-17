// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcpocketraster.h"

#include "gcfile.h"

#include <QElapsedTimer>
#ifndef __GNUC__
#include <execution>
#endif
#include "point.h"
#include <QFuture>
#include <QtConcurrent>

#include "leakdetector.h"

namespace GCode {
RasterCreator::RasterCreator()
{
}

void RasterCreator::create()
{
    if (m_gcp.params[GCodeParams::Fast].toBool())
        createRaster2(m_gcp.tools.front(), m_gcp.params[GCodeParams::Depth].toDouble(), m_gcp.params[GCodeParams::UseAngle].toDouble(), m_gcp.params[GCodeParams::Pass].toInt());
    else
        createRaster(m_gcp.tools.front(), m_gcp.params[GCodeParams::Depth].toDouble(), m_gcp.params[GCodeParams::UseAngle].toDouble(), m_gcp.params[GCodeParams::Pass].toInt());
}

void RasterCreator::createRaster(const Tool& tool, const double depth, const double angle, const int prPass)
{
    QElapsedTimer t;
    t.start();

    switch (m_gcp.side()) {
    case Outer:
        groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5));
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    case On:
        emit fileReady(nullptr);
        return;
    }

    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;

    Paths profilePaths;
    Paths fillPaths;

    for (Paths src : m_groupedPss) {
        if (prPass) {
            ClipperOffset offset(uScale);
            offset.AddPaths(src, jtRound, etClosedPolygon);
            offset.Execute(src, -m_dOffset);
            profilePaths.append(src);
            fillPaths.append(src);
        }

        if (src.size()) {
            for (Path& path : src)
                path.push_back(path.front());
            ClipperOffset offset(uScale);
            offset.AddPaths(src, jtRound, etClosedPolygon);
            offset.Execute(src, prPass ? -m_dOffset * 1.05 : -m_dOffset);
        } else
            continue;

        if (src.size()) {
            for (Path& path : src) {
                path.push_back(path.front());
            }

            Clipper clipper;
            clipper.AddPaths(src, ptClip, true);
            const IntRect r(clipper.GetBounds());
            clipper.Clear();
            const cInt size = static_cast<cInt>(IntPoint(r.left, r.top).distTo({ r.right, r.bottom }));
            const cInt end = static_cast<cInt>(r.bottom + (size - (r.bottom - r.top)) * 0.5);
            const cInt start = static_cast<cInt>(r.top - (size - (r.bottom - r.top)) * 0.5);
            const cInt left = static_cast<cInt>(r.left - (size - (r.right - r.left)) * 0.5);
            const cInt right = static_cast<cInt>(r.right + (size - (r.right - r.left)) * 0.5);
            const Point64 center(static_cast<cInt>(0.5 * (r.left + r.right)), static_cast<cInt>(0.5 * (r.top + r.bottom)));

            Paths acc;
            using Worck = mvector<std::tuple<cInt, cInt, cInt, cInt>>;
            /////////////////////////////////////////////////////////
            std::function<void(Worck)> scan = [this, &angle, &center, &src, &acc](Worck w) {
                static QMutex m;
                auto [_1, _2, _3, flag] = w.front();
                Q_UNUSED(_1)
                Q_UNUSED(_2)
                Q_UNUSED(_3)
                Paths scanLine;
                {
                    Clipper clipper;
                    clipper.AddPaths(src, ptClip, true);
                    for (auto& [left_, right_, var, _f] : w) {
                        Q_UNUSED(_f)
                        Path frame { { left_, var }, { right_, var } };
                        RotatePath(frame, angle, center);
                        clipper.AddPath(frame, ptSubject, false);
                    }
                    clipper.Execute(ctIntersection, scanLine, pftPositive);
                    if (qFuzzyCompare(angle, 90)) {
                        if (!flag) {
                            for (Path& path : scanLine)
                                if (path.front().Y > path.back().Y)
                                    ReversePath(path);
                        } else {
                            for (Path& path : scanLine)
                                if (path.front().Y < path.back().Y)
                                    ReversePath(path);
                        }
                    } else {
                        if (!flag) {
                            for (Path& path : scanLine)
                                if (path.front().X > path.back().X)
                                    ReversePath(path);
                        } else {
                            for (Path& path : scanLine)
                                if (path.front().X < path.back().X)
                                    ReversePath(path);
                        }
                    }
                }
                {

                    Paths toNext;
                    Clipper clipper;
                    clipper.AddPaths(src, ptSubject, false);
                    for (auto [left_, right_, var, _f] : w) {
                        Q_UNUSED(_f)
                        Path frame {
                            { left_, var },
                            { right_, var },
                            { right_, var += m_stepOver },
                            { left_, var }
                        };
                        RotatePath(frame, angle, center);
                        clipper.AddPath(frame, ptClip, true);
                    }
                    clipper.Execute(ctIntersection, toNext, pftPositive);
                    mergeSegments(toNext);
                    if (scanLine.empty()) {
                        QMutexLocker l(&m);
                        acc.append(toNext);
                    } else {
                        for (Path& dst : scanLine) {
                            for (size_t i = 0; i < toNext.size(); ++i) {
                                Path& next = toNext[i];
                                if (dst.back() == next.front()) {
                                    dst.append(next.mid(1));
                                    toNext.remove(i--);
                                } else if (dst.back() == next.back()) {
                                    ReversePath(next);
                                    dst.append(next.mid(1));
                                    toNext.remove(i--);
                                } else if (dst.front() == next.front()) {
                                    toNext.remove(i--);
                                } else if (dst.front() == next.back()) {
                                    toNext.remove(i--);
                                }
                            }
                        }
                        QMutexLocker l(&m);
                        acc.append(scanLine);
                        acc.append(toNext);
                    }
                }
            };
            /////////////////////////////////////////////////////////

            mvector<Worck> map;

            if (/* DISABLES CODE */ (0)) {
                for (cInt var = start, flag = 0; var < end + m_stepOver * 5; flag = (flag ? 0 : 1), var += flag ? m_stepOver : m_stepOver * 5) {
                    map.push_back({ { left, right, var, flag },
                        { left, right, static_cast<cInt>(var + m_stepOver * 2), flag },
                        { left, right, static_cast<cInt>(var + m_stepOver * 4), flag } });
                }
            } else {
                for (cInt var = start, flag = 0; var < end; flag = (flag ? 0 : 1), var += m_stepOver) {
                    map.push_back(Worck { { left, right, var, flag } });
                }
            }
            if (/* DISABLES CODE */ (0)) {
                //PROG  m_progressMax += m_groupedPss.size() + map.size();
                for (size_t i = 0; i < map.size(); ++i) {
                    scan(map[i]);
                    //++PROG m_progressVal;
                }
            } else {
                //PROG  m_progressMax += m_groupedPss.size() + map.size();
                for (size_t i = 0, c = QThread::idealThreadCount(); i < map.size(); i += c) {
                    auto m(map.mid(i, c));
                    QFuture<void> future = QtConcurrent::map(m, scan);
                    future.waitForFinished();
                    //PROG m_progressVal += m.size();
                }
            }

            if (!acc.empty()) {
                mergeSegments(acc);
                m_returnPs.append(acc);
                //sortB(m_returnPss.last());
            }
        }
    }

    sortB(m_returnPs);
    if (!profilePaths.empty() && prPass) {
        sortB(profilePaths);
        if (m_gcp.convent())
            ReversePaths(profilePaths);
        for (Path& path : profilePaths)
            path.push_back(path.front());
    }

    switch (prPass) {
    case NoProfilePass:
        m_returnPss.prepend(m_returnPs);
        break;
    case First:
        if (!profilePaths.empty())
            m_returnPss.prepend(profilePaths);
        m_returnPss.prepend(m_returnPs);
        break;
    case Last:
        m_returnPss.prepend(m_returnPs);
        if (!profilePaths.empty())
            m_returnPss.push_back(profilePaths);
        break;
    default:
        break;
    }

    if (m_returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = Raster;
        m_file = new File(m_returnPss, m_gcp, fillPaths);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void RasterCreator::createRaster2(const Tool& tool, const double depth, const double angle, const int prPass)
{

    QElapsedTimer t;
    t.start();

    switch (m_gcp.side()) {
    case Outer:
        groupedPaths(CutoffPaths, uScale);
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    case On:
        emit fileReady(nullptr);
        return;
    }

    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = static_cast<cInt>(tool.stepover() * uScale);

    Paths profilePaths;

    { // create exposure frames
        ClipperOffset o;
        for (auto& p : m_groupedPss)
            o.AddPaths(p, jtRound, etClosedPolygon);
        o.Execute(profilePaths, -tool.diameter() * uScale * 0.5);
    }

    { // get bounds of frames
        ClipperBase c;
        c.AddPaths(profilePaths, ptClip, true);
        rect = c.GetBounds();
    }

    const Point64 center { rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2 };

    Paths laserPath(profilePaths);

    if (!qFuzzyIsNull(angle)) { // Rotate Paths
        for (Path& path : laserPath)
            RotatePath(path, angle, center);
        // get bounds of frames if angle > 0.0
        ClipperBase c;
        c.AddPaths(laserPath, ptClip, true);
        rect = c.GetBounds();
    }

    rect.left -= uScale;
    rect.right += uScale;

    Path zPath;
    { // create "snake"
        cInt y = rect.top;
        while (y < rect.bottom) {
            zPath.append({ { rect.left, y }, { rect.right, y } });
            y += m_stepOver;
            zPath.append({ { rect.right, y }, { rect.left, y } });
            y += m_stepOver;
        }
    }

    //PROG //PROG .3setProgMaxAndVal(0, 0);

    { //  calculate
        Clipper c;
        c.AddPath(zPath, ptSubject, false);
        c.AddPaths(laserPath, ptClip, true);
        c.Execute(ctIntersection, laserPath, pftNonZero); // laser on
        addAcc(laserPath, m_gcp.params[GCodeParams::AccDistance].toDouble() * uScale); // add laser off paths
    }

    if (!qFuzzyIsNull(angle)) { // Rotate Paths
        for (Path& path : laserPath)
            RotatePath(path, -angle, center);
    }

    m_returnPss.push_back(laserPath);

    if (!profilePaths.empty() && prPass != NoProfilePass) {
        for (auto& p : profilePaths)
            p.push_back(p.front());
        m_returnPss.push_back(sortB(profilePaths));
    }

    if (m_returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = LaserHLDI;
        m_file = new File(m_returnPss, m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void RasterCreator::addAcc(Paths& src, const cInt accDistance)
{

    Paths pPath;
    pPath.reserve(src.size() * 2 + 1);
#ifndef __GNUC__
    std::sort(std::execution::par, src.begin(), src.end(), [](const Path& p1, const Path& p2) -> bool { return p1.front().Y > p2.front().Y; });
#else
    std::sort(src.begin(), src.end(), [](const Path& p1, const Path& p2) -> bool { return p1.first().Y > p2.first().Y; });
#endif
    bool reverse {};

    auto format = [&reverse](Path& src) -> Path& {
        if (reverse)
            std::sort(src.begin(), src.end(), [](const Point64& p1, const Point64& p2) -> bool { return p1.X > p2.X; });
        else
            std::sort(src.begin(), src.end(), [](const Point64& p1, const Point64& p2) -> bool { return p1.X < p2.X; });
        return src;
    };

    auto adder = [&reverse, &pPath, accDistance](Paths& paths) {
        std::sort(paths.begin(), paths.end(), [reverse](const Path& p1, const Path& p2) -> bool {
            if (reverse)
                return p1.front().X > p2.front().X;
            else
                return p1.front().X < p2.front().X;
        });
        if (pPath.size()) { // acc
            Path acc;
            {
                const Path& path = pPath.back();
                if (path.front().X < path.back().X) { // acc
                    acc.append(Path { path.back(), { path.back().X + accDistance, path.front().Y } });
                } else {
                    acc.append(Path { path.back(), { path.back().X - accDistance, path.front().Y } });
                }
            }
            {
                const Path& path = paths.front();
                if (path.front().X > path.back().X) { // acc
                    acc.append(Path { { path.front().X + accDistance, path.front().Y }, path.front() });
                } else {
                    acc.append(Path { { path.front().X - accDistance, path.front().Y }, path.front() });
                }
            }
            pPath.push_back(acc);
        } else { // acc first
            pPath.push_back(Path { { paths.front().front().X - accDistance, paths.front().front().Y }, paths.front().front() });
        }
        for (size_t j = 0; j < paths.size(); ++j) {
            if (j) // acc
                pPath.push_back(Path { paths[j - 1].back(), paths[j].front() });
            pPath.push_back(paths[j]);
        }
    };

    { //  calculate
        cInt yLast = src.front().front().Y;
        Paths paths;

        for (size_t i = 0; i < src.size(); ++i) {
            //PROG //PROG .3setProgMaxAndVal(src.size(), i);
            if (yLast != src[i].front().Y) {
                adder(paths);
                reverse = !reverse;
                yLast = src[i].front().Y;
                paths = { format(src[i]) };
            } else {
                paths.push_back(format(src[i]));
            }
        }

        adder(paths);
    }

    { // acc last
        Path& path = pPath.back();
        if (path.front().X < path.back().X) {
            pPath.push_back(Path { path.back(), { path.back().X + accDistance, path.front().Y } });
        } else {
            pPath.push_back(Path { path.back(), { path.back().X - accDistance, path.front().Y } });
        }
    }

    src = std::move(pPath);
}
}
