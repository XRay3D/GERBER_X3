/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_creator.h"
#include "gc_file.h"

class AbstractFile;

namespace Thermal {

constexpr auto THERMAL = "Thermal"_hash32;

class Creator : public ::GCode::Creator {
public:
    Creator();
    ~Creator() override = default;
    enum {
        FileId = GCode::Params::UserParam,
        IgnoreCopper,
    };

private:
    void createThermal(AbstractFile* file, const Tool& tool, const double depth);

protected:
    void create() override; // Creator interface
    uint32_t type() override { return THERMAL; }
};

class File final : public GCode::File {

public:
    explicit File();
    explicit File(GCode::Params&& gcp);
    QIcon icon() const override { return QIcon::fromTheme(u"thermal-path"_s); }
    uint32_t type() const override { return THERMAL; }
    void createGi() override;
    void genGcodeAndTile() override;
};

} // namespace Thermal
