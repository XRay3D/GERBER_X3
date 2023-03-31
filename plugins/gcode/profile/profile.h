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

namespace GCode {

inline constexpr auto PROFILE = md5::hash32("Profile");

class ProfileFile final : public File {
public:
    explicit ProfileFile();
    explicit ProfileFile(GCodeParams&& gcp, Pathss&& toolPathss);
    QIcon icon() const override { return QIcon::fromTheme("profile-path"); }
    uint32_t type() const override { return PROFILE; }
    void createGi() override;
    void genGcodeAndTile() override;
}; // ProfileFile

class ProfileCtr : public Creator {

public:
    ProfileCtr();
    ~ProfileCtr() override = default;

    enum {
        BridgeLen = GCodeParams::UserParam,
        TrimmingCorners,
        TrimmingOpenPaths,
        BridgeAlignType,
        BridgeValue,
        BridgeValue2,
    };

private:
    void createProfile(const Tool& tool, const double depth);
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
    uint32_t type() override { return PROFILE; }
};

} // namespace GCode
