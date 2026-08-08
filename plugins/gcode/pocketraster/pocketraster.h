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

namespace PocketRaster {

constexpr auto POCKET_RASTER = "PocketRaster"_hash32;

class Creator : public GCode::Creator {
public:
    Creator() { }
    ~Creator() override = default;

    static inline const QString AccDistance = u"AccDistance"_s;
    static inline const QString Fast = u"Fast"_s;
    static inline const QString Pass = u"Pass"_s;
    static inline const QString UseAngle = u"UseAngle"_s;

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
    void addAcc(Paths64& src, const /*PType*/ int32_t accDistance);

    Paths64 calcScanLines(const Paths64& src, const Path64& frame);
    Paths64 calcFrames(const Paths64& src, const Path64& frame);
    Path64 calcZigzag(const Paths64& src);

    Paths64 merge(const Paths64& scanLines, const Paths64& frames);

    Rect rect;
};

class File final : public GCode::File {

public:
    explicit File();
    explicit File(GCode::Params&& gcp);
    QIcon icon() const override { return QIcon::fromTheme(u"raster-path"_s); }
    uint32_t type() const override { return POCKET_RASTER; }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace PocketRaster
