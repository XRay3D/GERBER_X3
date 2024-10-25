/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gc_file.h"
#include "gc_types.h"

namespace Voronoi {

constexpr auto VORONOI = md5::hash32("Voronoi");

enum {
    FrameOffset = GCode::Params::UserParam,
    Tolerance,
    VoronoiType,
    Width,
};

class File final : public GCode::File {

public:
    explicit File();
    explicit File(GCode::Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths);
    QIcon icon() const override { return QIcon::fromTheme("voronoi-path"); }
    uint32_t type() const override { return VORONOI; }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace Voronoi
