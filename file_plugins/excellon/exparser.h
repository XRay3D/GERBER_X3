/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "extypes.h"

class FileInterface;
class FilePluginInterface;

namespace Excellon {

class Parser {
    FilePluginInterface* const interface;

public:
    explicit Parser(FilePluginInterface* const interface);
    FileInterface* parseFile(const QString& fileName);
    static double parseNumber(QString Str, const State& state);

private:
    bool parseComment(const QString& line);
    bool parseGCode(const QString& line);
    bool parseMCode(const QString& line);
    bool parseTCode(const QString& line);
    bool parsePos(const QString& line);
    bool parseSlot(const QString& line);
    bool parseRepeat(const QString& line);
    bool parseFormat(const QString& line);
    bool parseNumber(QString Str, double& val);

    void circularRout();

    QPolygonF arc(QPointF p1, QPointF p2, QPointF center);

protected:
    File* file = nullptr;
    State m_state;
};
} // namespace Excellon
