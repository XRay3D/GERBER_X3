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

class DrillFile final : public File {
public:
    explicit DrillFile();
    explicit DrillFile(GCodeParams&& gcp, Pathss&& toolPathss, Paths&& pocketPaths);
    QIcon icon() const override { return QIcon::fromTheme("drill-path"); }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace GCode

