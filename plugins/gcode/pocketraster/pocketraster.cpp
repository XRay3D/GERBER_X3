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
#include "gc_pathutils.h"
#include "gi_point.h"
#include "project.h"

#include "geo/boolean.h"
#include "geo/cancel.h"
#include "geo/util.h"

#include <algorithm>
#include <list>

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
    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover();

    switch(gcp.side()) {
    case GCode::Outer: groupedPaths(GCode::Grouping::Cutoff, toolDiameter + 1.0); break;
    case GCode::Inner: groupedPaths(GCode::Grouping::Copper); break;
    case GCode::On   : return;
    }

    Geo::Polylines profilePaths;

    for(const Geo::Polygon& group: groupedPss) {
        // Карман -- группа, сжатая на радиус (delta у Inflate -- полная ширина).
        const Geo::Polygons pocket = Geo::Inflate(Geo::Polygons{group}, -toolDiameter);
        if(pocket.empty()) continue;

        if(prPass) profilePaths.append_range(pocket.contours());

        // Карман поворачивается ПОД горизонтальную змейку, результат -- обратно.
        Geo::Polylines contours = pocket.contours();
        Geo::rotate(contours, angle);
        const Geo::Polygons region{contours};

        auto zigzag = calcZigzag(region.boundingRect());
        if(!zigzag) continue;

        Geo::Polyline combContour{*zigzag};
        combContour.closed = true;
        if(combContour.signedArea() < 0) combContour.reverse();
        const Geo::Polygons comb{Geo::Polylines{std::move(combContour)}};

        auto scanLines = calcScanLines(region, *zigzag);
        auto frames = calcFrames(contours, comb);
        if(scanLines.size() && frames.size()) {
            auto merged = merge(scanLines, frames);
            Geo::rotate(merged, -angle);
            returnPs.append_range(std::move(merged));
        }
    }

    GCode::mergePolylines(returnPs, Geo::exitWeldTolerance);

    GCode::sortByProximity(returnPs, App::home().pos() + App::zero().pos());

    if(!profilePaths.empty() && prPass) {
        GCode::sortByProximity(profilePaths, App::home().pos() + App::zero().pos());
        if(gcp.convent())
            r::for_each(profilePaths, &Geo::Polyline::reverse);
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
    std::erase_if(returnPss, [](auto&& paths) { return paths.empty(); });

    if(returnPss.size()) {
        gcp.toolPathss = std::move(returnPss);
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

void Creator::createRasterAccLaser(const Tool& tool, const double depth, const double angle, const int prPass) {
    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover();

    switch(gcp.side()) {
    case GCode::Outer: groupedPaths(GCode::Grouping::Cutoff, toolDiameter + 1.0); break;
    case GCode::Inner: groupedPaths(GCode::Grouping::Copper); break;
    case GCode::On   : return;
    }

    // Кадры экспозиции: все группы разом, сжатые на пол-луча (луч тоньше
    // фрезы, delta у Inflate -- полная ширина, граница уезжает на половину).
    const Geo::Polygons frames = Geo::Inflate(
        Geo::Polygons{std::span<const Geo::Polygon>{groupedPss}}, -dOffset);
    Geo::Polylines profilePaths = frames.contours();
    if(profilePaths.empty()) return;

    Geo::Polylines laserPath{profilePaths};

    QRectF rect = frames.boundingRect();
    const QPointF center = rect.center();

    if(!qFuzzyIsNull(angle)) { // поворот ПОД горизонтальную змейку
        Geo::rotate(laserPath, angle, center);
        rect = laserPath.front().boundingRect();
        for(const Geo::Polyline& path: laserPath)
            rect |= path.boundingRect();
    }

    rect.adjust(-1.0, 0.0, +1.0, 0.0);

    Geo::Polyline zPath;
    { // create "snake"
        auto y = rect.top();
        while(y < rect.bottom()) {
            zPath.emplace_back(rect.left(), y);
            zPath.emplace_back(rect.right(), y);
            y += stepOver;
            zPath.emplace_back(rect.right(), y);
            zPath.emplace_back(rect.left(), y);
            y += stepOver;
        }
    }

    { // calculate
        const Geo::Polygons region{laserPath};
        laserPath = Geo::clipOpen(Geo::ClipType_::Intersection, {zPath}, region); // laser on
        if(laserPath.empty()) return;
        addAcc(laserPath, gcp.params[AccDistance].toDouble()); // add laser off paths
    }

    if(!qFuzzyIsNull(angle)) // поворот обратно
        Geo::rotate(laserPath, -angle, center);

    returnPss.push_back(std::move(laserPath));

    if(!profilePaths.empty() && prPass != NoProfilePass) {
        GCode::sortByProximity(profilePaths, App::home().pos() + App::zero().pos());
        returnPss.push_back(std::move(profilePaths));
    }

    if(returnPss.size()) {
        std::erase_if(returnPss, [](auto& paths) { return paths.empty(); });
        for(auto& paths: returnPss)
            std::erase_if(paths, [](auto& path) { return path.empty(); });
        gcp.toolPathss = std::move(returnPss);
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

void Creator::addAcc(Geo::Polylines& src, const double accDistance) {
    Geo::Polylines pPath;
    pPath.reserve(src.size() * 2 + 1);
    std::sort(src.begin(), src.end(),
        [](const Geo::Polyline& p1, const Geo::Polyline& p2) { return p1.front().y() > p2.front().y(); });
    bool reverse{};

    auto format = [&reverse](Geo::Polyline& src) -> Geo::Polyline& {
        if(reverse)
            std::sort(src.begin(), src.end(), [](const QPointF& p1, const QPointF& p2) { return p1.x() > p2.x(); });
        else
            std::sort(src.begin(), src.end(), [](const QPointF& p1, const QPointF& p2) { return p1.x() < p2.x(); });
        return src;
    };

    auto adder = [&reverse, &pPath, accDistance](Geo::Polylines& paths) {
        std::sort(paths.begin(), paths.end(), [reverse](const Geo::Polyline& p1, const Geo::Polyline& p2) {
            if(reverse)
                return p1.front().x() > p2.front().x();
            else
                return p1.front().x() < p2.front().x();
        });
        if(pPath.size()) { // acc
            Geo::Polyline acc;
            {
                const Geo::Polyline& path = pPath.back();
                if(path.front().x() < path.back().x()) // acc
                    acc.append_range(Geo::Polyline{
                        path.back(), {path.back().x() + accDistance, path.front().y()}
                    });
                else
                    acc.append_range(Geo::Polyline{
                        path.back(), {path.back().x() - accDistance, path.front().y()}
                    });
            }
            {
                const Geo::Polyline& path = paths.front();
                if(path.front().x() > path.back().x()) // acc
                    acc.append_range(Geo::Polyline{
                        {path.front().x() + accDistance, path.front().y()},
                        path.front()
                    });
                else
                    acc.append_range(Geo::Polyline{
                        {path.front().x() - accDistance, path.front().y()},
                        path.front()
                    });
            }
            pPath.push_back(acc);
        } else { // acc first
            pPath.emplace_back(Geo::Polyline{
                {paths.front().front().x() - accDistance, paths.front().front().y()},
                paths.front().front()
            });
        }
        for(size_t j{}; j < paths.size(); ++j) {
            if(j) // acc
                pPath.emplace_back(Geo::Polyline{paths[j - 1].back(), paths[j].front()});
            pPath.push_back(paths[j]);
        }
    };

    { // calculate
        double yLast = src.front().front().y();
        Geo::Polylines paths;

        for(size_t i{}; i < src.size(); ++i) {
            if(yLast != src[i].front().y()) {
                adder(paths);
                reverse = !reverse;
                yLast = src[i].front().y();
                paths = Geo::Polylines{format(src[i])};
            } else {
                paths.push_back(format(src[i]));
            }
        }

        adder(paths);
    }

    { // acc last
        Geo::Polyline& path = pPath.back();
        if(path.front().x() < path.back().x())
            pPath.emplace_back(Geo::Polyline{
                path.back(), {path.back().x() + accDistance, path.front().y()}
            });
        else
            pPath.emplace_back(Geo::Polyline{
                path.back(), {path.back().x() - accDistance, path.front().y()}
            });
    }

    src = std::move(pPath);
}

// Строки выборки: змейка ∩ карман. Куски одной строки наследуют y своего
// сегмента змейки ТОЧНО (интерполяция горизонтали не трогает y), поэтому
// группировка строк по равенству y законна и в double.
Geo::Polylines Creator::calcScanLines(const Geo::Polygons& pocket, const Geo::Polyline& zigzag) {
    Geo::Polylines scanLines = Geo::clipOpen(Geo::ClipType_::Intersection, {zigzag}, pocket);
    if(scanLines.empty()) return scanLines;

    std::sort(scanLines.begin(), scanLines.end(),
        [](const Geo::Polyline& l, const Geo::Polyline& r) { return l.front().y() < r.front().y(); }); // vertical sort
    double start = scanLines.front().front().y();
    bool fl = {};
    for(size_t i{}, last{}; i < scanLines.size(); ++i) {
        if(auto y = scanLines[i].front().y(); y != start || i - 1 == scanLines.size()) {
            std::sort(scanLines.begin() + last, scanLines.begin() + i, [&fl](const Geo::Polyline& l, const Geo::Polyline& r) { // horizontal sort
                return fl ? l.front().x() < r.front().x() : l.front().x() > r.front().x();
            });
            for(size_t k = last; k < i; ++k) // fix direction
                if(fl ^ (scanLines[k].front().x() < scanLines[k].back().x()))
                    scanLines[k].reverse();
            start = y;
            fl = !fl;
            last = i;
        }
    }
    return scanLines;
}

// Куски границы кармана между соседними строками: контуры, разрезанные
// гребёнкой с обеих сторон -- то есть на каждом пересечении со строкой.
Geo::Polylines Creator::calcFrames(const Geo::Polylines& contours, const Geo::Polygons& comb) {
    Geo::Polylines frames = Geo::clipOpen(Geo::ClipType_::Intersection, contours, comb);
    frames.append_range(Geo::clipOpen(Geo::ClipType_::Difference, contours, comb));

    std::sort(frames.begin(), frames.end(),
        [](const Geo::Polyline& l, const Geo::Polyline& r) { return l.front().y() < r.front().y(); }); // vertical sort
    for(auto& path: frames)
        if(path.front().y() > path.back().y())
            path.reverse(); // fix vertical direction

    return frames;
}

// Змейка по габариту: строки через stepOver, соединители за краем (запас
// 1 мм), чтобы в карман попадали только горизонтальные куски.
std::optional<Geo::Polyline> Creator::calcZigzag(QRectF rect) {
    const double o = 1.0 - std::fmod(rect.height(), stepOver) * 0.5; // центровка строк
    rect.adjust(-1.0, -o, +1.0, +o);
    Geo::Polyline zigzag;
    bool fl{};
    for(double y = rect.top(); y <= rect.bottom() || fl; fl = !fl, y += stepOver) {
        if(!fl) {
            zigzag.emplace_back(rect.left(), y);
            zigzag.emplace_back(rect.right(), y);
        } else {
            zigzag.emplace_back(rect.right(), y);
            zigzag.emplace_back(rect.left(), y);
        }
    }
    if(zigzag.empty()) return std::nullopt;
    // Замыкающее ребро гребёнки должно пройти ещё левее соединителей.
    zigzag.front().rx() -= stepOver;
    zigzag.back().rx() -= stepOver;
    return zigzag;
}

namespace {
// Точки совпадают в пределах сварочного допуска: точки разреза считаются в
// double, и куски строки и границы сходятся не бит-в-бит, как это было на
// целочисленной сетке клиппера, а с погрешностью ~1e-12.
bool joined(const QPointF& a, const QPointF& b) {
    return Geo::distance(a, b) <= Geo::exitWeldTolerance;
}
} // namespace

Geo::Polylines Creator::merge(const Geo::Polylines& scanLines, const Geo::Polylines& frames) {
    Geo::Polylines merged;
    merged.reserve(scanLines.size() / 10);
    std::list<Geo::Polyline> bList;
    for(auto&& path: scanLines)
        bList.emplace_back(std::move(path));

    std::list<Geo::Polyline> fList;
    for(auto&& path: frames)
        fList.emplace_back(std::move(path));

    Progress::setMax(bList.size());
    while(bList.begin() != bList.end()) {
        Progress::setCurrent(bList.size());

        merged.resize(merged.size() + 1);
        auto& path = merged.back();
        for(auto bit = bList.begin(); bit != bList.end(); ++bit) {
            Geo::checkCancelled();
            if(path.empty() || joined(path.back(), bit->front())) {
                path.empty() ? path.append_range(*bit)
                             : path.append_range(*bit | v::drop(1));
                bList.erase(bit);
                for(auto fit = fList.begin(); fit != fList.end(); ++fit) {
                    if(joined(path.back(), fit->front()) && fit->front().y() < fit->at(1).y()) {
                        path.append_range(*fit | v::drop(1));
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
            if(joined(path.front(), fit->back()) && fit->front().y() > fit->at(1).y()) {
                fit->append_range(path | v::drop(1));
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
