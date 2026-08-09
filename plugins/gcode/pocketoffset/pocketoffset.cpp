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
#include "pocketoffset.h"
#include "project.h"

#include <gi_dbg.h>

#include <numbers>
#include <span>
// #include <QStringBuilder>

namespace PocketOffset {

// //dbgPaths(clipFrame, u"clipFrame %1"_s.arg(tIdx), Qt::red);

void Creator::create() {
    setMax(10000);

    assert(gcp.side() != GCode::On);

    if(gcp.tools.size() > 1)
        createMultiTool(gcp.tools, gcp.params[GCode::Params::Depth].toDouble());
    else if(gcp.params.contains(OffsetSteps) && gcp.params[OffsetSteps].toInt() > 0) // FIXME inside steps
        createFixedSteps(gcp.tools.front(), gcp.params[GCode::Params::Depth].toDouble(), gcp.params[OffsetSteps].toInt());
    else
        createStdFull(gcp.tools.front(), gcp.params[GCode::Params::Depth].toDouble());
}

namespace {

// Концентрические петли от region с шагом step: первая -- сам region, каждая
// следующая смещена на step. delta у Geo::Inflate -- ПОЛНАЯ ширина, а контур
// уезжает на её половину, отсюда удвоение. Отрицательный шаг идёт внутрь и
// кончается сам, когда сжимать нечего; положительный не кончится никогда, там
// limit обязателен.
Geo::Polylines concentricLoops(Geo::Polygons region, double step, int limit = {}) {
    Geo::Polylines loops;
    for(int i{}; !region.empty() && (!limit || i < limit); ++i) {
        loops.append_range(region.contours());
        region = Geo::Inflate(region, 2.0 * step);
    }
    return loops;
}

// Мелочь, которую фреза всё равно не выберет: обрывок, у которого и площадь, и
// периметр меньше самой фрезы, -- это не карман, а шум офсета.
void removeSmall(Geo::Polygons& region, double dOffset) {
    const double minArea = dOffset * dOffset * std::numbers::pi;
    const double minPerimeter = dOffset * 4.0;
    std::vector<Geo::Polygon> kept;
    for(const Geo::Polygon& polygon: region.all())
        if(std::abs(polygon.area()) >= minArea || polygon.perimeter() >= minPerimeter)
            kept.push_back(polygon);
    region = kept.empty() ? Geo::Polygons{} : Geo::Polygons{std::span<const Geo::Polygon>{kept}};
}

} // namespace

// Область, которую фреза заметает вдоль траектории. Прежде она собиралась
// отдельными офсетами внутрь и наружу; в точном домене это ровно раздувание
// самой траектории на диаметр.
void Creator::finishPocket(const Tool& tool, Geo::Polygons&& cutArea) {
    stacking(returnPs);
    if(returnPss.empty()) {
        emit fileReady(nullptr);
        return;
    }
    gcp.toolPathss = std::move(returnPss);
    gcp.setPocketAreaCurves(std::move(cutArea));
    file_ = new File{std::move(gcp)};
    file_->setFileName(tool.nameEnc());
}

void Creator::createFixedSteps(const Tool& tool, const double depth, int steps) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter * 0.5;
    stepOver = tool.stepover();

    if(gcp.side() == GCode::Inner) {
        for(const Geo::Polygon& body: groupedPaths(GCode::Grouping::Copper))
            returnPs.append_range(concentricLoops(
                Geo::Inflate(Geo::Polygons{body}, -toolDiameter), -stepOver, steps));
    } else {
        // Снаружи петли идут ОТ детали наружу и сами кончиться не могут:
        // ограничение по числу шагов здесь единственное.
        returnPs.append_range(concentricLoops(
            Geo::Inflate(closedSrc, +toolDiameter), +stepOver, steps));
    }

    if(returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    finishPocket(tool, Geo::Inflate(returnPs, toolDiameter));
}

void Creator::createStdFull(const Tool& tool, const double depth) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter * 0.5;
    stepOver = tool.stepover();

    // Снаружи карман -- то, что осталось от габаритной рамки после вычитания
    // детали, изнутри -- сама медь. Запас рамки чуть больше диаметра, чтобы
    // фреза прошла по внешнему контуру целиком.
    const auto& bodies = gcp.side() == GCode::Outer
        ? groupedPaths(GCode::Grouping::Cutoff, toolDiameter * 1.005)
        : groupedPaths(GCode::Grouping::Copper);

    // Первая петля отстоит от границы на РАДИУС: центр круглой фрезы ближе не
    // подойдёт. delta -- полная ширина, отсюда целый диаметр.
    for(const Geo::Polygon& body: bodies)
        returnPs.append_range(concentricLoops(
            Geo::Inflate(Geo::Polygons{body}, -toolDiameter), -stepOver));

    if(returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    finishPocket(tool, Geo::Inflate(returnPs, toolDiameter));
}

void Creator::createMultiTool(const std::vector<Tool>& tools, double depth) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    const auto& bodies = gcp.side() == GCode::Outer
        ? groupedPaths(GCode::Grouping::Cutoff, tools.front().getDiameter(depth) * 1.005)
        : groupedPaths(GCode::Grouping::Copper);

    // Выбранное предыдущими, более крупными фрезами: инструменты приходят
    // отсортированными по убыванию диаметра, и каждый следующий дорабатывает
    // только то, куда предыдущий не дотянулся.
    Geo::Polygons cleared;

    for(size_t tIdx{}, size = tools.size(); const Tool& tool: tools) {
        returnPs.clear();
        returnPss.clear();
        toolDiameter = tool.getDiameter(depth);
        dOffset = toolDiameter * 0.5;
        stepOver = tool.stepover();

        // Куда нельзя вести ЦЕНТР этой фрезы: вглубь уже выбранного, отступив
        // от его края на радиус. Ближе к краю фреза ещё снимает материал.
        const Geo::Polygons forbidden = cleared.empty()
            ? Geo::Polygons{}
            : Geo::Inflate(cleared, -toolDiameter);

        for(const Geo::Polygon& body: bodies) {
            Geo::Polygons region = Geo::Inflate(Geo::Polygons{body}, -toolDiameter);
            if(!forbidden.empty()) region -= forbidden;
            // Последней фрезе мелочь оставляем: дочищать после неё уже некому.
            removeSmall(region, tIdx + 1 == size ? dOffset * 0.5 : dOffset * 2.0);
            if(region.empty()) continue;
            returnPs.append_range(concentricLoops(std::move(region), -stepOver));
        }

        if(returnPs.empty()) {
            emit fileReady(nullptr);
            ++tIdx;
            continue;
        }

        Geo::Polygons cutArea = Geo::Inflate(returnPs, toolDiameter);
        cleared |= cutArea;

        stacking(returnPs);
        if(returnPss.empty()) {
            emit fileReady(nullptr);
            ++tIdx;
            continue;
        }

        // Параметры КОПИРУЮТСЯ, а не переносятся: прежний код делал
        // File{std::move(gcp)} прямо в цикле, и второму инструменту доставался
        // уже выпотрошенный gcp -- ни инструментов, ни исходной геометрии.
        GCode::Params toolGcp = gcp;
        toolGcp.params[GCode::Params::MultiToolIndex] = static_cast<ssize_t>(tIdx);
        toolGcp.toolPathss = std::move(returnPss);
        toolGcp.setPocketAreaCurves(std::move(cutArea));
        file_ = new File{std::move(toolGcp)};
        file_->setFileName(tool.nameEnc());

        // Последний файл отправит сам Creator::createGc.
        if(++tIdx < size) emit fileReady(file_);
    }
}

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
    auto& proj        = App::project();
    const QRectF rect = proj.worckRect();
    for(size_t x{}; x < proj.stepsX(); ++x) {
        for(size_t y{}; y < proj.stepsY(); ++y) {
            const QPointF offset{(rect.width() + proj.spaceX()) * x, (rect.height() + proj.spaceY()) * y};
            if(toolType == Tool::Laser)
                saveLaserPocket(offset);
            else
                saveMillingPocket(offset);

            if(gcp.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    // switch (gcp.gcType) {
    // case GCode::Raster:
    // createGiRaster();
    // break;
    // case GCode::Pocket:
    createGiPocket();
    // break;
    // default: break;
    // }

    itemGroup()->setVisible(true);
}

} // namespace PocketOffset
