#pragma once

#include "entities/dxf_graphicobject.h"
#include <QColor>
#include <map>
#include <vector>

namespace Dxf {

enum class ItemsType {
    Null = -1,
    Normal,
    Paths,
};

class Color : public QColor {
public:
    Color(double r, double g, double b)
        : QColor(r * 255, g * 255, b * 255)
    {
    }
};

extern const Color dxfColors[];

class Layer;
struct Block;
struct SectionParser;
struct AbstractTable;

using GraphicObjects = std::vector<GraphicObject>;

#if __cplusplus > 201703L
using Blocks = std::map<QString, Block*>;
using HeaderData = std::map<QString, std::map<int, QVariant>>;
using Layers = std::map<QString, Layer*>;
using Sections = std::map<int, SectionParser*>;
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
struct Tables : std::map<int, QVector<AbstractTable*>> {
    bool contains(int key) const { return find(key) != end(); }
};
#endif

}
