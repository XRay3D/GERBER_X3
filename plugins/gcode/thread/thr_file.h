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
#include "gi_drill.h"
#include "project.h"

#include <QFile>
#include <cmath>

namespace Threading {

constexpr auto THREAD = "THREAD"_hash32;

class[[= Serial::name("Threading")]] File final : public GCode::File {
public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    using GCode::File::File;
    explicit File(GCode::Params&& newGcp)
        : GCode::File{std::move(newGcp)} {
        if(gcp.tools.front().diameter())
            regenerate();
    }
    QIcon icon() const override { return QIcon::fromTheme(u"thread-path"_s); }
    uint32_t type() const override { return THREAD; }
    // Не createGiDrill(): та берёт только gcp.toolPathss.front().front() (первую
    // из N 2/3-точечных меток резьбы) и рисует её диаметром фрезы, а не
    // отверстия. Здесь -- по Gi::Drill на каждую метку, диаметром именно
    // отверстия (3-я точка кривой, см. Form::computePaths).
    void createGi() override {
        if(!gcp.toolPathss.size()) return;
        for(const Geo::Polyline& curve: gcp.toolPathss.front()) {
            if(curve.size() < 3) continue;
            const QPointF center{curve.front()};
            const double holeDiameter{2.0 * std::hypot(curve[2].x() - center.x(), curve[2].y() - center.y())};
            auto* item = new ::Gi::Drill{{center}, holeDiameter, this, gcp.tool().id()};
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
            item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
            itemGroup()->push_back(item);
        }
        itemGroup()->setVisible(true);
    }
};

} // namespace Threading
