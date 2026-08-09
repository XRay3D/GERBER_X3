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
#pragma once

#include "gc_creator.h"
#include "gc_file.h"

namespace PocketOffset {

constexpr auto POCKET_OFFSET = "PocketOffset"_hash32;

class File final : public GCode::File {
public:
    explicit File();
    explicit File(GCode::Params&& gcp);
    QIcon icon() const override { return QIcon::fromTheme(u"pocket-path"_s); }
    uint32_t type() const override { return POCKET_OFFSET; }
    void createGi() override;
    void genGcodeAndTile() override;
};

class Creator : public GCode::Creator {
public:
    using GCode::Creator::Creator;

    static inline const QString OffsetSteps = u"OffsetSteps"_s;

private:
    // Общий хвост всех режимов: разложить петли по группам «одна врезка -- один
    // проход» и собрать файл. cutArea -- область, заметённая фрезой.
    void finishPocket(const Tool& tool, Geo::Polygons&& cutArea);

    // Тела, внутри которых и ходит фреза. Результат живёт в groupedPss.
    const std::vector<Geo::Polygon>& pocketBodies(const double depth);

    // Силуэт детали -- медь без дырок.
    Geo::Polygons solidBodies() const;

    // Обход детали по внешнему контуру -- снаружи и не фиксированными шагами.
    // Петли кладёт в returnPs, возвращает заметённую ими полосу.
    Geo::Polygons outerContourPass();

    // Заливка одного тела концентрическими петлями.
    Geo::Polylines fillBody(const Geo::Polygon& body, const Geo::Polygons& forbidden, double minFeature);

    void createFixedSteps(const Tool& tool, const double depth, int steps);
    void createStdFull(const Tool& tool, const double depth);
    void createMultiTool(const std::vector<Tool>& tools, double depth);

protected:
    void create() override; // Creator interface
    uint32_t type() override { return POCKET_OFFSET; }
    bool possibleTest() const override { return true; }
};

} // namespace PocketOffset
