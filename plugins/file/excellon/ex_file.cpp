// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

namespace Excellon {

QDataStream& operator>>(QDataStream& s, Tools& c) {
    c.clear();
    quint32 n;
    s >> n;
    for (quint32 i = 0; i < n; ++i) {
        Tools::key_type key;
        Tools::mapped_type val;
        s >> key;
        s >> val;
        if (s.status() != QDataStream::Ok) {
            c.clear();
            break;
        }
        c.emplace(key, val);
    }
    return s;
}

QDataStream& operator<<(QDataStream& s, const Tools& c) {
    s << quint32(c.size());
    for (auto& [key, val] : c) {
        s << key << val;
    }
    return s;
}

File::File()
    : AbstractFile()
    , format_(Settings::format()) {
    format_.file = this;
}

File::~File() { }

Format File::format() const { return format_; }

void File::setFormat(const Format& value) {
    (format_ = value).file = this;
    for (Hole& hole : *this) {
        hole.state.updatePos();
        hole.item->update(hole.state.path.size() ? Path {hole.state.path} : Path {hole.state.pos}, hole.state.currentToolDiameter());
    }
}

double File::tool(int t) const {
    if (tools_.contains(t)) {
        return format_.unitMode == Inches ? tools_.at(t) * 25.4 : tools_.at(t);
    }
    return {};
}

Tools File::tools() const {
    Tools tools(tools_);
    if (format_.unitMode == Inches)
        for (auto& [_, tool] : tools)
            tool *= 25.4;
    return tools;
}

Paths Excellon::File::merge() const {
    for (GraphicsItem* item : *itemGroups_.back())
        mergedPaths_.append(item->paths());
    return mergedPaths_;
}

void File::write(QDataStream& stream) const {
    stream << *static_cast<const QList<Hole>*>(this);
    stream << tools_;
    stream << format_;
}

void File::read(QDataStream& stream) {
    stream >> *static_cast<QList<Hole>*>(this);
    stream >> tools_;
    stream >> format_;
    format_.file = this;
    for (Hole& hole : *this) {
        hole.file = this;
        hole.state.format = &format_;
    }
}

void File::createGi() {
    for (Hole& hole : *this)
        itemGroup()->push_back(hole.item = new GiDrill(hole.state.path.size() ? Path {hole.state.path} : Path {hole.state.pos}, hole.state.currentToolDiameter(), this, hole.state.toolId));
    itemGroup()->setVisible(true);
}

void File::initFrom(AbstractFile* file) {
    AbstractFile::initFrom(file);
    setFormat(static_cast<File*>(file)->format());
    static_cast<Excellon::Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Excellon::Node(this);
}

mvector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test) const {
    mvector<GraphicObject> retData;
    QTransform t = transform_;
    for (const Excellon::Hole& hole : *this) {
        double diam = tools_.at(hole.state.toolId);
        GraphicObject go;
        //        go.fill;
        if (bool slot = hole.state.path.size(); !slot) {
            go.pos = hole.state.pos;
            go.path.emplace_back(go.pos = hole.state.pos);
            go.fill.emplace_back(CirclePath(diam * uScale, go.pos));
        } else {
            go.path = hole.state.path;
            // go.pos = go.path.front();
            go.fill = C2::InflatePaths(Paths {hole.state.path}, diam * uScale, JoinType::Round, EndType::Round, uScale);
        }
        go.name = QString("T%1|Ø%2").arg(hole.state.toolId).arg(tools_.at(hole.state.toolId)).toUtf8(); // name;
        go.type = GraphicObject::FlStamp;                                                               // type{},//{Null};
                                                                                                        // go.id = int32_t {};                                                                             // id {-1};
        go.raw = tools_.at(hole.state.toolId);                                                          // raw;
        retData.emplace_back(go * transform_);

        //        if (bool slot = hole.state.path.size(); slot)
        //            retData[{hole.state.toolId, tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(t.map(hole.state.path));
        //        else
        //            retData[{hole.state.toolId, tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(t.map(hole.state.pos));
    }
    return retData;
}

} //  namespace Excellon
