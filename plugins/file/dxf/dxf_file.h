/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

// https://help.autodesk.com/view/OARX/2020/RUS/?guid=GUID-235B22E0-A567-4CF6-92D3-38A2306D73F3
#include "dxf_block.h"
#include "dxf_codedata.h"
#include "dxf_types.h"
#include "fileifce.h"
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

class File : public FileInterface {
    friend class LayerModel;
    friend class NodeLayer;
    friend class Plugin;
    friend struct SectionENTITIES;
    friend QDataStream& operator>>(QDataStream& stream, SectionParser*& sp);

public:
    explicit File();
    ~File();
    Layer* layer(const QString& name);
    HeaderData& header() { return header_; }
    Layers& layers() { return layers_; }
    Blocks& blocks() { return blocks_; }
    Styles& styles() { return styles_; }
    EntitiesUP& entities() { return entities_; }

    void setItemType(int type) override;
    int itemsType() const override;

private:
    Sections sections_;
    Blocks blocks_;
    HeaderData header_;
    Layers layers_;
    Styles styles_;
    EntitiesUP entities_;

    mutable std::map<QString, bool> layersVisible_;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);
    void grouping(PolyTree& node, Group group);

    // FileInterface interface
public:
    void initFrom(FileInterface* file) override;
    FileTree::Node* node() override;
    FileType type() const override;
    void createGi() override;
    bool isVisible() const override;
    void setVisible(bool visible) override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Paths merge() const override;
};

} // namespace Dxf
