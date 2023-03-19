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

#include "gc_file.h"

namespace GCode {

class ThermalFile final : public File {

public:
    explicit ThermalFile();
    explicit ThermalFile(GCodeParams&& gcp, Pathss&& toolPathss);
    QIcon icon() const override { return QIcon::fromTheme("thermal-path"); }
    FileType type() const override { return FileType(Thermal); }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace GCode
