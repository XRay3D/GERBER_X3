#include "exfile.h"
#include "exparser.h"
#include "exvars.h"

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
    rawPos.first.clear();
    rawPos.second.clear();
    gCode = G_NULL;
    mCode = M_NULL;
    tCode = 0;
    pos = QPointF();
    path.clear();
    line = 0;
}

void State::updatePos()
{
    pos = QPointF(Parser::parseNumber(rawPos.first, *this), Parser::parseNumber(rawPos.second, *this));
    for (int i = 0; i < rawPosList.size(); ++i) {
        path[i] = QPointF(Parser::parseNumber(rawPosList[i].first, *this), Parser::parseNumber(rawPosList[i].second, *this));
    }
}

double State::currentToolDiameter() const
{
    return format->file->tool(tCode);
}

} //namespace Excellon
