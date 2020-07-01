// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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
    if (format) {
        format->unitMode = Millimeters;
        format->decimal = 4;
        format->integer = 3;
    }
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
