/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "g2_file.h"
#include "g2_node.h"

#include "gi_datapath.h"
#include "gi_datasolid.h"

#include <QFile>
#include <QSaveFile>
#include <QTextStream>

namespace Gerber2 {

File::File()
    : AbstractFile{} {
    itemGroups_.push_back(new Gi::Group); // ApPaths (Normal создаёт база)
    layerTypes_ = {
        {Normal,  QObject::tr("Normal"),         QObject::tr("Filled copper")                      },
        {ApPaths, QObject::tr("Aperture paths"), QObject::tr("Centre lines of draws and arcs only")},
    };
}

bool File::setSource(const QString& text, QString* errorOut) {
    ParseResult result = parse(text);
    if(!result.valid()) {
        if(errorOut) *errorOut = result.error;
        return false;
    }
    source_ = text;
    parsed_ = std::move(result);
    lines_ = QStringView{source_}.split(u'\n')
        | v::transform([](QStringView s) { return s.toString(); })
        | r::to<std::vector>();
    rebuild();
    return true;
}

bool File::saveAs(const QString& fileName, QString* errorOut) const {
    QSaveFile file{fileName};
    if(!file.open(QFile::WriteOnly | QFile::Text)) {
        if(errorOut) *errorOut = file.errorString();
        return false;
    }
    QTextStream out{&file};
    out << source_;
    if(!source_.endsWith(u'\n')) out << u'\n';
    out.flush();
    if(!file.commit()) {
        if(errorOut) *errorOut = file.errorString();
        return false;
    }
    return true;
}

void File::clearGi() {
    for(auto* group: itemGroups_) group->clear();
    mergedCurves_.clear();
    groupedCurves_.clear();
}

void File::rebuild() {
    clearGi();
    createGi();
    addToScene();
    setTransform(transform_);
    setVisible(visible_);
}

void File::createGi() {
    if(Geo::Polygon image = flatten(parsed_.objects); !image.empty()) {
        groupedCurves_ = {image};
        mergedCurves_ = image;
        itemGroups_[Normal]->push_back(new Gi::DataFill{std::move(image), this});
    }

    for(const Geo::Polyline& stroke: parsed_.strokes)
        itemGroups_[ApPaths]->push_back(new Gi::DataPath{{stroke}, this});

    if(itemsType_ == NullType)
        itemsType_ = itemGroups_[Normal]->size() ? Normal : ApPaths;

    setColor(color_);

    layerTypes_[Normal].id = itemGroups_[Normal]->size() ? Normal : NullType;
    layerTypes_[ApPaths].id = itemGroups_[ApPaths]->size() ? ApPaths : NullType;

    for(auto* group: itemGroups_) group->setVisible(false);
    itemGroups_[itemsType_]->setVisible(visible_);
}

void File::setItemType(int type) {
    if(itemsType_ == type) return;
    itemsType_ = type;
    for(auto* group: itemGroups_) group->setVisible(false);
    itemGroups_[itemsType_]->setVisible(true);
}

void File::setColor(const QColor& color) {
    color_ = color;
    itemGroups_[Normal]->setBrushColor(color_);
    itemGroups_[ApPaths]->setPen(QPen{color_, 0.0});
}

Geo::Polygon File::merge() const { return mergedCurves_ = flatten(parsed_.objects); }

FileTree::Node* File::node() { return node_ ? node_ : node_ = new Node{this}; }

QIcon File::icon() const { return decoration(color_, u'2'); }

std::vector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType, bool test) const {
    // Плагин отдаёт итоговую заливку как один составной объект: этого хватает
    // для профиля/кармана, а пооперационные данные — задача основного плагина.
    std::vector<GraphicObject> data;
    GraphicObject go;
    go.fill = mergedCurves();
    TransformCurves(go.fill, transform_.toQTransform());
    if(go.fill.empty()) return data;
    go.type = GraphicObject::Composite;
    go.id = id_;
    go.name = shortName();
    for(auto&& criterion: criterias)
        if(criterion.test(go)) {
            data.emplace_back(go);
            if(test) break;
        }
    return data;
}

void File::write(QDataStream& stream) const { ::Block{stream}.write(source_, itemsType_); }

void File::read(QDataStream& stream) {
    QString src;
    ::Block{stream}.read(src, itemsType_);
    parsed_ = parse(src);
    source_ = std::move(src);
}

} // namespace Gerber2
