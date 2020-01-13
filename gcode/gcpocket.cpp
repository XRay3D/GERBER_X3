#include "gcpocket.h"
#include "gcfile.h"

#include <QElapsedTimer>
namespace GCode {
PocketCreator::PocketCreator()
{
}

void PocketCreator::create()
{
    if (m_gcp.dParam[TwoTools].toBool()) {
        createPocket2({ m_gcp.tool.first(), m_gcp.tool.last() }, m_gcp.dParam[Depth].toDouble(), m_gcp.dParam[MinArea].toDouble());
    } else if (m_gcp.dParam[UseRaster].toBool()) {
        createRaster(m_gcp.tool.first(), m_gcp.dParam[Depth].toDouble(), m_gcp.dParam[UseAngle].toDouble(), m_gcp.dParam[Pass].toInt());
    } else {
        createPocket(m_gcp.tool.first(), m_gcp.dParam[Depth].toDouble(), m_gcp.dParam[Steps].toInt());
    }
}

void PocketCreator::createRaster(const Tool& tool, const double depth, const double angle, const int profilePass)
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
        if (profilePass) {
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
            offset.Execute(src, profilePass ? -m_dOffset * 1.05 : -m_dOffset);
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
    if (!profilePaths.isEmpty() && profilePass) {
        sortB(profilePaths);
        if (m_gcp.convent)
            ReversePaths(profilePaths);
        for (Path& path : profilePaths)
            path.append(path.first());
    }

    switch (profilePass) {
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

void PocketCreator::createPocket(const Tool& tool, const double depth, const int steps)
{
    self = this;
    if (m_gcp.side == On)
        return;
    progress(0, 0);
    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;
    Paths fillPaths;
    if (steps) {
        groupedPaths(CopperPaths);
        if (m_gcp.side == Inner) {
            m_dOffset *= -1;
            for (Paths paths : m_groupedPss) {
                ClipperOffset offset(uScale);
                offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(paths, m_dOffset);
                fillPaths.append(paths);
                Paths tmpPaths;
                int counter = steps;
                if (counter > 1) {
                    do {
                        if (counter == 1)
                            fillPaths.append(paths);
                        tmpPaths.append(paths);
                        offset.Clear();
                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
                        offset.Execute(paths, m_dOffset);
                    } while (paths.size() && --counter);
                } else {
                    tmpPaths.append(paths);
                    fillPaths.append(paths);
                }
                m_returnPs.append(tmpPaths);
            }
        } else {
            ClipperOffset offset(uScale);
            for (Paths paths : m_groupedPss) {
                offset.AddPaths(paths, jtRound, etClosedPolygon);
            }
            Paths paths;
            offset.Execute(paths, m_dOffset);
            fillPaths.append(paths);
            int counter = steps;

            if (counter > 1) {
                do {
                    if (counter == 1)
                        fillPaths.append(paths);
                    m_returnPs.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, m_stepOver);
                } while (paths.size() && --counter);
            } else {
                m_returnPs.append(paths);
                fillPaths.append(paths);
            }
        }
    } else {
        switch (m_gcp.side) {
        case On:
            break;
        case Outer:
            groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5));
            if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
                m_groupedPss.removeFirst();
            break;
        case Inner:
            groupedPaths(CopperPaths);
            break;
        }
        for (Paths paths : m_groupedPss) {
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            fillPaths.append(paths);
            Paths tmpPaths;
            do {
                tmpPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size());
            m_returnPs.append(tmpPaths);
        }
    }

    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }

    stacking(m_returnPs);
    //        ReversePaths(m_returnPaths);
    //        sortByStratDistance(m_returnPaths);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        m_file = new GCode::File(m_returnPss, tool, depth, Pocket, fillPaths);
        m_file->setFileName(tool.name());
        emit fileReady(m_file);
    }
}

void PocketCreator::createPocket2(const QPair<Tool, Tool>& tool, double depth, double minArea)
{
    self = this;

    if (m_gcp.side == On)
        return;

    do {
        m_toolDiameter = tool.second.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.second.stepover() * uScale;

        Paths fillPaths;

        switch (m_gcp.side) {
        case On:
            break;
        case Outer:
            groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5.0));
            if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
                m_groupedPss.removeFirst();
            break;
        case Inner:
            groupedPaths(CopperPaths);
            break;
        }

        for (Paths paths : m_groupedPss) {
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            fillPaths.append(paths);

            Paths tmpPaths;
            do {
                tmpPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size());
            m_returnPs.append(tmpPaths);
        }

        if (m_returnPs.isEmpty()) {
            emit fileReady(nullptr);
            break;
        }
        //            for (int i = 0; i < m_returnPaths.size(); ++i) {
        //                if (Perimeter(m_returnPaths[i]) < m_dOffset)
        //                    m_returnPaths.remove(i--);
        //            }
        //            for (int i = 0; i < fillPaths.size(); ++i) {
        //                if (Perimeter(fillPaths[i]) < m_dOffset)
        //                    fillPaths.remove(i--);
        //            }
        stacking(m_returnPs);
        if (m_returnPss.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new GCode::File(m_returnPss, tool.second, depth, Pocket, fillPaths);
            m_file->setFileName(tool.second.name());
            emit fileReady(m_file);
        }
    } while (0);
    {
        m_returnPs.clear();

        m_toolDiameter = tool.first.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.first.stepover() * uScale;

        Paths fillPaths;

        for (Paths paths : m_groupedPss) {
            {
                double toolDiameter = tool.second.getDiameter(depth) * uScale;
                double dOffset = toolDiameter / 2;
                Paths tmpPaths;
                {
                    ClipperOffset offset(uScale);
                    offset.AddPaths(paths, jtRound, etClosedPolygon);
                    offset.Execute(tmpPaths, -dOffset);
                }
                {
                    ClipperOffset offset(uScale);
                    offset.AddPaths(tmpPaths, jtRound, etClosedPolygon);
                    offset.Execute(tmpPaths, dOffset - m_toolDiameter * 0.95);
                }
                if (m_gcp.side != Inner)
                    ReversePaths(tmpPaths);
                paths.append(tmpPaths);
            }
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            for (int i = 0; i < paths.size(); ++i) {
                if (abs(Area(paths[i])) < minArea * uScale * uScale)
                    paths.remove(i--);
            }
            fillPaths.append(paths);
            Paths tmpPaths;
            do {
                tmpPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size());
            m_returnPs.append(tmpPaths);
        }

        if (m_returnPs.isEmpty()) {
            emit fileReady(nullptr);
            return;
        }

        for (int i = 0; i < m_returnPs.size(); ++i) {
            if (Perimeter(m_returnPs[i]) < m_dOffset)
                m_returnPs.remove(i--);
        }
        for (int i = 0; i < fillPaths.size(); ++i) {
            if (Perimeter(fillPaths[i]) < m_dOffset)
                fillPaths.remove(i--);
        }
        stacking(m_returnPs);
        if (m_returnPss.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new GCode::File(m_returnPss, tool.first, depth, Pocket, fillPaths);
            m_file->setFileName(tool.first.name());
            emit fileReady(m_file);
        }
    }
}
}
