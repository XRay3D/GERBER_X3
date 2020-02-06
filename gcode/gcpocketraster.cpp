#include "gcpocketraster.h"

#include "gcfile.h"

#include <QElapsedTimer>
#include <point.h>

namespace GCode {
RasterCreator::RasterCreator()
{
}

void RasterCreator::create()
{
    if (m_gcp.dParam[Fast].toBool())
        createRaster2(m_gcp.tool.first(), m_gcp.dParam[Depth].toDouble(), m_gcp.dParam[UseAngle].toDouble(), m_gcp.dParam[Pass].toInt());
    else
        createRaster(m_gcp.tool.first(), m_gcp.dParam[Depth].toDouble(), m_gcp.dParam[UseAngle].toDouble(), m_gcp.dParam[Pass].toInt());
}

void RasterCreator::createRaster(const Tool& tool, const double depth, const double angle, const int prPass)
{
    QElapsedTimer t;
    t.start();

    switch (m_gcp.side()) {
    case Outer:
        groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5));
        if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
            m_groupedPss.removeFirst();
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
                path.append(path.first());
            ClipperOffset offset(uScale);
            offset.AddPaths(src, jtRound, etClosedPolygon);
            offset.Execute(src, prPass ? -m_dOffset * 1.05 : -m_dOffset);
        } else
            continue;

        if (src.size()) {
            for (Path& path : src) {
                path.append(path.first());
            }

            Clipper clipper;
            clipper.AddPaths(src, ptClip, true);
            const IntRect r(clipper.GetBounds());
            clipper.Clear();
            const cInt size = static_cast<cInt>(Length({ r.left, r.top }, { r.right, r.bottom }));
            const cInt end = static_cast<cInt>(r.bottom + (size - (r.bottom - r.top)) * 0.5);
            const cInt start = static_cast<cInt>(r.top - (size - (r.bottom - r.top)) * 0.5);
            const cInt left = static_cast<cInt>(r.left - (size - (r.right - r.left)) * 0.5);
            const cInt right = static_cast<cInt>(r.right + (size - (r.right - r.left)) * 0.5);
            const IntPoint center(static_cast<cInt>(0.5 * (r.left + r.right)), static_cast<cInt>(0.5 * (r.top + r.bottom)));

            Paths acc;
            using Worck = QVector<std::tuple<cInt, cInt, cInt, cInt>>;
            /////////////////////////////////////////////////////////
            std::function<void(Worck)> scan = [this, &angle, &center, &src, &acc](Worck w) {
                static QMutex m;
                auto [_1, _2, _3, flag] = w.first();
                Q_UNUSED(_1)
                Q_UNUSED(_2)
                Q_UNUSED(_3)
                Paths scanLine;
                {
                    Clipper clipper;
                    clipper.AddPaths(src, ptClip, true);
                    for (auto& [left_, right_, var, flag] : w) {
                        Q_UNUSED(flag)
                        Path frame { { left_, var }, { right_, var } };
                        RotatePath(frame, angle, center);
                        clipper.AddPath(frame, ptSubject, false);
                    }
                    clipper.Execute(ctIntersection, scanLine, pftPositive);
                    if (qFuzzyCompare(angle, 90)) {
                        if (!flag) {
                            for (Path& path : scanLine)
                                if (path.first().Y > path.last().Y)
                                    ReversePath(path);
                        } else {
                            for (Path& path : scanLine)
                                if (path.first().Y < path.last().Y)
                                    ReversePath(path);
                        }
                    } else {
                        if (!flag) {
                            for (Path& path : scanLine)
                                if (path.first().X > path.last().X)
                                    ReversePath(path);
                        } else {
                            for (Path& path : scanLine)
                                if (path.first().X < path.last().X)
                                    ReversePath(path);
                        }
                    }
                }
                {

                    Paths toNext;
                    Clipper clipper;
                    clipper.AddPaths(src, ptSubject, false);
                    for (auto [left_, right_, var, flag] : w) {
                        Q_UNUSED(flag)
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
                    if (scanLine.isEmpty()) {
                        QMutexLocker l(&m);
                        acc.append(toNext);
                    } else {
                        for (Path& dst : scanLine) {
                            for (int i = 0; i < toNext.size(); ++i) {
                                Path& src = toNext[i];
                                if (dst.last() == src.first()) {
                                    dst.append(src.mid(1));
                                    toNext.remove(i--);
                                } else if (dst.last() == src.last()) {
                                    ReversePath(src);
                                    dst.append(src.mid(1));
                                    toNext.remove(i--);
                                } else if (dst.first() == src.first()) {
                                    toNext.remove(i--);
                                } else if (dst.first() == src.last()) {
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

            QVector<Worck> map;

            if (/* DISABLES CODE */ (0)) {
                for (cInt var = start, flag = 0; var < end + m_stepOver * 5; flag = (flag ? 0 : 1), var += flag ? m_stepOver : m_stepOver * 5) {
                    map.append({ { left, right, var, flag },
                        { left, right, static_cast<cInt>(var + m_stepOver * 2), flag },
                        { left, right, static_cast<cInt>(var + m_stepOver * 4), flag } });
                }
            } else {
                for (cInt var = start, flag = 0; var < end; flag = (flag ? 0 : 1), var += m_stepOver) {
                    map.append({ { left, right, var, flag } });
                }
            }
            if (/* DISABLES CODE */ (0)) {
                m_progressMax += m_groupedPss.size() + map.size();
                for (int i = 0; i < map.size(); ++i) {
                    scan(map[i]);
                    ++m_progressVal;
                }
            } else {
                m_progressMax += m_groupedPss.size() + map.size();
                for (int i = 0, c = QThread::idealThreadCount(); i < map.size(); i += c) {
                    auto m(map.mid(i, c));
                    QFuture<void> future = QtConcurrent::map(m, scan);
                    future.waitForFinished();
                    m_progressVal += m.size();
                }
            }

            if (!acc.isEmpty()) {
                mergeSegments(acc);
                m_returnPs.append(acc);
                //sortB(m_returnPss.last());
            }
        }
    }

    sortB(m_returnPs);
    if (!profilePaths.isEmpty() && prPass) {
        sortB(profilePaths);
        if (m_gcp.convent())
            ReversePaths(profilePaths);
        for (Path& path : profilePaths)
            path.append(path.first());
    }

    switch (prPass) {
    case NoProfilePass:
        m_returnPss.prepend(m_returnPs);
        break;
    case First:
        if (!profilePaths.isEmpty())
            m_returnPss.prepend(profilePaths);
        m_returnPss.prepend(m_returnPs);
        break;
    case Last:
        m_returnPss.prepend(m_returnPs);
        if (!profilePaths.isEmpty())
            m_returnPss.append(profilePaths);
        break;
    default:
        break;
    }
    qDebug() << "createRaster" << (t.elapsed() / 1000);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        m_file = new File(m_returnPss, tool, depth, Raster, fillPaths);
        m_file->setFileName(tool.name());
        emit fileReady(m_file);
    }
}

void RasterCreator::createRaster2(const Tool& tool, const double depth, const double angle, const int prPass)
{
    //self = this;

    QElapsedTimer t;
    t.start();

    switch (m_gcp.side()) {
    case Outer:
        groupedPaths(CutoffPaths, uScale);
        if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
            m_groupedPss.removeFirst();
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

    Paths tempPath(profilePaths);

    if (!qFuzzyIsNull(angle)) { // Rotate Paths
        for (Path& path : tempPath)
            RotatePath(path, angle, center);
        // get bounds of frames if angle > 0.0
        ClipperBase c;
        c.AddPaths(tempPath, ptClip, true);
        rect = c.GetBounds();
    }

    rect.left -= static_cast<cInt>(m_gcp.dParam[AccDistance].toDouble() * uScale * 2);
    rect.right += static_cast<cInt>(m_gcp.dParam[AccDistance].toDouble() * uScale * 2);

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

    progress(0, 0);

    Paths laserPath;
    { //  calculate
        Clipper c;
        c.AddPath(zPath, ptSubject, false);
        c.AddPaths(tempPath, ptClip, true);

        // laser off
        c.Execute(ctDifference, tempPath, pftNonZero);
        msg = "laser resize";

        const cInt accDistance = static_cast<cInt>(m_gcp.dParam[AccDistance].toDouble() * uScale);
        auto setLen = [accDistance](const IntPoint& p1, IntPoint& p2) {
            if (p1.X > p2.X)
                p2.X = p1.X - accDistance;
            else
                p2.X = p1.X + accDistance;
        };
        for (int i = 1; i < tempPath.size() - 1; ++i) {
            progress(tempPath.size(), i);
            auto& p = tempPath[i];
            const auto s = p.size();
            if (s > 2) {
                setLen(p[0], p[1]);
                setLen(p[s - 1], p[s - 2]);
                if (s > 4)
                    p.remove(2, s - 4);
            }
        }

        tempPath.first().resize(2);
        setLen(tempPath.first()[0], tempPath.first()[1]);

        tempPath.last().remove(0, tempPath.last().size() - 2);
        setLen(tempPath.last()[1], tempPath.last()[0]);

        laserPath.append(tempPath);

        // laser on
        c.Execute(ctIntersection, tempPath, pftNonZero);
        laserPath.append(tempPath);

        sortSegments(laserPath);

        // test sorting
        for (int i = 0; i < laserPath.size() - 1; ++i) {
            if (double l = Length(laserPath[i].last(), laserPath[i + 1].first()); l > 1) {
                qDebug() << "sortBE2 err" << i << l;
            }
        }
    }

    if (!qFuzzyIsNull(angle)) { // Rotate Paths
        for (Path& path : laserPath)
            RotatePath(path, -angle, center);
    }

    m_returnPss.append(laserPath);

    if (!profilePaths.isEmpty() && prPass != NoProfilePass) {
        for (auto& p : profilePaths)
            p.append(p.first());
        m_returnPss.append(sortB(profilePaths));
    }

    qDebug() << "createRaster" << (t.elapsed() / 1000);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        m_file = new File(m_returnPss, tool, depth, Laser, {});
        m_file->setFileName(tool.name());
        emit fileReady(m_file);
    }
}

void RasterCreator::sortSegments(Paths& src)
{
    msg = "laser sort";

    //    auto same = [l = uScale / 100](const IntPoint& p1, const IntPoint& p2) -> bool {
    //        return Length(p1, p2) < l; //abs(p1.X - p2.X) < sor && abs(p1.Y - p2.Y) < sor;
    //    };

    std::function<bool(const IntPoint&, const IntPoint&)> same = [](const IntPoint& p1, const IntPoint& p2) -> bool { return p1 == p2; };

    IntPoint startPt;

    if (src[0].size() > src[1].size()) {
        if (src[0].first().X > src[1].first().X) {
            startPt = { std::min(src[1].first().X, src[1].last().X), src[1].first().Y };
        } else {
            startPt = { std::max(src[1].first().X, src[1].last().X), src[1].first().Y };
        }
    } else {
        if (src[1].first().X > src[0].first().X) {
            startPt = { std::min(src[0].first().X, src[0].last().X), src[0].first().Y };
        } else {
            startPt = { std::max(src[0].first().X, src[0].last().X), src[0].first().Y };
        }
    }

    ReversePaths(src);

    std::sort(src.begin(), src.end(), [](const Path& p1, const Path& p2) -> bool {
        return std::min(p1.first().Y, p1.last().Y) > std::min(p2.first().Y, p2.last().Y);
    });

    {
        using Worck = std::tuple<int, int, IntPoint>;
        /////////////////////////////////////////////////////////
        std::function<void(Worck)> scan = [src = src.data(), same](Worck w) {
            auto [from, to, startPt] = w;
            qDebug() << "scan" << from << to;
            for (int firstIdx = from /*0*/; firstIdx < to /*src.size()*/; ++firstIdx) {
                progress(to /*src.size()*/, firstIdx);
                int swapIdx = firstIdx;
                bool reverse = false;
                for (int secondIdx = firstIdx; secondIdx < to /*src.size()*/; ++secondIdx) {
                    if (same(startPt, src[secondIdx].first())) {
                        swapIdx = secondIdx;
                        reverse = false;
                        break;
                    }
                    if (same(startPt, src[secondIdx].last())) {
                        swapIdx = secondIdx;
                        reverse = true;
                        break;
                    }
                }
                if (reverse)
                    ReversePath(src[swapIdx]);
                startPt = src[swapIdx].last();
                if (swapIdx != firstIdx)
                    std::swap(src[firstIdx], src[swapIdx]);
            }
        };
        constexpr int k = 20000;
        if (src.size() > k * QThread::idealThreadCount()) {
            QVector<Worck> map;

            for (int i = 0; i < (src.size() - k); i += k) {
                map.append({ i, i + k - 1, src[i].first() });
            }
            m_progressMax += map.size();
            for (int i = 0, c = QThread::idealThreadCount(); i < map.size(); i += c) {
                auto m(map.mid(i, c));
                QFuture<void> future = QtConcurrent::map(m, scan);
                future.waitForFinished();
                m_progressVal += m.size();
            }
        }
        scan({ 0, src.size(), startPt });
    }

    //    for (int firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
    //        progress(src.size(), firstIdx);
    //        int swapIdx = firstIdx;
    //        bool reverse = false;
    //        for (int secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
    //            if (same(startPt, src[secondIdx].first())) {
    //                swapIdx = secondIdx;
    //                reverse = false;
    //                break;
    //            }
    //            if (same(startPt, src[secondIdx].last())) {
    //                swapIdx = secondIdx;
    //                reverse = true;
    //                break;
    //            }
    //        }
    //        if (reverse)
    //            ReversePath(src[swapIdx]);
    //        startPt = src[swapIdx].last();
    //        if (swapIdx != firstIdx)
    //            std::swap(src[firstIdx], src[swapIdx]);
    //    }
}

}
