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

namespace GCode {

class PocketOffsetFile final : public File {
public:
    explicit PocketOffsetFile();
    explicit PocketOffsetFile(GCodeParams&& gcp, Pathss&& toolPathss, Paths&& pocketPaths);
    QIcon icon() const override { return QIcon::fromTheme("pocket-path"); }
    FileType type() const override { return FileType(Pocket); }
    void createGi() override;
    void genGcodeAndTile() override;
};

class PocketCtr : public Creator {
public:
    using Creator::Creator;

    enum {
        OffsetSteps = GCodeParams::UserParam,
    };

private:
    void createFixedSteps(const Tool& tool, const double depth, int steps);
    void createStdFull(const Tool& tool, const double depth);
    void createMultiTool(const mvector<Tool>& tools, double depth);

protected:
    void create() override; // Creator interface
    GCodeType type() override { return Pocket; }
};

} // namespace GCode
