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
#include "topor_file.h"

#include "topor_node.h"

#include "gi_datapath.h"
#include "gi_datasolid.h"

namespace TopoR {

File::File()
    : AbstractFile() {
    itemsType_ = int(ItemsType::Normal);
    layerTypes_ = {
        {int(ItemsType::Normal), TopoRObj::tr("Normal"), TopoRObj::tr("Displays filled pads/tracks.")   },
        {int(ItemsType::Paths),  TopoRObj::tr("Paths"),  TopoRObj::tr("Displays centerlines/outlines.")},
    };
}

File::~File() {
    for(Layer* layer: layers_) delete layer;
}

void File::setItemType(int type) {
    itemsType_ = type;
    for(Layer* layer: layers_)
        layer->setItemsType(ItemsType(itemsType_));
}

int File::itemsType() const { return itemsType_; }

Layer* File::layer(const QString& name, LayerKind kind) {
    for(Layer* l: layers_)
        if(l->name() == name) return l;
    Layer* l = new Layer{this, kind, name};
    layers_.push_back(l);
    return l;
}

void File::initFrom(AbstractFile* file) {
    AbstractFile::initFrom(file);
    static_cast<Node*>(node_)->file = this;
}

FileTree::Node* File::node() { return node_ ? node_ : node_ = new Node{this}; }

QIcon File::icon() const { return decoration(Qt::lightGray, u'T'); }

void File::createGi() {
    Gi::Group* igNorm = itemGroups_.back();
    Gi::Group* igPath = new Gi::Group;
    itemGroups_.push_back(igPath);

    int i{};
    mergedCurves_ = {};

    for(Layer* layer: layers_) {
        if(layer->graphicObjects().empty()) continue;

        if(i++) {
            itemGroups_.push_back(igNorm = new Gi::Group);
            itemGroups_.push_back(igPath = new Gi::Group);
        }

        Geo::Polygons fill;
        for(const GraphicObject& go: layer->graphicObjects()) {
            if(go.path.size() > 1) {
                auto gItem = new Gi::DataPath{{go.path}, this};
                gItem->setToolTip(go.name);
                gItem->setPenColorPtr(&layer->colorPath_);
                igPath->push_back(gItem);
            }
            if(!go.fill.empty()) fill |= go.fill;
        }

        for(const Geo::Polygon& polygon: fill) {
            auto gItem = new Gi::DataFill{Geo::Polygons{polygon}, this};
            gItem->setColorPtr(&layer->colorNorm_);
            igNorm->push_back(gItem);
        }

        mergedCurves_ |= fill;

        igNorm->shrink_to_fit();
        igPath->shrink_to_fit();
        layer->itemGroupNorm = igNorm;
        layer->itemGroupPath = igPath;

        const bool fresh = layer->itemsType_ == ItemsType::Null;
        if(fresh) layer->itemsType_ = igNorm->size() ? ItemsType::Normal : ItemsType::Paths;
        layer->setVisible(fresh ? true : layer->visible_);
        layer->applyItemsType();
    }

    groupedCurves_ = {};
}

bool File::isVisible() const { return visible_; }

void File::setVisible(bool visible) {
    if(visible == visible_) return;
    visible_ = visible;
    if(visible_) {
        for(const auto& [name, vis]: layersVisible_)
            for(Layer* layer: layers_)
                if(layer->name() == name) layer->setVisible(vis);
    } else {
        layersVisible_.clear();
        for(Layer* layer: layers_)
            if(!layer->isEmpty()) {
                layersVisible_[layer->name()] = layer->isVisible();
                layer->setVisible(false);
            }
    }
}

std::vector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test) const {
    std::vector<GraphicObject> retData;
    auto t = transform_.toQTransform();
    for(auto&& criterion: criterias) {
        for(Layer* layer: layers_) {
            if(!layer->isVisible()) continue;
            for(const auto& go: layer->graphicObjects()) {
                auto transformedGo = go * t;
                if(criterion.test(transformedGo)) {
                    auto& g = retData.emplace_back(transformedGo);
                    if(g.type & GraphicObject::Circle)
                        g.path.clear();
                    if(test) return retData;

                    switch(gcType) {
                    case GCType::Drill: {
                        const QRectF rect = g.fill.boundingRect();
                        double drillDiameter = std::min(rect.width(), rect.height());
                        g.raw = drillDiameter;
                        g.name = QString::number(drillDiameter);
                    } break;
                    default: break;
                    }
                }
            }
        }
    }
    return retData;
}

void File::preSave() const {
    if(!layersVisible_.size() && visible_)
        for(Layer* layer: layers_)
            if(!layer->isEmpty()) layersVisible_[layer->name()] = layer->isVisible();
    AbstractFile::preSave();
}

Geo::Polygons File::merge() const { return mergedCurves_; }

} // namespace TopoR
