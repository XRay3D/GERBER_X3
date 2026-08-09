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

// Всё, чему нужен полный QTransform. В заголовке его нет намеренно: <QTransform>
// -- заголовок QtGui, и попав в plugintypes.h, который подключают все, он
// оказывается раньше ядра QtCore, а такой порядок на gcc-16 ломает разбор
// qmath.h.

#include "plugintypes.h"

#include "geo/util.h"

#include <QTransform>

QTransform Transform::toQTransform() const {
    QTransform t;
    t.translate(translate.x(), translate.y());
    t.rotate(angle);
    t.scale(scale.x(), scale.y());
    return t;
}

Transform::operator QTransform() const { return toQTransform(); }

GraphicObject operator*(GraphicObject go, const QTransform& t) {
    if(t.isIdentity()) return go;

    // Заливка живёт в точном домене, поэтому не правится на месте, а
    // пересобирается из преобразованных контуров.
    go.fill = Geo::transformed(go.fill, t);
    Geo::transform(go.path, t);
    go.pos = t.map(go.pos);
    return go;
}
