///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  March 25, 2023                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2023                                          *
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
    uint32_t type() const override { return md5::hash32("Thermal"); }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace GCode
