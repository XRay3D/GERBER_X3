/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "ex_file.h"

#include "gi_drill.h"

#include "ex_node.h"

#include "geo/boolean.h"
#include "geo/util.h"

namespace Excellon {

File::File()
    : AbstractFile()
    , format_(Settings::format()) {
    format_.file = this;
}

File::~File() { }

Format File::format() const { return format_; }

void File::setFormat(const Format& value) {
    (format_ = value).file = this;
    for(Hole& hole: *this) {
        hole.state.updatePos();
        hole.item->updatePath(hole.state.path.size() ? hole.state.path
                                                     : QPolygonF{hole.state.pos},
            hole.state.currentToolDiameter());
    }
}

double File::tool(unsigned t) const {
    if(tools_.contains(t))
        return format_.unitMode == Inches ? tools_.at(t) * 25.4 : tools_.at(t);
    return {};
}

Tools File::tools() const {
    Tools tools(tools_);
    if(format_.unitMode == Inches)
        for(auto& [_, tool]: tools)
            tool *= 25.4;
    return tools;
}

// Сверления и пазы -- независимые тела, лежащие поодиночке; регион слоя это
// просто их объединение. Контуры берутся у самих элементов сцены: Gi::Drill
// строит их точно (окружность у сверла, полоса Geo::Inflate у паза), и второй
// раз считать то же самое здесь незачем.
Geo::Polygons File::merge() const {
    Geo::Polylines contours;
    for(auto&& item: *itemGroup())
        contours.append_range(item->curves());
    mergedCurves_ = Geo::Polygons{contours};
    return mergedCurves_;
}

void File::createGi() {
    for(Hole& hole: *this)
        itemGroup()->push_back(hole.item = new Gi::Drill{
                                   hole.state.path.size() ? hole.state.path
                                                          : QPolygonF{hole.state.pos},
                                   hole.state.currentToolDiameter(),
                                   this,
                                   static_cast<Tool::ID>(hole.state.toolId)});
    itemGroup()->setVisible(true);
}

void File::initFrom(AbstractFile* file) {
    AbstractFile::initFrom(file);
    setFormat(static_cast<File*>(file)->format());
    static_cast<Excellon::Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Excellon::Node{this};
}

std::vector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType /*gcType*/, bool test) const {
    std::vector<GraphicObject> retData;
    const QTransform t = transform_.toQTransform();
    for(const Excellon::Hole& hole: *this) {
        // Диаметр -- уже в миллиметрах: у дюймового файла tool() пересчитывает
        // сам, а сырой tools_ отдал бы дюймы, и сверло уехало бы в 25.4 раза.
        const double diam = tool(hole.state.toolId);
        GraphicObject go;
        if(hole.state.path.size() < 2) { // сверление
            go.pos = hole.state.pos;
            go.path = Geo::Polyline{Geo::Vertex{go.pos}};
            go.fill = Geo::Polygons{Geo::Polylines{Geo::circle(diam, go.pos)}};
        } else { // паз: полоса вдоль прохода шириной ровно в диаметр сверла
            go.path = Geo::Polyline{hole.state.path};
            go.fill = Geo::Inflate(Geo::Polylines{go.path}, diam);
        }
        go.name = u"T%1|Ø%2"_s.arg(+hole.state.toolId).arg(diam);
        go.type = GraphicObject::FlStamp;
        go.raw = diam;

        auto transformedGo = go * t;
        for(auto&& criterion: criterias)
            if(criterion.test(transformedGo)) {
                retData.emplace_back(std::move(transformedGo));
                if(test) return retData;
                break;
            }
    }
    return retData;
}

} // namespace Excellon
