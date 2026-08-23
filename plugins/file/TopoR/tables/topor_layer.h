/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "plugintypes.h"
#include "serial.h"
#include "topor_types.h"

#include <QColor>
#include <vector>

namespace Gi {
class Group;
}

namespace TopoR {

class File;

// Слои TopoR — как у DXF: один физический .fst -> один AbstractFile с
// внутренней таблицей слоёв, а не файл на каждый слой (см. план в
// /home/x-ray/.claude/plans/topor-silly-riddle.md). Медь/шёлк/маска -- по
// стороне платы; vias и раскладка компонентов -- тоже просто слои, для
// единообразия с DXF.
enum class LayerKind {
    CopperTop,
    CopperInner, // внутренний слой многослойной платы -- различаются именем, не kind
    CopperBottom,
    SilkTop,
    SilkBottom,
    MaskTop,
    MaskBottom,
    BoardOutline,
    Vias,
    Components,
};

// Normal/Paths -- тот же переключатель вида заливка/траектория, что у
// Gerber::File::ItemsType и Dxf::ItemsType, только здесь на уровне ОТДЕЛЬНОГО
// слоя, а не всего файла.
enum class ItemsType {
    Null = -1,
    Normal,
    Paths
};

class Layer {
    friend class File;
    // NOTE use private crutch
    friend struct ::Serial::Adapter<TopoR::Layer*>;

public:
    explicit Layer(File* file, LayerKind kind = {}, QString name = {});
    ~Layer() = default;

    QString name() const { return name_; }
    LayerKind kind() const { return kind_; }
    File* file() const { return file_; }

    const std::vector<GraphicObject>& graphicObjects() const { return graphicObjects_; }
    std::vector<GraphicObject>& graphicObjects() { return graphicObjects_; }
    void addGraphicObject(GraphicObject&& go) { graphicObjects_.emplace_back(std::move(go)); }

    Gi::Group* itemGroup() const;
    bool isEmpty() const;

    ItemsType itemsType() const { return itemsType_; }
    void setItemsType(ItemsType itemsType);
    // Показать группы согласно ТЕКУЩЕМУ типу -- см. Dxf::Layer::applyItemsType,
    // вызывается отдельно от setItemsType сразу после createGi().
    void applyItemsType();

    QColor color() const;
    void setColor(const QColor& color);

    bool isVisible() const { return visible_; }
    void setVisible(bool visible);

private:
    File* file_ = nullptr;

    [[= Serial::skip]] Gi::Group* itemGroupNorm = nullptr;
    [[= Serial::skip]] Gi::Group* itemGroupPath = nullptr;

    QColor colorNorm_{Qt::darkGreen};
    QColor colorPath_{Qt::darkRed};

    QString name_;
    LayerKind kind_{};

    std::vector<GraphicObject> graphicObjects_;
    ItemsType itemsType_ = ItemsType::Null;
    bool visible_ = true;
};

// "Layers" -- уже занято схемой (раздел <Layers> файла .fst, TopoR::Layers в
// TopoR_PCB_File.h), поэтому список слоёв дерева называется LayerList.
using LayerList = std::vector<Layer*>;

} // namespace TopoR

// Слои живут в File по указателю (как Dxf::Layers -- map<QString, Layer*>):
// писать разыменованный объект, а при чтении заводить новый слой, привязанный
// к текущему файлу (крюк File::crutch, как у Dxf::Layer/апертур Gerber). Тела
// в topor_layer.cpp.
template <>
struct Serial::Adapter<TopoR::Layer*> {
    static void write(Writer& sb, TopoR::Layer* const& layer);
    static simdjson::error_code read(simdjson::ondemand::value& val, TopoR::Layer*& layer);
};
