/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "abstract_file.h"
#include "tables/topor_component.h"
#include "tables/topor_layer.h"
#include "topor_types.h"

namespace TopoR {

class[[= Serial::name("TopoR")]] File : public AbstractFile {
    friend class Node;
    friend class NodeLayer;
    friend class Plugin;
    friend class Parser;
    // NOTE use private crutch
    friend struct ::Serial::Adapter<TopoR::Layer*>;

public:
    explicit File();
    ~File() override;

    // Ищет слой по имени (как Dxf::File::layer); заводит новый с указанным
    // Kind, если такого имени ещё нет.
    Layer* layer(const QString& name, LayerKind kind);

    const LayerList& layers() const { return layers_; }
    LayerList& layers() { return layers_; }

    const TopoR::Components& components() const { return components_; }
    TopoR::Components& components() { return components_; }

    void setItemType(int type) override;
    int itemsType() const override;

    // AbstractFile interface
public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    // Свёрнутый файл держит видимость слоёв в layersVisible_, как у Dxf::File.
    void preSave() const;
    void preLoad() { crutch = this; }

    void initFrom(AbstractFile* file) override;
    FileTree::Node* node() override;
    uint32_t type() const override { return TOPOR; }
    void createGi() override;
    bool isVisible() const override;
    void setVisible(bool visible) override;
    std::vector<GraphicObject> getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test = {}) const override;
    QIcon icon() const override;

protected:
    Geo::Polygons merge() const override;

private:
    LayerList layers_;
    TopoR::Components components_;

    mutable std::map<QString, bool> layersVisible_;

    // Слой при чтении цепляется к текущему файлу — крюк ДО чтения полей.
    static inline File* crutch;
};

} // namespace TopoR

Q_DECLARE_METATYPE(TopoR::File)
