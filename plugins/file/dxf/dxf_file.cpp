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
#include "geo/boolean.h"
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

// Разбор вложенности вручную больше не нужен: Geo::Polygons и ЕСТЬ разобранный
// регион -- его собственные полигоны это тело, а полигоны того, что осталось от
// габаритной рамки после вычитания тела, -- вырезы. Прежде то же самое
// получалось объединением с рамкой по NonZero и обходом PolyTree.
Geo::Polygons& File::groupedPaths(File::Group group, bool /*fl*/) {
    if(!groupedCurves_.empty())
        return groupedCurves_;

    const Geo::Polygons region = mergedCurves();

    if(group == CopperGroup) {
        groupedCurves_ = region;
    } else {
        // Поле рамки вокруг детали: миллиметр с каждой стороны, лишь бы вырезы
        // оказались внутри неё и отделились от бесконечности.
        constexpr double margin = 1.0;
        QRectF box = region.boundingRect();
        box.adjust(-margin, -margin, margin, margin);
        const Geo::Polygons frame{Geo::Polylines{Geo::rectangle(box.width(), box.height(), box.center())}};
        groupedCurves_ = frame - region;
    }

    return groupedCurves_;
}

void File::initFrom(AbstractFile* file) {
    AbstractFile::initFrom(file);
    static_cast<Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node{this};
}

Layer* File::layer(const QString& name) {
    if(auto it = layers_.find(name); it != layers_.end())
        return it->second;
    // Секции может ещё не быть: ENTITIES без HEADER и TABLES разбирается раньше,
    // чем в sections_ хоть что-то попадёт (правая часть присваивания
    // sections_[type] = new Section... считается до вставки в карту), и
    // sections_.begin() смотрел бы в end(). Такой слой цепляется прямо к файлу.
    Layer* l = sections_.empty()
        ? new Layer{this}
        : new Layer{sections_.begin()->second, name};
    l->name_ = name;
    return layers_[name] = l;
}

uint32_t File::type() const { return DXF; }

void File::createProjectionLayers() {
    if(mesh_.empty()) return;

    Timer t{__FUNCTION__};

    for(int i{}; i < int(View::Count); ++i) {
        auto view = View(i);
        if(!(Settings::views() & viewBit(view))) continue;

        Geo::Polygons silhouette = mesh_.project(view);
        if(silhouette.empty()) continue;

        const QRectF rect = silhouette.boundingRect();
        qInfo("Dxf: %s - %zu contours, %g x %g mm",
            qPrintable(viewName(view)), silhouette.size(),
            rect.width(), rect.height());

        Layer* l = layer(viewName(view));
        l->setColor(viewColor(view));
        // Только заливка: силуэт — это замкнутый контур с дырками, отдельных
        // рёбер у него нет, поэтому path оставляем пустым.
        l->addGraphicObject(DxfGo{-1, Geo::Polyline{}, std::move(silhouette)});
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
    mergedCurves_ = {}; // пересобирается из слоёв тут же, ниже

    for(auto& [name, layer]: layers_) {
        if(layer->graphicObjects_.size()) {
            if(i++) {
                itemGroups_.push_back(igNorm = new Gi::Group);
                itemGroups_.push_back(igPath = new Gi::Group);
            }

            // Слой, пришедший из проекта, уже разобран; при разборе файла его
            // заливку надо собрать из объектов -- и ВСЕМ слоем разом, а не
            // пообъектно. Замкнутый контур объекта -- это ещё не тело: телом
            // или дыркой его делает ВЛОЖЕННОСТЬ в соседние контуры слоя
            // (even-odd, как у HATCH), и объединение готовых пообъектных тел
            // её теряло -- рамка платы проглатывала отверстия и внутреннюю
            // рамку, нарисованные отдельными сущностями внутри неё.
            //
            // Объекты без своего замкнутого контура (текст, штриховка, штрих
            // ненулевой ширины, силуэт проекции) в чередование не входят: их
            // заливка -- готовый регион, дырки у него известны точно, и он
            // объединяется поверх.
            if(layer->groupedCurves_.empty()) {
                Geo::Polylines contours;
                std::vector<Geo::Polygon> parts;
                for(const auto& go: layer->graphicObjects_)
                    if(go.path.isClosed()) contours.push_back(go.path);
                    else parts.append_range(go.fill.all());
                layer->groupedCurves_ = Geo::evenOdd(contours);
                if(!parts.empty())
                    layer->groupedCurves_ |= Geo::Polygons{std::span<const Geo::Polygon>{parts}};
            }
            mergedCurves_ |= layer->groupedCurves_;

            for(auto& go: layer->graphicObjects_) {
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

            for(const Geo::Polygon& polygon: layer->groupedCurves_) {
                auto gItem = new Gi::DataFill{Geo::Polygons{polygon}, this};
                gItem->setColorPtr(&layer->colorNorm_);
                igNorm->push_back(gItem);
            }

            igNorm->shrink_to_fit();
            igPath->shrink_to_fit();
            layer->itemGroupNorm = igNorm;
            layer->itemGroupPath = igPath;

            // Тип слоя -- что показывать: заливку, контуры или и то и другое.
            // У свежего разбора он ещё не выбран (Null), и берётся по факту --
            // какие группы непусты; у файла из проекта он прочитан вместе со
            // слоем, и трогать его нельзя. Прежде здесь стояло безусловное
            // setItemsType(Both): оно затирало и выбранное, и прочитанное, а
            // следующее сохранение записывало Both -- отсюда «слои не помнят тип».
            const bool fresh = layer->itemsType_ == ItemsType::Null;
            if(fresh) layer->itemsType_ = igNorm->size() ? ItemsType::Normal : ItemsType::Paths;
            // Видимость -- своя у каждого слоя и приходит из проекта вместе с
            // ним. Файловый снимок layersVisible_ для этого не годится: он
            // частичный (preSave обновляет его, лишь когда тот пуст, а
            // NodeLayer::setData дописывает по одному слою) и нужен для другого
            // -- вернуть видимость слоёв после того, как файл прятали целиком.
            layer->setVisible(fresh ? true : layer->visible_);
            // Применить тип к ТОЛЬКО ЧТО созданным группам. Через setItemsType
            // нельзя: у него отсечка «тип не менялся», и по прочитанному из
            // проекта типу она бы как раз и сработала.
            layer->applyItemsType();
        }
    }

    // Файловая группировка (её спрашивает gcode) считается по объединённой
    // геометрии всех слоёв -- по одному слою вырезы не определить.
    groupedCurves_ = {};
    groupedPaths();
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

std::vector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test) const {

    std::vector<GraphicObject> retData;
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
                        const QRectF rect = g.fill.boundingRect();
                        // auto& ap = *apertures_.at(go.state.aperture());

                        // auto name {ap.name()};
                        // if (ap.withHole())
                        // drillDiameter = ap.drillDiameter();
                        // else
                        // drillDiameter = ap.minSize();
                        drillDiameter = std::min(rect.width(), rect.height());
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

void File::preSave() const {
    if(!layersVisible_.size() && visible_)
        for(const auto& [name, layer]: layers_)
            if(!layer->isEmpty()) layersVisible_[name] = layer->isVisible();
    AbstractFile::preSave();
}

Geo::Polygons File::merge() const { return mergedCurves_; }

} // namespace Dxf
