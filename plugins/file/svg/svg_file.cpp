/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "svg_file.h"

#include "geo/polygon.h"
#include "geo/util.h"
#include "gi_datapath.h"
#include "svg_node.h"

namespace Svg {

File::File()
    : AbstractFile{} {
}

void File::createGi() {
    for(const SvgElement& el: *this) {
        if(el.painterPath.isEmpty()) continue;
        Geo::Polylines curves = Geo::fromPath(el.painterPath);
        if(!curves.empty())
            itemGroup()->push_back(new Gi::DataPath{std::move(curves), this});
    }
    itemGroup()->setVisible(true);
}

void File::initFrom(AbstractFile* file) {
    AbstractFile::initFrom(file);
    static_cast<Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node{this};
}

void File::write(QDataStream& stream) const {
    stream << static_cast<const QList<SvgElement>&>(*this);
}

void File::read(QDataStream& stream) {
    stream >> static_cast<QList<SvgElement>&>(*this);
}

Geo::Polygons File::merge() const {
    // Все контуры файла собираются плоским списком и становятся регионом одним
    // вызовом: вложенность в нём выражена ориентацией, и подавать контуры
    // порознь нельзя -- дырка, поданная отдельно, вычитать будет не из чего.
    Geo::Polylines contours;
    for(const SvgElement& el: *this)
        contours.append_range(Geo::fromPath(el.painterPath));
    mergedCurves_ = Geo::Polygons{contours};
    return mergedCurves_;
}

} // namespace Svg
