// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "exfile.h"

#include "drillitem.h"

#include "exnode.h"
#include "leakdetector.h"

namespace Excellon {

QDataStream& operator>>(QDataStream& s, Tools& c)
{
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

QDataStream& operator<<(QDataStream& s, const Tools& c)
{
    s << quint32(c.size());
    for (auto& [key, val] : c) {
        s << key << val;
    }
    return s;
}

File::File()
    : FileInterface()
    , m_format(this)
{
}

File::~File() { }

Format File::format() const
{
    return m_format;
}

void File::setFormat(const Format& value)
{
    m_format.zeroMode = value.zeroMode;
    m_format.unitMode = value.unitMode;
    m_format.decimal = value.decimal;
    m_format.integer = value.integer;
    m_format.offsetPos = value.offsetPos;
    for (Hole& hole : *this) {
        hole.state.updatePos();
        hole.item->updateHole();
    }
}

double File::tool(int t) const
{
    double tool = 0.0;
    if (m_tools.contains(t)) {
        tool = m_tools.at(t);
        if (m_format.unitMode == Inches)
            tool *= 25.4;
    }
    return tool;
}

Tools File::tools() const
{
    Tools tools(m_tools);
    QMap<int, double>::iterator toolIt;
    if (m_format.unitMode == Inches)
        for (auto& [_, tool] : tools)
            tool *= 25.4;
    return tools;
}

Paths Excellon::File::merge() const
{
    for (GraphicsItem* item : *m_itemGroups.back())
        m_mergedPaths.append(item->paths());
    return m_mergedPaths;
}

void File::write(QDataStream& stream) const
{
    stream << *static_cast<const QList<Hole>*>(this);
    stream << m_tools;
    stream << m_format;
}

void File::read(QDataStream& stream)
{
    stream >> *static_cast<QList<Hole>*>(this);
    stream >> m_tools;
    stream >> m_format;
    m_format.file = this;
    for (Hole& hole : *this) {
        hole.file = this;
        hole.state.format = &m_format;
    }
}

void File::createGi()
{
    for (Hole& hole : *this) {
        DrillItem* item = new DrillItem(&hole, this);
        hole.item = item;
        itemGroup()->push_back(item);
    }
    itemGroup()->setVisible(true);
}

void File::initFrom(FileInterface* file)
{
    FileInterface::initFrom(file);
    static_cast<Excellon::Node*>(m_node)->file = this;
}

FileTree::Node* File::node()
{
    return m_node ? m_node : m_node = new Excellon::Node(this, &m_id);
}

} //  namespace Excellon
