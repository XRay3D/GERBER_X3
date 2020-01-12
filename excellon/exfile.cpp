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

File::~File()
{
    saveFormat();
}

Format File::format() const
{
    return m_format;
}

void File::setFormat(const Format& value)
{
    //    m_format = value;
    //    m_format.file = this;
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

void File::setFormatForFile(const Format& /*value*/)
{
    //    QList<QString> lines;
    //    QFile file(fileName());
    //    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    //        return;
    //    QTextStream in(&file);
    //    while (!in.atEnd()) {
    //        lines.append(in.readLine());
    //    }
}

void File::saveFormat()
{
    //    QFile file(m_name + ".fmt");
    //    if (file.open(QFile::WriteOnly)) {
    //        QDataStream out(&file);
    //        qDebug("saveFormat()");
    //        out << m_format.zeroMode;
    //        out << m_format.unitMode;
    //        out << m_format.decimal;
    //        out << m_format.integer;
    //        out << m_format.offsetPos;
    //    }
}

void File::restoreFormat()
{
    //    QFile file(m_name + ".fmt");
    //    if (file.exists() && file.open(QFile::ReadOnly)) {
    //        QDataStream in(&file);
    //        qDebug("saveFormat()");
    //        int tmp;
    //        in >> tmp;
    //        m_format.zeroMode = static_cast<ZeroMode>(tmp);
    //        in >> tmp;
    //        m_format.unitMode = static_cast<UnitMode>(tmp);
    //        in >> m_format.decimal;
    //        in >> m_format.integer;
    //        in >> m_format.offsetPos;
    //        setFormat(m_format);
    //    }
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
    for (GraphicsItem* item : *m_itemGroup.last().data())
        m_mergedPaths.append(item->paths());
    return m_mergedPaths;
}
} //  namespace Excellon

void Excellon::File::write(QDataStream& stream) const
{
    stream << *this;
    stream << m_tools;
    stream << m_format;
    _write(stream);
}

void Excellon::File::read(QDataStream& stream)
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

void Excellon::File::createGi()
{
    for (Hole& hole : *this) {
        DrillItem* item = new DrillItem(&hole, this);
        hole.item = item;
        itemGroup()->append(item);
    }
    restoreFormat();
}
