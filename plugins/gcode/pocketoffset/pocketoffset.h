/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
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
    void createGi() override;
    void genGcodeAndTile() override;
};

class PocketCreator : public Creator {
public:
    using Creator::Creator;

private:
    void createFixedSteps(const Tool& tool, const double depth, const int steps);
    void createStdFull(const Tool& tool, const double depth);
    void createMultiTool(const mvector<Tool>& tools, double depth);

protected:
    void create() override; // Creator interface
    GCodeType type() override { return Pocket; }
};

} // namespace GCode
