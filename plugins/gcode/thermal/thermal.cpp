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
#include "thermal.h"
#include "gc_pathutils.h"
#include "gi_point.h"
#include "project.h"

#include "geo/boolean.h"

namespace Thermal {

Creator::Creator() { }

void Creator::create() {
    createThermal(
        App::project().file(gcp.params[FileId].toInt()),
        gcp.tools.front(),
        gcp.params[GCode::Params::Depth].toDouble());
}

void Creator::createThermal(AbstractFile* file, const Tool& tool, const double depth) {
    toolDiameter = tool.getDiameter(depth);

    { // create tool path: изоляционное кольцо -- офсет пада на радиус фрезы
      // (delta у Geo::Inflate -- ПОЛНАЯ ширина, контур уезжает на половину).
        returnPs = Geo::Inflate(closedSrc, toolDiameter).contours();

        // fix direction
        if(gcp.side() == GCode::Outer && !gcp.convent())
            r::for_each(returnPs, &Geo::Polyline::reverse);
        else if(gcp.side() == GCode::Inner && gcp.convent())
            r::for_each(returnPs, &Geo::Polyline::reverse);

        if(returnPs.empty()) return;
    }

    Geo::Polygons frame;
    { // create frame: то, где резать НЕЛЬЗЯ, -- позитивы файла (а без
      // IgnoreCopper -- вся медь), раздутые чуть меньше чем на радиус, чтобы
      // кольцо у соседней меди не задевало её, плюс спицы-мосты из превью.
      // Офсет ОДИН на объединение: сумма Минковского дистрибутивна, прежний
      // пообъектный офсет со сборкой давал то же самое, только за N вызовов.
      //
      // Медь собирается ОДНИМ конструктором (параллельное дерево слияний в
      // joinAll), а не цепочкой |=: пообъектное объединение -- квадратичное
      // число точных свипов, на плотном гербере это зависание.
        const auto graphicObjects(file->graphicObjects());
        const bool allCopper = !gcp.params[IgnoreCopper].toInt();
        std::vector<Geo::Polygon> copper;
        for(auto go: graphicObjects | v::filter([allCopper](auto* go) { return allCopper || go->positive(); }))
            copper.append_range(go->fill.all());
        frame = Geo::Inflate(Geo::Polygons{std::span<const Geo::Polygon>{copper}}, toolDiameter - 0.01);

        // Спицы уже в своём размере -- их не раздувать; и тоже одним слиянием.
        std::vector<Geo::Polygon> spokes;
        for(const Geo::Polylines& bridges: supportPss)
            spokes.append_range(Geo::Polygons{bridges}.all());
        if(!spokes.empty())
            frame |= Geo::Polygons{std::span<const Geo::Polygon>{spokes}};
    }

    { // Execute: кольца режутся рамкой КАК ПУТИ (Clipper2::AddOpenSubject) --
      // разрезанное кольцо возвращается открытыми дугами, целое остаётся
      // замкнутым как есть.
        returnPs = Geo::clipOpen(Geo::ClipType_::Difference, returnPs, frame);
        GCode::sortByProximity(returnPs, App::home().pos() + App::zero().pos());
    }

    if(returnPs.size())
        returnPss.push_back(std::move(returnPs));

    if(returnPss.size()) {
        gcp.toolPathss = std::move(returnPss);
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

//////////////////////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        regenerate();
    }
}

void File::createGi() {
    createGiProfile();
    itemGroup()->setVisible(true);
}
} // namespace Thermal
