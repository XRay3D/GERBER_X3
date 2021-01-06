/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

// https://help.autodesk.com/view/OARX/2020/RUS/?guid=GUID-235B22E0-A567-4CF6-92D3-38A2306D73F3
#include "dxf_block.h"
#include "dxf_codedata.h"
#include "dxf_types.h"
#include "interfaces/file.h"
#include "itemgroup.h"

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
    friend QDataStream& operator>>(QDataStream& stream, SectionParser*& sp);

public:
    explicit File();
    ~File();
    Layer* layer(const QString& name);
    HeaderData& header() { return m_header; }
    Layers& layers() { return m_layers; }
    Blocks& blocks() { return m_blocks; }
    Styles& styles() { return m_styles; }

    void setItemType(int type) override;
    int itemsType() const override;

private:
    Sections m_sections;
    Blocks m_blocks;
    HeaderData m_header;
    Layers m_layers;
    Styles m_styles;
    mutable std::map<QString, bool> m_layersVisible;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);
    void grouping(PolyNode* node, Pathss* pathss, Group group);

    // FileInterface interface
public:
    void initFrom(FileInterface* file) override;
    FileType type() const override;
    void createGi() override;
    bool isVisible() const override;
    void setVisible(bool visible) override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Paths merge() const override;
};

}
