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
#pragma once

#include "ex_types.h"

class AbstractFile;
class AbstractFilePlugin;

namespace Excellon {

class Parser {
    class AbstractFilePlugin* const filePlugin;

public:
    explicit Parser(class AbstractFilePlugin* const afp);
    AbstractFile* parseFile(const QString& fileName);
    static double parseNumber(QString Str, const State& state);

private:
    bool parseComment(QString line);
    bool parseGCode(const QString& line);
    bool parseMCode(const QString& line);
    bool parseTCode(const QString& line);
    bool parsePos(const QString& line);
    bool parseSlot(const QString& line);
    bool parseRepeat(const QString& line);
    bool parseFormat(const QString& line);
    bool parseNumber(QStringView str, double& val);

    void circularRout();

    QPolygonF arc(QPointF p1, QPointF p2, QPointF center);

    Tools::iterator toolIt{};

protected:
    File* file = nullptr;
    State state_;
};

} // namespace Excellon
