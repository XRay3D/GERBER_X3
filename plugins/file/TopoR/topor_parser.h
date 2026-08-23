/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "topor_types.h"
#include <QString>

namespace TopoR {

// Разбор одного .fst в один TopoR::File с внутренними слоями (см. план:
// /home/x-ray/.claude/plans/topor-silly-riddle.md, "слои как в DXF").
class Parser {
public:
    File* parseFile(const QString& fileName);
};

} // namespace TopoR
