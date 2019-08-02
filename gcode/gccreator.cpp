#include "gccreator.h"
#include "forms/gcodepropertiesform.h"
#include "gccreator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <QStack>
#include <algorithm>
#include <filetree/filemodel.h>
#include <gbraperture.h>
#include <gcvoronoi.h>
#include <gi/bridgeitem.h>
#include <limits>
#include <scene.h>

namespace GCode {

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

Creator* Creator::self = nullptr;

////////////////////////////////////////////////////////////////
/// \brief Creator::Creator
/// \param value
/// \param convent
///
//Creator::Creator(const Paths& workingPaths, const bool convent, SideOfMilling side)
//    : m_workingPaths(workingPaths)
//    , m_side(side)
//    , m_convent(convent)
//{
//}

Creator::~Creator()
{
    self = nullptr;
}
////////////////////////////////////////////////////////////////
/// \brief Creator::create
/// \param gcp
///

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

    if (m_gcp.side == On) {
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
    if (m_gcp.convent ^ !node->IsHole())
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

Paths& Creator::sortByStratDistance(Paths& src)
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

Paths& Creator::sortByStratEndDistance(Paths& src)
{
    IntPoint startPt(toIntPoint(GCodePropertiesForm::homePoint->pos() + GCodePropertiesForm::zeroPoint->pos()));
    for (int firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        int swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse = false;
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

bool Creator::PointOnPolygon(const QLineF& l2, const Path& path, IntPoint* ret)
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
}
