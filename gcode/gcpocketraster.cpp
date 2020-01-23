#include "gcpocketraster.h"

#include "gcfile.h"

#include <QElapsedTimer>

namespace GCode {
RasterCreator::RasterCreator()
{
}

void RasterCreator::create()
{
    if (m_gcp.dParam[AccDistance].toDouble() != 0.0)
        createRaster2(m_gcp.tool.first(), m_gcp.dParam[Depth].toDouble(), m_gcp.dParam[UseAngle].toDouble(), m_gcp.dParam[Pass].toInt());
    else
        createRaster(m_gcp.tool.first(), m_gcp.dParam[Depth].toDouble(), m_gcp.dParam[UseAngle].toDouble(), m_gcp.dParam[Pass].toInt());
}

void RasterCreator::createRaster(const Tool& tool, const double depth, const double angle, const int prPass)
{
    enum {
        NoProfilePass,
        First,
        Last,
    };
    QElapsedTimer t;
    t.start();
    if (m_gcp.side == On) {
        emit fileReady(nullptr);
        return;
    }

    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;

    Paths profilePaths;
    Paths fillPaths;

    switch (m_gcp.side) {
    case Outer:
        groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5));
        if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
            m_groupedPss.removeFirst();
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    default:;
    }
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

            if (0) {
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
            if (0) {
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
        if (m_gcp.convent)
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
    enum {
        NoProfilePass,
        First,
        Last,
    };
    QElapsedTimer t;
    t.start();
    if (m_gcp.side == On) {
        emit fileReady(nullptr);
        return;
    }

    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;

    Paths profilePaths;
    Paths fillPaths;

    switch (m_gcp.side) {
    case Outer:
        groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5));
        if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
            m_groupedPss.removeFirst();
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    default:;
    }
    IntRect r;

    {
        Clipper c;
        for (auto& p : m_groupedPss)
            c.AddPaths(p, ptClip, true);
        r = c.GetBounds();
    }
    int c {};
    Path rrr;
    progress(10, ++c);
    r.left -= m_gcp.dParam[AccDistance].toDouble() * uScale * 2;
    r.right += m_gcp.dParam[AccDistance].toDouble() * uScale * 2;
    for (cInt i = r.top; i <= r.bottom; /*++i*/) {
        rrr.append({ { r.left, i }, { r.right, i } });
        i += tool.stepover() * uScale;
        rrr.append({ { r.right, i }, { r.left, i } });
        i += tool.stepover() * uScale;
    }
    progress(10, ++c);
    //    {
    //        ClipperOffset o;
    //        for (auto& p : m_groupedPss)
    //            o.AddPaths(p, jtRound, etClosedPolygon);
    //        Paths ps;
    //        o.Execute(ps, m_gcp.dParam[AccDistance].toDouble() * uScale);
    //        progress(10, ++c);
    //        Clipper c;
    //        c.AddPath(rrr, ptSubject, false);
    //        c.AddPaths(ps, ptClip, true);
    //        c.Execute(ctIntersection, ps, pftNonZero);
    //        sortBE(ps);
    //        mergeSegments(ps, tool.stepover() * uScale * 1.01);
    //        m_returnPss.append(ps);
    //    }
    progress(10, ++c);
    //    sortB(m_returnPs);
    //    if (!profilePaths.isEmpty() && prPass) {
    //        sortB(profilePaths);
    //        if (m_gcp.convent)
    //            ReversePaths(profilePaths);
    //        for (Path& path : profilePaths)
    //            path.append(path.first());
    //    }

    //    switch (prPass) {
    //    case NoProfilePass:
    //        m_returnPss.prepend(m_returnPs);
    //        break;
    //    case First:
    //        if (!profilePaths.isEmpty())
    //            m_returnPss.prepend(profilePaths);
    //        m_returnPss.prepend(m_returnPs);
    //        break;
    //    case Last:
    //        m_returnPss.prepend(m_returnPs);
    //        if (!profilePaths.isEmpty())
    //            m_returnPss.append(profilePaths);
    //        break;
    //    default:
    //        break;
    //    }

    m_returnPss.append({ rrr });

    qDebug() << "createRaster" << (t.elapsed() / 1000);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        m_file = new File(m_returnPss, tool, depth, Raster /*Laser*/, fillPaths);
        m_file->setFileName(tool.name());
        emit fileReady(m_file);
    }
}
}
