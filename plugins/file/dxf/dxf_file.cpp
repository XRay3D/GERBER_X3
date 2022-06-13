// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_file.h"

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_sectionparser.h"
#include "section/dxf_tables.h"

#include "entities/dxf_entity.h"
#include "tables/dxf_layer.h"

//#include "section/dxf_classes.h"
//#include "section/dxf_objects.h"
//#include "section/dxf_thumbnailimage.h"
//#include "gc_creator.h" //////////////////////

#include "gi_datapath.h"
#include "gi_datasolid.h"
#include "dxf_node.h"
#include "gi_gcpath.h"
#include "settings.h"

#include <QDebug>
#include <QElapsedTimer>

namespace Dxf {

File::File()
    : FileInterface() {
    itemsType_ = int(ItemsType::Normal);
    layerTypes_ = {
        { int(ItemsType::Normal), DxfObj::tr("Normal"), DxfObj::tr("Displays paths with pen width and fill.") },
        { int(ItemsType::Paths), DxfObj::tr("Paths"), DxfObj::tr("Displays paths without pen width.") },
        { int(ItemsType::Both), DxfObj::tr("Both"), DxfObj::tr("Displays paths without and with pen width.") },
    };
}

File::~File() {
    for (auto [k, v] : m_sections)
        delete v;
    for (const auto& [k, v] : m_blocks)
        delete v;
}

void File::setItemType(int type) {
    itemsType_ = type;
    for (const auto& [name, layer] : m_layers)
        layer->setItemsType(ItemsType(itemsType_));
}

int File::itemsType() const { return itemsType_; }

Pathss& File::groupedPaths(File::Group group, bool fl) {
    if (groupedPaths_.empty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddPaths(mergedPaths(), ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = /*uScale*/ 1;
        Path outer = {
            IntPoint(r.left - k, r.bottom + k),
            IntPoint(r.right + k, r.bottom + k),
            IntPoint(r.right + k, r.top - k),
            IntPoint(r.left - k, r.top - k)
        };
        if (fl)
            ReversePath(outer);
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftNonZero);
        grouping(polyTree.GetFirst(), &groupedPaths_, group);
    }
    return groupedPaths_;
}

void File::grouping(PolyNode* node, Pathss* pathss, File::Group group) {
    Path path;
    Paths paths;
    switch (group) {
    case CutoffGroup:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (size_t i = 0; i < node->ChildCount(); ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node->ChildCount(); ++i) {
            grouping(node->Childs[i], pathss, group);
        }
        break;
    case CopperGroup:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (size_t i = 0; i < node->ChildCount(); ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node->ChildCount(); ++i) {
            grouping(node->Childs[i], pathss, group);
        }
        break;
    }
}

void File::initFrom(FileInterface* file) {
    FileInterface::initFrom(file);
    static_cast<Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node(this, &id_);
}

Layer* File::layer(const QString& name) {
    if (m_layers.contains(name)) {
        return m_layers[name];
    } else {
        return m_layers[name] = new Layer(m_sections.begin()->second, name);
    }
    return nullptr;
}

FileType File::type() const { return FileType::Dxf; }

void File::createGi() {

    for (auto& [name, layer] : m_layers)
        for (auto& go : layer->m_graphicObjects)
            go.m_file = this;

    GiGroup* igNorm = itemGroups_.back();
    GiGroup* igPath = new GiGroup;
    itemGroups_.push_back(igPath);

    int i = 0;

    for (auto& [name, layer] : m_layers) {
        if (layer->m_graphicObjects.size()) {
            if (i++) {
                itemGroups_.push_back(igNorm = new GiGroup);
                itemGroups_.push_back(igPath = new GiGroup);
            }

            Clipper clipper; // Clipper

            for (auto& go : layer->m_graphicObjects) {
                if (layer->m_groupedPaths.empty() && go.paths().size())
                    clipper.AddPaths(go.paths(), ptSubject, true); // Clipper

                if (go.path().size() > 1) {
                    auto gItem = new GiDataPath(go.path(), this);
                    if (go.entity()) {
                        //                        gItem->setToolTip(QString("Line %1\n%2")
                        //                                              .arg(go.entity()->data[0].line())
                        //                                              .arg(go.entity()->name()));
                        gItem->setToolTip(go.entity()->name());
                    }
                    gItem->setPenColorPtr(&layer->m_colorPath);
                    igPath->push_back(gItem);
                }
            }

            if (layer->m_groupedPaths.empty()) {
                clipper.Execute(ctUnion, mergedPaths_, pftNonZero); // Clipper
                //                dbgPaths(m_mergedPaths, "m_mergedPaths", true);
                layer->m_groupedPaths = std::move(groupedPaths());
                for (auto& paths : layer->m_groupedPaths)
                    CleanPolygons(paths, uScale * 0.0005);
                mergedPaths_.clear();
            }

            for (Paths& paths : layer->m_groupedPaths) {
                auto gItem = new GiDataSolid(paths, this);
                gItem->setColorPtr(&layer->m_colorNorm);
                igNorm->push_back(gItem);
            }

            igNorm->shrink_to_fit();
            igPath->shrink_to_fit();
            layer->itemGroupNorm = igNorm;
            layer->itemGroupPath = igPath;

            if (layer->m_itemsType == ItemsType::Null) {
                layer->setItemsType(ItemsType::Normal);
                layer->setVisible(true);
            } else
                layer->setVisible(visible_);
        }
    }
}

bool File::isVisible() const { return visible_; }

void File::setVisible(bool visible) {
    if (visible == visible_)
        return;
    visible_ = visible;
    if (visible_) {
        for (const auto& [name, vis] : m_layersVisible) {
            m_layers[name]->setVisible(vis);
        }
    } else {
        m_layersVisible.clear();
        for (const auto& [name, layer] : m_layers) {
            if (!layer->isEmpty()) {
                m_layersVisible[name] = layer->isVisible();
                m_layers[name]->setVisible(false);
            }
        }
    }
}

void File::write(QDataStream& stream) const {
    stream << m_header;
    {
        stream << int(m_layers.size());
        for (auto& [name, layer] : m_layers) {
            stream << name;
            stream << *layer;
        }
    }
    stream << itemsType_;
    if (!m_layersVisible.size() && visible_) {
        for (const auto& [name, layer] : m_layers) {
            if (!layer->isEmpty()) {
                m_layersVisible[name] = layer->isVisible();
            }
        }
    }
    stream << m_layersVisible;
    stream << m_entities;
}

void File::read(QDataStream& stream) {
    stream >> m_header;
    {
        int size;
        stream >> size;
        while (size--) {
            QString name;
            Layer* layer = new Layer(this);
            stream >> name;
            stream >> *layer;
            m_layers[name] = layer;
        }
    }
    stream >> itemsType_;
    stream >> m_layersVisible;
    stream >> m_entities;
}

Paths File::merge() const { return mergedPaths_; }
} // namespace Dxf
