// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "ex_file.h"
#include "ex_parser.h"
#include "ex_types.h"

namespace Excellon {

void State::updatePos() {
    pos = QPointF(Parser::parseNumber(rawPos.x, *this), Parser::parseNumber(rawPos.y, *this));
    for(int i = 0; i < rawPosList.size(); ++i)
        path[i] = QPointF(Parser::parseNumber(rawPosList[i].x, *this), Parser::parseNumber(rawPosList[i].y, *this));
}

double State::currentToolDiameter() const {
    return format->file->tool(toolId);
}

} // namespace Excellon
