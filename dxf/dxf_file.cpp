#include "dxf_file.h"

#include "section/dxf_blocks.h"
#include "section/dxf_classes.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_objects.h"
#include "section/dxf_sectionparser.h"
#include "section/dxf_tables.h"
#include "section/dxf_thumbnailimage.h"

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
    //    Layer* l;
    //    m_tables[TableItem::LAYER].append(l = new Layer(this));
    //    m_tables[TableItem::LAYER].last()->parse(code);
    //    m_layers[l->name] = l;
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
    if (m_itemsType == type)
        return;
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

//struct Check {
//    double area = 0.0;
//    double len = 0.0;
//    int size = 0;
//    bool operator==(Check ch)
//    {
//        return qFuzzyCompare(area, ch.area)
//            && qFuzzyCompare(len, ch.len)
//            && size == ch.size;
//    }
//};
//bool operator==(Check l, Check r)
//{
//    return qFuzzyCompare(l.area, r.area)
//        && qFuzzyCompare(l.len, r.len)
//        && l.size == r.size;
//}

void File::createGi()
{
    ItemGroup* igNorm = m_itemGroups.last();
    ItemGroup* igPath = new ItemGroup;
    m_itemGroups.append(igPath);

    int i = 0;

    //    QVector<Check> checkList;
    auto contains = [&](const Path& path) -> bool {
        return false;
        //        constexpr double k = 0.001 * uScale;
        //        for (const Path& chPath : checkList) { // find copy
        //            int counter = 0;
        //            if (chPath.size() == path.size()) {
        //                for (const Point64& p1 : chPath) {
        //                    for (const Point64& p2 : path) {
        //                        if ((abs(p1.X - p2.X) < k) && (abs(p1.Y - p2.Y) < k)) {
        //                            ++counter;
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //            if (counter == path.size())
        //                return true;
        //        }
        //        return false;
        //        Check ch;
        //        ch.area = abs(Area(path));
        //        ch.len = Perimeter(path);
        //        ch.size = path.size();
        //        if (checkList.contains(ch))
        //            return true;
        //        else
        //            checkList.append(ch);
        //        return false;
    };

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
                if (!contains(go.path())) {
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
                m_mergedPaths.clear();
            }

            for (Paths& paths : layer->m_groupedPaths) {
                auto gItem = new GiGerber(paths, this);
                gItem->setColorP(&layer->m_colorNorm);
                igNorm->append(gItem);
            }
            layer->itemGroupNorm = igNorm;
            layer->itemGroupPath = igPath;

            if (layer->m_itemsType == ItemsType::Null)
                layer->m_itemsType = ItemsType::Normal;

            if (layer->m_itemsType == ItemsType::Normal) {
                layer->itemGroupNorm->setVisible(true);
                layer->itemGroupPath->setVisible(false);
            } else {
                layer->itemGroupNorm->setVisible(false);
                layer->itemGroupPath->setVisible(true);
            }
        }
    }
}

bool File::isVisible() const { return m_visible; }

void File::setVisible(bool visible)
{
    //    if (m_visible == visible)
    //        return;
    m_visible = visible;
    if (m_visible) {
        for (auto [name, visible] : m_layersVisible) {
            setVisibleLayer(name, visible);
        }
    } else {
        m_layersVisible.clear();
        for (auto [name, layer] : m_layers) {
            if (layer->itemGroupNorm && layer->itemGroupPath) {
                m_layersVisible[name] = visibleLayer(name);
                setVisibleLayer(name, false);
            }
        }
    }
}

void File::setVisibleLayer(const QString& name, bool visible)
{
    if (m_itemsType == ItemsType::Normal)
        m_layers[name]->itemGroupNorm->setVisible(visible);
    else
        m_layers[name]->itemGroupPath->setVisible(visible);
}

bool File::visibleLayer(const QString& name) const
{
    if (m_itemsType == ItemsType::Normal)
        return m_layers.at(name)->itemGroupNorm->isVisible();
    else
        return m_layers.at(name)->itemGroupPath->isVisible();
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
