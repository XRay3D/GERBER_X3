#include "gbrfile.h"
#include <QElapsedTimer>
#include <QSemaphore>
#include <QThread>
#include <project.h>
#include <settings.h>

using namespace Gerber;

static Format* crutch;

File::File(const QString& fileName) { m_name = fileName; }

template <typename T>
void addData(QByteArray& dataArray, const T& data)
{
    dataArray.append(reinterpret_cast<const char*>(&data), sizeof(T));
}

File::~File()
{
}

const QMap<int, QSharedPointer<AbstractAperture>>* File::apertures() const { return &m_apertures; }

Paths File::merge() const
{
    QElapsedTimer t;
    t.start();
    m_mergedPaths.clear();
    int i = 0;
    while (i < size()) {
        Clipper clipper;
        clipper.AddPaths(m_mergedPaths, ptSubject, true);
        const auto exp = at(i).state().imgPolarity();
        do {
            const GraphicObject& go = at(i++);
            clipper.AddPaths(go.paths(), ptClip, true);
        } while (i < size() && exp == at(i).state().imgPolarity());
        if (at(i - 1).state().imgPolarity() == Positive)
            clipper.Execute(ctUnion, m_mergedPaths, pftPositive);
        else
            clipper.Execute(ctDifference, m_mergedPaths, pftNonZero);
    }
    if (Settings::cleanPolygons())
        CleanPolygons(m_mergedPaths, 0.0005 * uScale);
    return m_mergedPaths;
}

void File::grouping(PolyNode* node, Pathss* pathss, File::Group group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CutoffGroup:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int var = 0; var < node->ChildCount(); ++var) {
                path = node->Childs[var]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int var = 0; var < node->ChildCount(); ++var) {
            grouping(node->Childs[var], pathss, group);
        }
        break;
    case CopperGroup:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int var = 0; var < node->ChildCount(); ++var) {
                path = node->Childs[var]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int var = 0; var < node->ChildCount(); ++var) {
            grouping(node->Childs[var], pathss, group);
        }
        break;
    }
}

File::ItemsType File::itemsType() const { return m_itemsType; }

void File::setRawItemGroup(ItemGroup* itemGroup) { m_rawItemGroup = QSharedPointer<ItemGroup>(itemGroup); }

ItemGroup* File::rawItemGroup() const { return m_rawItemGroup.data(); }

Pathss& File::groupedPaths(File::Group group, bool fl)
{
    if (m_groupedPaths.isEmpty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddPaths(mergedPaths(), ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = /*uScale*/ 1;
        Path outer = {
            IntPoint(r.left - k, r.bottom + k),
            IntPoint(r.right + k, r.bottom + k),
            IntPoint(r.right + k, r.top - k),
            IntPoint(r.left - k, r.top - k)
        };
        if (fl)
            ReversePath(outer);
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftNonZero);
        grouping(polyTree.GetFirst(), &m_groupedPaths, group);
    }
    return m_groupedPaths;
}

bool File::flashedApertures() const
{
    for (QSharedPointer<AbstractAperture> a : m_apertures) {
        if (a.data()->isFlashed())
            return true;
    }
    return false;
}

ItemGroup* File::itemGroup() const
{
    if (m_itemsType == Normal)
        return m_itemGroup.data();
    else
        return m_rawItemGroup.data();
}

void File::addToScene() const
{
    m_itemGroup.data()->addToScene();
    m_rawItemGroup.data()->addToScene();
    m_itemGroup.data()->setZValue(-m_id);
    m_rawItemGroup.data()->setZValue(-m_id);
}

void File::setColor(const QColor& color)
{
    AbstractFile::setColor(color);
    m_itemGroup->setBrush(color);
    m_rawItemGroup->setPen(QPen(color, 0.0));
}

void File::setItemType(File::ItemsType type)
{
    if (m_itemsType == type)
        return;
    m_itemsType = type;
    if (m_itemsType == Normal) {
        m_itemGroup.data()->setVisible(true);
        m_rawItemGroup.data()->setVisible(false);
    } else {
        m_itemGroup.data()->setVisible(false);
        m_rawItemGroup.data()->setVisible(true);
    }
}

void Gerber::File::write(QDataStream& stream) const
{
    stream << *this;
    stream << m_apertures;
    stream << m_format;
    //stream << layer;
    //stream << miror;
    stream << rawIndex;
    stream << m_itemsType;
    _write(stream);
}

void Gerber::File::read(QDataStream& stream)
{
    crutch = &m_format;
    stream >> *this;
    stream >> m_apertures;
    stream >> m_format;
    //stream >> layer;
    //stream >> miror;
    stream >> rawIndex;
    stream >> m_itemsType;
    for (GraphicObject& go : *this) {
        go.m_gFile = this;
        go.m_state.m_format = format();
    }
    _read(stream);
}

void Gerber::File::createGi()
{
    for (Paths& paths : groupedPaths()) {
        GraphicsItem* item = new GerberItem(paths, this);
        item->m_id = m_itemGroup->size();
        item->setToolTip(QString("ID: %1").arg(item->m_id));
        m_itemGroup->append(item);
    }

    setRawItemGroup(new ItemGroup);
    if (!Settings::skipDuplicates()) {
        for (const GraphicObject& go : *this) {
            GraphicsItem* item = new RawItem(go.path(), this);
            item->m_id = rawItemGroup()->size();
            item->setToolTip(QString("ID: %1").arg(item->m_id));
            m_rawItemGroup->append(item);
        }
    } else {
        QList<Path> checkList;
        for (int i = 0; i < size(); ++i) {
            const GraphicObject& go = at(i);
            if (go.path().size() > 1) { // skip empty
                bool contains = false;
                for (const Path& path : checkList) { // find copy
                    int counter = 0;
                    if (path.size() == go.path().size()) {
                        for (const IntPoint& p1 : path) {
                            for (const IntPoint& p2 : go.path()) {
                                const double k = 0.001 * uScale;
                                if ((abs(p1.X - p2.X) < k) && (abs(p1.Y - p2.Y) < k)) {
                                    ++counter;
                                    break;
                                }
                            }
                        }
                    }
                    if (counter == go.path().size()) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    checkList.append(go.path());
                    GraphicsItem* item = new RawItem(go.path(), this);
                    item->m_id = rawItemGroup()->size();
                    m_rawItemGroup->append(item);
                }
            }
        }
    }
    if (m_itemsType == Normal)
        m_rawItemGroup->setVisible(false);
    else
        m_itemGroup->setVisible(false);
}

QDataStream& operator<<(QDataStream& stream, const QSharedPointer<AbstractAperture>& aperture)
{
    stream << aperture->type();
    aperture->write(stream);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QSharedPointer<AbstractAperture>& aperture)
{
    int type;
    stream >> type;
    switch (type) {
    case Circle:
        aperture = QSharedPointer<AbstractAperture>(new ApCircle(stream, crutch));
        break;
    case Rectangle:
        aperture = QSharedPointer<AbstractAperture>(new ApRectangle(stream, crutch));
        break;
    case Obround:
        aperture = QSharedPointer<AbstractAperture>(new ApObround(stream, crutch));
        break;
    case Polygon:
        aperture = QSharedPointer<AbstractAperture>(new ApPolygon(stream, crutch));
        break;
    case Macro:
        aperture = QSharedPointer<AbstractAperture>(new ApMacro(stream, crutch));
        break;
    case Block:
        aperture = QSharedPointer<AbstractAperture>(new ApBlock(stream, crutch));
        break;
    }
    return stream;
}
