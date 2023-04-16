/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_creator.h"
#include "gc_file.h"

#include <QIcon>
#include <QPixmap>

namespace Thread {

struct Settings {
    int sort{};
};

inline Settings settings;

inline constexpr auto THREAD = md5::hash32("Thread");

class File final : public GCode::File {
public:
    explicit File();
    explicit File(GCode::Params&& gcp, Pathss&& toolPathss);
    QIcon icon() const override { return QIcon::fromTheme("crosshairs"); } // FIXME
    uint32_t type() const override { return THREAD; }
    void createGi() override;
    void genGcodeAndTile() override;
}; // File

class Creator : public GCode::Creator {

public:
    Creator() = default;
    ~Creator() override = default;

    enum {
        BridgeLen = GCode::Params::UserParam,
        TrimmingCorners,
        TrimmingOpenPaths,
        BridgeAlignType,
        BridgeValue,
        BridgeValue2,
    };

private:
    void createThread(const Tool& tool, const double depth);
    void trimmingOpenPaths(Paths& paths);

    Point from;

    void cornerTrimming();
    void makeBridges();

    void reorder();
    void reduceDistance(Point& from, Path& to);
    enum NodeType {
        ntAny,
        ntOpen,
        ntClosed
    };
    void polyTreeToPaths(PolyTree& polytree, Paths& rpaths);
    //    void addPolyNodeToPaths(PolyTree& polynode, NodeType nodetype, Paths& paths);
    //    void closedPathsFromPolyTree(PolyTree& polytree, Paths& paths);
    //    void openPathsFromPolyTree(const PolyTree& polytree, Paths& paths);

protected:
    void create() override; // Creator interface
    uint32_t type() override { return THREAD; }
    bool possibleTest() const override { return true; }
};

} // namespace Thread
