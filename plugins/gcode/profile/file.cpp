// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "profile.h"

#include "gi_gcpath.h"
#include "project.h"
#include "settings.h"

#include <QDir>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QTextStream>

namespace Profile {

File::File()
    : GCode::File() { }

File::File(GCode::Params&& gcp, Pathss&& toolPathss)
    : GCode::File(std::move(gcp), std::move(toolPathss)) {
    if (gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project()->worckRect();
    for (size_t x = 0; x < App::project()->stepsX(); ++x) {
        for (size_t y = 0; y < App::project()->stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project()->spaceX()) * x, (rect.height() + App::project()->spaceY()) * y);

            if (toolType() == Tool::Laser)
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);

            if (gcp_.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {

    GraphicsItem* item;
    for (const Paths& paths : toolPathss_) {
        item = new GiGcPath(paths, this);
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }

    for (size_t i {}; const Paths& paths : toolPathss_) {
        item = new GiGcPath(toolPathss_[i], this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for (size_t j = 0; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if (i < toolPathss_.size() - 1) {
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
        }
    }

    item = new GiGcPath(g0path_);
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);

    itemGroup()->setVisible(true);
}

} // namespace Profile
