/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_creator.h"
#include "gc_file.h"

namespace CrossHatch {

constexpr auto CROSS_HATCH = md5::hash32("CrossHatch");

class File final : public GCode::File {
public:
    explicit File();
    explicit File(GCode::Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths);
    QIcon icon() const override { return QIcon::fromTheme("crosshatch-path"); }
    uint32_t type() const override { return CROSS_HATCH; }
    void createGi() override;
    void genGcodeAndTile() override;
};

class Creator : public GCode::Creator {
public:
    Creator() { }
    ~Creator() override = default;

    enum {
        UseAngle = GCode::Params::UserParam,
        HathStep,
        Pass
    };

    enum PassE {
        NoProfilePass,
        First,
        Last
    };

protected:
    // Creator interface
    void create() override; // Creator interface
    uint32_t type() override { return CROSS_HATCH; }

private:
    void createRaster(const Tool& tool, const double depth, const double angle, const double hatchStep, const int prPass);
    Rect rect;
};

} // namespace CrossHatch
