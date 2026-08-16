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
#include "voronoi.h"
#include "gc_pathutils.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "project.h"

namespace Voronoi {

namespace {

// Концентрические петли заливки от region с шагом step: первая -- сам
// region, каждая следующая смещена на step (см. то же в pocketoffset.cpp;
// не вынесено в общий gc_pathutils, чтобы не трогать уже отлаженный код
// соседнего плагина ради единственного второго вызова).
Geo::Polylines concentricLoops(const Geo::Polygons& region, double step) {
    Geo::Polylines loops;
    for(int i{};; ++i) {
        Geo::checkCancelled();
        const Geo::Polygons loop = i ? Geo::Inflate(region, 2.0 * step * i) : region;
        if(loop.empty()) break;
        loops.append_range(loop.contours());
    }
    return loops;
}

} // namespace

void Creator::create() {
    const auto& tool = gcp.tools.front();
    const auto depth = gcp.params[GCode::Params::Depth].toDouble();
    const auto width = gcp.params[Width].toDouble();

    groupedPaths(GCode::Grouping::Copper);
    switch(gcp.params[VoronoiType].toInt()) {
    case 0: boostVoronoi(); break;
    case 1: jcVoronoi(); break;
    }

    if(returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }

    if(width < tool.getDiameter(depth)) {
        returnPs.resize(returnPs.size() - 1); // remove frame
        if(returnPs.empty()) { // ничего, кроме рамки, -- скелетить не между чем
            emit fileReady(nullptr);
            return;
        }
        GCode::sortByProximity(returnPs, App::home().pos() + App::zero().pos());
        gcp.toolPathss = {returnPs};
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    } else {
        Geo::Polylines copy{returnPs};
        copy.resize(copy.size() - 1); // remove frame
        createOffset(tool, depth, width);

        Geo::Polygons milledArea;
        { // создание перемычек: куски скелета Вороного, до которых карман не
          // дотянулся (слишком узкое место для полного прохода фрезой),
          // остаются одиночными проходами по самой линии.
            Geo::Polylines bridges = Geo::clipOpen(Geo::ClipType_::Difference, copy, Geo::Polygons{openSrcPaths});
            GCode::sortByProximity(bridges, App::home().pos() + App::zero().pos());
            for(auto&& p: bridges)
                returnPss.push_back(Geo::Polylines{std::move(p)});

            { // заметённая площадь -- для setPocketAreaCurves: первый виток
              // кармана (openSrcPaths -- он отстоит от края полосы на радиус)
              // и перемычки, раздутые на диаметр фрезы. Прежние «dOffset + 10»
              // -- это радиус плюс 10 ЕДИНИЦ uScale (0.1 мкм), не 10 мм.
                milledArea = Geo::Inflate(Geo::Polygons{openSrcPaths}, toolDiameter);
                if(!bridges.empty())
                    milledArea |= Geo::Inflate(bridges, toolDiameter);
            }
        }

        // erase empty groups
        std::erase_if(returnPss, [](const Geo::Polylines& p) { return p.empty(); });

        if(returnPss.empty()) {
            emit fileReady(nullptr);
            return;
        }

        gcp.toolPathss = std::move(returnPss);
        gcp.setPocketAreaCurves(std::move(milledArea));
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

void Creator::createOffset(const Tool& tool, double depth, const double width) {
    setMsg(tr("Create Offset"));
    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter * 0.5;
    stepOver = tool.stepover();

    // create offset: раздуть скелет Вороного (вместе с рамкой) в полосу
    // шириной width -- delta у Geo::Inflate уже ПОЛНАЯ ширина.
    //
    // Рамка -- ЛИНИЯ, а не тело: замкнутый контур Geo::Inflate раздувает как
    // площадь, и вместо полосы вдоль рамки получалась бы вся плата целиком,
    // минус медь -- «карман на всё свободное место». Clipper2 (EndType::Round)
    // рамку как раз вёл линией; здесь для того же снимаем флаг замкнутости и
    // замыкаем путь повтором первой точки.
    Geo::Polylines lines = returnPs;
    if(!lines.empty() && lines.back().closed) {
        Geo::Polyline& frame = lines.back();
        frame.closed = false;
        frame.push_back(frame.front());
    }
    Geo::Polygons band = Geo::Inflate(lines, width);

    // fit offset to copper: не резать медь.
    if(!groupedPss.empty())
        band -= Geo::Polygons{std::span<const Geo::Polygon>{groupedPss}};

    // create pocket: концентрическая заливка получившейся области, первая
    // петля -- сама область, отступившая от границы на радиус фрезы.
    const Geo::Polygons region = Geo::Inflate(band, -toolDiameter);
    openSrcPaths = region.contours();
    returnPs = concentricLoops(region, -stepOver);

    if(returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    stacking(returnPs);
}

/////////////////////////////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        regenerate();
    }
}

void File::createGi() {
    if(gcp.toolPathss.size() > 1) {
        Gi::Item* item;
        item = new Gi::GcPath{{gcp.toolPathss.back().back()}, this};
        itemGroup()->push_back(item);
        createGiPocket();
    } else
        createGiProfile();

    itemGroup()->setVisible(true);
}
} // namespace Voronoi
