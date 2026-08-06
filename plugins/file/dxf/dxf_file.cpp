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
#include <gi_dbg.h>

namespace Dxf {

File::File()
    : AbstractFile() {
    itemsType_ = int(ItemsType::Normal);
    layerTypes_ = {
        {int(ItemsType::Normal), DxfObj::tr("Normal"), DxfObj::tr("Displays paths with pen width and fill.")   },
        {int(ItemsType::Paths),  DxfObj::tr("Paths"),  DxfObj::tr("Displays paths without pen width.")         },
        {int(ItemsType::Both),   DxfObj::tr("Both"),   DxfObj::tr("Displays paths without and with pen width.")},
    };
}

File::~File() {
    for(auto [k, v]: sections_)
        delete v;
    for(const auto& [k, v]: blocks_)
        delete v;
}

void File::setItemType(int type) {
    itemsType_ = type;
    for(const auto& [name, layer]: layers_)
        layer->setItemsType(ItemsType(itemsType_));
}

int File::itemsType() const { return itemsType_; }

Curvess& File::groupedPaths(File::Group group, bool fl) {
    // if(groupedCurves_.empty()) {
    //     PolyTree polyTree;
    //     cl::Clipper64 clipper;
    //     clipper.AddSubject(toPaths(mergedCurves()));
    //     auto r = BoundingRect(mergedCurves());
    //     int k = /*uScale*/ 1;
    //     Path64 outer{
    //         Point64{uScale + r.left() - k,  uScale + r.bottom() + k},
    //         Point64{uScale + r.right() + k, uScale + r.bottom() + k},
    //         Point64{uScale + r.right() + k, uScale + r.top() - k   },
    //         Point64{uScale + r.left() - k,  uScale + r.top() - k   }
    //     };
    //     if(!fl) ReversePath(outer);
    //     clipper.AddSubject({outer});
    //     clipper.Execute(cl::ClipType::Union, cl::FillRule::NonZero, polyTree);
    //     grouping(polyTree, group);
    // }
    // return groupedCurves_;
    if(groupedCurves_.empty()) {
        PolyTree polyTree;
        cl::Clipper64 clipper;
        auto paths = toPaths(mergedCurves());
        clipper.AddSubject(paths);
        Rect r = GetBounds(paths);
        int k = /*uScale*/ 1;
        Path64 outer{
            Point64(r.left - k, r.bottom + k),
            Point64(r.right + k, r.bottom + k),
            Point64(r.right + k, r.top - k),
            Point64(r.left - k, r.top - k)};
        if(fl)
            ReversePath(outer);
        clipper.AddSubject({outer});
        clipper.Execute(cl::ClipType::Union, cl::FillRule::NonZero, polyTree);
        grouping(polyTree, group);
        // Gi::Debug(groupedCurves_ | v::join | r::to<std::vector>());
    }
    return groupedCurves_;
}

void File::grouping(PolyTree& node, File::Group group) {
    Path64 path;
    Paths64 paths;
    switch(group) {
    case CutoffGroup:
        if(!node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for(size_t i{}; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            groupedCurves_.push_back(toCurves(paths));
            r::for_each(groupedCurves_ | v::join, [](Curve& c) { c.emplace_back(c.front().pt); });
        }
        for(size_t i{}; i < node.Count(); ++i)
            grouping(*node[i], group);
        break;
    case CopperGroup:
        if(node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for(size_t i{}; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            groupedCurves_.push_back(toCurves(paths));
            r::for_each(groupedCurves_ | v::join, [](Curve& c) { c.emplace_back(c.front().pt); });
        }
        for(size_t i{}; i < node.Count(); ++i)
            grouping(*node[i], group);
        break;
    }
}

void File::initFrom(AbstractFile* file) {
    AbstractFile::initFrom(file);
    static_cast<Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node{this};
}

Layer* File::layer(const QString& name) {
    if(layers_.contains(name))
        return layers_[name];
    else
        return layers_[name] = new Layer{sections_.begin()->second, name};
    return nullptr;
}

uint32_t File::type() const { return DXF; }

void File::createProjectionLayers() {
    if(mesh_.empty()) return;

    Timer t{__FUNCTION__};

    for(int i{}; i < int(View::Count); ++i) {
        auto view = View(i);
        if(!(Settings::views() & viewBit(view))) continue;

        Paths64 silhouette = mesh_.project(view);
        if(silhouette.empty()) continue;

        auto rect = GetBounds(silhouette);
        qInfo("Dxf: %s - %zu contours, %g x %g mm",
            qPrintable(viewName(view)), silhouette.size(),
            std::abs(rect.right - rect.left) / double(uScale),
            std::abs(rect.bottom - rect.top) / double(uScale));

        Layer* l = layer(viewName(view));
        l->setColor(viewColor(view));
        // Только заливка: силуэт — это замкнутый контур с дырками, отдельных
        // рёбер у него нет, поэтому path оставляем пустым.
        l->addGraphicObject(DxfGo{-1, Curve{}, toCurves(silhouette)});
    }

    mesh_.clear();
}

void File::createGi() {
    Timer t{__FUNCTION__};

    for(auto& [name, layer]: layers_)
        for(auto& go: layer->graphicObjects_)
            go.file_ = this;

    // r::fill(layers_
    // | v::transform(&Layers::value_type::second)
    // | v::transform(&Layer::graphicObjects_)
    // | v::join
    // | v::transform(&DxfGo::file_),
    // this);

    Gi::Group* igNorm = itemGroups_.back();
    Gi::Group* igPath = new Gi::Group;
    itemGroups_.push_back(igPath);

    int i{};

    for(auto& [name, layer]: layers_) {
        if(layer->graphicObjects_.size()) {
            if(i++) {
                itemGroups_.push_back(igNorm = new Gi::Group);
                itemGroups_.push_back(igPath = new Gi::Group);
            }

            cl::Clipper64 clipper; // cl::Clipper64
            const bool empty{layer->groupedCurves_.empty()};
            for(auto& go: layer->graphicObjects_) {
                if(empty && go.fill.size()) {
                    // if(Area(go.fill) < 0.) ReversePaths(go.fill); // FIXME add settings
                    clipper.AddSubject(toPaths(go.fill));
                }

                if(go.path.size() > 1) {
                    auto gItem = new Gi::DataPath{{go.path}, this};
                    if(go.entity()) {
                        // gItem->setToolTip(u"Line %1\n%2"_s
                        // .arg(go.entity()->data[0].line())
                        // .arg(go.entity()->name()));
                        gItem->setToolTip(go.entity()->name());
                    }
                    gItem->setPenColorPtr(&layer->colorPath_);
                    igPath->push_back(gItem);
                }
            }

            if(empty) {
                auto paths = toPaths(mergedCurves_);
                clipper.Execute(cl::ClipType::Union, cl::FillRule::NonZero, paths);
                CleanPaths(paths, uScale * 0.0005);

                for(Path64& path: paths)
                    if(path.back() != path.front())
                        path.emplace_back(path.front());
                mergedCurves_ = toCurves(paths);
                layer->groupedCurves_ = std::move(groupedPaths());
            }

            for(auto& paths: layer->groupedCurves_) {
                auto gItem = new Gi::DataFill{paths, this};
                gItem->setColorPtr(&layer->colorNorm_);
                igNorm->push_back(gItem);
            }

            igNorm->shrink_to_fit();
            igPath->shrink_to_fit();
            layer->itemGroupNorm = igNorm;
            layer->itemGroupPath = igPath;

            if(layer->itemsType_ == ItemsType::Null) {
                if(igNorm->size()) {
                    igNorm->setVisible(true);
                    igPath->setVisible(false);
                    layer->setItemsType(ItemsType::Normal);
                } else {
                    igNorm->setVisible(false);
                    igPath->setVisible(true);
                    layer->setItemsType(ItemsType::Paths);
                }
                layer->setVisible(true);
            } else
                layer->setVisible(visible_);
            layer->setItemsType(ItemsType::Both);
        }
    }
}

bool File::isVisible() const { return visible_; }

void File::setVisible(bool visible) {
    if(visible == visible_)
        return;
    visible_ = visible;
    if(visible_) {
        for(const auto& [name, vis]: layersVisible_)
            layers_[name]->setVisible(vis);
    } else {
        layersVisible_.clear();
        for(const auto& [name, layer]: layers_) {
            if(!layer->isEmpty()) {
                layersVisible_[name] = layer->isVisible();
                layers_[name]->setVisible(false);
            }
        }
    }
}

mvector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test) const {

    mvector<GraphicObject> retData;
    auto t = transform_.toQTransform(); // cached  QTransform
    for(auto&& criterion: criterias) {
        for(auto&& [name, layer]: layers()) {
            if(!layer->isVisible()) continue;
            for(const auto& go: layer->graphicObjects()) {
                auto transformedGo = go * t; // return copy
                if(criterion.test(transformedGo)) {
                    auto& g = retData.emplace_back(transformedGo);
                    if(g.type & GraphicObject::Circle)
                        g.path.clear();
                    if(test)
                        return retData;

                    switch(gcType) {
                    case GCType::Drill: {
                        double drillDiameter{};
                        auto rect = BoundingRect(g.fill);
                        // auto& ap = *apertures_.at(go.state.aperture());

                        // auto name {ap.name()};
                        // if (ap.withHole())
                        // drillDiameter = ap.drillDiameter();
                        // else
                        // drillDiameter = ap.minSize();
                        drillDiameter = std::min(rect.bottom() - rect.top(), rect.right() - rect.left());
                        // name += QObject::tr(", drill Ø%1mm").arg(drillDiameter);
                        g.raw = drillDiameter /** go.scaleX()*/;
                        g.name = /*u"С Ø"_s +*/ QString::number(drillDiameter);
                    } break;
                    default: break;
                    }
                }
            }
        }
    }

    return retData;

    // std::any Plugin::getDataForGC(AbstractFile* file, GCode::Plugin* plugin, std::any param) {
    // if (plugin->type() == ::GCode::Drill) {
    // DrillPlugin::Preview retData;
    // auto const dxfFile = static_cast<File*>(file);
    // QTransform t {dxfFile->transform()};
    // for (int ctr {}; auto&& [name, layer] : dxfFile->layers()) {
    // for (auto&& go : layer->graphicObjects())
    // if (auto circle = (const Circle*)go.entity(); go.entity()->type() == Entity::CIRCLE)
    // retData[{ctr, circle->radius * 2, false, name + u": CIRCLE"_s}].posOrPath.emplace_back(t.map(circle->centerPoint));
    // ctr++;
    // }
    // return retData;
    // }
    // return {};
    // }
}

void File::write(QDataStream& stream) const {
    stream << header_;
    {
        stream << int(layers_.size());
        for(auto& [name, layer]: layers_) {
            stream << name;
            stream << *layer;
        }
    }
    stream << itemsType_;
    if(!layersVisible_.size() && visible_) {
        for(const auto& [name, layer]: layers_)
            if(!layer->isEmpty())
                layersVisible_[name] = layer->isVisible();
    }
    stream << layersVisible_;
    stream << entities_;
}

void File::read(QDataStream& stream) {
    stream >> header_;
    {
        int size;
        stream >> size;
        while(size--) {
            QString name;
            Layer* layer = new Layer{this};
            stream >> name;
            stream >> *layer;
            layers_[name] = layer;
        }
    }
    stream >> itemsType_;
    stream >> layersVisible_;
    stream >> entities_;
}

Curves File::merge() const { return mergedCurves_; }

} // namespace Dxf
