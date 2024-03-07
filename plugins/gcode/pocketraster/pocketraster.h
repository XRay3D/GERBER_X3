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

namespace PocketRaster {

constexpr auto POCKET_RASTER = md5::hash32("PocketRaster");

class Creator : public GCode::Creator {
public:
    Creator() { }
    ~Creator() override = default;

    enum {
        AccDistance = GCode::Params::UserParam,
        Fast,
        Pass,
        UseAngle,
    };

    enum {
        NoProfilePass,
        First,
        Last
    };
    // Creator interface
protected:
    void create() override; // Creator interface
    uint32_t type() override;
    bool possibleTest() const override { return true; }

private:
    void createRaster(const Tool& tool, const double depth, const double angle, const int prPass);
    void createRasterAccLaser(const Tool& tool, const double depth, const double angle, const int prPass);
    void addAcc(Paths& src, const /*Point::Type*/int32_t accDistance);

    Paths calcScanLines(const Paths& src, const Path& frame);
    Paths calcFrames(const Paths& src, const Path& frame);
    Path calcZigzag(const Paths& src);

    Paths merge(const Paths& scanLines, const Paths& frames);

    Rect rect;
};

class File final : public GCode::File {

public:
    explicit File();
    explicit File(GCode::Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths);
    QIcon icon() const override { return QIcon::fromTheme("raster-path"); }
    uint32_t type() const override { return POCKET_RASTER; }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace PocketRaster
