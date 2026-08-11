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

// https://help.autodesk.com/view/OARX/2020/RUS/?guid=GUID-235B22E0-A567-4CF6-92D3-38A2306D73F3
#include "abstract_file.h"
#include "dxf_block.h"
#include "dxf_codedata.h"
#include "dxf_model3d.h"
#include "dxf_types.h"
#include "gi_group.h"

#include <QDebug>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QVector>
#include <algorithm>
#include <forward_list>

namespace Dxf {

class Layer;
class LayerModel;
class NodeLayer;
struct SectionParser;

class[[= Serial::name("Dxf")]] File : public AbstractFile {
    friend class LayerModel;
    friend class NodeLayer;
    friend class Plugin;
    friend struct SectionENTITIES;
    // NOTE use private crutch
    friend struct Serial::Adapter<Dxf::Layer*>;

public:
    explicit File();
    ~File();
    Layer* layer(const QString& name);
    HeaderData& header() { return header_; }

    const Layers& layers() const { return layers_; }
    Layers& layers() { return layers_; }

    Blocks& blocks() { return blocks_; }
    Styles& styles() { return styles_; }
    EntitiesUP& entities() { return entities_; }

    void setItemType(int type) override;
    int itemsType() const override;

    Model3D& mesh() { return mesh_; }
    // Создаёт по слою на каждый включённый в настройках вид, кладя туда
    // силуэт 3D-модели с этой стороны. Вызывается один раз после разбора файла;
    // при загрузке проекта слои приходят уже готовыми из потока.
    void createProjectionLayers();

private:
    // Разбор владеет секциями, блоками и стилями лишь пока идёт сам разбор:
    // всё, что нужно дальше, уже разложено по слоям. Сетка 3D тоже транзиент --
    // createProjectionLayers превращает её в слои и очищает.
    [[= Serial::skip]] Sections sections_;
    [[= Serial::skip]] Model3D mesh_;
    [[= Serial::skip]] Blocks blocks_;
    [[= Serial::skip]] Styles styles_;

    HeaderData header_;
    Layers layers_;
    EntitiesUP entities_;

    mutable std::map<QString, bool> layersVisible_;

    // Слой при чтении цепляется к текущему файлу — крюк ДО чтения полей.
    static inline File* crutch;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };
    Geo::Polygons& groupedPaths(Group group = CopperGroup, bool fl = false);

    // AbstractFile interface
public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    // Свёрнутый файл держит видимость слоёв в layersVisible_; у развёрнутого она
    // живёт в самих слоях, и снять её надо до записи.
    void preSave() const;
    void preLoad() { crutch = this; }

    void initFrom(AbstractFile* file) override;
    FileTree::Node* node() override;
    uint32_t type() const override;
    void createGi() override;
    bool isVisible() const override;
    void setVisible(bool visible) override;
    std::vector<GraphicObject> getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test = {}) const override;
    QIcon icon() const override { return QIcon::fromTheme(u"crosshairs"_s); }

protected:
    Geo::Polygons merge() const override;
};

} // namespace Dxf
