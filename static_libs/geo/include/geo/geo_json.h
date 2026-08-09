/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  August 09, 2026                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

// JSON-адаптеры типов Geo для движка Serial (см. common/serial.h): PIMPL и
// Qt-базы рефлексией не взять, поэтому — специализации Serial::Adapter.
// Тела — в polyline.cpp и polygon.cpp: Polygon::Impl виден только там.
//
// Geo не зависит от common, поэтому здесь лишь повторное объявление
// первичного шаблона Adapter (сигнатуры обязаны совпадать с serial.h).
//
// Кодировка (короткие ключи — полилинии доминируют в размере файла;
// чтение толерантно, отсутствие ключа = дефолт):
//   Vertex   ↔ [x, y] | [x, y, bulge≠0]
//   Polyline ↔ {"c":true?, "w":width≠0?, "v":[Vertex...]}
//   Polygon  ↔ {"o":Polyline, "h":[Polyline...]?, "i":true?}
//   Polygons ↔ [Polygon...]

#include "polygon.h"
#include "polyline.h"

#include <simdjson.h>

namespace Serial {

template <typename T>
struct Adapter;

template <>
struct Adapter<Geo::Vertex> {
    static void write(simdjson::builder::string_builder& sb, const Geo::Vertex& vertex);
    static simdjson::error_code read(simdjson::ondemand::value& val, Geo::Vertex& vertex);
};

template <>
struct Adapter<Geo::Polyline> {
    static void write(simdjson::builder::string_builder& sb, const Geo::Polyline& polyline);
    static simdjson::error_code read(simdjson::ondemand::value& val, Geo::Polyline& polyline);
};

template <>
struct Adapter<Geo::Polylines> {
    static void write(simdjson::builder::string_builder& sb, const Geo::Polylines& polylines);
    static simdjson::error_code read(simdjson::ondemand::value& val, Geo::Polylines& polylines);
};

template <>
struct Adapter<Geo::Polygon> {
    static void write(simdjson::builder::string_builder& sb, const Geo::Polygon& polygon);
    static simdjson::error_code read(simdjson::ondemand::value& val, Geo::Polygon& polygon);
};

template <>
struct Adapter<Geo::Polygons> {
    static void write(simdjson::builder::string_builder& sb, const Geo::Polygons& polygons);
    static simdjson::error_code read(simdjson::ondemand::value& val, Geo::Polygons& polygons);
};

} // namespace Serial
