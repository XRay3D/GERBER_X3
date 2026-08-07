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

// Альтернативная реализация Gerber X3.
// Вся геометрия строится исключительно средствами curve.h (Curve/Curves,
// BoolOp, Inflate) — myclipper.h здесь напрямую не используется.

#include "curve.h"

#include <QPointF>
#include <QString> // QStringView
#include <map>
#include <memory>
#include <vector>

namespace Gerber2 {

enum class Unit {
    Millimeters,
    Inches
};

enum class Zeros {
    OmitLeading,
    OmitTrailing
};

enum class Notation {
    Absolute,
    Incremental
};

// Формат координат, задаваемый командами FS и MO. См. 4.2 спецификации.
struct Format {
    int xInt = 3, xDec = 6;
    int yInt = 3, yDec = 6;
    Unit unit = Unit::Millimeters;
    Zeros zeros = Zeros::OmitLeading;
    Notation notation = Notation::Absolute;

    // Перевод «сырой» координаты файла в миллиметры.
    double toMm(QStringView token, int intDigits, int decDigits) const;
    double x(QStringView token) const { return toMm(token, xInt, xDec); }
    double y(QStringView token) const { return toMm(token, yInt, yDec); }
    // Размеры апертур/шагов SR задаются десятичными числами в единицах MO.
    double lenToMm(double value) const {
        return unit == Unit::Inches ? value * 25.4 : value;
    }
};

enum class Polarity {
    Dark,
    Clear
};

enum class Mirror {
    None,
    X,
    Y,
    XY
};

enum class PlotMode {
    Linear,
    Cw,
    Ccw
};

// Один готовый графический объект изображения: набор контуров и полярность.
struct Object {
    Curves curves;
    Polarity polarity = Polarity::Dark;
};

using Objects = std::vector<Object>;

// Преобразования апертуры (LM/LR/LS) — 4.9 спецификации.
struct ApTransform {
    Mirror mirror = Mirror::None;
    double rotation = 0.0; // градусы, против часовой
    double scale = 1.0;

    QTransform toQTransform() const {
        QTransform tr;
        tr.rotate(rotation);
        tr.scale(scale, scale);
        switch(mirror) {
        case Mirror::None: break;
        case Mirror::X   : tr.scale(-1, +1); break;
        case Mirror::Y   : tr.scale(+1, -1); break;
        case Mirror::XY  : tr.scale(-1, -1); break;
        }
        return tr;
    }
};

// Апертура: заранее посчитанные контуры в мм с началом в (0,0).
// Отверстие (hole) хранится отдельно — оно не «стирает» объекты под собой
// (4.4.6), поэтому вычитается только из тела самой апертуры.
struct Aperture {
    QString source;   // исходный текст AD для повторного сохранения
    Curves body;      // тело апертуры с уже вычтенным отверстием
    double holeDia{}; // диаметр отверстия, мм (0 — сплошная)
    bool isBlock{};   // апертура-блок (AB)
    Objects block;    // содержимое блока
};

using ApertureMap = std::map<int, std::shared_ptr<Aperture>>;

// Определение макроса AM: имя и тело (список слов-примитивов).
struct Macro {
    QString name;
    std::vector<QString> body;
};

// Состояние графики (2.7).
struct State {
    QPointF current{};
    int aperture = 0;
    PlotMode plot = PlotMode::Linear;
    Polarity polarity = Polarity::Dark;
    ApTransform tr;
    bool region = false;
    bool multiQuadrant = true; // G74/G75
};

} // namespace Gerber2
