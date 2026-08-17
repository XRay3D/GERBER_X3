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

#include "geo/polygon.h"
#include "geo/polyline.h"

#include <QJSValue>
#include <QObject>
#include <QPointF>
#include <optional>
#include <vector>

namespace GCode {
class GcFileProxy;
}

namespace Profile {

class File;

// Объект file.ext в profile.js: геометрия мостов (табов). Куски траектории
// живут здесь в пуле и наружу уходят номерами -- скрипт ими только
// распоряжается (порядок проходов, рампы, чередование), а вывод строк идёт
// тем же File::savePath, что и у обычных путей.
class BridgesApi final : public QObject {
    Q_OBJECT
    // Есть ли мосты у этой УП вообще: не лазер, центры мостов приехали, длина
    // и высота заданы.
    Q_PROPERTY(bool hasBridges READ hasBridges CONSTANT)
    // Верх таба (отрицательный, от поверхности), от дна реза на BridgeHeight.
    Q_PROPERTY(double tabTop READ tabTop CONSTANT)

public:
    BridgesApi(File* file, GCode::GcFileProxy* proxy);

    bool hasBridges() const;
    double tabTop() const;

    // Куски пути pathss[pathssIdx][pathIdx] последнего getToolPaths() по
    // кругам мостов, в порядке обхода: [{id, bridge, perimeter}].
    Q_INVOKABLE QJSValue chain(int pathssIdx, int pathIdx);
    // Горб над мостом: {up, flat, down} -- id кусков (flat == -1, если полки нет).
    Q_INVOKABLE QJSValue split(int id);
    // Тот же кусок в обратную сторону -- новый id.
    Q_INVOKABLE int reverse(int id);
    Q_INVOKABLE double perimeter(int id) const;
    // Строки УП по куску (perimeter > 0 && depth != 0 -- рампа, иначе ровно).
    Q_INVOKABLE QJSValue lines(int id, double perimeter, double depth);

private:
    int add(Geo::Polyline&& path);
    const Geo::Polygons& region();

    File* file_;
    GCode::GcFileProxy* proxy_;
    std::vector<Geo::Polyline> pool_;
    // Регион кругов реза считается на смещение текущего тайла и кэшируется.
    std::optional<QPointF> regionOffset_;
    Geo::Polygons region_;
};

} // namespace Profile
