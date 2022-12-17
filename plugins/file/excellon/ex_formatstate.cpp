// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "ex_file.h"
#include "ex_parser.h"
#include "ex_types.h"

namespace Excellon {

void State::updatePos() {
    pos = QPointF(Parser::parseNumber(rawPos.X, *this), Parser::parseNumber(rawPos.Y, *this));
    for (int i = 0; i < rawPosList.size(); ++i)
        path[i] = QPointF(Parser::parseNumber(rawPosList[i].X, *this), Parser::parseNumber(rawPosList[i].Y, *this));
}

double State::currentToolDiameter() const {
    return format->file->tool(toolId);
}

} // namespace Excellon
