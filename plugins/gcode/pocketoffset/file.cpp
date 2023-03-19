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
#include "pocketoffset.h"

namespace GCode {

PocketOffsetFile::PocketOffsetFile()
    : File() { }

PocketOffsetFile::PocketOffsetFile(GCodeParams&& gcp, Pathss&& toolPathss, Paths&& pocketPaths)
    : File(std::move(gcp), std::move(pocketPaths), std::move(toolPathss)) {
    if (gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void PocketOffsetFile::genGcodeAndTile() {
    const QRectF rect = App::project()->worckRect();
    for (size_t x = 0; x < App::project()->stepsX(); ++x) {
        for (size_t y = 0; y < App::project()->stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project()->spaceX()) * x, (rect.height() + App::project()->spaceY()) * y);
            if (toolType() == Tool::Laser)
                saveLaserPocket(offset);
            else
                saveMillingPocket(offset);

            if (gcp_.params.contains(GCodeParams::NotTile))
                return;
        }
    }
}

void PocketOffsetFile::createGi() {
    //    switch (gcp_.gcType) {
    //    case GCode::Raster:
    //        createGiRaster();
    //        break;
    //    case GCode::Pocket:
    createGiPocket();
    //        break;
    //    default:
    //        break;
    //    }

    itemGroup()->setVisible(true);
}

} // namespace GCode
