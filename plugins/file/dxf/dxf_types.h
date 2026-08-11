/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_model3d.h"
#include "entities/dxf_graphicobject.h"
#include "md5.h"
#include "serial.h"
#include "utils.h" // using namespace Qt::Literals

#include <QColor>
#include <QVariant>

#include <map>
#include <numbers>
#include <vector>

namespace Dxf {

constexpr auto DXF = md5::hash32("Dxf");

class DxfObj : public QObject {
    Q_OBJECT
public:
    DxfObj() { }
    virtual ~DxfObj() { }
};

enum class ItemsType {
    Null = -1,
    Normal,
    Paths,
    Both
};

struct Color : QColor {
    constexpr Color(double r, double g, double b)
        : QColor{
              static_cast<int>(r * 255),
              static_cast<int>(g * 255),
              static_cast<int>(b * 255),
          } { }
};

extern const Color dxfColors[];

class Layer;
struct AbstractTable;
struct Block;
struct SectionParser;
struct Style;

using GraphicObjects = std::vector<DxfGo>;
using EntitiesUP = std::vector<std::shared_ptr<Entity>>;
using Entities = std::vector<Entity*>;

using Blocks = std::map<QString, Block*>;
using HeaderData = std::map<QString, std::map<int, QVariant>>;
using Layers = std::map<QString, Layer*>;
using Sections = std::map<int, SectionParser*>;
using Styles = std::map<QString, Style*>;
using Tables = std::map<int, QVector<AbstractTable*>>;

class Settings {
protected:
    static inline QString defaultFont_{u"Arial"_s};
    static inline bool boldFont_{false};
    static inline bool italicFont_{false};
    static inline bool overrideFonts_{false};
    // Битовая маска видов (Dxf::viewBit), для которых строятся проекционные слои.
    static inline uint8_t views_{AllViews};

public:
    static QString defaultFont() { return defaultFont_; }
    static bool boldFont() { return boldFont_; }
    static bool italicFont() { return italicFont_; }
    static bool overrideFonts() { return overrideFonts_; }
    static uint8_t views() { return views_; }
};

} // namespace Dxf

// Адаптеры полиморфных членов File объявлены ЗДЕСЬ, а не рядом со своими
// классами: File::serialize инстанцирует движок прямо в dxf_file.h, и к тому
// месту специализация должна быть уже видна, иначе Serial::write уходит в общую
// ветку и падает static_assert'ом на указателе.

// Слои живут в File по указателю (Dxf::Layers -- map<QString, Layer*>): писать
// разыменованный объект, а при чтении заводить новый слой, привязанный к
// текущему файлу (крюк File::crutch, как у апертур Gerber).
// Тела в dxf_layer.cpp.
template <>
struct Serial::Adapter<Dxf::Layer*> {
    static void write(Writer& sb, Dxf::Layer* const& layer);
    static simdjson::error_code read(simdjson::ondemand::value& val, Dxf::Layer*& layer);
};

// Полиморфные сущности: {"type":<Entity::Type>, <поля flatten>}. Тип числовой --
// именно значение и есть ключ фабрики createEntity; поля пишет рефлексия по
// КОНКРЕТНОМУ классу, отсюда switch-диспетчер в dxf_entity.cpp -- двойник
// createEntity. Тела там же.
template <>
struct Serial::Adapter<std::shared_ptr<Dxf::Entity>> {
    static void write(Writer& sb, const std::shared_ptr<Dxf::Entity>& entity);
    static simdjson::error_code read(simdjson::ondemand::value& val, std::shared_ptr<Dxf::Entity>& entity);
};

// Значения секции HEADER после загрузки нужны только окну просмотра, а оно
// показывает их строкой (см. TreeWidget в dxf_node.cpp), -- строкой они и
// хранятся, без исходного типа кода группы.
template <>
struct Serial::Adapter<QVariant> {
    static void write(Writer& sb, const QVariant& v) {
        Serial::Adapter<QString>::write(sb, v.toString());
    }
    static simdjson::error_code read(simdjson::ondemand::value& val, QVariant& v) {
        QString s;
        if(auto err = Serial::Adapter<QString>::read(val, s); err) return err;
        return v = s, simdjson::SUCCESS;
    }
};
