#include "abstractfile.h"

//Paths AbstractFile::merge()
//{
//    Paths paths;
//    Paths tmpPaths;
//    //    AbstractFile = gerberAbstractFile;
//    int i = 0, exp = -1;
//    Clipper clipper(ioStrictlySimple);
//    while (i < size()) {
//        clipper.Clear();
//        clipper.AddPaths(paths, ptSubject, true);
//        exp = at(i).state().imgPolarity;
//        do {
//            tmpPaths = at(i++).paths;
//            SimplifyPolygons(tmpPaths, pftNonZero);
//            clipper.AddPaths(tmpPaths, ptClip, true);
//        } while (i < size() && exp == at(i).state().imgPolarity);
//        if (at(i - 1).state().imgPolarity == G::Positive) {
//            clipper.Execute(ctUnion, paths, pftPositive);
//        } else {
//            clipper.Execute(ctDifference, paths, pftNonZero);
//        }
//    }
//    m_mergedPaths = paths;
//    return m_mergedPaths;
//}

AbstractFile::AbstractFile()
    : m_itemGroup(QSharedPointer<ItemGroup>(new ItemGroup))
{
}

AbstractFile::~AbstractFile() {}

QString AbstractFile::shortName() const { return QFileInfo(m_name).fileName(); }

QString AbstractFile::name() const { return m_name; }

void AbstractFile::setFileName(const QString& fileName) { m_name = fileName; }

ItemGroup* AbstractFile::itemGroup() const { return m_itemGroup.data(); }

Paths AbstractFile::mergedPaths() const { return m_mergedPaths.size() ? m_mergedPaths : merge(); }

Pathss AbstractFile::groupedPaths() const { return m_groupedPaths; }

QList<QString>& AbstractFile::lines() { return m_lines; }

Side AbstractFile::side() const { return m_side; }

void AbstractFile::setSide(Side side) { m_side = side; }

QColor AbstractFile::color() const { return m_color; }

void AbstractFile::setColor(const QColor& color) { m_color = color; }

int AbstractFile::id() const
{
    return m_id;
}

void AbstractFile::_write(QDataStream& stream) const
{
    stream << m_id;
    stream << m_lines;
    stream << m_name;
    stream << m_mergedPaths;
    stream << m_groupedPaths;
    stream << m_side;
    stream << m_color;
    stream << m_date;
    stream << itemGroup()->isVisible();
}

void AbstractFile::_read(QDataStream& stream)
{
    stream >> m_id;
    stream >> m_lines;
    stream >> m_name;
    stream >> m_mergedPaths;
    stream >> m_groupedPaths;
    stream >> (int&)(m_side);
    stream >> m_color;
    stream >> m_date;
    createGi();
    bool fl;
    stream >> fl;
    itemGroup()->setVisible(fl);
}
