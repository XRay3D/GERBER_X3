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
#include "file.h"

namespace GCode {

CrosshatchFile::CrosshatchFile()
    : File() { }

CrosshatchFile::CrosshatchFile(GCode::Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths)
    : File(std::move(gcp), std::move(pocketPaths), std::move(toolPathss)) {
    if (gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void CrosshatchFile::genGcodeAndTile() {
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

void CrosshatchFile::createGi() {
    createGiRaster();
    itemGroup()->setVisible(true);
}

} // namespace GCode
