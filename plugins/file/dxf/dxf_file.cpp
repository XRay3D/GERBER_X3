// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
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

//////////////////////

#include "dxf_node.h"
#include "gi_datapath.h"
#include "gi_datasolid.h"
#include "utils.h"

#include <QDebug>
#include <QElapsedTimer>

namespace Dxf {

File::File()
    : FileInterface() {
    itemsType_ = int(ItemsType::Normal);
    layerTypes_ = {
        {int(ItemsType::Normal), DxfObj::tr("Normal"),    DxfObj::tr("Displays paths with pen width and fill.")},
        { int(ItemsType::Paths),  DxfObj::tr("Paths"),          DxfObj::tr("Displays paths without pen width.")},
        {  int(ItemsType::Both),   DxfObj::tr("Both"), DxfObj::tr("Displays paths without and with pen width.")},
    };
}

File::~File() {
    for (auto [k, v] : sections_)
        delete v;
    for (const auto& [k, v] : blocks_)
        delete v;
}

void File::setItemType(int type) {
    itemsType_ = type;
    for (const auto& [name, layer] : layers_)
        layer->setItemsType(ItemsType(itemsType_));
}

int File::itemsType() const { return itemsType_; }

Pathss& File::groupedPaths(File::Group group, bool fl) {
    if (groupedPaths_.empty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddSubject(mergedPaths());
        Rect r(Bounds(mergedPaths()));
        int k = /*uScale*/ 1;
        Path outer = {
            Point(r.left - k, r.bottom + k),
            Point(r.right + k, r.bottom + k),
            Point(r.right + k, r.top - k),
            Point(r.left - k, r.top - k)};
        if (fl)
            ReversePath(outer);
        clipper.AddSubject({outer});
        clipper.Execute(ClipType::Union, FillRule::NonZero, polyTree);
        grouping(polyTree, group);
    }
    return groupedPaths_;
}

void File::grouping(PolyTree& node, File::Group group) {
    Path path;
    Paths paths;
    switch (group) {
    case CutoffGroup:
        if (!node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for (size_t i = 0; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            groupedPaths_.push_back(paths);
        }
        for (size_t i = 0; i < node.Count(); ++i) {
            grouping(*node[i], group);
        }
        break;
    case CopperGroup:
        if (node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for (size_t i = 0; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            groupedPaths_.push_back(paths);
        }
        for (size_t i = 0; i < node.Count(); ++i) {
            grouping(*node[i], group);
        }
        break;
    }
}

void File::initFrom(FileInterface* file) {
    FileInterface::initFrom(file);
    static_cast<Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node(this);
}

Layer* File::layer(const QString& name) {
    if (layers_.contains(name)) {
        return layers_[name];
    } else {
        return layers_[name] = new Layer(sections_.begin()->second, name);
    }
    return nullptr;
}

FileType File::type() const { return FileType::Dxf; }

void File::createGi() {
    Timer t {__FUNCTION__};

    for (auto& [name, layer] : layers_)
        for (auto& go : layer->graphicObjects_)
            go.file_ = this;

    GiGroup* igNorm = itemGroups_.back();
    GiGroup* igPath = new GiGroup;
    itemGroups_.push_back(igPath);

    int i = 0;

    for (auto& [name, layer] : layers_) {
        if (layer->graphicObjects_.size()) {
            if (i++) {
                itemGroups_.push_back(igNorm = new GiGroup);
                itemGroups_.push_back(igPath = new GiGroup);
            }

            Clipper clipper; // Clipper
            const bool empty {layer->groupedPaths_.empty()};
            for (auto& go : layer->graphicObjects_) {
                if (empty && go.paths().size())
                    clipper.AddSubject(go.paths());

                if (go.path().size() > 1) {
                    auto gItem = new GiDataPath(go.path(), this);
                    if (go.entity()) {
                        //                        gItem->setToolTip(QString("Line %1\n%2")
                        //                                              .arg(go.entity()->data[0].line())
                        //                                              .arg(go.entity()->name()));
                        gItem->setToolTip(go.entity()->name());
                    }
                    gItem->setPenColorPtr(&layer->colorPath_);
                    igPath->push_back(gItem);
                }
            }

            if (empty) {
                clipper.Execute(ClipType::Union, FillRule::NonZero, mergedPaths_);
                layer->groupedPaths_ = std::move(groupedPaths());
                for (auto& paths : layer->groupedPaths_)
                    CleanPolygons(paths, uScale * 0.0005);
                mergedPaths_.clear();
            }

            for (Paths& paths : layer->groupedPaths_) {
                auto gItem = new GiDataSolid(paths, this);
                gItem->setColorPtr(&layer->colorNorm_);
                igNorm->push_back(gItem);
            }

            igNorm->shrink_to_fit();
            igPath->shrink_to_fit();
            layer->itemGroupNorm = igNorm;
            layer->itemGroupPath = igPath;

            if (layer->itemsType_ == ItemsType::Null) {
                layer->setItemsType(ItemsType::Both);
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
        for (const auto& [name, vis] : layersVisible_) {
            layers_[name]->setVisible(vis);
        }
    } else {
        layersVisible_.clear();
        for (const auto& [name, layer] : layers_) {
            if (!layer->isEmpty()) {
                layersVisible_[name] = layer->isVisible();
                layers_[name]->setVisible(false);
            }
        }
    }
}

void File::write(QDataStream& stream) const {
    stream << header_;
    {
        stream << int(layers_.size());
        for (auto& [name, layer] : layers_) {
            stream << name;
            stream << *layer;
        }
    }
    stream << itemsType_;
    if (!layersVisible_.size() && visible_) {
        for (const auto& [name, layer] : layers_) {
            if (!layer->isEmpty()) {
                layersVisible_[name] = layer->isVisible();
            }
        }
    }
    stream << layersVisible_;
    stream << entities_;
}

void File::read(QDataStream& stream) {
    stream >> header_;
    {
        int size;
        stream >> size;
        while (size--) {
            QString name;
            Layer* layer = new Layer(this);
            stream >> name;
            stream >> *layer;
            layers_[name] = layer;
        }
    }
    stream >> itemsType_;
    stream >> layersVisible_;
    stream >> entities_;
}

Paths File::merge() const { return mergedPaths_; }

} // namespace Dxf
