#include "gcpocket.h"
#include "gcfile.h"

#include <QElapsedTimer>
namespace GCode {
PocketCreator::PocketCreator()
{
}

void PocketCreator::create(const GCodeParams& gcp)
{
    m_gcp = gcp;
    if (!qFuzzyIsNull(m_gcp.dParam[TwoTools])) {
        createPocket2({ m_gcp.tool.first(), m_gcp.tool.last() }, gcp.dParam[Depth]);
    } else if (!qFuzzyIsNull(m_gcp.dParam[UseRaster])) {
        createRaster(m_gcp.tool.first(), gcp.dParam[Depth], gcp.dParam[UseAngle], gcp.dParam[Pass]);
    } else {
        createPocket(m_gcp.tool.first(), gcp.dParam[Depth], gcp.dParam[Steps]);
    }
}

void PocketCreator::createRaster(const Tool& tool, const double depth, const double angle, const int pPass)
{
    try {
        enum {
            NoProfilePass,
            First,
            Last,
        };
        QElapsedTimer t;
        t.start();
        if (m_gcp.side == On)
            return;

        m_toolDiameter = tool.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.stepover() * uScale;

        Paths profilePaths;

        switch (m_gcp.side) {
        case Outer:
            groupedPaths(CutoffPaths, m_toolDiameter + 5);
            if (m_groupedPathss.size() > 1 && m_groupedPathss.first().size() == 2)
                m_groupedPathss.removeFirst();
            break;
        case Inner:
            groupedPaths(CopperPaths);
            break;
        default:;
        }

        int maxCounter = 0;
        int counter = 0;

        for (Paths src : m_groupedPathss) {
            if (pPass) {
                ClipperOffset offset(uScale);
                offset.AddPaths(src, jtRound, etClosedPolygon);
                offset.Execute(src, -m_dOffset);
                profilePaths.append(src);
            }

            if (src.size()) {
                for (Path& path : src)
                    path.append(path.first());
                ClipperOffset offset(uScale);
                offset.AddPaths(src, jtRound, etClosedPolygon);
                offset.Execute(src, -m_stepOver);
            } else
                continue;

            if (src.size()) {
                for (Path& path : src)
                    path.append(path.first());

                Clipper clipper;
                clipper.AddPaths(src, ptClip, true);
                const IntRect r(clipper.GetBounds());
                clipper.Clear();
                const cInt size = Length({ r.left, r.top }, { r.right, r.bottom });
                const cInt end = r.bottom + (size - (r.bottom - r.top)) * 0.5;
                const cInt start = r.top - (size - (r.bottom - r.top)) * 0.5;
                const cInt left = r.left - (size - (r.right - r.left)) * 0.5;
                const cInt right = r.right + (size - (r.right - r.left)) * 0.5;
                const IntPoint center(0.5 * (r.left + r.right), 0.5 * (r.top + r.bottom));

                Paths acc;
                maxCounter += end / 100;
                for (int var = start, flag = 0; var < end; /*var += m_stepOver,*/ flag = (flag ? 0 : 1)) {
                    counter += m_stepOver / 100;
                    progressOrCancel(maxCounter, counter);

                    Paths scanLine;
                    {
                        Path frame{ { left, var }, { right, var } };
                        RotatePath(frame, angle, center);
                        Clipper clipper;
                        clipper.AddPaths(src, ptClip, true);
                        clipper.AddPath(frame, ptSubject, false);
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
                        //var += m_stepOver; //for next step
                        Path frame{ { left, var }, { right, var }, { right, var += m_stepOver }, { left, var } };
                        RotatePath(frame, angle, center);
                        Paths toNext;
                        Clipper clipper;
                        clipper.AddPaths(src, ptSubject, false);
                        clipper.AddPath(frame, ptClip, true);
                        clipper.Execute(ctIntersection, toNext, pftPositive);
                        mergeSegments(toNext);
                        if (scanLine.isEmpty()) {
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
                                //                                for (Path& src : toNext) {
                                //                                    if (dst.last() == src.first()) {
                                //                                        dst.append(src.mid(1));
                                //                                        break;
                                //                                    }
                                //                                    if (dst.last() == src.last()) {
                                //                                        ReversePath(src);
                                //                                        dst.append(src.mid(1));
                                //                                        break;
                                //                                    }
                                //                                }
                            }
                            acc.append(scanLine);
                            acc.append(toNext);
                        }
                    }
                }
                mergeSegments(acc);
                m_returnPaths.append(acc);
            }
        }
        sortByStratEndDistance(m_returnPaths);

        if (pPass) {
            for (Path& path : profilePaths)
                path.append(path.first());
            sortByStratDistance(profilePaths);
            if (m_gcp.convent)
                ReversePaths(profilePaths);
        }

        switch (pPass) {
        case NoProfilePass:
            break;
        case First:
            profilePaths.append(m_returnPaths);
            m_returnPaths = profilePaths;
            break;
        case Last:
            m_returnPaths.append(profilePaths);
            break;
        default:
            break;
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            return;
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new File(m_returnPaths, tool, depth, Profile);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
        qDebug() << "createRaster" << (t.elapsed() / 1000);
    } catch (...) {
        //qDebug() << "catch";
    }
}

void PocketCreator::createPocket(const Tool& tool, const double depth, const int steps)
{
    try {
        self = this;

        if (m_gcp.side == On)
            return;

        m_toolDiameter = tool.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.stepover() * uScale;

        Paths fillPaths;

        if (steps) {
            groupedPaths(CopperPaths);
            if (m_gcp.side == Inner) {
                m_dOffset *= -1;
                for (Paths paths : m_groupedPathss) {
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
                    m_returnPaths.append(tmpPaths);
                }
            } else {
                ClipperOffset offset(uScale);
                for (Paths paths : m_groupedPathss) {
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
                        m_returnPaths.append(paths);
                        offset.Clear();
                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
                        offset.Execute(paths, m_dOffset);
                    } while (paths.size() && --counter);
                } else {
                    m_returnPaths.append(paths);
                    fillPaths.append(paths);
                }
            }
        } else {
            switch (m_gcp.side) {
            case Outer:
                groupedPaths(CutoffPaths, m_toolDiameter + 5);
                if (m_groupedPathss.size() > 1 && m_groupedPathss.first().size() == 2)
                    m_groupedPathss.removeFirst();
                break;
            case Inner:
                groupedPaths(CopperPaths);
                break;
            }
            for (Paths paths : m_groupedPathss) {
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
                m_returnPaths.append(tmpPaths);
            }
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            return;
        }

        grouping3(m_returnPaths);

        ReversePaths(m_returnPaths);
        sortByStratDistance(m_returnPaths);

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new GCode::File(m_returnPaths, tool, depth, Pocket, fillPaths);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
    } catch (...) {
        //qDebug() << "catch";
    }
}

void PocketCreator::createPocket2(const QPair<Tool, Tool>& tool, double depth)
{
    try {
        self = this;

        if (m_gcp.side == On)
            return;

        do {
            m_toolDiameter = tool.second.getDiameter(depth) * uScale;
            m_dOffset = m_toolDiameter / 2;
            m_stepOver = tool.second.stepover() * uScale;

            Paths fillPaths;

            switch (m_gcp.side) {
            case Outer:
                groupedPaths(CutoffPaths, m_toolDiameter + 5);
                if (m_groupedPathss.size() > 1 && m_groupedPathss.first().size() == 2)
                    m_groupedPathss.removeFirst();
                break;
            case Inner:
                groupedPaths(CopperPaths);
                break;
            }

            for (Paths paths : m_groupedPathss) {
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
                m_returnPaths.append(tmpPaths);
            }

            if (m_returnPaths.isEmpty()) {
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

            grouping3(m_returnPaths);

            ReversePaths(m_returnPaths);
            sortByStratDistance(m_returnPaths);

            if (m_returnPaths.isEmpty()) {
                emit fileReady(nullptr);
            } else {
                m_file = new GCode::File(m_returnPaths, tool.second, depth, Pocket, fillPaths);
                m_file->setFileName(tool.second.name());
                emit fileReady(m_file);
            }
        } while (0);
        {
            m_returnPaths.clear();

            m_toolDiameter = tool.first.getDiameter(depth) * uScale;
            m_dOffset = m_toolDiameter / 2;
            m_stepOver = tool.first.stepover() * uScale;

            Paths fillPaths;

            for (Paths paths : m_groupedPathss) {
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
                        offset.Execute(tmpPaths, dOffset - m_toolDiameter * 0.9);
                    }
                    if (m_gcp.side != Inner)
                        ReversePaths(tmpPaths);
                    paths.append(tmpPaths);
                }
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
                m_returnPaths.append(tmpPaths);
            }

            if (m_returnPaths.isEmpty()) {
                emit fileReady(nullptr);
                return;
            }

            for (int i = 0; i < m_returnPaths.size(); ++i) {
                if (Perimeter(m_returnPaths[i]) < m_dOffset)
                    m_returnPaths.remove(i--);
            }
            for (int i = 0; i < fillPaths.size(); ++i) {
                if (Perimeter(fillPaths[i]) < m_dOffset)
                    fillPaths.remove(i--);
            }

            grouping3(m_returnPaths);

            ReversePaths(m_returnPaths);
            sortByStratDistance(m_returnPaths);

            if (m_returnPaths.isEmpty()) {
                emit fileReady(nullptr);
            } else {
                m_file = new GCode::File(m_returnPaths, tool.first, depth, Pocket, fillPaths);
                m_file->setFileName(tool.first.name());
                emit fileReady(m_file);
            }
        }
    } catch (...) {
        //qDebug() << "catch";
    }
}
}
