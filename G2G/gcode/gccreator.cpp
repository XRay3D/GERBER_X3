#include "gccreator.h"
#include "forms/gcodepropertiesform.h"
#include "gccreator.h"
#include "voroni/jc_voronoi.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <QStack>
#include <QThread>
#include <algorithm>
#include <filetree/filemodel.h>
#include <gbraperture.h>
#include <gi/bridgeitem.h>
#include <limits>
#include <scene.h>
#include <voronoi.h>

struct Pair {
    IntPoint first;
    IntPoint second;
    int id;
    bool operator==(const Pair& b) const { return first == b.first && second == b.second; }
};

using Pairs = QSet<Pair>;
using Pairss = QVector<Pairs>;
inline uint qHash(const Pair& tag, uint seed = 0) { return qHash(tag.first.X * tag.second.X, seed ^ 0xa317a317) ^ qHash(tag.first.Y * tag.second.Y, seed ^ 0x17a317a3); }

struct OrdPath {
    IntPoint Pt;
    OrdPath* Next = nullptr;
    OrdPath* Prev = nullptr;
    OrdPath* Last = nullptr;
    inline void append(OrdPath* opt)
    {
        Last->Next = opt;
        Last = opt->Prev->Last;
        opt->Prev = this;
    }
};

void fixBegin(Path& path)
{
    IntPoint pt(path.first());
    int rotate = 0;
    for (int i = 1, end = path.size(); i < end; ++i) {
        if (pt.Y >= path[i].Y) {
            pt.Y = path[i].Y;
            rotate = i;
            if (pt.X > path[i].X) {
                pt.X = path[i].X;
                rotate = i;
            }
        }
    }
    if (rotate)
        std::rotate(path.begin(), path.begin() + rotate, path.end());
}

namespace GCode {

Creator* Creator::self = nullptr;

Paths& sortByStratDistance(Paths& src)
{
    IntPoint startPt(toIntPoint(GCodePropertiesForm::homePoint->pos() + GCodePropertiesForm::zeroPoint->pos()));
    for (int firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        int swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for (int secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = Length(startPt, src[secondIdx].first());
            if (destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].last();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Paths& sortByStratEndDistance(Paths& src)
{
    IntPoint startPt(toIntPoint(GCodePropertiesForm::homePoint->pos() + GCodePropertiesForm::zeroPoint->pos()));
    for (int firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        int swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse;
        for (int secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = Length(startPt, src[secondIdx].first());
            const double lenLast = Length(startPt, src[secondIdx].last());
            if (lenFirst < lenLast) {
                if (destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else {
                if (destLen > lenLast) {
                    destLen = lenLast;
                    swapIdx = secondIdx;
                    reverse = true;
                }
            }
        }
        if (reverse)
            ReversePath(src[swapIdx]);
        startPt = src[swapIdx].last();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

bool PointOnPolygon(const QLineF& l2, const Path& path, IntPoint* ret = nullptr)
{
    int cnt = path.size();
    if (cnt < 3)
        return false;
    IntPoint pt1 = path[0];
    QPointF p;
    for (int i = 1; i <= cnt; ++i) {
        IntPoint pt2(i == cnt ? path[0] : path[i]);
        QLineF l1(toQPointF(pt1), toQPointF(pt2));
        if (QLineF::BoundedIntersection == l1.intersect(l2, &p)) {
            if (ret)
                *ret = toIntPoint(p);
            return true;
        }
        pt1 = pt2;
    }
    return false;
}
////////////////////////////////////////////////////////////////
/// \brief Creator::Creator
/// \param value
/// \param convent
///
Creator::Creator(const Paths& workingPaths, const bool convent, SideOfMilling side)
    : m_side(side)
    , m_workingPaths(workingPaths)
    , m_convent(convent)
{
}

Creator::~Creator()
{
    self = nullptr;
}
////////////////////////////////////////////////////////////////
/// \brief Creator::createRaster
/// \param tool
/// \param depth
/// \param angle
///
void Creator::createRaster(const Tool& tool, const double depth, const double angle, const int pPass)
{
    try {
        enum {
            NoProfilePass,
            First,
            Last,
        };
        QElapsedTimer t;
        t.start();
        if (m_side == On)
            return;

        m_toolDiameter = tool.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.stepover() * uScale;

        Paths profilePaths;

        switch (m_side) {
        case Outer:
            groupedPaths(CutoffPaths, m_toolDiameter + 5);
            if (m_groupedPathss.size() > 1 && m_groupedPathss.first().size() == 2)
                m_groupedPathss.removeFirst();
            break;
        case Inner:
            groupedPaths(CopperPaths);
            break;
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
                        if (angle == 90) {
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
            if (m_convent)
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
            m_file = new GCode::File(m_returnPaths, tool, depth, Profile);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
        qDebug() << "createRaster" << (t.elapsed() / 1000);
    } catch (...) {
        //qDebug() << "catch";
    }
}
////////////////////////////////////////////////////////////////
/// \brief Creator::createPocket
/// \param tool
/// \param depth
/// \param side
/// \param steps
/// \param ex
/// \return
///
void Creator::createPocket(const Tool& tool, const double depth, const int steps)
{
    try {
        self = this;

        if (m_side == On)
            return;

        m_toolDiameter = tool.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.stepover() * uScale;

        Paths fillPaths;

        if (steps) {
            groupedPaths(CopperPaths);
            if (m_side == Inner) {
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
            switch (m_side) {
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

void Creator::createPocket2(const QPair<Tool, Tool>& tool, double depth)
{
    try {
        self = this;

        if (m_side == On)
            return;

        do {
            m_toolDiameter = tool.second.getDiameter(depth) * uScale;
            m_dOffset = m_toolDiameter / 2;
            m_stepOver = tool.second.stepover() * uScale;

            Paths fillPaths;

            switch (m_side) {
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
                    if (m_side != Inner)
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

////////////////////////////////////////////////////////////////
/// \brief Creator::createProfile
/// \param tool
/// \param depth
/// \param side
/// \return
///
void Creator::createProfile(const Tool& tool, double depth)
{
    try {
        self = this;

        m_toolDiameter = tool.getDiameter(depth);
        // execute offset
        if (m_side == On) {
            m_returnPaths = m_workingPaths;

            for (Path& path : m_returnPaths)
                path.append(path.first());

            // fix direction
            if (m_convent)
                ReversePaths(m_returnPaths);

            if (m_workingRawPaths.size())
                m_returnPaths.append(m_workingRawPaths);

        } else {
            // calc offset
            const double dOffset = (m_side == Outer) ? +m_toolDiameter * uScale * 0.5 : -m_toolDiameter * uScale * 0.5;

            // execute offset
            if (!m_workingPaths.isEmpty()) {
                ClipperOffset offset;
                for (Paths& paths : groupedPaths(CopperPaths))
                    offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(m_returnPaths, dOffset);
            }
            if (!m_workingRawPaths.isEmpty()) {
                ClipperOffset offset;
                offset.AddPaths(m_workingRawPaths, jtRound, etOpenRound);
                offset.Execute(m_workingRawPaths, dOffset);
            }

            if (!m_workingRawPaths.isEmpty())
                m_returnPaths.append(m_workingRawPaths);

            // fix direction
            if (m_side == Outer && !m_convent)
                ReversePaths(m_returnPaths);
            else if (m_side == Inner && m_convent)
                ReversePaths(m_returnPaths);

            for (Path& path : m_returnPaths)
                path.append(path.first());
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            return;
        }

        // find Bridges
        QVector<BridgeItem*> bridgeItems;
        for (QGraphicsItem* item : Scene::items()) {
            if (item->type() == BridgeType)
                bridgeItems.append(static_cast<BridgeItem*>(item));
        }
        // create Bridges
        if (bridgeItems.size()) {
            for (int index = 0, size = m_returnPaths.size(); index < size; ++index) {
                Path& path = m_returnPaths[index];
                QList<QPair<BridgeItem*, IntPoint>> biStack;
                for (BridgeItem* bi : bridgeItems) {
                    IntPoint pt;
                    if (PointOnPolygon(bi->getPath(), path, &pt))
                        biStack.append({ bi, pt });
                }
                if (!biStack.isEmpty()) {
                    Paths tmpPaths;
                    // create frame
                    {
                        ClipperOffset offset;
                        offset.AddPath(path, jtMiter, etClosedLine);
                        offset.Execute(tmpPaths, +m_toolDiameter * uScale * 0.1);

                        Clipper clipper;
                        clipper.AddPaths(tmpPaths, ptSubject, true);
                        for (const QPair<BridgeItem*, IntPoint>& bip : biStack) {
                            clipper.AddPath(CirclePath((bip.first->lenght() + m_toolDiameter) * uScale, bip.second), ptClip, true);
                        }
                        clipper.Execute(ctIntersection, tmpPaths, pftPositive);
                    }
                    // cut toolPath
                    {
                        Clipper clipper;
                        clipper.AddPath(path, ptSubject, false);
                        clipper.AddPaths(tmpPaths, ptClip, true);
                        PolyTree polytree;
                        clipper.Execute(ctDifference, polytree, pftPositive);
                        PolyTreeToPaths(polytree, tmpPaths);
                    }
                    // merge result toolPaths
                    for (int i = 0; i < tmpPaths.size(); ++i) {
                        for (int j = 0; j < tmpPaths.size(); ++j) {
                            if (i == j)
                                continue;
                            if (tmpPaths[i].last() == tmpPaths[j].first()) {
                                tmpPaths[j].removeFirst();
                                tmpPaths[i].append(tmpPaths[j]);
                                tmpPaths.remove(j--);
                                continue;
                            }
                            if (tmpPaths[i].first() == tmpPaths[j].last()) {
                                tmpPaths[i].removeFirst();
                                tmpPaths[j].append(tmpPaths[i]);
                                tmpPaths.remove(i--);
                                break;
                            }
                            if (tmpPaths[i].last() == tmpPaths[j].last()) {
                                ReversePath(tmpPaths[j]);
                                tmpPaths[j].removeFirst();
                                tmpPaths[i].append(tmpPaths[j]);
                                tmpPaths.remove(j--);
                                continue;
                            }
                        }
                    }

                    --size;
                    m_returnPaths.remove(index--);
                    m_returnPaths.append(tmpPaths);
                }
            }
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new GCode::File(sortByStratDistance(m_returnPaths), tool, depth, Profile);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
    } catch (...) {
        //qDebug() << "catch";
    }
}

void Creator::createThermal(Gerber::File* file, const Tool& tool, double depth)
{
    try {
        self = this;
        m_toolDiameter = tool.getDiameter(depth);
        const double dOffset = m_toolDiameter * uScale * 0.5;

        // execute offset
        ClipperOffset offset;
        offset.AddPaths(m_workingPaths, jtRound, etClosedPolygon);
        offset.Execute(m_returnPaths, dOffset);

        // fix direction
        if (m_side == Outer && !m_convent)
            ReversePaths(m_returnPaths);
        else if (m_side == Inner && m_convent)
            ReversePaths(m_returnPaths);

        for (Path& path : m_returnPaths)
            path.append(path.first());

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            return;
        }

        // create frame
        Paths framePaths;
        {
            ClipperOffset offset;

            offset.AddPaths(m_returnPaths, jtMiter, etClosedLine);

            offset.Execute(framePaths, +m_toolDiameter * uScale * 0.1);
            Clipper clipper;
            clipper.AddPaths(framePaths, ptSubject, true);
            {
                ClipperOffset offset;
                for (const Gerber::GraphicObject& go : *file) {
                    if (go.state().type() == Gerber::Line && go.state().imgPolarity() == Gerber::Positive) {
                        offset.AddPaths(go.paths(), jtMiter, etClosedPolygon);
                    }
                }
                offset.Execute(framePaths, dOffset - 0.005 * uScale);
            }
            for (const Paths& paths : m_supportPathss) {
                clipper.AddPaths(paths, ptClip, true);
            }
            clipper.AddPaths(framePaths, ptClip, true);
            clipper.Execute(ctIntersection, framePaths, pftPositive);
        }

        // create thermal

        //self = nullptr;

        for (int index = 0, size = m_returnPaths.size(); index < size; ++index) {
            Path& path = m_returnPaths[index];
            //            {
            //                ClipperOffset offset;
            //                offset.AddPaths(paths, jtRound, etClosedPolygon);
            //                offset.Execute(paths, dOffset);
            //                for (Path& path : paths) {
            //                    path.append(path.last());
            //                }
            //            }

            Paths tmpPaths;
            // cut toolPath
            {
                Clipper clipper;
                clipper.AddPath(path, ptSubject, false);
                clipper.AddPaths(framePaths, ptClip, true);
                //                clipper.AddPaths(m_supportPathss[index], ptClip, true);
                PolyTree polytree;
                clipper.Execute(ctDifference, polytree, pftPositive);
                PolyTreeToPaths(polytree, tmpPaths);
            }
            // merge result toolPaths
            mergeSegments(tmpPaths);

            --size;
            m_returnPaths.remove(index--);
            m_returnPaths.append(tmpPaths);
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new GCode::File(sortByStratDistance(m_returnPaths), tool, depth, Thermal);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
    } catch (...) {
        //qDebug() << "catch";
    }
}

void Creator::createVoronoi(const Tool& tool, double depth, const double tolerance, const double width)
{
    try {
        QVector<jcv_point> points;
        points.reserve(100000);
        CleanPolygons(m_workingPaths, tolerance * 0.1 * uScale);
        groupedPaths(CopperPaths);
        int id = 0;

        auto condei = [&](IntPoint tmp, IntPoint point) {
            QLineF line(toQPointF(tmp), toQPointF(point));
            if (line.length() > tolerance) {
                for (int i = 1, total = line.length() / tolerance; i < total; ++i) {
                    line.setLength(i * tolerance);
                    IntPoint point(toIntPoint(line.p2()));
                    points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                }
            }
        };

        for (const Paths& paths : m_groupedPathss) {
            for (const Path& path : paths) {
                IntPoint tmp(path.first());
                for (const IntPoint& point : path) {
                    condei(tmp, point);
                    points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                    tmp = point;
                }
                condei(tmp, path.first());
            }
            ++id;
        }

        for (const Path& path : m_workingRawPaths) {
            IntPoint tmp(path.first());
            for (const IntPoint& point : path) {
                condei(tmp, point);
                points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                tmp = point;
            }
            condei(tmp, path.first());
            ++id;
        }

        QList<QSharedPointer<OrdPath>> holder;
        QList<OrdPath*> merge;

        Clipper clipper;
        for (const Paths& paths : m_groupedPathss) {
            clipper.AddPaths(paths, ptClip, true);
        }
        clipper.AddPaths(m_workingRawPaths, ptClip, true);
        const IntRect r(clipper.GetBounds());

        progressOrCancel(3, 1); // progress
        {
            jcv_rect bounding_box = {
                { static_cast<jcv_real>(r.left - uScale * 1.1), static_cast<jcv_real>(r.top - uScale * 1.1) },
                { static_cast<jcv_real>(r.right + uScale * 1.1), static_cast<jcv_real>(r.bottom + uScale * 1.1) }
            };

            Pairss edges;
            edges.resize(id);
            {
                auto toIntPoint = [](const jcv_point& pos) -> const IntPoint {
                    return { static_cast<cInt>(pos.x), static_cast<cInt>(pos.y) };
                };

                jcv_diagram diagram;
                jcv_diagram_generate(points.size(), points.data(), &bounding_box, &diagram);

                const jcv_site* sites = jcv_diagram_get_sites(&diagram);
                for (int i = 0; i < diagram.numsites; i++) {
                    jcv_graphedge* graph_edge = sites[i].edges;
                    while (graph_edge) {
                        const Pair pair1{ toIntPoint(graph_edge->pos[0]), toIntPoint(graph_edge->pos[1]), sites[i].p.id };
                        const Pair pair2{ toIntPoint(graph_edge->pos[1]), toIntPoint(graph_edge->pos[0]), sites[i].p.id };
                        if (edges[sites[i].p.id].contains(pair1))
                            edges[sites[i].p.id].remove(pair1);
                        else if (edges[sites[i].p.id].contains(pair2))
                            edges[sites[i].p.id].remove(pair2);
                        else
                            edges[sites[i].p.id].insert(pair1);
                        graph_edge = graph_edge->next;
                    }
                }
                jcv_diagram_free(&diagram);
            }

            progressOrCancel(3, 2); // progress

            Pairs tmp;
            for (const Pairs& edge : edges) {
                for (const Pair& pair1 : edge) {
                    Pair pair2(pair1);
                    std::swap(pair2.first, pair2.second);
                    if (!tmp.contains(pair1) && !tmp.contains(pair2)) {
                        tmp.insert(pair1);
                        progressOrCancel(edges.size() * edge.size(), tmp.size()); // progress
                    }
                }
            }

            QList<Pair> tmp2(tmp.toList());
            std::sort(tmp2.begin(), tmp2.end(), [](const Pair& a, const Pair& b) { return a.id > b.id; });

            for (const Pair& path : tmp2) {
                OrdPath* pt1 = new OrdPath;
                OrdPath* pt2 = new OrdPath;
                pt1->Pt = path.first;
                pt1->Next = pt2;
                pt1->Last = pt2;
                pt2->Pt = path.second;
                pt2->Prev = pt1;
                holder.append(QSharedPointer<OrdPath>(pt1));
                holder.append(QSharedPointer<OrdPath>(pt2));
                merge.append(pt1);
                progressOrCancel(tmp.size(), merge.size()); // progress
            }

            progressOrCancel(3, 3); // progress
        }
        {
            const int max = merge.size();
            for (int k = 0; k < 10; ++k) {
                for (int i = 0; i < merge.size(); ++i) {
                    progressOrCancel(max, max - merge.size()); // progress
                    for (int j = 0; j < merge.size(); ++j) {
                        if (i == j)
                            continue;
                        if (merge[i]->Last->Pt == merge[j]->Pt) {
                            merge[i]->append(merge[j]->Next);
                            merge.removeAt(j--);
                            continue;
                        }
                        if (merge[i]->Pt == merge[j]->Last->Pt) {
                            merge[j]->append(merge[i]->Next);
                            merge.removeAt(i--);
                            break;
                        }
                    }
                }
            }
            m_returnPaths.resize(merge.size());
            for (int i = 0; i < merge.size(); ++i) {
                progressOrCancel(merge.size(), i); // progress
                m_returnPaths[i].append(merge[i]->Pt);
                OrdPath* Next = merge[i]->Next;
                while (Next) {
                    m_returnPaths[i].append(Next->Pt);
                    Next = Next->Next;
                }
            }
        }
        const int max = m_returnPaths.size();
        for (int k = 0; k < 3; ++k) {
            for (int i = 0; i < m_returnPaths.size(); ++i) {
                progressOrCancel(max, max - m_returnPaths.size()); // progress
                for (int j = 0; j < m_returnPaths.size(); ++j) {
                    if (i == j)
                        continue;
                    if (m_returnPaths[i].first() == m_returnPaths[j].first()) {
                        ////qDebug("1 f f");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[j].append(m_returnPaths[i].mid(1));
                        m_returnPaths.remove(i--);
                        break;
                    }
                    if (m_returnPaths[i].last() == m_returnPaths[j].last()) {
                        ////qDebug("2 l l");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[i].append(m_returnPaths[j].mid(1));
                        m_returnPaths.remove(j--);
                        break;
                    }
                    if (Length(m_returnPaths[i].last(), m_returnPaths[j].last()) < 0.001 * uScale) {
                        ////qDebug("3 l l l");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[i].append(m_returnPaths[j].mid(1));
                        m_returnPaths.remove(j--);
                        break;
                    }
                    if (Length(m_returnPaths[i].last(), m_returnPaths[j].first()) < 0.001 * uScale) {
                        ////qDebug("4 l f l");
                        m_returnPaths[i].append(m_returnPaths[j].mid(1));
                        m_returnPaths.remove(j--);
                        break;
                    }
                    if (Length(m_returnPaths[i].first(), m_returnPaths[j].last()) < 0.001 * uScale) {
                        ////qDebug("5 f l l");
                        m_returnPaths[j].append(m_returnPaths[i].mid(1));
                        m_returnPaths.remove(i--);
                        break;
                    }
                    if (Length(m_returnPaths[i].first(), m_returnPaths[j].first()) < 0.001 * uScale) {
                        ////qDebug("6 f f l");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[j].append(m_returnPaths[i].mid(1));
                        m_returnPaths.remove(i--);
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < m_returnPaths.size(); ++i) {
            if (m_returnPaths[i].size() < 4 && Length(m_returnPaths[i].first(), m_returnPaths[i].last()) < tolerance * 0.5 * uScale)
                m_returnPaths.remove(i--);
        }

        QVector<QSet<int>> intersect(m_returnPaths.size());
        for (int i = 0; i < m_returnPaths.size(); ++i) {
            Path& path = m_returnPaths[i];
            progressOrCancel(m_returnPaths.size(), i); // progress
            for (int pi = 0; pi < path.size(); ++pi) {
                for (int j = 0; j < m_returnPaths.size(); ++j) {
                    Path& iPath = m_returnPaths[j];
                    if (abs(path[pi].X - iPath.first().X) < (tolerance * 0.001 * uScale)
                        && abs(path[pi].Y - iPath.first().Y) < (tolerance * 0.001 * uScale))
                        intersect[i].insert(pi);
                    if (abs(path[pi].X - iPath.last().X) < (tolerance * 0.001 * uScale)
                        && abs(path[pi].Y - iPath.last().Y) < (tolerance * 0.001 * uScale))
                        intersect[i].insert(pi);
                }
            }
        }

        auto Clean = [tolerance](Path& path, QList<int> skip) {
            std::sort(skip.begin(), skip.end());
            for (int i = 0, o = 0; i < path.size() - 1; ++i) {
                if (i > (skip.first() + o))
                    skip.removeFirst();
                if ((i == (skip.first() + o)) || ((i + 1) == (skip.first() + o)))
                    continue;
                QLineF line(toQPointF(path[i]), toQPointF(path[i + 1]));
                if (line.length() < tolerance) {
                    path[i] = toIntPoint(line.center());
                    path.remove(i + 1);
                    --o;
                    --i;
                }
            }

            for (int i = 1; i < path.size() - 1; ++i) {
                const double a1 = Angle(path[i - 1], path[i]);
                const double a2 = Angle(path[i], path[i + 1]);
                if (abs(a1 - a2) < 0.2) {
                    path.remove(i--);
                }
            }
        };
        double size1 = 0, size2 = 0;
        for (int i = 0; i < m_returnPaths.size(); ++i) {
            size1 += m_returnPaths[i].size();
            Clean(m_returnPaths[i], intersect[i].toList());
            size2 += m_returnPaths[i].size();
        }

        if (width < tool.getDiameter(depth)) {
            m_file = new GCode::File(sortByStratEndDistance(m_returnPaths), tool, depth, Voronoi);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        } else {
            m_toolDiameter = tool.getDiameter(depth) * uScale;
            m_dOffset = m_toolDiameter / 2;
            m_stepOver = tool.stepover() * uScale;
            { // create frame
                ClipperOffset offset;
                offset.AddPaths(m_returnPaths, jtRound /*jtMiter*/, etOpenRound);
                offset.Execute(m_returnPaths, width * uScale * 0.5);
            }
            { // fit frame to copper
                Clipper clipper;
                clipper.AddPaths(m_returnPaths, ptSubject, true);
                for (const Paths& paths : m_groupedPathss) {
                    clipper.AddPaths(paths, ptClip, true);
                }
                clipper.Execute(ctDifference, m_returnPaths, pftPositive, pftNegative);
            }
            { // cut to copper rect
                cInt k = -m_dOffset * 0.5;
                Path outer{
                    IntPoint(r.right + k, r.bottom + k),
                    IntPoint(r.left - k, r.bottom + k),
                    IntPoint(r.left - k, r.top - k),
                    IntPoint(r.right + k, r.top - k)
                };
                Clipper clipper;
                clipper.AddPaths(m_returnPaths, ptSubject, true);
                clipper.AddPath(outer, ptClip, true);
                clipper.Execute(ctIntersection, m_returnPaths, pftPositive);
                CleanPolygons(m_returnPaths, 0.001 * uScale);
            }
            { // create pocket
                ClipperOffset offset(uScale);
                offset.AddPaths(m_returnPaths, jtRound, etClosedPolygon);
                Paths tmpPaths1;
                offset.Execute(tmpPaths1, -m_dOffset);
                Paths tmpPaths;
                do {
                    tmpPaths.append(tmpPaths1);
                    offset.Clear();
                    offset.AddPaths(tmpPaths1, jtMiter, etClosedPolygon);
                    offset.Execute(tmpPaths1, -m_stepOver);
                } while (tmpPaths1.size());
                m_returnPaths = tmpPaths;
            }
            if (m_returnPaths.isEmpty()) {
                emit fileReady(nullptr);
                return;
            }
            { // stacking
                Clipper clipper;
                clipper.AddPaths(m_returnPaths, ptSubject, true);
                IntRect r(clipper.GetBounds());
                int k = tool.diameter() * uScale;
                Path outer = {
                    IntPoint(r.left - k, r.bottom + k),
                    IntPoint(r.right + k, r.bottom + k),
                    IntPoint(r.right + k, r.top - k),
                    IntPoint(r.left - k, r.top - k)
                };
                clipper.AddPath(outer, ptSubject, true);
                PolyTree polyTree;
                clipper.Execute(ctUnion, polyTree, pftEvenOdd);
                m_returnPaths.clear();
                grouping2(polyTree.GetFirst(), &m_returnPaths);
            }

            ReversePaths(m_returnPaths);
            sortByStratDistance(m_returnPaths);

            m_file = new GCode::File(m_returnPaths, tool, depth, Voronoi);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
    } catch (bool) {
        qWarning() << "canceled";
    } catch (...) {
        qWarning() << "create voronoi exeption:" << strerror(errno);
    }
}

Pathss& Creator::groupedPaths(Grouping group, cInt k, bool fl)
{
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddPaths(m_workingPaths, ptSubject, true);
    IntRect r(clipper.GetBounds());
    //int k = /*uScale*/ 1;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };
    if (fl)
        ReversePath(outer);
    clipper.AddPath(outer, ptSubject, true);
    clipper.Execute(ctUnion, polyTree, pftNonZero);
    grouping(polyTree.GetFirst(), &m_groupedPathss, group);
    return m_groupedPathss;
}
////////////////////////////////////////////////////////////////
/// \brief Creator::addRawPaths
/// \param paths
///
void Creator::addRawPaths(Paths rawPaths)
{
    if (rawPaths.isEmpty())
        return;

    if (m_side == On) {
        m_workingRawPaths.append(rawPaths);
        return;
    }

    //qDebug() << rawPaths.size();

    const double glueLen = GCodePropertiesForm::glue * uScale;
    Paths paths;

    do {
        Path path = rawPaths.takeFirst();
        for (int i = 0; i < rawPaths.size();) {
            const IntPoint& pt1 = path.last();
            const IntPoint& pt2 = rawPaths[i].first();
            const IntPoint& pt3 = rawPaths[i].last();
            if (Length(pt1, pt2) < glueLen) {
                path.append(rawPaths.takeAt(i));
                i = 0;
            } else if (Length(pt1, pt3) < glueLen) {
                ReversePath(rawPaths[i]);
                path.append(rawPaths.takeAt(i));
                i = 0;
            } else
                ++i;
        }
        paths.append(path);
    } while (rawPaths.size());

    Clipper clipper;

    for (Path path : paths) {
        if (Length(path.first(), path.last()) < glueLen) //path.first() == path.last())
            clipper.AddPath(path, ptSubject, true);
        else
            m_workingRawPaths.append(path);
    }

    IntRect r(clipper.GetBounds());
    int k = uScale * 10;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };

    clipper.AddPath(outer, ptClip, true);
    clipper.Execute(ctXor, paths, pftEvenOdd);
    paths.takeFirst();
    //qDebug() << paths.size();
    m_workingPaths.append(paths);
}

void Creator::addSupportPaths(Pathss supportPaths)
{
    m_supportPathss.append(supportPaths);
}

void Creator::addPaths(const Paths& paths)
{
    m_workingPaths.append(paths);
}

GCode::File* Creator::file() const
{
    return m_file;
}

void Creator::grouping(PolyNode* node, Pathss* pathss, Grouping group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CutoffPaths:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }

        for (int i = 0, end = node->ChildCount(); i < end; ++i)
            grouping(node->Childs[i], pathss, group);
        break;
    case CopperPaths:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int i = 0, end = node->ChildCount(); i < end; ++i)
            grouping(node->Childs[i], pathss, group);
        break;
    }
}

Path& Creator::fixPath(PolyNode* node)
{
    if (m_convent ^ !node->IsHole())
        ReversePath(node->Contour);
    else
        fixBegin(node->Contour);

    node->Contour.append(node->Contour.first());
    return node->Contour;
}

void Creator::grouping2(PolyNode* node, Paths* paths, bool fl)
{

    static bool newPath = false;
    //    static Path* lastPaph = nullptr;
    if (fl) {
        Path path(fixPath(node));
        if (paths->isEmpty() || newPath) {
            paths->append(path);
        } else {
            if (Length(paths->last().last(), path.last()) < m_toolDiameter * 1.5)
                paths->last().append(path);
            else
                paths->append(path);
        }
        if (node->ChildCount() == 1) {
            newPath = false;
            grouping2(node->Childs[0], paths, true);
        } else {
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                newPath = true;
                grouping2(node->Childs[i], paths, true);
            }
        }
    } else {
        // Start from non hole paths
        for (int i = 0, end = node->ChildCount(); i < end; ++i) {
            newPath = true;
            grouping2(node->Childs[i], paths, true);
        }
    }
}

void Creator::grouping3(Paths& paths)
{
    Clipper clipper;
    clipper.AddPaths(paths, ptSubject, true);
    IntRect r(clipper.GetBounds());

    int k = /*tool.diameter() **/ uScale;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };
    PolyTree polyTree;
    clipper.AddPath(outer, ptSubject, true);
    clipper.Execute(ctUnion, polyTree, pftEvenOdd);

    paths.clear();
    grouping2(polyTree.GetFirst(), &paths);
}

void Creator::mergeSegments(Paths& paths)
{

    for (int i = 0; i < paths.size(); ++i) {
        for (int j = 0; j < paths.size(); ++j) {
            if (i == j)
                continue;
            if (paths[i].last() == paths[j].first()) {
                paths[j].removeFirst();
                paths[i].append(paths[j]);
                paths.remove(j--);
                continue;
            }
            if (paths[i].first() == paths[j].last()) {
                paths[i].removeFirst();
                paths[j].append(paths[i]);
                paths.remove(i--);
                break;
            }
            if (paths[i].last() == paths[j].last()) {
                ReversePath(paths[j]);
                paths[j].removeFirst();
                paths[i].append(paths[j]);
                paths.remove(j--);
                continue;
            }
        }
    }
}

void Creator::progressOrCancel(int progressMax, int progressValue)
{
    m_progressValue = progressValue;
    m_progressMax = progressMax;
    if (!(progressValue % 100)) {
        if (QThread::currentThread()->isInterruptionRequested())
            throw true;
    }
}

void Creator::progressOrCancel()
{
    if (!Creator::self)
        return;
    if (!(++self->m_progressValue % 1000)) {
        if (self->m_progressMax == self->m_progressValue)
            self->m_progressMax += 1000000;
        if (QThread::currentThread()->isInterruptionRequested())
            throw true;
    }
}

int Creator::progressValue() const
{
    if (QThread::currentThread()->isInterruptionRequested())
        throw true;
    return m_progressValue;
}

int Creator::progressMax() const { return m_progressMax; }
}
