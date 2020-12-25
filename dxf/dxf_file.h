#pragma once

// https://help.autodesk.com/view/OARX/2020/RUS/?guid=GUID-235B22E0-A567-4CF6-92D3-38A2306D73F3
#include "abstractfile.h"
#include "dxf_block.h"
#include "dxf_codedata.h"
#include "dxf_values.h"
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QVector>
#include <algorithm>
#include <forward_list>
#include <gi/itemgroup.h>

namespace Dxf {

class LayerModel;
class Layer;
struct SectionParser;

class File : public AbstractFile {
    friend class Parser;
    friend class LayerModel;
    friend QDataStream& operator>>(QDataStream& stream, SectionParser*& sp);

public:
    explicit File();
    ~File();
    Layer* layer(const QString& name);
    HeaderData& header() { return m_header; }
    Layers& layers() { return m_layers; }
    Blocks& blocks() { return m_blocks; }

    void setItemType(ItemsType type);
    ItemsType itemsType() const;

private:
    Sections m_sections;
    Blocks m_blocks;
    HeaderData m_header;
    Layers m_layers;
    std::map<QString, bool> m_layersVisible;
    ItemsType m_itemsType = ItemsType::Normal;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);
    void grouping(PolyNode* node, Pathss* pathss, Group group);

    // AbstractFile interface
public:
    ItemGroup* itemGroup() const override;
    FileType type() const override;
    void createGi() override;
    bool isVisible() const override;
    void setVisible(bool visible) override;
    void setVisibleLayer(const QString& name, bool visible);
    bool visibleLayer(const QString& name) const;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Paths merge() const override;
};

}
