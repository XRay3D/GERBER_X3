/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include <QColor>
#include <QVariant>

#include <map>
#include <vector>

#include "plugintypes.h"

namespace Hpgl {

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

class GraphicObject final : public AbstrGraphicObject {
    Path m_path;
    Paths m_paths;
    // AbstrGraphicObject interface
public:
    Path line() const override {
        return {};
    }
    Path lineW() const override {
        return {};
    }
    Path polyLine() const override {
        return {};
    }
    Paths polyLineW() const override {
        return {};
    }
    Path elipse() const override {
        return {};
    }
    Paths elipseW() const override {
        return {};
    }
    Path arc() const override {
        return {};
    }
    Path arcW() const override {
        return {};
    }
    Path polygon() const override {
        return {};
    }
    Paths polygonWholes() const override {
        return {};
    }
    Path hole() const override {
        return {};
    }
    Paths holes() const override {
        return {};
    }
    bool positive() const override {
        return {};
    }
    bool closed() const override {
        return {};
    }
    const Path& path() const override {
        return m_path;
    }
    const Paths& paths() const override {
        return m_paths;
    }
    Path& rPath() override {
        return m_path;
    }
    Paths& rPaths() override {
        return m_paths;
    }
};

using GraphicObjects = std::vector<GraphicObject>;

#if __cplusplus > 201703L
using Blocks = std::map<QString, Block*>;
using HeaderData = std::map<QString, std::map<int, QVariant>>;
using Layers = std::map<QString, Layer*>;
using Sections = std::map<int, SectionParser*>;
using Styles = std::map<QString, Style*>;
using Tables = std::map<int, QVector<AbstractTable*>>;
#else
struct Blocks : std::map<QString, Block*> {
    bool contains(const QString& key) const { return find(key) != end(); }
};
struct HeaderData : std::map<QString, std::map<int, QVariant>> {
    bool contains(const QString& key) const { return find(key) != end(); }
};
struct Layers : std::map<QString, Layer*> {
    bool contains(const QString& key) const { return find(key) != end(); }
};
struct Sections : std::map<int, SectionParser*> {
    bool contains(int key) const { return find(key) != end(); }
};
struct Styles : std::map<QString, Style*> {
    bool contains(const QString& key) const { return find(key) != end(); }
};
struct Tables : std::map<int, QVector<AbstractTable*>> {
    bool contains(int key) const { return find(key) != end(); }
};
#endif

class Settings {
protected:
    static inline QString m_defaultFont {"Arial"};
    static inline bool m_boldFont {false};
    static inline bool m_italicFont {false};
    static inline bool m_overrideFonts {false};

public:
    static QString defaultFont() { return m_defaultFont; }
    static bool boldFont() { return m_boldFont; }
    static bool italicFont() { return m_italicFont; }
    static bool overrideFonts() { return m_overrideFonts; }
};

} // namespace Hpgl
