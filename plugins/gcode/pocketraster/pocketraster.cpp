/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketraster.h"
#include "gi_point.h"
#include "project.h"
#include <QElapsedTimer>
#include <gi_dbg.h>

#ifdef Q_OS_UNIX
    #undef emit
    #include <execution>
    #define emit
#endif

namespace PocketRaster {

void Creator::create() {
    if(gcp.params[Fast].toBool())
        createRasterAccLaser(gcp.tools.front(),
            gcp.params[GCode::Params::Depth].toDouble(),
            gcp.params[UseAngle].toDouble(),
            gcp.params[Pass].toInt());
    else
        createRaster(gcp.tools.front(),
            gcp.params[GCode::Params::Depth].toDouble(),
            gcp.params[UseAngle].toDouble(),
            gcp.params[Pass].toInt());
}

uint32_t Creator::type() { return POCKET_RASTER; }

void Creator::createRaster(const Tool& tool, const double depth, const double angle, const int prPass) {
    switch(gcp.side()) {
    case GCode::Outer: groupedPaths(GCode::Grouping::Cutoff, toolDiameter + uScale); break;
    case GCode::Inner: groupedPaths(GCode::Grouping::Copper); break;
    case GCode::On   : return;
    }

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;

    Paths64 profilePaths;
    Paths64 fillPaths;

    for(Paths64& src: groupedPss) {
        src = Inflate64(src, -dOffset * 2, cl::JoinType::Round, cl::EndType::Polygon, uScale);

        for(auto& path: src) path.push_back(path.front());

        if(prPass) profilePaths.append_range(src);

        if(src.size()) {
            for(auto& path: src) RotatePath(path, angle);
            auto zigzag{calcZigzag(src)};
            auto scanLines{calcScanLines(src, zigzag)};
            auto frames{calcFrames(src, zigzag)};
            if(scanLines.size() && frames.size()) {
                auto merged{merge(scanLines, frames)};
                for(auto& path: merged) RotatePath(path, -angle);
                returnPs.append_range(std::move(merged));
            }
        }
    }

    mergePaths(returnPs);

    sortB(returnPs, ~(App::home().pos() + App::zero().pos()));

    if(!profilePaths.empty() && prPass) {
        sortB(profilePaths, ~(App::home().pos() + App::zero().pos()));
        if(gcp.convent())
            ReversePaths(profilePaths);
        for(Path64& path: profilePaths)
            path.push_back(path.front());
    }

    switch(prPass) {
    case NoProfilePass: returnPss.insert(returnPss.begin(), returnPs); break;
    case First:
        if(!profilePaths.empty())
            returnPss.insert(returnPss.begin(), profilePaths);
        returnPss.insert(returnPss.begin(), returnPs);
        break;
    case Last:
        returnPss.insert(returnPss.begin(), returnPs);
        if(!profilePaths.empty())
            returnPss.push_back(profilePaths);
        break;
    default: break;
    }

    for(auto&& paths: returnPss)
        std::erase_if(paths, [](auto&& path) { return path.size() < 2; });

    constexpr auto empty = std::bind(&Paths64::empty, _1);
    std::erase_if(returnPss, empty);
    // Gi::Debug(returnPss | v::join | r::to<Paths64>(), Qt::magenta);
    if(returnPss.size()) {
        gcp.toolPathss = toCurvess(returnPss);
        gcp.setPocketAreaCurves(toCurves(fillPaths));
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

void Creator::createRasterAccLaser(const Tool& tool, const double depth, const double angle, const int prPass) {

    QElapsedTimer t;
    t.start();

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = static_cast</*PType*/ int32_t>(tool.stepover() * uScale);

    switch(gcp.side()) {
    case GCode::Outer: groupedPaths(GCode::Grouping::Cutoff, toolDiameter + uScale); break;
    case GCode::Inner: groupedPaths(GCode::Grouping::Copper); break;
    case GCode::On   : return;
    }

    Paths64 profilePaths;

    { // create exposure frames
      // ClipperOffset o;
      // for(auto& p: groupedPss)
      // o.AddPaths(p, cl::JoinType::Round, cl::EndType::Polygon);
      // profilePaths = o.Execute(-tool.diameter() * uScale);
        // auto it = v::join(groupedPss);
        profilePaths = Inflate64(join(groupedPss), -tool.diameter() * uScale, cl::JoinType::Round, cl::EndType::Polygon);
    }
    auto pss = v::join(groupedPss);
    profilePaths = Inflate64(Paths64{pss.begin(), pss.end()}, -dOffset, cl::JoinType::Round, cl::EndType::Polygon, uScale);

    // get bounds of frames
    rect = GetBounds(profilePaths);

    const Point64 center{rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2};

    Paths64 laserPath(profilePaths);

    if(!qFuzzyIsNull(angle)) { // Rotate Paths64
        for(Path64& path: laserPath)
            RotatePath(path, angle, center);
        // get bounds of frames if angle > 0.0
        rect = GetBounds(laserPath);
    }

    rect.left -= uScale;
    rect.right += uScale;

    Path64 zPath;
    { // create u"snake"_s
        auto y = rect.top;
        while(y < rect.bottom) {
            zPath.append_range(Path64{
                Point64{rect.left,  y},
                Point64{rect.right, y},
            });
            y += stepOver;
            zPath.append_range(Path64{
                Point64{rect.right, y},
                Point64{rect.left,  y},
            });
            y += stepOver;
        }
    }

    { // calculate
        cl::Clipper64 c;
        c.AddOpenSubject({zPath});
        c.AddClip(laserPath);
        c.Execute(cl::ClipType::Intersection, cl::FillRule::NonZero, laserPath, laserPath); // laser on
        addAcc(laserPath, gcp.params[AccDistance].toDouble() * uScale);             // add laser off paths
    }

    if(!qFuzzyIsNull(angle)) // Rotate Paths64
        for(Path64& path: laserPath)
            RotatePath(path, -angle, center);

    returnPss.push_back(laserPath);

    if(!profilePaths.empty() && prPass != NoProfilePass) {
        for(auto& p: profilePaths)
            p.push_back(p.front());
        returnPss.push_back(sortB(profilePaths, ~(App::home().pos() + App::zero().pos())));
    }

    if(returnPss.size()) {
        std::erase_if(returnPss, [](auto& paths) { return paths.empty(); });
        for(auto& paths: returnPss)
            std::erase_if(paths, [](auto& path) { return path.empty(); });
        gcp.toolPathss = toCurvess(returnPss);
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

void Creator::addAcc(Paths64& src, const /*PType*/ int32_t accDistance) {

    Paths64 pPath;
    pPath.reserve(src.size() * 2 + 1);
    std::sort(
#ifdef Q_OS_UNIX
        std::execution::par,
#endif
        src.begin(), src.end(), [](const Path64& p1, const Path64& p2) -> bool { return p1.front().y > p2.front().y; });
    bool reverse{};

    auto format = [&reverse](Path64& src) -> Path64& {
        if(reverse)
            std::sort(src.begin(), src.end(), [](const Point64& p1, const Point64& p2) -> bool { return p1.x > p2.x; });
        else
            std::sort(src.begin(), src.end(), [](const Point64& p1, const Point64& p2) -> bool { return p1.x < p2.x; });
        return src;
    };

    auto adder = [&reverse, &pPath, accDistance](Paths64& paths) {
        std::sort(paths.begin(), paths.end(), [reverse](const Path64& p1, const Path64& p2) -> bool {
            if(reverse)
                return p1.front().x > p2.front().x;
            else
                return p1.front().x < p2.front().x;
        });
        if(pPath.size()) { // acc
            Path64 acc;
            {
                const Path64& path = pPath.back();
                if(path.front().x < path.back().x) // acc
                    acc.append_range(Path64{
                        path.back(), {path.back().x + accDistance, path.front().y}
                    });
                else
                    acc.append_range(Path64{
                        path.back(), {path.back().x - accDistance, path.front().y}
                    });
            }
            {
                const Path64& path = paths.front();
                if(path.front().x > path.back().x) // acc
                    acc.append_range(Path64{
                        {path.front().x + accDistance, path.front().y},
                        path.front()
                    });
                else
                    acc.append_range(Path64{
                        {path.front().x - accDistance, path.front().y},
                        path.front()
                    });
            }
            pPath.push_back(acc);
        } else { // acc first
            pPath.emplace_back(Path64{
                {paths.front().front().x - accDistance, paths.front().front().y},
                paths.front().front()
            });
        }
        for(size_t j{}; j < paths.size(); ++j) {
            if(j) // acc
                pPath.emplace_back(Path64{paths[j - 1].back(), paths[j].front()});
            pPath.push_back(paths[j]);
        }
    };

    { // calculate
        /*PType*/ int32_t yLast = src.front().front().y;
        Paths64 paths;

        for(size_t i{}; i < src.size(); ++i) {

            if(yLast != src[i].front().y) {
                adder(paths);
                reverse = !reverse;
                yLast = src[i].front().y;
                paths = Paths64{format(src[i])};
            } else {
                paths.push_back(format(src[i]));
            }
        }

        adder(paths);
    }

    { // acc last
        Path64& path = pPath.back();
        if(path.front().x < path.back().x)
            pPath.emplace_back(Path64{
                path.back(), {path.back().x + accDistance, path.front().y}
            });
        else
            pPath.emplace_back(Path64{
                path.back(), {path.back().x - accDistance, path.front().y}
            });
    }

    src = std::move(pPath);
}

Paths64 Creator::calcScanLines(const Paths64& src, const Path64& frame) {
    Paths64 scanLines;
    cl::Clipper64 clipper;
    // clipper.AddOpenSubject(src);
    // clipper.AddClip({frame});
    clipper.AddClip(src);
    clipper.AddOpenSubject({frame});
    clipper.Execute(cl::ClipType::Intersection, cl::FillRule::Positive, scanLines, scanLines);
    if(!scanLines.size()) return scanLines;
    std::sort(scanLines.begin(), scanLines.end(), [](const Path64& l, const Path64& r) { return l.front().y < r.front().y; }); // vertical sort
    /*PType*/ int32_t start = scanLines.front().front().y;
    bool fl = {};
    for(size_t i{}, last{}; i < scanLines.size(); ++i) {
        if(auto y = scanLines[i].front().y; y != start || i - 1 == scanLines.size()) {
            std::sort(scanLines.begin() + last, scanLines.begin() + i, [&fl](const Path64& l, const Path64& r) { // horizontal sort
                return fl ? l.front().x < r.front().x : l.front().x > r.front().x;
            });
            for(size_t k = last; k < i; ++k) // fix direction
                if(fl ^ (scanLines[k].front().x < scanLines[k].back().x))
                    std::swap(scanLines[k].front().x, scanLines[k].back().x);
            start = y;
            fl = !fl;
            last = i;
        }
    }
    return scanLines;
}

Paths64 Creator::calcFrames(const Paths64& src, const Path64& frame) {
    Paths64 frames;

    Paths64 tmp;
    cl::Clipper64 clipper;
    clipper.AddOpenSubject(src);
    clipper.AddClip({frame});
    clipper.Execute(cl::ClipType::Intersection, cl::FillRule::Positive, tmp, tmp); // FillRule::Positive
    // dbgPaths(tmp, u"ClipType::Intersection"_s);
    frames.append_range(std::move(tmp));
    clipper.Execute(cl::ClipType::Difference, cl::FillRule::Positive, tmp, tmp); // FillRule::Positive
    // dbgPaths(tmp, u"ClipType::Difference"_s);
    frames.append_range(std::move(tmp));
    std::sort(frames.begin(), frames.end(), [](const Path64& l, const Path64& r) { return l.front().y < r.front().y; }); // vertical sort
    for(auto& path: frames)
        if(path.front().y > path.back().y)
            ReversePath(path); // fix vertical direction

    return frames;
}

Path64 Creator::calcZigzag(const Paths64& src) {
    cl::Clipper64 clipper;
    clipper.AddClip(src);
    Rect rect(GetBounds(src));
    /*PType*/ int32_t o = uScale - (rect.Height() % stepOver) / 2;
    rect.top -= o;
    rect.bottom += o;
    rect.left -= uScale;
    rect.right += uScale;
    Path64 zigzag;
    auto start = rect.top;
    bool fl{};

    for(; start <= rect.bottom || fl; fl = !fl, start += stepOver) {
        if(!fl) {
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
}

Paths64 Creator::merge(const Paths64& scanLines, const Paths64& frames) {
    Paths64 merged;
    merged.reserve(scanLines.size() / 10);
    std::list<Path64> bList;
    for(auto&& path: scanLines)
        bList.emplace_back(std::move(path));

    std::list<Path64> fList;
    for(auto&& path: frames)
        fList.emplace_back(std::move(path));

    setMax(bList.size());
    while(bList.begin() != bList.end()) {
        setCurrent(bList.size());

        merged.resize(merged.size() + 1);
        auto& path = merged.back();
        for(auto bit = bList.begin(); bit != bList.end(); ++bit) {
            throwIfCancel();
            if(path.empty() || path.back() == bit->front()) {
                path.empty() ? path.append_range(*bit)
                             : path.append_range(*bit | skipFront);
                bList.erase(bit);
                for(auto fit = fList.begin(); fit != fList.end(); ++fit) {
                    if(path.back() == fit->front() && fit->front().y < fit->at(1).y) {
                        path.append_range(*fit | skipFront);
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
                fit->append_range(path | skipFront);
                std::swap(*fit, path);
                fList.erase(fit);
                break;
            }
        }
    }
    merged.shrink_to_fit();
    return merged;
}
//////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project().worckRect();
    for(size_t x{}; x < App::project().stepsX(); ++x) {
        for(size_t y{}; y < App::project().stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);
            if(toolType == Tool::Laser)
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);
            if(gcp.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    if(toolType == Tool::Laser)
        createGiLaser();
    else
        createGiRaster();
    itemGroup()->setVisible(true);
}

} // namespace PocketRaster
