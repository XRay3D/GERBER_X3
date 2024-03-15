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

namespace PocketOffset {

constexpr auto POCKET_OFFSET = md5::hash32("PocketOffset");

class File final : public GCode::File {
public:
    explicit File();
    explicit File(GCode::Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths);
    QIcon icon() const override { return QIcon::fromTheme("pocket-path"); }
    uint32_t type() const override { return POCKET_OFFSET; }
    void createGi() override;
    void genGcodeAndTile() override;
};

class Creator : public GCode::Creator {
public:
    using GCode::Creator::Creator;

    enum {
        OffsetSteps = GCode::Params::UserParam,
    };

private:
    void createFixedSteps(const Tool& tool, const double depth, int steps);
    void createStdFull(const Tool& tool, const double depth);
    void createMultiTool(const mvector<Tool>& tools, double depth);

protected:
    void create() override; // Creator interface
    uint32_t type() override { return POCKET_OFFSET; }
    bool possibleTest() const override { return true; }
};

} // namespace PocketOffset
