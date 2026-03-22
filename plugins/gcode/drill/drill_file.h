/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  ХХ ХХХ 2025                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gc_file.h"
#include "project.h"

namespace Drilling {

constexpr auto DRILLING = "Drilling"_hash32;

class File final : public GCode::File {
public:
    using GCode::File::File;
    explicit File(GCode::Params&& gcp, Pathss&& toolPathss)
        : GCode::File(std::move(gcp), std::move(toolPathss), {}) {
        if(gcp_.tools.front().diameter()) {
            initSave();
            addInfo();
            statFile();
            genGcodeAndTile();
            endFile();
        }
    }
    QIcon icon() const override { return QIcon::fromTheme(u"drill-path"_s); }
    uint32_t type() const override { return DRILLING; }
    void createGi() override { createGiDrill(), itemGroup()->setVisible(true); }
    void genGcodeAndTile() override {
        const QRectF rect = App::project().worckRect();
        for(size_t x{}; x < App::project().stepsX(); ++x) {
            for(size_t y{}; y < App::project().stepsY(); ++y) {
                const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);
                saveDrill(offset);
                if(gcp_.params.contains(GCode::Params::NotTile))
                    return;
            }
        }
    }
};

} // namespace Drilling
