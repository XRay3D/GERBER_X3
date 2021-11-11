// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "exfile.h"
#include "exparser.h"
#include "extypes.h"

namespace Excellon {

Format::Format(File* file)
    : file(file)
{
}

void State::reset(Format* f)
{
    format = f;
    rawPos.clear();
    gCode = G_NULL;
    wm = DrillMode;
    mCode = M_NULL;
    tCode = 0;
    pos = QPointF();
    path.clear();
}

void State::updatePos()
{
    pos = QPointF(Parser::parseNumber(rawPos.X, *this), Parser::parseNumber(rawPos.Y, *this));
    for (int i = 0; i < rawPosList.size(); ++i) {
        path[i] = QPointF(Parser::parseNumber(rawPosList[i].X, *this), Parser::parseNumber(rawPosList[i].Y, *this));
    }
}

double State::currentToolDiameter() const
{
    return format->file->tool(tCode);
}

} //namespace Excellon
