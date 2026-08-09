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

// JSON-сериализация типов Geo (tag_invoke для simdjson). Объявления здесь,
// тела — в polyline.cpp и polygon.cpp: Polygon::Impl виден только там.
//
// ВАЖНО: этот заголовок обязан быть виден всюду, где Geo-типы уходят в
// Json::append — иначе диспетчер не увидит tag_invoke и молча запишет
// полилинию как «массив объектов с одним полем bulge» (рефлексия не видит
// базу QPointF). Включается из plugintypes.h, так что прикладной код
// получает его автоматически.
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

namespace Geo {

void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& sb, const Vertex& vertex);
simdjson::error_code tag_invoke(simdjson::deserialize_tag, simdjson::ondemand::value& val, Vertex& vertex);

void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& sb, const Polyline& polyline);
simdjson::error_code tag_invoke(simdjson::deserialize_tag, simdjson::ondemand::value& val, Polyline& polyline);

void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& sb, const Polylines& polylines);
simdjson::error_code tag_invoke(simdjson::deserialize_tag, simdjson::ondemand::value& val, Polylines& polylines);

void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& sb, const Polygon& polygon);
simdjson::error_code tag_invoke(simdjson::deserialize_tag, simdjson::ondemand::value& val, Polygon& polygon);

void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& sb, const Polygons& polygons);
simdjson::error_code tag_invoke(simdjson::deserialize_tag, simdjson::ondemand::value& val, Polygons& polygons);

} // namespace Geo
