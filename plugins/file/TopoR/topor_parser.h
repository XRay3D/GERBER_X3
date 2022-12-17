/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "topor_types.h"

class FileInterface;
class FilePlugin;

namespace TopoR {

class Parser {
    FilePlugin* const interface;

public:
    explicit Parser(FilePlugin* const interface);
    FileInterface* parseFile(const QString& fileName);

protected:
    File* file = nullptr;
};

} // namespace TopoR
