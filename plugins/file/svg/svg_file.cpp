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

#include "curve.h"
#include "gi_datapath.h"
#include "svg_node.h"

namespace Svg {

File::File()
    : AbstractFile{} {
}

void File::createGi() {
    for(const SvgElement& el: *this) {
        if(el.painterPath.isEmpty()) continue;
        Curves curves = toCurves(el.painterPath);
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

Paths File::merge() const {
    for(const SvgElement& el: *this)
        mergedPaths_.append_range(toPaths(el.painterPath));
    return mergedPaths_;
}

} // namespace Svg
