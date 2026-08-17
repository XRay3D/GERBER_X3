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
#include "geo/fill.h"
#include "geo/util.h"

#include <algorithm>

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

        returnPs.append_range(Geo::zigzagFill(pocket, stepOver, angle));
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

//////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        regenerate();
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
