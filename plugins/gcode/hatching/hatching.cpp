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
#include "hatching.h"
#include "gc_pathutils.h"
#include "gi_point.h"
#include "project.h"

#include "geo/boolean.h"
#include "geo/cancel.h"
#include "geo/util.h"

#include <algorithm>
#include <list>
#include <optional>

namespace CrossHatch {

namespace {

// Штриховка -- та же гребёнка, что и у растровой выборки, только в два
// прохода со сдвигом на 90°. Вся резка -- Geo::clipOpen: змейка (открытый
// путь) режется КАРМАНОМ, а граница кармана -- ГРЕБЁНКОЙ (замкнутая змейка
// как площадь), и куски сшиваются в один боустрофедон.

// Змейка по габариту: строки через step, соединители за левым/правым краем
// (запас 1 мм), чтобы в карман попадали только горизонтальные куски.
std::optional<Geo::Polyline> calcZigzag(QRectF rect, double step) {
    const double o = 1.0 - std::fmod(rect.height(), step) * 0.5; // центровка строк
    rect.adjust(-1.0, -o, +1.0, +o);
    Geo::Polyline zigzag;
    bool fl{};
    for(double y = rect.top(); y <= rect.bottom() || fl; fl = !fl, y += step) {
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
    zigzag.front().rx() -= step;
    zigzag.back().rx() -= step;
    return zigzag;
}

// Строки штриховки: змейка ∩ карман. Куски одной строки наследуют y своего
// сегмента змейки ТОЧНО (интерполяция горизонтали не трогает y), поэтому
// группировка строк по равенству y законна и в double.
Geo::Polylines calcScanLines(const Geo::Polygons& pocket, const Geo::Polyline& zigzag) {
    Geo::Polylines sl = Geo::clipOpen(Geo::ClipType_::Intersection, {zigzag}, pocket);
    if(sl.empty()) return sl;

    r::sort(sl, {}, [](const Geo::Polyline& p) { return p.front().y(); }); // vertical sort

    double start = sl.front().front().y();
    bool fl = {};
    for(size_t i{}, last{}; i < sl.size(); ++i) {
        if(auto y = sl[i].front().y(); y != start || i - 1 == sl.size()) {

            fl ? r::sort(sl.begin() + last, sl.begin() + i, {}, [](const Geo::Polyline& p) { return p.front().x(); }) :           // horizontal sort
                r::sort(sl.begin() + last, sl.begin() + i, std::greater(), [](const Geo::Polyline& p) { return p.front().x(); }); // horizontal sort

            for(size_t k = last; k < i; ++k) // fix direction
                if(fl ^ (sl[k].front().x() < sl[k].back().x()))
                    sl[k].reverse();

            start = y;
            fl = !fl;
            last = i;
        }
    }
    return sl;
}

// Куски границы кармана между соседними строками: контуры, разрезанные
// гребёнкой с ОБЕИХ сторон (внутри неё и вне) -- то есть на каждом
// пересечении со строкой.
Geo::Polylines calcFrames(const Geo::Polylines& contours, const Geo::Polygons& comb) {
    Geo::Polylines frames = Geo::clipOpen(Geo::ClipType_::Intersection, contours, comb);
    frames.append_range(Geo::clipOpen(Geo::ClipType_::Difference, contours, comb));

    r::sort(frames, {}, [](const Geo::Polyline& p) { return p.front().y(); }); // vertical sort
    for(auto& path: frames)
        if(path.front().y() > path.back().y())
            path.reverse(); // fix vertical direction
    return frames;
}

// Точки совпадают в пределах сварочного допуска: точки разреза считаются в
// double, и куски строки и границы сходятся не бит-в-бит, как это было на
// целочисленной сетке клиппера, а с погрешностью ~1e-12.
bool joined(const QPointF& a, const QPointF& b) {
    return Geo::distance(a, b) <= Geo::exitWeldTolerance;
}

Geo::Polylines merge(const Geo::Polylines& scanLines, const Geo::Polylines& frames) {
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

} // namespace

void Creator::create() {
    createRaster(
        gcp.tools.front(),
        gcp.params[GCode::Params::Depth].toDouble(),
        gcp.params[UseAngle].toDouble(),
        gcp.params[HathStep].toDouble(),
        gcp.params[Pass].toInt());
}

void Creator::createRaster(const Tool& tool, const double depth, const double angle, const double hatchStep, const int prPass) {
    switch(gcp.side()) {
    case GCode::Outer: groupedPaths(GCode::Grouping::Cutoff, 1.0); break;
    case GCode::Inner: groupedPaths(GCode::Grouping::Copper); break;
    case GCode::On   : return;
    }

    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter * 0.5;
    stepOver = tool.stepover();

    Geo::Polylines profilePaths;

    for(const Geo::Polygon& group: groupedPss) {
        // Карман -- группа, сжатая на радиус (delta у Inflate -- полная ширина).
        const Geo::Polygons pocket = Geo::Inflate(Geo::Polygons{group}, -toolDiameter);
        if(pocket.empty()) continue;

        if(prPass) profilePaths.append_range(pocket.contours());

        // Два прохода штриховки: угол и угол + 90°. Карман поворачивается ПОД
        // горизонтальную змейку, результат -- обратно.
        auto hatchPass = [&](double passAngle) {
            Geo::Polylines contours = pocket.contours();
            Geo::rotate(contours, passAngle);
            const Geo::Polygons region{contours};

            auto zigzag = calcZigzag(region.boundingRect(), hatchStep);
            if(!zigzag) return;

            Geo::Polyline combContour{*zigzag};
            combContour.closed = true;
            if(combContour.signedArea() < 0) combContour.reverse();
            const Geo::Polygons comb{Geo::Polylines{std::move(combContour)}};

            auto scanLines = calcScanLines(region, *zigzag);
            auto frames = calcFrames(contours, comb);
            if(scanLines.size() && frames.size()) {
                auto merged = merge(scanLines, frames);
                Geo::rotate(merged, -passAngle);
                returnPs.append_range(std::move(merged));
            }
        };
        hatchPass(angle);
        hatchPass(angle + 90);
    }

    GCode::mergePolylines(returnPs, Geo::exitWeldTolerance);
    GCode::sortByProximity(returnPs, App::home().pos() + App::zero().pos());

    if(!profilePaths.empty() && prPass) {
        GCode::sortByProximity(profilePaths, App::home().pos() + App::zero().pos());
        if(gcp.convent())
            r::for_each(profilePaths, &Geo::Polyline::reverse);
    }

    returnPss.clear();
    switch(prPass) {
    case NoProfilePass:
        if(!returnPs.empty()) returnPss.push_back(std::move(returnPs));
        break;
    case First:
        if(!profilePaths.empty()) returnPss.push_back(std::move(profilePaths));
        if(!returnPs.empty()) returnPss.push_back(std::move(returnPs));
        break;
    case Last:
        if(!returnPs.empty()) returnPss.push_back(std::move(returnPs));
        if(!profilePaths.empty()) returnPss.push_back(std::move(profilePaths));
        break;
    default: break;
    }

    if(returnPss.size()) {
        gcp.toolPathss = std::move(returnPss);
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

/////////////////////////////////////////
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
    createGiRaster();
    itemGroup()->setVisible(true);
}

} // namespace CrossHatch
