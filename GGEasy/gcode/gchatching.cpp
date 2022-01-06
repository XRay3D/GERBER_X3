// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "gchatching.h"

#include "gcfile.h"

#include <QElapsedTimer>
#ifndef __GNUC__
#include <execution>
#endif
#include "point.h"
//#include <QFuture>
//#include <QtConcurrent>
#include <algorithm>
#include <forward_list>
#include <ranges>

#include "leakdetector.h"

//struct sort_fn {
//    template <std::random_access_iterator I, std::sentinel_for<I> S, class Comp = std::ranges::less, class Proj = std::identity>
//    requires std::sortable<I, Comp, Proj> constexpr I
//    operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
//    {
//        if (first == last)
//            return { first };

//        const auto pivot = *std::ranges::next(first, std::ranges::distance(first, last) / 2, last);

//        auto tail1 = std::ranges::partition(first, last, [&pivot, &comp, &proj](const auto& em) { return std::invoke(comp, std::invoke(proj, em), std::invoke(proj, pivot)); });
//        auto tail2 = std::ranges::partition(tail1, [&pivot, &comp, &proj](const auto& em) { return !std::invoke(comp, std::invoke(proj, pivot), std::invoke(proj, em)); });

//        (*this)(first, tail1.begin(), std::ref(comp), std::ref(proj));
//        (*this)(tail2, std::ref(comp), std::ref(proj));

//        return { std::ranges::next(first, last) };
//    }

//    template <std::ranges::random_access_range R, class Comp = std::ranges::less, class Proj = std::identity>
//    requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj> constexpr std::ranges::borrowed_iterator_t<R>
//    operator()(R&& r, Comp comp = {}, Proj proj = {}) const
//    {
//        return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
//    }
//};
//inline constexpr sort_fn sort {};

namespace GCode {
HatchingCreator::HatchingCreator()
{
}

void HatchingCreator::create()
{
    createRaster(
        m_gcp.tools.front(),
        m_gcp.params[GCodeParams::Depth].toDouble(),
        m_gcp.params[GCodeParams::UseAngle].toDouble(),
        m_gcp.params[GCodeParams::HathStep].toDouble(),
        m_gcp.params[GCodeParams::Pass].toInt());
}

void HatchingCreator::createRaster(const Tool& tool, const double depth, const double angle, const double hatchStep, const int prPass)
{
    QElapsedTimer t;
    t.start();

    switch (m_gcp.side()) {
    case Outer:
        groupedPaths(CutoffPaths, uScale /*static_cast<cInt>(m_toolDiameter + 5)*/);
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

    auto calcScanLines = [](const Paths& src, const Path& frame) {
        Paths sl; // Scan Lines

        Clipper clipper;
        clipper.AddPaths(src, ptClip);
        clipper.AddPath(frame, ptSubject, false);
        clipper.Execute(ctIntersection, sl);
        if (!sl.size())
            return sl;

        std::ranges::sort(sl, {}, [](const Path& p) { return p.front().Y; }); // vertical sort

        cInt start = sl.front().front().Y;
        bool fl = {};
        for (size_t i {}, last {}; i < sl.size(); ++i) {
            if (auto y = sl[i].front().Y; y != start || i - 1 == sl.size()) {

                fl ? std::ranges::sort(sl.begin() + last, sl.begin() + i, {}, [](const Path& p) { return p.front().X; }) // horizontal sort
                   : std::ranges::sort(sl.begin() + last, sl.begin() + i, std::greater(), [](const Path& p) { return p.front().X; }); // horizontal sort

                for (size_t k = last; k < i; ++k) { // fix direction
                    if (fl ^ (sl[k].front().X < sl[k].back().X))
                        std::swap(sl[k].front().X, sl[k].back().X);
                }

                start = y;
                fl = !fl;
                last = i;
            }
        }
        return sl;
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

            std::ranges::sort(frames, {}, [](const Path& p) { return p.front().Y; }); // vertical sort

            std::sort(frames.begin(), frames.end(), [](const Path& l, const Path& r) { return l.front().Y < r.front().Y; }); // vertical sort
            for (auto& path : frames) {
                if (path.front().Y > path.back().Y)
                    ReversePath(path); // fix vertical direction
            }
        }
        return frames;
    };
    auto calcZigzag = [hatchStep](const Paths& src) {
        Clipper clipper;
        clipper.AddPaths(src, ptClip, true);
        IntRect rect(clipper.GetBounds());
        cInt o = uScale - (rect.height() % static_cast<cInt>(hatchStep * uScale)) / 2;
        rect.top -= o;
        rect.bottom += o;
        rect.left -= uScale;
        rect.right += uScale;
        Path zigzag;
        cInt step = hatchStep * uScale;
        cInt start = rect.top;
        bool fl {};

        for (; start <= rect.bottom || fl; fl = !fl, start += step) {
            if (!fl) {
                zigzag.emplace_back(rect.left, start);
                zigzag.emplace_back(rect.right, start);
            } else {
                zigzag.emplace_back(rect.right, start);
                zigzag.emplace_back(rect.left, start);
            }
        }

        zigzag.front().X -= step;
        zigzag.back().X -= step;
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
                ifCancelThenThrow();
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
        {
            ClipperOffset offset(uScale);
            offset.AddPaths(src, jtRound, etClosedPolygon);
            offset.Execute(src, -m_dOffset);
            for (auto& path : src)
                path.push_back(path.front());
            if (prPass)
                profilePaths.append(src);
        }

        QElapsedTimer t;
        t.start();
        if (src.size()) {
            {
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
            {
                for (auto& path : src)
                    RotatePath(path, 90);
                auto zigzag { calcZigzag(src) };
                auto scanLines { calcScanLines(src, zigzag) };
                auto frames { calcFrames(src, zigzag) };
                if (scanLines.size() && frames.size()) {
                    auto merged { merge(scanLines, frames) };
                    for (auto& path : merged)
                        RotatePath(path, -(angle + 90));
                    m_returnPs.append(merged);
                }
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

    m_returnPss.clear();
    switch (prPass) {
    case NoProfilePass:
        m_returnPss.push_back(m_returnPs);
        break;
    case First:
        if (!profilePaths.empty())
            m_returnPss.push_back(profilePaths);
        m_returnPss.push_back(m_returnPs);
        break;
    case Last:
        m_returnPss.push_back(m_returnPs);
        if (!profilePaths.empty())
            m_returnPss.push_back(profilePaths);
        break;
    default:
        break;
    }

    if (m_returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = Hatching;
        m_file = new File(m_returnPss, m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

}
