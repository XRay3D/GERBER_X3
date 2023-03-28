// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "hpgl_file.h"

// #include "section/dxf_classes.h"
// #include "section/dxf_objects.h"
// #include "section/dxf_thumbnailimage.h"
// #include "gc_creator.h" //////////////////////

#include "gi_datapath.h"
#include "gi_datasolid.h"
#include "gi_gcpath.h"
#include "hpgl_node.h"
#include "settings.h"

#include <QDebug>
#include <QElapsedTimer>

namespace Hpgl {

File::File()
    : AbstractFile() {
    m_itemsType = int(ItemsType::Normal);
    m_layerTypes = {
        {int(ItemsType::Normal), DxfObj::tr("Normal"), DxfObj::tr("Displays paths with pen width and fill.")},
        { int(ItemsType::Paths),  DxfObj::tr("Paths"),       DxfObj::tr("Displays paths without pen width.")},
    };
}

File::~File() {
}

void File::setItemType(int type) {
    m_itemsType = type;
}

int File::itemsType() const { return m_itemsType; }

Pathss& File::groupedPaths(File::Group group, bool fl) {
    if (m_groupedPaths.empty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddPaths(mergedPaths(), ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = /*uScale*/ 1;
        Path outer = {
            IntPoint(r.left - k, r.bottom + k),
            IntPoint(r.right + k, r.bottom + k),
            IntPoint(r.right + k, r.top - k),
            IntPoint(r.left - k, r.top - k)};
        if (fl)
            ReversePath(outer);
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftNonZero);
        grouping(polyTree.GetFirst(), &m_groupedPaths, group);
    }
    return m_groupedPaths;
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

void File::initFrom(AbstractFile* file) {
    //    AbstractFile::initFrom(file);
    //    static_cast<Node*>(m_node)->file = this;
}

FileTree_::Node* File::node() {
    return m_node ? m_node : m_node = new Node(this, &m_id);
}

FileType File::type() const { return FileType::Hpgl; }

void File::createGi() {
    //    ItemGroup* igNorm = itemGroups_.back();
    //    ItemGroup* igPath = new ItemGroup;
    //    itemGroups_.push_back(igPath);

    //    int i = 0;

    //    for (auto& [name, layer] : m_layers) {
    //        if (layer->m_graphicObjects.size()) {
    //            if (i++) {
    //                itemGroups_.push_back(igNorm = new ItemGroup);
    //                itemGroups_.push_back(igPath = new ItemGroup);
    //            }

    //            Clipper clipper; // Clipper

    //            for (auto& go : layer->m_graphicObjects) {
    //                if (layer->m_groupedPaths.empty() && go.paths().size())
    //                    clipper.AddPaths(go.paths(), ptSubject, true); // Clipper

    //                if (go.path().size() > 1) {
    //                    auto gItem = new GiDataPath(go.path(), this);
    //                    if (go.entity()) {
    //                        gItem->setToolTip(QString("Line %1\n%2")
    //                                              .arg(go.entity()->data[0].line())
    //                                              .arg(go.entity()->name()));
    //                    }
    //                    gItem->setPenColorPtr(&layer->m_colorPath);
    //                    igPath->push_back(gItem);
    //                }
    //            }

    //            if (layer->m_groupedPaths.empty()) {
    //                clipper.Execute(ctUnion, m_mergedPaths, pftNonZero); // Clipper
    //                //                dbgPaths(m_mergedPaths, "m_mergedPaths", true);
    //                layer->m_groupedPaths = std::move(groupedPaths());
    //                for (auto& paths : layer->m_groupedPaths)
    //                    CleanPolygons(paths, uScale * 0.0005);
    //                m_mergedPaths.clear();
    //            }

    //            for (Paths& paths : layer->m_groupedPaths) {
    //                auto gItem = new GiDataSolid(paths, this);
    //                gItem->setColorPtr(&layer->m_colorNorm);
    //                igNorm->push_back(gItem);
    //            }

    //            igNorm->shrink_to_fit();
    //            igPath->shrink_to_fit();
    //            layer->itemGroupNorm = igNorm;
    //            layer->itemGroupPath = igPath;

    //            if (layer->m_itemsType == ItemsType::Null) {
    //                layer->setItemsType(ItemsType::Normal);
    //                layer->setVisible(true);
    //            } else
    //                layer->setVisible(m_visible);
    //        }
    //    }
}

bool File::isVisible() const { return m_visible; }

void File::setVisible(bool visible) {
    if (visible == m_visible)
        return;
    m_visible = visible;
    //    if (m_visible) {
    //        for (const auto& [name, vis] : m_layersVisible) {
    //            m_layers[name]->setVisible(vis);
    //        }
    //    } else {
    //        m_layersVisible.clear();
    //        for (const auto& [name, layer] : m_layers) {
    //            if (!layer->isEmpty()) {
    //                m_layersVisible[name] = layer->isVisible();
    //                m_layers[name]->setVisible(false);
    //            }
    //        }
    //    }
}

void File::write(QDataStream& stream) const {
    //    stream << m_header;
    //    {
    //        stream << int(m_layers.size());
    //        for (auto& [name, layer] : m_layers) {
    //            stream << name;
    //            stream << *layer;
    //        }
    //    }
    //    stream << m_itemsType;
    //    if (!m_layersVisible.size() && m_visible) {
    //        for (const auto& [name, layer] : m_layers) {
    //            if (!layer->isEmpty()) {
    //                m_layersVisible[name] = layer->isVisible();
    //            }
    //        }
    //    }
    //    stream << m_layersVisible;
}

void File::read(QDataStream& stream) {
    //    stream >> m_header;
    //    {
    //        int size;
    //        stream >> size;
    //        while (size--) {
    //            QString name;
    //            Layer* layer = new Layer(this);
    //            stream >> name;
    //            stream >> *layer;
    //            m_layers[name] = layer;
    //        }
    //    }
    //    stream >> m_itemsType;
    //    stream >> m_layersVisible;
}

Paths File::merge() const { return m_mergedPaths; }
} // namespace Hpgl
