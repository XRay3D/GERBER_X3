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
#include "gbrfile.h"
#include "componentitem.h"

#include "componentitem.h"
#include "datapathitem.h"
#include "datasoliditem.h"
#include "ft_node.h"
#include "project.h"
#include "settings.h"

#include <QElapsedTimer>
#include <QSemaphore>
#include <QThread>

#include "gbrnode.h"
#include "leakdetector.h"

namespace Gerber {

static Format* crutch;

Path GraphicObject::elipse() const { return m_state.dCode() == D03
            && m_gFile->apertures()->at(m_state.aperture())->type() == ApertureType::Circle
        ? m_path
        : Path(); } // circle
Paths GraphicObject::elipseW() const { return m_state.dCode() == D03
            && m_gFile->apertures()->at(m_state.aperture())->type() == ApertureType::Circle
            && m_gFile->apertures()->at(m_state.aperture())->withHole()
        ? m_paths
        : Paths(); }

QDebug operator<<(QDebug debug, const State& state)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "State("
                    << "D0" << state.dCode() << ", "
                    << "G0" << state.gCode() << ", "
                    << QStringLiteral("Positive|Negative").split('|').at(state.imgPolarity()) << ", "
                    << QStringLiteral("Linear|ClockwiseCircular|CounterclockwiseCircular").split('|').at(state.interpolation() - 1) << ", "
                    << QStringLiteral("Aperture|Line|Region").split('|').at(state.type()) << ", "
                    << QStringLiteral("Undef|Single|Multi").split('|').at(state.quadrant()) << ", "
                    << QStringLiteral("Off|On").split('|').at(state.region()) << ", "
                    << QStringLiteral("NoMirroring|X_Mirroring|Y_Mirroring|XY_Mirroring").split('|').at(state.mirroring()) << ", "
                    << QStringLiteral("aperture") << state.aperture() << ", "
                    << state.curPos() << ", "
                    << QStringLiteral("scaling") << state.scaling() << ", "
                    << QStringLiteral("rotating") << state.rotating() << ", "
                    << ')';
    return debug;
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

QDataStream& operator>>(QDataStream& s, ApertureMap& c)
{
    //    c.clear();
    //    quint32 n;
    //    s >> n;
    //    for (quint32 i = 0; i < n; ++i) {
    //        ApertureMap::key_type key;
    //        ApertureMap::mapped_type val;
    //        s >> key;
    //        s >> val;
    //        if (s.status() != QDataStream::Ok) {
    //            c.clear();
    //            break;
    //        }
    //        c.emplace(key, val);
    //    }
    s >> c.map();
    return s;
}

QDataStream& operator<<(QDataStream& s, const ApertureMap& c)
{
    //    s << quint32(c.size());
    //    for (auto& [key, val] : c) {
    //        s << key << val;
    //    }
    s << c.map();
    return s;
}

File::File(const QString& fileName)
    : FileInterface()
{
    m_itemGroups.append({ new ItemGroup, new ItemGroup });
    m_name = fileName;
    m_layerTypes = {
        { Normal, GbrObj::tr("Normal"), GbrObj::tr("Normal view") },
        { ApPaths, GbrObj::tr("Aperture paths"), GbrObj::tr("Displays only aperture paths of copper\n"
                                                            "without width and without contacts") },
        { Components, GbrObj::tr("Components"), GbrObj::tr("Show components") }
    };
}

File::~File() { }

const ApertureMap* File::apertures() const { return &m_apertures; }

Paths File::merge() const
{
    QElapsedTimer t;
    t.start();
    m_mergedPaths.clear();
    size_t i = 0;
    while (i < m_graphicObjects.size()) {
        Clipper clipper;
        clipper.AddPaths(m_mergedPaths, ptSubject, true);
        const auto exp = m_graphicObjects.at(i).state().imgPolarity();
        do {
            const GraphicObject& go = m_graphicObjects.at(i++);
            clipper.AddPaths(go.paths(), ptClip, true);
        } while (i < m_graphicObjects.size() && exp == m_graphicObjects.at(i).state().imgPolarity());
        if (m_graphicObjects.at(i - 1).state().imgPolarity() == Positive)
            clipper.Execute(ctUnion, m_mergedPaths, pftPositive);
        else
            clipper.Execute(ctDifference, m_mergedPaths, pftNonZero);
    }
    if (Settings::cleanPolygons())
        CleanPolygons(m_mergedPaths, Settings::cleanPolygonsDist() * uScale);
    return m_mergedPaths;
}

const QList<Component>& File::components() const { return m_components; }

void File::grouping(PolyNode* node, Pathss* pathss, File::Group group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CutoffGroup:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (size_t i = 0; i < node->ChildCount(); ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node->ChildCount(); ++i) {
            grouping(node->Childs[i], pathss, group);
        }
        break;
    case CopperGroup:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (size_t i = 0; i < node->ChildCount(); ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node->ChildCount(); ++i) {
            grouping(node->Childs[i], pathss, group);
        }
        break;
    }
}

Pathss& File::groupedPaths(File::Group group, bool fl)
{
    if (m_groupedPaths.empty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddPaths(mergedPaths(), ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = /*uScale*/ 1;
        Path outer = {
            Point64(r.left - k, r.bottom + k),
            Point64(r.right + k, r.bottom + k),
            Point64(r.right + k, r.top - k),
            Point64(r.left - k, r.top - k)
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
    for (auto [_, aperture] : m_apertures) {
        if (aperture->isFlashed())
            return true;
    }
    return false;
}

void File::addToScene() const
{
    for (const auto var : m_itemGroups) {
        var->addToScene();
        var->setZValue(-m_id);
    }
}

void File::setColor(const QColor& color)
{
    m_color = color;
    m_itemGroups[Normal]->setBrushColor(color);
    m_itemGroups[ApPaths]->setPen(QPen(color, 0.0));
}

mvector<const AbstrGraphicObject*> File::graphicObjects() const
{
    mvector<const AbstrGraphicObject*> go(m_graphicObjects.size());
    size_t i = 0;
    for (auto& refGo : m_graphicObjects)
        go[i++] = &refGo;
    return go;
}

void File::initFrom(FileInterface* file)
{
    FileInterface::initFrom(file);
    static_cast<Node*>(m_node)->file = this;

    setColor(file->color());
    // Normal
    itemGroup(Gerber::File::Normal)->setBrushColor(file->itemGroup(Gerber::File::Normal)->brushColor());
    itemGroup(Gerber::File::Normal)->addToScene();
    itemGroup(Gerber::File::Normal)->setZValue(-id());
    // ApPaths
    itemGroup(Gerber::File::ApPaths)->setPen(file->itemGroup(Gerber::File::ApPaths)->pen());
    itemGroup(Gerber::File::ApPaths)->addToScene();
    itemGroup(Gerber::File::ApPaths)->setZValue(-id());
    // Components
    itemGroup(Gerber::File::Components)->addToScene();
    itemGroup(Gerber::File::Components)->setZValue(-id());
    setItemType(file->itemsType());
}

FileTree::Node* File::node()
{
    return m_node ? m_node : m_node = new Node(this, &m_id);
}

void File::setItemType(int type)
{
    if (m_itemsType == type)
        return;

    m_itemsType = type;

    m_itemGroups[Normal]->setVisible(false);
    m_itemGroups[ApPaths]->setVisible(false);
    m_itemGroups[Components]->setVisible(false);

    m_itemGroups[m_itemsType]->setVisible(true /*m_visible*/);
}

int File::itemsType() const { return m_itemsType; }

void File::write(QDataStream& stream) const
{
    stream << m_graphicObjects; // write  QList<GraphicObject>
    stream << m_apertures;
    stream << m_format;
    //stream << layer;
    //stream << miror;
    stream << rawIndex;
    stream << m_itemsType;
    stream << m_components;
}

void File::read(QDataStream& stream)
{
    crutch = &m_format; ///////////////////
    stream >> m_graphicObjects; // read  QList<GraphicObject>
    stream >> m_apertures;
    stream >> m_format;
    //stream >> layer;
    //stream >> miror;
    stream >> rawIndex;
    stream >> m_itemsType;
    stream >> m_components;
    for (GraphicObject& go : m_graphicObjects) {
        go.m_gFile = this;
        go.m_state.m_format = format();
    }
}

void File::createGi()
{
    if (1) { // fill copper
        for (Paths& paths : groupedPaths()) {
            GraphicsItem* item = new GiDataSolid(paths, this);
            m_itemGroups[Normal]->push_back(item);
        }
        m_itemGroups[Normal]->shrink_to_fit();
    }
    if (1) { // add components
        for (const Component& c : qAsConst(m_components)) {
            if (!c.referencePoint.isNull())
                m_itemGroups[Components]->push_back(new ComponentItem(c, this));
        }
        m_itemGroups[Components]->shrink_to_fit();
    }
    if (1) { // add aperture paths
        auto contains = [&](const Path& path) -> bool {
            constexpr double k = 0.001 * uScale;
            for (const Path& chPath : checkList) { // find copy
                size_t counter = 0;
                if (chPath.size() == path.size()) {
                    for (const Point64& p1 : chPath) {
                        for (const Point64& p2 : path) {
                            if ((abs(p1.X - p2.X) < k) && (abs(p1.Y - p2.Y) < k)) {
                                ++counter;
                                break;
                            }
                        }
                    }
                }
                if (counter == path.size())
                    return true;
            }
            return false;
        };

        for (const GraphicObject& go : m_graphicObjects) {
            if (!go.path().empty()) {
                if (Settings::simplifyRegions() && go.path().front() == go.path().back()) {
                    Paths paths;
                    SimplifyPolygon(go.path(), paths);
                    for (Path& path : paths) {
                        path.push_back(path.front());
                        if (!Settings::skipDuplicates()) {
                            checkList.push_front(path);
                            m_itemGroups[ApPaths]->push_back(new GiDataPath(checkList.front(), this));
                        } else if (!contains(path)) {
                            checkList.push_front(path);
                            m_itemGroups[ApPaths]->push_back(new GiDataPath(checkList.front(), this));
                        }
                    }
                } else {
                    if (!Settings::skipDuplicates()) {
                        m_itemGroups[ApPaths]->push_back(new GiDataPath(go.path(), this));
                    } else if (!contains(go.path())) {
                        m_itemGroups[ApPaths]->push_back(new GiDataPath(go.path(), this));
                        checkList.push_front(go.path());
                    }
                }
            }
        }
        m_itemGroups[ApPaths]->shrink_to_fit();
    }

    if (m_itemsType == NullType) {
        if (m_itemGroups[Components]->size())
            m_itemsType = Components;
        else if (m_itemGroups[Normal]->size())
            m_itemsType = Normal;
        else
            m_itemsType = ApPaths;
    }

    m_layerTypes[Normal].id = m_itemGroups[Normal]->size() ? Normal : NullType;
    m_layerTypes[ApPaths].id = m_itemGroups[ApPaths]->size() ? ApPaths : NullType;
    m_layerTypes[Components].id = m_itemGroups[Components]->size() ? Components : NullType;

    m_itemGroups[ApPaths]->setVisible(false);
    m_itemGroups[Components]->setVisible(false);
    m_itemGroups[Normal]->setVisible(false);

    m_itemGroups[m_itemsType]->setVisible(m_visible);
}

}
