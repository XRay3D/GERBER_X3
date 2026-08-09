/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "g2_types.h"

#include <QStringList>

namespace Gerber2 {

// Результат разбора одного Gerber-файла.
struct ParseResult {
    Format format;
    ApertureMap apertures;
    std::map<QString, Macro> macros;
    Objects objects;      // изображение в порядке наложения
    Geo::Polygon strokes;       // осевые линии draw/arc — для отображения «путей»
    QStringList warnings; // нереализованные/подозрительные команды
    QString error;        // фатальная ошибка (пусто — всё хорошо)
    bool valid() const { return error.isEmpty(); }
};

// Разбор исходного текста Gerber-файла. Ничего не рисует и не зависит от Qt-GUI.
ParseResult parse(const QString& source);

// Схлопывание потока объектов в итоговые контуры с учётом полярности (2.3.2).
Geo::Polygon flatten(const Objects& objects);

} // namespace Gerber2
