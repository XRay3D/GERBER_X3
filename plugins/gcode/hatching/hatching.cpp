// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "hatching.h"
#include "gi_point.h"
#include "project.h"

#include <QElapsedTimer>
#ifndef __GNUC__
#undef emit
#include <execution>
#define emit

#endif

#include <algorithm>
#include <forward_list>
#include <ranges>

// struct sort_fn {
//     template <std::randoaccess_iterator_ I, std::sentinel_for<I> S, class Comp = std::ranges::less, class Proj = std::identity>
//     requires std::sortable<I, Comp, Proj> constexpr I
//     operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
//     {
//         if (first == last)
//             return { first };

//        const auto pivot = *std::ranges::next(first, std::ranges::distance(first, last) / 2, last);

//        auto tail1 = std::ranges::partition(first, last, [&pivot, &comp, &proj](const auto& em) { return std::invoke(comp, std::invoke(proj, em), std::invoke(proj, pivot)); });
//        auto tail2 = std::ranges::partition(tail1, [&pivot, &comp, &proj](const auto& em) { return !std::invoke(comp, std::invoke(proj, pivot), std::invoke(proj, em)); });

//        (*this)(first, tail1.begin(), std::ref(comp), std::ref(proj));
//        (*this)(tail2, std::ref(comp), std::ref(proj));

//        return { std::ranges::next(first, last) };
//    }

//    template <std::ranges::randoaccess_range_ R, class Comp = std::ranges::less, class Proj = std::identity>
//    requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj> constexpr std::ranges::borrowed_iterator_t<R>
//    operator()(R&& r, Comp comp = {}, Proj proj = {}) const
//    {
//        return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
//    }
//};
// inline constexpr sort_fn sort {};

namespace CrossHatch {

void Creator::create() {
    createRaster(
        gcp_.tools.front(),
        gcp_.params[GCode::Params::Depth].toDouble(),
        gcp_.params[UseAngle].toDouble(),
        gcp_.params[HathStep].toDouble(),
        gcp_.params[Pass].toInt());
}

void Creator::createRaster(const Tool& tool, const double depth, const double angle, const double hatchStep, const int prPass) {
    QElapsedTimer t;
    t.start();

    switch(gcp_.side()) {
    case GCode::Outer:
        groupedPaths(GCode::Grouping::Cutoff, uScale); // toolDiameter_ + 5
        break;
    case GCode::Inner:
        groupedPaths(GCode::Grouping::Copper);
        break;
    case GCode::On:
        emit fileReady(nullptr);
        return;
    }

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;

    Paths profilePaths;

    auto calcScanLines = [](const Paths& src, const Path& frame) {
        Paths sl; // Scan Lines

        Clipper clipper;
        clipper.AddClip(src);
        clipper.AddOpenSubject({frame});
        clipper.Execute(ClipType::Intersection, FillRule::NonZero, sl, sl);
        if(!sl.size())
            return sl;

        std::ranges::sort(sl, {}, [](const Path& p) { return p.front().y; }); // vertical sort

        /*Point::Type*/ int32_t start = sl.front().front().y;
        bool fl = {};
        for(size_t i{}, last{}; i < sl.size(); ++i) {
            if(auto y = sl[i].front().y; y != start || i - 1 == sl.size()) {

                fl ? std::ranges::sort(sl.begin() + last, sl.begin() + i, {}, [](const Path& p) { return p.front().x; }) :           // horizontal sort
                    std::ranges::sort(sl.begin() + last, sl.begin() + i, std::greater(), [](const Path& p) { return p.front().x; }); // horizontal sort

                for(size_t k = last; k < i; ++k) // fix direction
                    if(fl ^ (sl[k].front().x < sl[k].back().x))
                        std::swap(sl[k].front().x, sl[k].back().x);

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
            clipper.AddOpenSubject(src);
            clipper.AddClip({frame});
            clipper.Execute(ClipType::Intersection, FillRule::NonZero, tmp, tmp); // FillRule::NonZero
            // dbgPaths(tmp, "ClipType::Intersection");
            frames += std::move(tmp);
            clipper.Execute(ClipType::Difference, FillRule::NonZero, tmp, tmp); // FillRule::NonZero //-V1030
            // dbgPaths(tmp, "ClipType::Difference");
            frames += std::move(tmp);

            std::ranges::sort(frames, {}, [](const Path& p) { return p.front().y; }); // vertical sort

            std::sort(frames.begin(), frames.end(), [](const Path& l, const Path& r) { return l.front().y < r.front().y; }); // vertical sort
            for(auto& path: frames)
                if(path.front().y > path.back().y)
                    ReversePath(path); // fix vertical direction
        }
        return frames;
    };
    auto calcZigzag = [hatchStep](const Paths& src) -> std::optional<Path> {
        Clipper clipper;
        clipper.AddClip(src);
        Rect rect(GetBounds(src));
        /*Point::Type*/ int32_t o = uScale - (rect.Height() % static_cast</*Point::Type*/ int32_t>(hatchStep * uScale)) / 2;
        rect.top -= o;
        rect.bottom += o;
        rect.left -= uScale;
        rect.right += uScale;
        Path zigzag;
        auto step = hatchStep * uScale;
        auto start = rect.top;
        bool fl{};

        for(; start <= rect.bottom || fl; fl = !fl, start += step) {
            if(!fl) {
                zigzag.emplace_back(rect.left, start);
                zigzag.emplace_back(rect.right, start);
            } else {
                zigzag.emplace_back(rect.right, start);
                zigzag.emplace_back(rect.left, start);
            }
        }
        if(zigzag.empty()) return std::nullopt;
        zigzag.front().x -= step;
        zigzag.back().x -= step;
        return zigzag;
    };

    auto merge = [](const Paths& scanLines, const Paths& frames) {
        Paths merged;
        merged.reserve(scanLines.size() / 10);
        std::list<Path> bList;
        for(auto&& path: scanLines)
            bList.emplace_back(std::move(path));

        std::list<Path> fList;
        for(auto&& path: frames)
            fList.emplace_back(std::move(path));

        setMax(bList.size());
        while(bList.begin() != bList.end()) {
            setCurrent(bList.size());

            merged.resize(merged.size() + 1);
            auto& path = merged.back();
            for(auto bit = bList.begin(); bit != bList.end(); ++bit) {
                ifCancelThenThrow();
                if(path.empty() || path.back() == bit->front()) {
                    path.empty() ? path += * bit
                                 : path += *bit | skipFront;
                    bList.erase(bit);
                    for(auto fit = fList.begin(); fit != fList.end(); ++fit) {
                        if(path.back() == fit->front() && fit->front().y < fit->at(1).y) {
                            path += *fit | skipFront;
                            fList.erase(fit);
                            bit = bList.begin();
                            break;
                        }
                    }
                    bit = bList.begin();
                }
                if(bList.begin() == bList.end())
                    break;
            }
            for(auto fit = fList.begin(); fit != fList.end(); ++fit) {
                if(path.front() == fit->back() && fit->front().y > fit->at(1).y) {
                    *fit += path | skipFront;
                    std::swap(*fit, path);
                    fList.erase(fit);
                    break;
                }
            }
        }
        merged.shrink_to_fit();
        return merged;
    };

    for(Paths src: groupedPss) {
        {
            src = InflateRoundPolygon(src, -dOffset * 2);
            for(auto& path: src)
                path.push_back(path.front());
            if(prPass)
                profilePaths += src;
        }

        QElapsedTimer t;
        t.start();
        if(src.size()) {
            {
                for(auto& path: src)
                    RotatePath(path, angle);
                if(auto zigzag{calcZigzag(src)}; zigzag) { // NOTE C++23 -> and_then
                    auto scanLines{calcScanLines(src, *zigzag)};
                    auto frames{calcFrames(src, *zigzag)};
                    if(scanLines.size() && frames.size()) {
                        auto merged{merge(scanLines, frames)};
                        for(auto& path: merged)
                            RotatePath(path, -angle);
                        returnPs += std::move(merged);
                    }
                }
            }
            {
                for(auto& path: src)
                    RotatePath(path, 90);
                if(auto zigzag{calcZigzag(src)}; zigzag) { // NOTE C++23 -> and_then
                    auto scanLines{calcScanLines(src, *zigzag)};
                    auto frames{calcFrames(src, *zigzag)};
                    if(scanLines.size() && frames.size()) {
                        auto merged{merge(scanLines, frames)};
                        for(auto& path: merged)
                            RotatePath(path, -(angle + 90));
                        returnPs += std::move(merged);
                    }
                }
            }
        }
    }

    mergePaths(returnPs);
    sortB(returnPs, ~(App::home().pos() + App::zero().pos()));

    if(!profilePaths.empty() && prPass) {
        sortB(profilePaths, ~(App::home().pos() + App::zero().pos()));
        if(gcp_.convent())
            ReversePaths(profilePaths);
        for(Path& path: profilePaths)
            path.push_back(path.front());
    }

    returnPss.clear();
    switch(prPass) {
    case NoProfilePass:
        if(!returnPs.empty()) returnPss.push_back(returnPs);
        break;
    case First:
        if(!profilePaths.empty()) returnPss.push_back(profilePaths);
        if(!returnPs.empty()) returnPss.push_back(returnPs);
        break;
    case Last:
        if(!returnPs.empty()) returnPss.push_back(returnPs);
        if(!profilePaths.empty()) returnPss.push_back(profilePaths);
        break;
    default:
        break;
    }

    if(returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        file_ = new File{std::move(gcp_), std::move(returnPss), {}};
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

/////////////////////////////////////////
File::File()
    : GCode::File() { }

File::File(GCode::Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths)
    : GCode::File(std::move(gcp), std::move(toolPathss), std::move(pocketPaths)) {
    if(gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project().worckRect();
    for(size_t x = 0; x < App::project().stepsX(); ++x) {
        for(size_t y = 0; y < App::project().stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);

            if(toolType() == Tool::Laser)
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);

            if(gcp_.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    createGiRaster();
    itemGroup()->setVisible(true);
}

} // namespace CrossHatch
