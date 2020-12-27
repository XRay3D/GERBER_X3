// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "dxf_file.h"

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_sectionparser.h"
#include "section/dxf_tables.h"
//#include "section/dxf_classes.h"
//#include "section/dxf_objects.h"
//#include "section/dxf_thumbnailimage.h"

#include "entities/dxf_entity.h"

#include "tables/dxf_layer.h"

#include "gi/aperturepathitem.h"
#include "gi/gerberitem.h"
#include "gi/pathitem.h"
#include "settings.h"

#include <QDebug>
#include <QElapsedTimer>

namespace Dxf {

File::File()
{
    m_layerTypes = {
        { int(ItemsType::Normal), DxfObj::tr("Normal"), DxfObj::tr("Displays paths with pen width and fill.") },
        { int(ItemsType::Paths), DxfObj::tr("Paths"), DxfObj::tr("Displays paths without pen width.") },
    };
}

File::~File()
{
    for (auto [k, v] : m_sections)
        delete v;
    for (auto [k, v] : m_blocks)
        delete v;
}

void File::setItemType(ItemsType type)
{
    m_itemsType = type;
    for (auto [name, layer] : m_layers) {
        if (layer->itemGroupNorm && layer->itemGroupPath) {
            layer->m_itemsType = m_itemsType;
            if (m_itemsType == ItemsType::Normal) {
                layer->itemGroupNorm->setVisible(layer->itemGroupPath->isVisible());
                layer->itemGroupPath->setVisible(false);
            } else {
                layer->itemGroupPath->setVisible(layer->itemGroupNorm->isVisible());
                layer->itemGroupNorm->setVisible(false);
            }
        }
    }
}

ItemsType File::itemsType() const { return m_itemsType; }

Pathss& File::groupedPaths(File::Group group, bool fl)
{
    if (m_groupedPaths.isEmpty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddPaths(mergedPaths(), ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = /*uScale*/ 1;
        Path outer = {
            Point64(r.left - k, r.bottom + k),
            Point64(r.right + k, r.bottom + k),
            Point64(r.right + k, r.top - k),
            Point64(r.left - k, r.top - k)
        };
        if (fl)
            ReversePath(outer);
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftNonZero);
        grouping(polyTree.GetFirst(), &m_groupedPaths, group);
    }
    return m_groupedPaths;
}

void File::grouping(PolyNode* node, Pathss* pathss, File::Group group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CutoffGroup:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int var = 0; var < node->ChildCount(); ++var) {
                path = node->Childs[var]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int var = 0; var < node->ChildCount(); ++var) {
            grouping(node->Childs[var], pathss, group);
        }
        break;
    case CopperGroup:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int var = 0; var < node->ChildCount(); ++var) {
                path = node->Childs[var]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int var = 0; var < node->ChildCount(); ++var) {
            grouping(node->Childs[var], pathss, group);
        }
        break;
    }
}

Layer* File::layer(const QString& name)
{
    if (m_layers.contains(name)) {
        return m_layers[name];
    } else {
        return m_layers[name] = new Layer(m_sections.begin()->second, name);
    }
    return nullptr;
}

ItemGroup* File::itemGroup() const
{
    return m_itemGroups.first();
}

FileType File::type() const { return FileType::Dxf; }

void File::createGi()
{
    ItemGroup* igNorm = m_itemGroups.last();
    ItemGroup* igPath = new ItemGroup;
    m_itemGroups.append(igPath);

    int i = 0;

    for (auto& [name, layer] : m_layers) {
        if (layer->m_graphicObjects.size()) {
            if (i++) {
                m_itemGroups.append(igNorm = new ItemGroup);
                m_itemGroups.append(igPath = new ItemGroup);
            }

            Clipper clipper; // Clipper

            for (auto& go : layer->m_graphicObjects) {
                if (layer->m_groupedPaths.isEmpty()) {
                    clipper.AddPath(go.path(), ptClip, true); // Clipper
                    clipper.AddPaths(go.paths(), ptClip, true); // Clipper
                }
                if (go.path().size()) {
                    auto gItem = new AperturePathItem(go.path(), this);
                    if (go.entity())
                        gItem->setToolTip(QString("Line %1\n%2")
                                              .arg(go.entity()->data[0].line())
                                              .arg(go.entity()->name()));
                    gItem->setPenColor(&layer->m_colorPath);
                    igPath->append(gItem);
                }
            }

            if (layer->m_groupedPaths.isEmpty()) {
                clipper.Execute(ctUnion, m_mergedPaths, pftPositive); // Clipper
                layer->m_groupedPaths = std::move(groupedPaths());
                for (auto& paths : layer->m_groupedPaths)
                    CleanPolygons(paths, uScale * 0.0005);
                m_mergedPaths.clear();
            }

            for (Paths& paths : layer->m_groupedPaths) {
                auto gItem = new GiGerber(paths, this);
                gItem->setColorP(&layer->m_colorNorm);
                igNorm->append(gItem);
            }
            layer->itemGroupNorm = igNorm;
            layer->itemGroupPath = igPath;

            if (layer->m_itemsType == ItemsType::Null) {
                layer->m_itemsType = ItemsType::Normal;
                layer->setVisible(true);
            } else
                layer->setVisible(m_visible);
        }
    }
}

bool File::isVisible() const { return m_visible; }

void File::setVisible(bool visible)
{
    if (visible == m_visible)
        return;
    m_visible = visible;
    if (m_visible) {
        for (auto [name, vis] : m_layersVisible) {
            m_layers[name]->setVisible(vis);
        }
    } else {
        m_layersVisible.clear();
        for (auto [name, layer] : m_layers) {
            if (!layer->isEmpty()) {
                m_layersVisible[name] = layer->isVisible();
                m_layers[name]->setVisible(false);
            }
        }
    }
}

void File::write(QDataStream& stream) const
{
    stream << m_header;
    {
        stream << int(m_layers.size());
        for (auto& [name, layer] : m_layers) {
            stream << name;
            stream << *layer;
        }
    }
    stream << m_itemsType;
    stream << m_layersVisible;
}

void File::read(QDataStream& stream)
{
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
    stream >> m_itemsType;
    stream >> m_layersVisible;
}

Paths File::merge() const { return m_mergedPaths; }
}
