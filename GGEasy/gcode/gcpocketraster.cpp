// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

    auto calcScanLines = [](const Paths& src, const Path& frame) {
        Paths scanLines;

        Clipper clipper;
        clipper.AddPaths(src, ptClip);
        clipper.AddPath(frame, ptSubject, false);
        clipper.Execute(ctIntersection, scanLines);
        if (!scanLines.size())
            return scanLines;
        std::sort(scanLines.begin(), scanLines.end(), [](const Path& l, const Path& r) { return l.front().Y < r.front().Y; }); // vertical sort
        cInt start = scanLines.front().front().Y;
        bool fl = {};
        for (size_t i {}, last {}; i < scanLines.size(); ++i) {
            if (auto y = scanLines[i].front().Y; y != start || i - 1 == scanLines.size()) {
                std::sort(scanLines.begin() + last, scanLines.begin() + i, [&fl](const Path& l, const Path& r) { // horizontal sort
                    return fl ? l.front().X < r.front().X
                              : l.front().X > r.front().X;
                });
                for (size_t k = last; k < i; ++k) { // fix direction
                    if (fl ^ (scanLines[k].front().X < scanLines[k].back().X))
                        std::swap(scanLines[k].front().X, scanLines[k].back().X);
                }
                start = y;
                fl = !fl;
                last = i;
            }
        }
        return scanLines;
    };
    auto calcFrames = [](const Paths& src, const Path& frame) {
        Paths frames;
        {
            Paths tmp;
            Clipper clipper;
            clipper.AddPaths(src, ptSubject, false);
            clipper.AddPath(frame, ptClip);
            clipper.Execute(ctIntersection, tmp);
            //dbgPaths(tmp, "ctIntersection");
            frames.append(tmp);
            clipper.Execute(ctDifference, tmp);
            //dbgPaths(tmp, "ctDifference");
            frames.append(tmp);
            std::sort(frames.begin(), frames.end(), [](const Path& l, const Path& r) { return l.front().Y < r.front().Y; }); // vertical sort
            for (auto& path : frames) {
                if (path.front().Y > path.back().Y)
                    ReversePath(path); // fix vertical direction
            }
        }
        return frames;
    };
    auto calcZigzag = [this](const Paths& src) {
        Clipper clipper;
        clipper.AddPaths(src, ptClip, true);
        IntRect rect(clipper.GetBounds());
        cInt o = uScale - (rect.height() % m_stepOver) / 2;
        rect.top -= o;
        rect.bottom += o;
        rect.left -= uScale;
        rect.right += uScale;
        Path zigzag;
        cInt start = rect.top;
        bool fl {};

        for (; start <= rect.bottom || fl; fl = !fl, start += m_stepOver) {
            if (!fl) {
                zigzag.emplace_back(rect.left, start);
                zigzag.emplace_back(rect.right, start);
            } else {
                zigzag.emplace_back(rect.right, start);
                zigzag.emplace_back(rect.left, start);
            }
        }

        zigzag.front().X -= m_stepOver;
        zigzag.back().X -= m_stepOver;
        return zigzag;
    };
    auto merge = [](const Paths& scanLines, const Paths& frames) {
        Paths merged;
        merged.reserve(scanLines.size() / 10);
        std::list<Path> bList;
        for (auto&& path : scanLines)
            bList.emplace_back(std::move(path));

        std::list<Path> fList;
        for (auto&& path : frames)
            fList.emplace_back(std::move(path));

        setMax(bList.size());
        while (bList.begin() != bList.end()) {
            setCurrent(bList.size());

            merged.resize(merged.size() + 1);
            auto& path = merged.back();
            for (auto bit = bList.begin(); bit != bList.end(); ++bit) {
                getCancelThrow();
                if (path.empty() || path.back() == bit->front()) {
                    path.append(path.empty() ? *bit : bit->mid(1));
                    bList.erase(bit);
                    for (auto fit = fList.begin(); fit != fList.end(); ++fit) {
                        if (path.back() == fit->front() && fit->front().Y < fit->at(1).Y) {
                            path.append(fit->mid(1));
                            fList.erase(fit);
                            bit = bList.begin();
                            break;
                        }
                    }
                    bit = bList.begin();
                }
                if (bList.begin() == bList.end())
                    break;
            }
            for (auto fit = fList.begin(); fit != fList.end(); ++fit) {
                if (path.front() == fit->back() && fit->front().Y > fit->at(1).Y) {
                    fit->append(path.mid(1));
                    std::swap(*fit, path);
                    fList.erase(fit);
                    break;
                }
            }
        }
        merged.shrink_to_fit();
        return merged;
    };

    for (Paths src : m_groupedPss) {
        ClipperOffset offset(uScale);
        offset.AddPaths(src, jtRound, etClosedPolygon);
        offset.Execute(src, -m_dOffset);
        for (auto& path : src)
            path.push_back(path.front());
        if (prPass)
            profilePaths.append(src);

        if (src.size()) {
            for (auto& path : src)
                RotatePath(path, angle);
            auto zigzag { calcZigzag(src) };
            auto scanLines { calcScanLines(src, zigzag) };
            auto frames { calcFrames(src, zigzag) };
            if (scanLines.size() && frames.size()) {
                auto merged { merge(scanLines, frames) };
                for (auto& path : merged)
                    RotatePath(path, -angle);
                m_returnPs.append(merged);
            }
        }
    }

    mergeSegments(m_returnPs);

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

    const IntPoint center { rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2 };

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
    std::sort(src.begin(), src.end(), [](const Path& p1, const Path& p2) -> bool { return p1.front().Y > p2.front().Y; });
#endif
    bool reverse {};

    auto format = [&reverse](Path& src) -> Path& {
        if (reverse)
            std::sort(src.begin(), src.end(), [](const IntPoint& p1, const IntPoint& p2) -> bool { return p1.X > p2.X; });
        else
            std::sort(src.begin(), src.end(), [](const IntPoint& p1, const IntPoint& p2) -> bool { return p1.X < p2.X; });
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
