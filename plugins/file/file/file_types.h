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
#include "datastream.h"
#include <QPolygonF>
#include <type_traits>

class GiDrill;

namespace TmpFile {

class File;

class Settings {
protected:
    static inline QString parseZeroMode_;
    static inline QString parseUnitMode_;
    static inline QString parseDecimalAndInteger_;

public:
    static QString parseZeroMode() { return parseZeroMode_; }
    static QString parseUnitMode() { return parseUnitMode_; }
    static QString parseDecimalAndInteger() { return parseDecimalAndInteger_; }
};

} // namespace TmpFile
