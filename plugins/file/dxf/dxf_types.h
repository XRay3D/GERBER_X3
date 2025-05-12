/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "entities/dxf_graphicobject.h"
#include "md5.h"

#include <QColor>
#include <QVariant>

#include <map>
#include <numbers>
#include <vector>

namespace Dxf {

struct Exception final : std::exception {
    std::string str;
    Exception(QString&& str)
        : str{str.toStdString()} { }
    explicit Exception(std::string&& str)
        : str{std::move(str)} { }
    ~Exception() noexcept override = default;
    // exception interface
    const char* what() const noexcept override { return str.c_str(); }
};

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

class Color : public QColor {
public:
    Color(double r, double g, double b)
        : QColor(r * 255, g * 255, b * 255) {
    }
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
    static inline QString defaultFont_{"Arial"};
    static inline bool boldFont_{false};
    static inline bool italicFont_{false};
    static inline bool overrideFonts_{false};

public:
    static QString defaultFont() { return defaultFont_; }
    static bool boldFont() { return boldFont_; }
    static bool italicFont() { return italicFont_; }
    static bool overrideFonts() { return overrideFonts_; }
};

} // namespace Dxf
