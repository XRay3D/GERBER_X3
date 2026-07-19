/********************************************************************************
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

#include "md5.h"

#include <QPainterPath>
#include <QString>

namespace Svg {

constexpr auto SVG = "Svg"_hash32;

struct SvgElement {
    QPainterPath painterPath; // mm, Y-up
    QString id;

    friend QDataStream& operator<<(QDataStream& s, const SvgElement& e) {
        return s << e.painterPath << e.id;
    }
    friend QDataStream& operator>>(QDataStream& s, SvgElement& e) {
        return s >> e.painterPath >> e.id;
    }
};

} // namespace Svg
