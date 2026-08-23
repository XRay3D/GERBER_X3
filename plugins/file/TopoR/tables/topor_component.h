/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "plugintypes.h"
#include "serial.h"

#include <QPointF>
#include <QString>
#include <vector>

namespace TopoR {

// Компонент на плате -- нативные поля TopoR (ComponentsOnBoard::CompInstance),
// а не разбор текстовых атрибутов Gerber X3 (см. план: свой тип вместо
// Gerber::Comp::Component, чтобы не тянуть зависимость file_TopoR -> file_gerber).
struct Component {
    QString refDes;      // CompInstance::name
    QString value;       // Attribute со значением type::PartName, если есть
    QString footprint;   // FootprintRef::name
    QString componentRef; // ComponentRef::name
    Side side{Top};
    double angle{};
    QPointF pos;
    bool fixed{};
};

using Components = std::vector<Component>;

} // namespace TopoR
