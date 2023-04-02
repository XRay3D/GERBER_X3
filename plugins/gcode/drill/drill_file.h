/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gc_file.h"

namespace Drilling {

constexpr auto DRILLING = md5::hash32("Drilling");

class File final : public GCode::File {

public:
    explicit File();
    explicit File(GCode::Params&& gcp, Pathss&& toolPathss);
    QIcon icon() const override { return QIcon::fromTheme("drill-path"); }
    uint32_t type() const override { return DRILLING; }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace Drilling
