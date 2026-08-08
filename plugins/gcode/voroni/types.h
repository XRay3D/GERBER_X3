/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gc_file.h"
#include "gc_types.h"

namespace Voronoi {

constexpr auto VORONOI = "Voronoi"_hash32;

inline const QString FrameOffset = u"FrameOffset"_s;
inline const QString Tolerance = u"Tolerance"_s;
inline const QString VoronoiType = u"VoronoiType"_s;
inline const QString Width = u"Width"_s;

class File final : public GCode::File {

public:
    explicit File();
    explicit File(GCode::Params&& gcp);
    QIcon icon() const override { return QIcon::fromTheme(u"voronoi-path"_s); }
    uint32_t type() const override { return VORONOI; }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace Voronoi
