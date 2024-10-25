/********************************************************************************
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

#include "topor_types.h"

class AbstractFile;
class AbstractFilePlugin;

namespace TopoR {

class Parser {
    AbstractFilePlugin* const interface;

public:
    explicit Parser(AbstractFilePlugin* const interface);
    AbstractFile* parseFile(const QString& fileName);

protected:
    File* file = nullptr;
};

} // namespace TopoR
