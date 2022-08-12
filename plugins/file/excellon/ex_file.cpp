// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
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
    : FileInterface()
    , format_(Settings::format()) {
    format_.file = this;
}

File::~File() { }

Format File::format() const {
    return format_;
}

void File::setFormat(const Format& value) {
   
    (format_ = value).file = this;
    for (Hole& hole : *this) {
        hole.state.updatePos();
        hole.item->update(hole.state.path.size() ? Path { hole.state.path } : Path { hole.state.pos }, hole.state.currentToolDiameter());

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
        itemGroup()->push_back(hole.item = new GiDrill(hole.state.path.size() ? Path { hole.state.path } : Path { hole.state.pos }, hole.state.currentToolDiameter(), this, hole.state.toolId));
    itemGroup()->setVisible(true);
}

void File::initFrom(FileInterface* file) {
    FileInterface::initFrom(file);
    setFormat(static_cast<File*>(file)->format());
    static_cast<Excellon::Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Excellon::Node(this);
}

} //  namespace Excellon
