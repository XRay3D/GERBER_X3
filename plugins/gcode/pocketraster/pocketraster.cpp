// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketraster.h"

#include "gc_file.h"

#include <QElapsedTimer>
#ifndef __GNUC__
    #include <execution>
#endif
#include "gi_point.h"

namespace GCode {
RasterCreator::RasterCreator() {
}

void RasterCreator::create() {
    if (gcp_.params[GCodeParams::Fast].toBool())
        createRaster2(gcp_.tools.front(), gcp_.params[GCodeParams::Depth].toDouble(), gcp_.params[GCodeParams::UseAngle].toDouble(), gcp_.params[GCodeParams::Pass].toInt());
    else
        createRaster(gcp_.tools.front(), gcp_.params[GCodeParams::Depth].toDouble(), gcp_.params[GCodeParams::UseAngle].toDouble(), gcp_.params[GCodeParams::Pass].toInt());
}

void RasterCreator::createRaster(const Tool& tool, const double depth, const double angle, const int prPass) {
    switch (gcp_.side()) {
    case Outer:
        groupedPaths(Grouping::Cutoff, static_cast<Point::Type>(toolDiameter + 5));
        break;
    case Inner:
        groupedPaths(Grouping::Copper);
        break;
    case On:
        emit fileReady(nullptr);
        return;
    }

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;

    Paths profilePaths;
    Paths fillPaths;

    auto calcScanLines = [](const Paths& src, const Path& frame) {
        Paths scanLines;

        Clipper clipper;
        clipper.AddClip(src);
        clipper.AddOpenSubject({frame});
        clipper.Execute(ClipType::Intersection, FillRule::Positive, scanLines); // FIXME  FillRule::Positive
        if (!scanLines.size())
            return scanLines;
        std::sort(scanLines.begin(), scanLines.end(), [](const Path& l, const Path& r) { return l.front().y < r.front().y; }); // vertical sort
        Point::Type start = scanLines.front().front().y;
        bool fl = {};
        for (size_t i {}, last {}; i < scanLines.size(); ++i) {
            if (auto y = scanLines[i].front().y; y != start || i - 1 == scanLines.size()) {
                std::sort(scanLines.begin() + last, scanLines.begin() + i, [&fl](const Path& l, const Path& r) { // horizontal sort
                    return fl ? l.front().x < r.front().x : l.front().x > r.front().x;
                });
                for (size_t k = last; k < i; ++k) { // fix direction
                    if (fl ^ (scanLines[k].front().x < scanLines[k].back().x))
                        std::swap(scanLines[k].front().x, scanLines[k].back().x);
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
            clipper.AddOpenSubject(src);
            clipper.AddClip({frame});
            clipper.Execute(ClipType::Intersection, FillRule::Positive, tmp, tmp); // FIXME  FillRule::Positive
            // dbgPaths(tmp, "ClipType::Intersection");
            frames.append(tmp);
            clipper.Execute(ClipType::Difference, FillRule::Positive, tmp, tmp); // FIXME  FillRule::Positive
            // dbgPaths(tmp, "ClipType::Difference");
            frames.append(tmp);
            std::sort(frames.begin(), frames.end(), [](const Path& l, const Path& r) { return l.front().y < r.front().y; }); // vertical sort
            for (auto& path : frames) {
                if (path.front().y > path.back().y)
                    ReversePath(path); // fix vertical direction
            }
        }
        return frames;
    };
    auto calcZigzag = [this](const Paths& src) {
        Clipper clipper;
        clipper.AddClip(src);
        Rect rect(Bounds(src));
        Point::Type o = uScale - (rect.Height() % stepOver) / 2;
        rect.top -= o;
        rect.bottom += o;
        rect.left -= uScale;
        rect.right += uScale;
        Path zigzag;
        Point::Type start = rect.top;
        bool fl {};

        for (; start <= rect.bottom || fl; fl = !fl, start += stepOver) {
            if (!fl) {
                zigzag.emplace_back(rect.left, start);
                zigzag.emplace_back(rect.right, start);
            } else {
                zigzag.emplace_back(rect.right, start);
                zigzag.emplace_back(rect.left, start);
            }
        }

        zigzag.front().x -= stepOver;
        zigzag.back().x -= stepOver;
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
                        if (path.back() == fit->front() && fit->front().y < fit->at(1).y) {
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
                if (path.front() == fit->back() && fit->front().y > fit->at(1).y) {
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

    for (Paths src : groupedPss) {
        ClipperOffset offset(uScale);
        offset.AddPaths(src, JoinType::Round, EndType::Polygon);
        src = offset.Execute(-dOffset);
        for (auto& path : src)
            path.push_back(path.front());
        if (prPass)
            profilePaths.append(src);

        if (src.size()) {
            for (auto& path : src)
                RotatePath(path, angle);
            auto zigzag {calcZigzag(src)};
            auto scanLines {calcScanLines(src, zigzag)};
            auto frames {calcFrames(src, zigzag)};
            if (scanLines.size() && frames.size()) {
                auto merged {merge(scanLines, frames)};
                for (auto& path : merged)
                    RotatePath(path, -angle);
                returnPs.append(merged);
            }
        }
    }

    mergeSegments(returnPs);

    sortB(returnPs);
    if (!profilePaths.empty() && prPass) {
        sortB(profilePaths);
        if (gcp_.convent())
            ReversePaths(profilePaths);
        for (Path& path : profilePaths)
            path.push_back(path.front());
    }

    switch (prPass) {
    case NoProfilePass:
        returnPss.prepend(returnPs);
        break;
    case First:
        if (!profilePaths.empty())
            returnPss.prepend(profilePaths);
        returnPss.prepend(returnPs);
        break;
    case Last:
        returnPss.prepend(returnPs);
        if (!profilePaths.empty())
            returnPss.push_back(profilePaths);
        break;
    default:
        break;
    }

    if (returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        gcp_.gcType = Raster;
        file_ = new File(returnPss, std::move(gcp_), fillPaths);
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

void RasterCreator::createRaster2(const Tool& tool, const double depth, const double angle, const int prPass) {

    QElapsedTimer t;
    t.start();

    switch (gcp_.side()) {
    case Outer:
        groupedPaths(Grouping::Cutoff, uScale);
        break;
    case Inner:
        groupedPaths(Grouping::Copper);
        break;
    case On:
        emit fileReady(nullptr);
        return;
    }

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = static_cast<Point::Type>(tool.stepover() * uScale);

    Paths profilePaths;

    { // create exposure frames
        ClipperOffset o;
        for (auto& p : groupedPss)
            o.AddPaths(p, JoinType::Round, EndType::Polygon);
        profilePaths = o.Execute(-tool.diameter() * uScale);
    }

    // get bounds of frames
    rect = Bounds(profilePaths);

    const Point center {rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2};

    Paths laserPath(profilePaths);

    if (!qFuzzyIsNull(angle)) { // Rotate Paths
        for (Path& path : laserPath)
            RotatePath(path, angle, center);
        // get bounds of frames if angle > 0.0
        rect = Bounds(laserPath);
    }

    rect.left -= uScale;
    rect.right += uScale;

    Path zPath;
    { // create "snake"
        Point::Type y = rect.top;
        while (y < rect.bottom) {
            zPath.append({
                { rect.left, y},
                {rect.right, y}
            });
            y += stepOver;
            zPath.append({
                {rect.right, y},
                { rect.left, y}
            });
            y += stepOver;
        }
    }

    // PROG //PROG .3setProgMaxAndVal(0, 0);

    { //  calculate
        Clipper c;
        c.AddOpenSubject({zPath});
        c.AddClip(laserPath);
        c.Execute(ClipType::Intersection, FillRule::NonZero, laserPath, laserPath);   // laser on
        addAcc(laserPath, gcp_.params[GCodeParams::AccDistance].toDouble() * uScale); // add laser off paths
    }

    if (!qFuzzyIsNull(angle)) { // Rotate Paths
        for (Path& path : laserPath)
            RotatePath(path, -angle, center);
    }

    returnPss.push_back(laserPath);

    if (!profilePaths.empty() && prPass != NoProfilePass) {
        for (auto& p : profilePaths)
            p.push_back(p.front());
        returnPss.push_back(sortB(profilePaths));
    }

    if (returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        gcp_.gcType = LaserHLDI;
        file_ = new File(returnPss, std::move(gcp_));
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

void RasterCreator::addAcc(Paths& src, const Point::Type accDistance) {

    Paths pPath;
    pPath.reserve(src.size() * 2 + 1);
#ifndef __GNUC__
    std::sort(std::execution::par, src.begin(), src.end(), [](const Path& p1, const Path& p2) -> bool { return p1.front().y > p2.front().y; });
#else
    std::sort(src.begin(), src.end(), [](const Path& p1, const Path& p2) -> bool { return p1.front().y > p2.front().y; });
#endif
    bool reverse {};

    auto format = [&reverse](Path& src) -> Path& {
        if (reverse)
            std::sort(src.begin(), src.end(), [](const Point& p1, const Point& p2) -> bool { return p1.x > p2.x; });
        else
            std::sort(src.begin(), src.end(), [](const Point& p1, const Point& p2) -> bool { return p1.x < p2.x; });
        return src;
    };

    auto adder = [&reverse, &pPath, accDistance](Paths& paths) {
        std::sort(paths.begin(), paths.end(), [reverse](const Path& p1, const Path& p2) -> bool {
            if (reverse)
                return p1.front().x > p2.front().x;
            else
                return p1.front().x < p2.front().x;
        });
        if (pPath.size()) { // acc
            Path acc;
            {
                const Path& path = pPath.back();
                if (path.front().x < path.back().x) { // acc
                    acc.append(Path {
                        path.back(), {path.back().x + accDistance, path.front().y}
                    });
                } else {
                    acc.append(Path {
                        path.back(), {path.back().x - accDistance, path.front().y}
                    });
                }
            }
            {
                const Path& path = paths.front();
                if (path.front().x > path.back().x) { // acc
                    acc.append(Path {
                        {path.front().x + accDistance, path.front().y},
                        path.front()
                    });
                } else {
                    acc.append(Path {
                        {path.front().x - accDistance, path.front().y},
                        path.front()
                    });
                }
            }
            pPath.push_back(acc);
        } else { // acc first
            pPath.emplace_back(Path {
                {paths.front().front().x - accDistance, paths.front().front().y},
                paths.front().front()
            });
        }
        for (size_t j = 0; j < paths.size(); ++j) {
            if (j) // acc
                pPath.emplace_back(Path {paths[j - 1].back(), paths[j].front()});
            pPath.push_back(paths[j]);
        }
    };

    { //  calculate
        Point::Type yLast = src.front().front().y;
        Paths paths;

        for (size_t i = 0; i < src.size(); ++i) {
            // PROG //PROG .3setProgMaxAndVal(src.size(), i);
            if (yLast != src[i].front().y) {
                adder(paths);
                reverse = !reverse;
                yLast = src[i].front().y;
                paths = {format(src[i])};
            } else {
                paths.push_back(format(src[i]));
            }
        }

        adder(paths);
    }

    { // acc last
        Path& path = pPath.back();
        if (path.front().x < path.back().x) {
            pPath.emplace_back(Path {
                path.back(), {path.back().x + accDistance, path.front().y}
            });
        } else {
            pPath.emplace_back(Path {
                path.back(), {path.back().x - accDistance, path.front().y}
            });
        }
    }

    src = std::move(pPath);
}

} // namespace GCode
