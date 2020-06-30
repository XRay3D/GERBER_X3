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
    gCode = G05 /*G_NULL*/;
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
