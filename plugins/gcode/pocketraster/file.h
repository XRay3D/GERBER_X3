///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  03 October 2022                                                 *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2022                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
#pragma once

#include "file.h"
#include "gc_file.h"

namespace GCode {

class PocketRasterFile final : public File {

public:
    explicit PocketRasterFile();
    explicit PocketRasterFile(GCodeParams&& gcp, Pathss&& toolPathss, Paths&& pocketPaths);
    QIcon icon() const override { return QIcon::fromTheme("raster-path"); }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace GCode