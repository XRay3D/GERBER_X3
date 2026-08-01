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
        if(gcp.tools.front().diameter()) {
            initSave();
            addInfo();
            statFile();
            genGcodeAndTile();
            endFile();
        }
    }
    QIcon icon() const override { return QIcon::fromTheme(u"thread-path"_s); }
    uint32_t type() const override { return THREAD; }
    void createGi() override { createGiDrill(), itemGroup()->setVisible(true); }
    // Thread milling has no C++ implementation - the actual toolpath math
    // (helical interpolation, multi-start, inside/outside, chamfer...) lives
    // entirely in scripts/threading.js. saveDrill() here was a leftover from
    // this file being copied from Drill and produced wrong (drill-style)
    // g-code instead of thread milling. Run the plugin's script directly
    // instead of going through GCode::File::regenerate(), since this override
    // is also invoked directly (bypassing regenerate()) from this class's own
    // constructor above and from MainWindow's "save all to one file" path.
    void genGcodeAndTile() override {
        if(auto* plugin = App::gCodePlugin(type())) {
            const QString scriptPath = u"/home/x-ray/projects/qt/GERBER_X3/bin/Qt6._GNU_Debug_x64/bin/scripts/threading.js"_s; // u"/scripts/" + plugin->gcName() + u".js"; // App::gcSettings().scriptPath(plugin->gcName());
            if(!scriptPath.isEmpty() && QFile::exists(scriptPath) && runJsScript(scriptPath))
                return;
        }
        qWarning("Threading: no G-code script configured, thread milling not generated");
    }
};

} // namespace Threading
