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

#include "md5.h"
#include <QObject>

namespace TopoR {

constexpr auto TOPOR = "TopoR"_hash32;

class File;
class Layer;

// File/Node/Layer -- не QObject (как AbstractFile/FileTree::Node вообще), а
// голый tr() внутри них не резолвится. Тот же приём, что у Dxf::DxfObj.
class TopoRObj : public QObject {
    Q_OBJECT
public:
    TopoRObj() = default;
    ~TopoRObj() override = default;
};

} // namespace TopoR
