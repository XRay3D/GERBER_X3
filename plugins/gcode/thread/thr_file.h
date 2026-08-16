/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  ХХ ХХХ 2026                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "app.h"
#include "gc_file.h"
#include "gc_plugin.h"
#include "project.h"

#include <QFile>

namespace Threading {

constexpr auto THREAD = "THREAD"_hash32;

class File final : public GCode::File {
public:
    using GCode::File::File;
    explicit File(GCode::Params&& newGcp)
        : GCode::File{std::move(newGcp)} {
        if(gcp.tools.front().diameter())
            regenerate();
    }
    QIcon icon() const override { return QIcon::fromTheme(u"thread-path"_s); }
    uint32_t type() const override { return THREAD; }
    void createGi() override { createGiDrill(), itemGroup()->setVisible(true); }
};

} // namespace Threading
