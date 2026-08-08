/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_creator.h"
#include "gc_file.h"

#include <QIcon>
#include <QPixmap>

namespace Profile {

struct Settings {
    int sort{};
};

inline Settings settings;

inline constexpr auto PROFILE = "Profile"_hash32;

class File final : public GCode::File {
public:
    explicit File();
    explicit File(GCode::Params&& gcp);
    QIcon icon() const override { return QIcon::fromTheme(u"profile-path"_s); }
    uint32_t type() const override { return PROFILE; }
    void createGi() override;
    void genGcodeAndTile() override;
}; // File

class Creator : public GCode::Creator {

public:
    Creator() = default;
    ~Creator() override = default;

    static inline const QString BridgeLen = u"BridgeLen"_s;
    static inline const QString TrimmingCorners = u"TrimmingCorners"_s;
    static inline const QString TrimmingOpenPaths = u"TrimmingOpenPaths"_s;
    static inline const QString BridgeAlignType = u"BridgeAlignType"_s;
    static inline const QString BridgeValue = u"BridgeValue"_s;
    static inline const QString BridgeValue2 = u"BridgeValue2"_s;

private:
    void createProfile(const Tool& tool, const double depth);
    void trimmingOpenPaths(Paths64& paths);

    Point64 from;

    void cornerTrimming();
    void makeBridges();

    void reorder();
    void reduceDistance(Point64& from, Path64& to);
    enum NodeType {
        ntAny,
        ntOpen,
        ntClosed
    };
    void polyTreeToPaths(PolyTree& polytree, Paths64& rpaths);
    // void addPolyNodeToPaths(PolyTree& polynode, NodeType nodetype, Paths64& paths);
    // void closedPathsFromPolyTree(PolyTree& polytree, Paths64& paths);
    // void openPathsFromPolyTree(const PolyTree& polytree, Paths64& paths);

protected:
    void create() override; // Creator interface
    uint32_t type() override { return PROFILE; }
    bool possibleTest() const override { return true; }
};

} // namespace Profile
