// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "file.h"

// #include "gc_node.h"
// #include "gi_datapath.h"
// #include "gi_datasolid.h"
// #include "gi_drill.h"
// #include "gi_gcpath.h"
// #include "gi_point.h"
// #include "graphicsview.h"
// #include "project.h"
// #include "settings.h"

// #include <QDir>
// #include <QFile>
// #include <QPainter>
// #include <QRegularExpression>
// #include <QTextStream>

namespace GCode {

PocketRasterFile::PocketRasterFile()
    : File() { }

PocketRasterFile::PocketRasterFile(GCodeParams&& gcp, Pathss&& toolPathss, Paths&& pocketPaths)
    : File(std::move(gcp), std::move(pocketPaths), std::move(toolPathss)) {
    if (gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void PocketRasterFile::genGcodeAndTile() {
    const QRectF rect = App::project()->worckRect();
    for (size_t x = 0; x < App::project()->stepsX(); ++x) {
        for (size_t y = 0; y < App::project()->stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project()->spaceX()) * x, (rect.height() + App::project()->spaceY()) * y);

            if (toolType() == Tool::Laser)
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);

            if (gcp_.params.contains(GCodeParams::NotTile))
                return;
        }
    }
}

void PocketRasterFile::createGi() {
    createGiRaster();
    itemGroup()->setVisible(true);
}

} // namespace GCode