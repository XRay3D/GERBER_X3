// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "exfile.h"
#include <gi/drillitem.h>

namespace Excellon {

File::File(QDataStream& stream)
    : m_format(this)
{
    read(stream);
}

File::File()
    : m_format(this)
{
}

File::~File() {}

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
        tool = m_tools[t];
        if (m_format.unitMode == Inches)
            tool *= 25.4;
    }
    return tool;
}

QMap<int, double> File::tools() const
{
    QMap<int, double> tools(m_tools);
    QMap<int, double>::iterator toolIt;
    if (m_format.unitMode == Inches)
        for (toolIt = tools.begin(); toolIt != tools.end(); ++toolIt)
            toolIt.value() *= 25.4;
    return tools;
}

Paths Excellon::File::merge() const
{
    for (GraphicsItem* item : *m_itemGroup.last())
        m_mergedPaths.append(item->paths());
    return m_mergedPaths;
}

void File::write(QDataStream& stream) const
{
    stream << *this;
    stream << m_tools;
    stream << m_format;
    _write(stream);
}

void File::read(QDataStream& stream)
{
    stream >> *this;
    stream >> m_tools;
    stream >> m_format;
    m_format.file = this;
    for (Hole& hole : *this) {
        hole.file = this;
        hole.state.format = &m_format;
    }
    _read(stream);
}

void File::createGi()
{
    for (Hole& hole : *this) {
        DrillItem* item = new DrillItem(&hole, this);
        hole.item = item;
        item->m_id = itemGroup()->size();
        itemGroup()->append(item);
    }
}

} //  namespace Excellon
